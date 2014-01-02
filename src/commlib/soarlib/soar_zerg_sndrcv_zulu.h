/******************************************************************************************
Copyright           : 2000-2004, Fullsail Technology (Shenzhen) Company Limited.
FileName            :
Author              : 原作者Scottxu ，移植,Sailzeng
Version             :
Date Of Creation    : 2006年9月8日
Description         :

Others              :
Function List       :
1.  ......
    Modification History:
1.Date  :
Author  :Sailzeng 将这段代码移动到COMM,模板化
Modification  :
******************************************************************************************/

#ifndef SOARING_LIB_SNDRCV_TCP_ZULU_H_
#define SOARING_LIB_SNDRCV_TCP_ZULU_H_

#include "soar_zerg_sndrcv_base.h"

/******************************************************************************************
class Zulu_SendRecv_Package
******************************************************************************************/
class Zulu_SendRecv_Package : public Tibetan_SendRecv_Package
{

protected:

    //ZULU 是否链接
    bool                   zulu_connected_;

    //ZULU链接的IP地址
    ZCE_Sockaddr_In        zulu_svc_ip_;

    //使用的SOCKET包装
    ZCE_Socket_Stream      zulu_stream_;

public:

    //构造函数和析构函数
    Zulu_SendRecv_Package();
    ~Zulu_SendRecv_Package();

protected:

    //接收一个APPFRAME，放入内部的tibetan_recv_appframe_
    int receive_svc_package(ZCE_Time_Value *time_wait = NULL);

    //将内部的tibetan_send_appframe_发送出去
    int send_svc_package(ZCE_Time_Value *time_wait = NULL);

public:

    //设置相应的SVC INFO,
    int set_zulu_svcinfo(const char *svc_ip,
                         unsigned short svc_port,
                         const SERVICES_ID &recv_service,
                         const SERVICES_ID &send_service,
                         const SERVICES_ID &proxy_service,
                         size_t frame_len = Zerg_App_Frame::MAX_LEN_OF_APPFRAME);

    //链接服务器,time_wait不能用const是有原因的，因为部分select的ZCE_Time_Value是返回剩余时间的
    int connect_zulu_server(ZCE_Time_Value *time_wait);

    //关闭内部的连接。
    void close();

    //取得本地的地址信息
    int getsockname (ZCE_Sockaddr *addr)  const;

    //发送和接收数据，会提前进行连接的。
    template< class T1, class T2>
    int send_recv_package(unsigned int snd_cmd,
                          unsigned int qq_uin,
                          const T1 &send_info,
                          ZCE_Time_Value *time_wait,
                          bool if_recv,
                          unsigned int rcv_cmd,
                          T2 &recv_info,
                          bool error_continue = true,
                          unsigned int app_id = 0,
                          unsigned int backfill_trans_id = 0);

    //发送数据
    template< class T1>
    int send_svc_package(unsigned int qq_uin,
                         unsigned int cmd,
                         const T1 &snd_info,
                         ZCE_Time_Value *time_out = NULL,
                         unsigned int app_id = 0,
                         unsigned int backfill_trans_id = 0);

    //接受数据
    template< class T2>
    int receive_svc_package(unsigned int cmd,
                            T2 &rcv_info ,
                            bool error_continue = true,
                            ZCE_Time_Value *time_out = NULL);

    //接收一个数据包，得到命令字，你可以调用get_recv_appframe进行后续的处理，
    int receive_svc_package(unsigned int &recv_cmd,
                            ZCE_Time_Value *time_out = NULL);

};

/******************************************************************************************
Author          : Sail (ZengXing)  Date Of Creation: 2006年9月8日
Function        : receive_svc_package
Return          : int
Parameter List  :
Param1: unsigned int cmd 发送的命令
Param2: T& info 发送的信息数据
Param3: bool error_continue 如果收到的数据不是期望的，就继续等待，直到等待相应的命令
Param4: ZCE_Time_Value *time_wait 超时时长，如果要一直阻塞，就用NULL,
Description     : 阻塞的接收一个APPFRAME数据
Calls           :
Called By       :
Other           : cmd为预期命令，info为预期包结构
Modify Record   :
******************************************************************************************/
template<class T2>
int Zulu_SendRecv_Package::receive_svc_package(unsigned int cmd,
                                               T2 &info ,
                                               bool error_continue ,
                                               ZCE_Time_Value *time_wait)
{
    int ret = SOAR_RET::SOAR_RET_SUCC;

    //用死循环保证错误继续处理和资源统一释放
    for (;;)
    {
        ret = receive_svc_package(time_wait);

        if ( SOAR_RET::SOAR_RET_SUCC != ret )
        {
            break;
        }

        //如果收到的数据帧不是俺期待的
        if (cmd != tibetan_recv_appframe_->frame_command_ )
        {
            //如果发生错误,继续处理,则继续接受
            if (error_continue )
            {
                continue;
            }
            else
            {
                ZLOG_ERROR("[framework] recv a error or unexpect frame,expect cmd =%u,recv cmd =%u.", cmd, tibetan_recv_appframe_->frame_command_);
                ret =  SOAR_RET::ERROR_ZULU_RECEIVE_OTHERS_COMMAND;
                break;
            }
        }
        else
        {
            //如果还有data的数据，进行解码
            if (tibetan_recv_appframe_->frame_length_ > Zerg_App_Frame::LEN_OF_APPFRAME_HEAD )
            {
                ret = tibetan_recv_appframe_->appdata_decode(info);

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
    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        zulu_stream_.close();
        zulu_connected_ = false;
        return ret;
    }

    return SOAR_RET::SOAR_RET_SUCC;
}

/******************************************************************************************
Author          : Sail (ZengXing)  Date Of Creation: 2006年9月8日
Function        : send_svc_package
Return          : int
Parameter List  :
Param1: qq_uin                    QQUIN
Param3: unsigned int cmd          处理的发送命令
Param4: const T1& info             处理的发送结构
Param5: ZCE_Time_Value *time_wait 发送的超时时间
Param6: unsigned int sndtrans_id  接收的超时时间
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template< class T1>
int Zulu_SendRecv_Package::send_svc_package(unsigned int qq_uin,
                                            unsigned int cmd,
                                            const T1 &info,
                                            ZCE_Time_Value *time_wait ,
                                            unsigned int app_id,
                                            unsigned int backfill_trans_id)
{
    int ret = 0;

    //在没有链接的情况下链接服务器
    if ( false == zulu_connected_ )
    {
        ret = connect_zulu_server(time_wait);

        if (SOAR_RET::SOAR_RET_SUCC != ret)
        {
            return ret;
        }
    }

    tibetan_send_appframe_->frame_command_ = cmd;
    tibetan_send_appframe_->backfill_trans_id_ = backfill_trans_id;
    tibetan_send_appframe_->app_id_ = app_id;
    tibetan_send_appframe_->frame_uin_ = qq_uin;

    //编码
    ret = tibetan_send_appframe_->appdata_encode(Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

    if (ret != 0 )
    {
        return ret;
    }

    ret = send_svc_package(time_wait);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        zulu_stream_.close();
        zulu_connected_ = false;
        return ret;
    }

    return SOAR_RET::SOAR_RET_SUCC;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年4月25日
Function        : Zulu_SendRecv_Package::send_recv_package
Return          : int
Parameter List  :
Param1: unsigned int snd_cmd      发送命令
Param2: unsigned int qq_uin       QQUIN的效果
Param3: const T1& send_info       发送的数据
Param4: ZCE_Time_Value* time_wait 时间等待
Param5: bool if_recv              是否接收
Param6: unsigned int rcv_cmd      接收的命令
Param7: T2& recv_info             接收的数据
Param8: bool error_continue       错误下是否继续
Description     : 发送和接收PACKAGE
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template< class T1, class T2>
int Zulu_SendRecv_Package::send_recv_package(unsigned int snd_cmd,
                                             unsigned int qq_uin,
                                             const T1 &send_info,
                                             ZCE_Time_Value *time_wait,
                                             bool if_recv,
                                             unsigned int rcv_cmd,
                                             T2 &recv_info,
                                             bool error_continue,
                                             unsigned int app_id,
                                             unsigned int backfill_trans_id)
{
    int ret = 0;

    //发送数据
    ret = send_svc_package(qq_uin,
                           snd_cmd,
                           send_info,
                           time_wait,
                           app_id,
                           backfill_trans_id);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        return ret;
    }

    //为什么要有这个参数…………，我已经想不起来了，晕
    if (false == if_recv)
    {
        return SOAR_RET::SOAR_RET_SUCC;
    }

    //收取数据，
    ret = receive_svc_package(rcv_cmd,
                              recv_info,
                              error_continue,
                              time_wait
                             );

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        return ret;
    }

    return SOAR_RET::SOAR_RET_SUCC;
}

#endif //SOARING_LIB_SNDRCV_TCP_ZULU_H_

