/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar/zerg/sndrcv_zulu.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       Friday, September 05, 2014
* @brief      2006年9月8日,原作者Scottxu ，移植,Sailzeng
*
*
* @details
*
*
*
* @note       将这段代码移动到COMM,模板化
*
*/

#ifndef SOARING_LIB_SNDRCV_TCP_ZULU_H_
#define SOARING_LIB_SNDRCV_TCP_ZULU_H_

#include "soar/zerg/sndrcv_base.h"

/******************************************************************************************
class Zulu_SendRecv_Msg
******************************************************************************************/
class Zulu_SendRecv_Msg : public SendRecv_Msg_Base
{
protected:

    //ZULU 是否链接
    bool                   zulu_connected_;

    //ZULU链接的IP地址
    zce::skt::addr_in        zulu_svc_ip_;

    //使用的SOCKET包装
    zce::skt::stream      zulu_stream_;

public:

    //构造函数和析构函数
    Zulu_SendRecv_Msg();
    ~Zulu_SendRecv_Msg();

protected:

    //接收一个APPFRAME，放入内部的tibetan_recv_appframe_
    int receive_svc_msg(zce::time_value* time_wait = NULL);

    //将内部的tibetan_send_appframe_发送出去
    int send_svc_msg(zce::time_value* time_wait = NULL);

public:

    //设置相应的SVC INFO,
    int set_zulu_svcinfo(const char* svc_ip,
                         unsigned short svc_port,
                         const soar::SERVICES_ID& recv_service,
                         const soar::SERVICES_ID& send_service,
                         const soar::SERVICES_ID& proxy_service,
                         size_t frame_len = soar::Zerg_Frame::MAX_LEN_OF_FRAME);

    //链接服务器,time_wait不能用const是有原因的，因为部分select的zce::Time_Value是返回剩余时间的
    int connect_zulu_server(zce::time_value* time_wait);

    //关闭内部的连接。
    void close();

    //取得本地的地址信息
    int getsockname(zce::skt::addr_base* addr)  const;

    /*!
    * @brief      发送数据
    * @tparam     T1
    * @return     int
    * @param      user_id  USER ID
    * @param      cmd      发送的命令
    * @param      snd_info 处理的发送结构
    * @param      time_out 发送的超时时间
    * @param      app_id   应用ID
    * @param      backfill_trans_id 回填的事物ID，默认为0，表示不会填
    */
    template< class T1>
    int send_svc_msg(uint32_t user_id,
                     uint32_t cmd,
                     const T1& snd_info,
                     zce::time_value* time_out = NULL,
                     uint32_t backfill_fsm_id = 0,
                     uint16_t business_id = 0);

    /*!
    * @brief      接受数据，阻塞的接收一个APPFRAME数据
    * @tparam     T2
    * @return     int       OK
    * @param      cmd       预计接受的的命令字
    * @param      rcv_info  接收的信息数据
    * @param      error_continue 如果收到的数据不是期望的，就继续等待，直到等待相应的命令
    * @param      time_out  超时时长，如果要一直阻塞，就用NULL
    */
    template< class T2>
    int receive_svc_msg(uint32_t cmd,
                        T2& rcv_info,
                        bool error_continue = true,
                        zce::time_value* time_out = NULL);

    //接收一个数据包，得到命令字，你可以调用get_recv_appframe进行后续的处理，
    int receive_svc_msg(unsigned int& recv_cmd,
                        zce::time_value* time_out = NULL);

    /*!
    * @brief      发送和接收数据，会提前进行连接的。
    * @note       参数请参考send_svc_package,receive_svc_package
    */
    template< class T1, class T2>
    int send_recv_msg(unsigned int snd_cmd,
                      uint32_t user_id,
                      const T1& send_info,
                      zce::time_value* time_wait,
                      unsigned int rcv_cmd,
                      T2& recv_info,
                      bool error_continue = true,
                      uint32_t backfill_fsm_id = 0,
                      uint16_t app_id = 0);
};

//阻塞的接收一个APPFRAME数据
template<class T2>
int Zulu_SendRecv_Msg::receive_svc_msg(uint32_t cmd,
                                       T2& info,
                                       bool error_continue,
                                       zce::time_value* time_wait)
{
    int ret = 0;

    //用死循环保证错误继续处理和资源统一释放
    for (;;)
    {
        ret = receive_svc_msg(time_wait);
        if (0 != ret)
        {
            break;
        }

        //如果收到的数据帧不是俺期待的
        if (cmd != msg_recv_frame_->command_)
        {
            //如果发生错误,继续处理,则继续接受
            if (error_continue)
            {
                continue;
            }
            else
            {
                ZCE_LOG(RS_ERROR, "[framework] recv a error or unexpect frame,expect cmd =%u,recv cmd =%u.",
                        cmd,
                        msg_recv_frame_->command_);
                ret = SOAR_RET::ERROR_ZULU_RECEIVE_OTHERS_COMMAND;
                break;
            }
        }
        else
        {
            //如果还有data的数据，进行解码
            if (msg_recv_frame_->length_ > soar::Zerg_Frame::LEN_OF_HEAD)
            {
                ret = msg_recv_frame_->appdata_decode(info);
                if (ret != 0)
                {
                    return ret;
                }
            }
        }

        //跳出循环
        break;
    }

    //接收有错误，进行特殊处理
    if (ret != 0)
    {
        zulu_stream_.close();
        zulu_connected_ = false;
        return ret;
    }

    return 0;
}

//发送一个数据包
template< class T1>
int Zulu_SendRecv_Msg::send_svc_msg(uint32_t user_id,
                                    uint32_t cmd,
                                    const T1& info,
                                    zce::time_value* time_wait,
                                    uint32_t backfill_fsm_id,
                                    uint16_t business_id)
{
    int ret = 0;

    //在没有链接的情况下链接服务器
    if (false == zulu_connected_)
    {
        ret = connect_zulu_server(time_wait);

        if (0 != ret)
        {
            return ret;
        }
    }

    msg_send_frame_->command_ = cmd;
    msg_send_frame_->backfill_fsm_id_ = backfill_fsm_id;
    msg_send_frame_->user_id_ = user_id;
    msg_send_frame_->business_id_ = business_id;
    //编码
    ret = msg_send_frame_->appdata_encode(soar::Zerg_Frame::MAX_LEN_OF_DATA, info);

    if (ret != 0)
    {
        return ret;
    }

    ret = send_svc_msg(time_wait);

    if (ret != 0)
    {
        zulu_stream_.close();
        zulu_connected_ = false;
        return ret;
    }

    return 0;
}

//发送一个数据包，并且接收一个数据包
template< class T1, class T2>
int Zulu_SendRecv_Msg::send_recv_msg(unsigned int snd_cmd,
                                     uint32_t user_id,
                                     const T1& send_info,
                                     zce::time_value* time_wait,
                                     unsigned int rcv_cmd,
                                     T2& recv_info,
                                     bool error_continue,
                                     uint32_t backfill_fsm_id,
                                     uint16_t business_id)
{
    int ret = 0;

    //发送数据
    ret = send_svc_package(user_id,
                           snd_cmd,
                           send_info,
                           time_wait,
                           backfill_fsm_id,
                           business_id);
    if (ret != 0)
    {
        return ret;
    }

    //收取数据，
    ret = receive_svc_package(rcv_cmd,
                              recv_info,
                              error_continue,
                              time_wait
    );
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

#endif //SOARING_LIB_SNDRCV_TCP_ZULU_H_
