/*!
* @copyright  2004-2015  Apache License, Version 2.0 FULLSAIL
* @filename   soar/zerg/sndrcv_lolo.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2006年9月8日
* @brief
*
*
* @details
*
*
*
* @note       倮倮这个词的意思其实是汉族人对少数民族的蔑称，当然我在这儿不敢有半点
*             的民族倾向，只是为了纪念一个很老的歌手
*             倮倮，云南彝族人，LOLO
*
*             菩提树上的叮当
*             倮倮
*             请你带我去从前你住过的村庄
*             让我敲一敲菩提树上的那个叮当
*             踩一踩凤尾竹下老式的水车
*             爬一爬只是听说过的那座高山
*             请你带我去到从前你住过的村庄
*             让我敲一敲菩提树上的那个叮当
*             踩一踩凤尾竹下老式的水车
*             爬一爬只是听说过的那座高山
*             你说你的山会渐渐长高
*             你说你的水会源远流长
*             你说你的菩提响着叮当
*             你说你的村庄没有悲伤
*             让我跟你去到你不老的村庄
*             一起享受不知不觉地那份孤单
*             和你在菩提树下看往事流淌
*             陪你在河塘边把少年的泪水喝干
*             让我跟你住在你不老的村庄
*             一起度过不言不语的那段时光
*             和你在水车上面让月光旋转
*             陪你在烛光里把那太阳露出山
*             噢~你的村庄也有个叮当
*             噢~你的村庄也没有悲伤
*             噢~你的村庄也有个叮当
*             噢~你的村庄也没有悲伤
*/

#ifndef COMM_SNDRCV_UDP_NET_LOLO_H_
#define COMM_SNDRCV_UDP_NET_LOLO_H_

#include "soar/zerg/sndrcv_base.h"

/******************************************************************************************
class Lolo_SendRecv_Package
******************************************************************************************/
class Lolo_SendRecv_Package : public SendRecv_Msg_Base
{
protected:

    //ZULU链接的IP地址
    zce::Sockaddr_In          lolo_svc_ip_;

    //收到数据的对端地址，99.99999的情况和上面一样
    zce::Sockaddr_In          lolo_recvdata_ip_;

    //使用的SOCKET包装
    zce::Socket_DataGram      lolo_datagram_;

public:
    //
    Lolo_SendRecv_Package();
    virtual ~Lolo_SendRecv_Package();

    //设置相应的SVC INFO,
    int set_lolo_svcinfo(const char* svc_ip,
                         unsigned short svc_port,
                         const soar::SERVICES_ID& recv_service,
                         const soar::SERVICES_ID& send_service,
                         const soar::SERVICES_ID& proxy_service,
                         size_t frame_len = soar::Zerg_Frame::MAX_LEN_OF_FRAME,
                         bool is_check_conn_info = false);

    //发送数据
    template<class T1>
    int send_svc_msg(uint32_t user_id,
                     uint32_t cmd,
                     const T1& msg,
                     uint32_t backfill_fsm_id = 0,
                     uint16_t business_id = 0);

    //接受数据
    template<class T2>
    int receive_svc_msg(uint32_t cmd,
                        T2& msg,
                        zce::Time_Value* time_wait = NULL);

    //发送和接收数据，一起一锅搞掂的方式
    template< class T1, class T2>
    int send_recv_msg(unsigned int snd_cmd,
                      uint32_t user_id,
                      const T1& send_info,
                      zce::Time_Value* time_wait,
                      bool if_recv,
                      unsigned int rcv_cmd,
                      T2& recv_info,
                      uint32_t backfill_fsm_id = 0,
                      uint16_t business_id = 0);
};

//收数据
template<class T2>
int Lolo_SendRecv_Package::receive_svc_msg(uint32_t cmd,
                                           T2& msg,
                                           zce::Time_Value* time_wait)
{
    int ret = 0;
    ssize_t socket_ret = 0;
    int data_len = 0;

    // 接收数据
    socket_ret = lolo_datagram_.recvfrom((void*)(msg_recv_frame_),
                                         test_frame_len_,
                                         &lolo_recvdata_ip_,
                                         time_wait);

    msg_recv_frame_->ntoh();
    msg_recv_frame_->dump_frame_head("UDP recv", RS_DEBUG);

#if defined _DEBUG || defined DEBUG
    ZCE_ASSERT(test_frame_len_ >= msg_recv_frame_->length_);
#endif //#if defined _DEBUG || defined DEBUG

    //ret == 0
    if (socket_ret == 0)
    {
        ZCE_LOG(RS_INFO, "[framework] Link is disconnect recv ret =%d, error[%u|%s].",
                socket_ret,
                zce::last_error(),
                strerror(zce::last_error()));
        return SOAR_RET::ERROR_ZULU_LINK_DISCONNECT;
    }
    //小于0一般表示错误,对于阻塞，这儿也表示一个错误,我给了你等待时间作为选择
    else if (socket_ret < 0)
    {
        //如果错误是信号导致的重入
        int last_error = zce::last_error();

        ZCE_LOG(RS_ERROR, "[framework] RECV soar::Zerg_Frame head error or time out. Ret:%d, error[%u|%s].",
                socket_ret,
                last_error,
                strerror(last_error));

        return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
    }

    //保存接收到的事务ID
    recv_trans_id_ = msg_recv_frame_->fsm_id_;
    //数据包的长度
    data_len = msg_recv_frame_->length_ - soar::Zerg_Frame::LEN_OF_HEAD;

    if (data_len < 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] UDP Receive soar::Zerg_Frame head len error ,frame len:%d,error[%u|%s].",
                msg_recv_frame_->length_,
                zce::last_error(),
                strerror(zce::last_error()));
        return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
    }

    //只处理包头信息
    if (data_len == 0)
    {
        return 0;
    }

    //ZCE_LOG(RS_DEBUG,"[framework] Recv %u BYTES  Frame From Svr Succ. ",tibetan_recv_appframe_->frame_length_);

    if (data_len > 0)
    {
        //解码
        ret = msg_recv_frame_->appdata_decode(msg);

        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
}

//发送数据
template< class T1>
int Lolo_SendRecv_Package::send_svc_msg(uint32_t user_id,
                                        uint32_t cmd,
                                        const T1& info,
                                        uint32_t backfill_fsm_id,
                                        uint16_t business_id)
{
    int ret = 0;
    msg_send_frame_->command_ = cmd;

    msg_send_frame_->send_service_ = msg_send_service_;
    msg_send_frame_->recv_service_ = msg_recv_service_;
    msg_send_frame_->proxy_service_ = msg_proxy_service_;

    //保证事务ID不是0
    ++trans_id_builder_;

    if (++trans_id_builder_ == 0)
    {
        ++trans_id_builder_;
    }

    msg_send_frame_->fsm_id_ = trans_id_builder_;

    //如果有回填数据
    msg_send_frame_->backfill_fsm_id_ = backfill_fsm_id;
    msg_send_frame_->user_id_ = user_id;

    //填写GAME ID
    msg_send_frame_->business_id_ = business_id;

    ret = msg_send_frame_->appdata_encode(soar::Zerg_Frame::MAX_LEN_OF_DATA,
                                          info);

    if (ret != 0)
    {
        return ret;
    }

    int len = msg_send_frame_->length_;

    //阻塞发送所有的数据
    msg_send_frame_->hton();
    ssize_t socket_ret = lolo_datagram_.sendto((void*)(msg_send_frame_),
                                               len,
                                               &lolo_svc_ip_,
                                               NULL
    );

    //ZCE_LOG(RS_DEBUG,"[framework] SEND %u BYTES  Frame To Svr Succ. ",ret);
    if (socket_ret <= 0)
    {
        ZCE_LOG(RS_ERROR, "[framework]UDP Send soar::Zerg_Frame head len error ,frame len:%d,error[%u|%s].",
                msg_recv_frame_->length_,
                zce::last_error(),
                strerror(zce::last_error()));
        return SOAR_RET::ERROR_ZULU_SEND_PACKAGE_FAIL;
    }

    return 0;
}

//发送和接收数据，一起一锅搞掂的方式
template< class T1, class T2>
int Lolo_SendRecv_Package::send_recv_msg(unsigned int snd_cmd,
                                         uint32_t user_id,
                                         const T1& send_info,
                                         zce::Time_Value* time_wait,
                                         bool if_recv,
                                         unsigned int rcv_cmd,
                                         T2& recv_info,
                                         uint32_t backfill_fsm_id,
                                         uint16_t business_id)
{
    int ret = 0;

    //发送数据
    ret = send_svc_msg(user_id,
                       snd_cmd,
                       send_info,
                       backfill_fsm_id,
                       business_id);

    if (ret != 0)
    {
        return ret;
    }
    if (false == if_recv)
    {
        return 0;
    }

    //收取数据，
    ret = receive_svc_msg(rcv_cmd,
                          recv_info,
                          time_wait
    );
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

#endif //COMM_SNDRCV_UDP_NET_LO_LO_H_
