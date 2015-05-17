/*!
* @copyright  2004-2015  Apache License, Version 2.0 FULLSAIL
* @filename   soar_zerg_sndrcv_lolo.h
* @author     Sailzeng <sailerzeng@gmail.com>
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

#include "soar_zerg_sndrcv_base.h"

/******************************************************************************************
class Lolo_SendRecv_Package
******************************************************************************************/
class Lolo_SendRecv_Package  : public Tibetan_SendRecv_Package
{
protected:

    //ZULU链接的IP地址
    ZCE_Sockaddr_In          lolo_svc_ip_;

    //收到数据的对端地址，99.99999的情况和上面一样
    ZCE_Sockaddr_In          lolo_recvdata_ip_;

    //使用的SOCKET包装
    ZCE_Socket_DataGram      lolo_datagram_;

public:
    //
    Lolo_SendRecv_Package();
    virtual ~Lolo_SendRecv_Package();

    //设置相应的SVC INFO,
    int set_lolo_svcinfo(const char *svc_ip,
                         unsigned short svc_port,
                         const SERVICES_ID &recv_service,
                         const SERVICES_ID &send_service,
                         const SERVICES_ID &proxy_service,
                         size_t frame_len = Zerg_App_Frame::MAX_LEN_OF_APPFRAME,
                         bool is_check_conn_info = false);

    //发送数据
    template< class T1>
    int send_svc_package(unsigned int user_id,
                         unsigned int cmd,
                         const T1 &snd_info,
                         unsigned int app_id = 0,
                         unsigned int backfill_trans_id = 0);

    //接受数据
    template< class T2>
    int receive_svc_package(unsigned int cmd,
                            T2 &rcv_info ,
                            ZCE_Time_Value *time_out = NULL);

    //发送和接收数据，一起一锅搞掂的方式
    template< class T1, class T2>
    int send_recv_package(unsigned int snd_cmd,
                          unsigned int user_id,
                          const T1 &send_info,
                          ZCE_Time_Value *time_wait,
                          bool if_recv,
                          unsigned int rcv_cmd,
                          T2 &recv_info,
                          unsigned int app_id = 0,
                          unsigned int backfill_trans_id = 0);

};

//收数据
template<class T>
int Lolo_SendRecv_Package::receive_svc_package(unsigned int cmd,
                                               T &info ,
                                               ZCE_Time_Value *time_wait)
{
    int ret = 0;
    ssize_t socket_ret  = 0;
    int data_len = 0;

    // 接收数据
    socket_ret = lolo_datagram_.recvfrom((void *)(tibetan_recv_appframe_),
                                         test_frame_len_,
                                         &lolo_recvdata_ip_,
                                         time_wait);

    tibetan_recv_appframe_->framehead_decode();

    tibetan_recv_appframe_->dumpoutput_framehead("UDP recv", RS_DEBUG);

#if defined _DEBUG || defined DEBUG
    ZCE_ASSERT(test_frame_len_ >= tibetan_recv_appframe_->frame_length_);
#endif //#if defined _DEBUG || defined DEBUG

    //ret == 0
    if (socket_ret == 0 )
    {
        ZCE_LOG(RS_INFO, "[framework] Link is disconnect recv ret =%d, error[%u|%s].",
                socket_ret,
                ZCE_LIB::last_error(),
                strerror(ZCE_LIB::last_error()));
        return SOAR_RET::ERROR_ZULU_LINK_DISCONNECT;
    }
    //小于0一般表示错误,对于阻塞，这儿也表示一个错误,我给了你等待时间作为选择
    else if (socket_ret < 0)
    {
        //如果错误是信号导致的重入
        int last_error =  ZCE_LIB::last_error();

        ZCE_LOG(RS_ERROR, "[framework] RECV Zerg_App_Frame head error or time out. Ret:%d, error[%u|%s].",
                socket_ret,
                last_error,
                strerror(last_error));

        return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
    }

    //保存接收到的事务ID
    recv_trans_id_ = tibetan_recv_appframe_->transaction_id_;
    //数据包的长度
    data_len = tibetan_recv_appframe_->frame_length_ - Zerg_App_Frame::LEN_OF_APPFRAME_HEAD;

    if (data_len < 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] UDP Receive Zerg_App_Frame head len error ,frame len:%d,error[%u|%s].",
                tibetan_recv_appframe_->frame_length_,
                ZCE_LIB::last_error(),
                strerror(ZCE_LIB::last_error()) );
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
        ret = tibetan_recv_appframe_->appdata_decode(info);

        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
}

//发送数据
template< class T>
int Lolo_SendRecv_Package::send_svc_package(unsigned int user_id,
                                            unsigned int cmd,
                                            const T &info,
                                            unsigned int app_id,
                                            unsigned int backfill_trans_id)
{
    int ret = 0;
    tibetan_send_appframe_->frame_command_ = cmd;

    tibetan_send_appframe_->send_service_ = tibetan_send_service_;
    tibetan_send_appframe_->recv_service_ = tibetan_recv_service_;
    tibetan_send_appframe_->proxy_service_ = tibetan_proxy_service_;

    //保证事务ID不是0
    ++trans_id_builder_;

    if (++trans_id_builder_ == 0)
    {
        ++trans_id_builder_;
    }

    tibetan_send_appframe_->transaction_id_ = trans_id_builder_;

    //如果有回填数据
    tibetan_send_appframe_->backfill_trans_id_ = backfill_trans_id;
    tibetan_send_appframe_->frame_uid_ = user_id;

    //填写GAME ID
    tibetan_send_appframe_->app_id_ = app_id;

    ret = tibetan_send_appframe_->appdata_encode(Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

    if (ret != 0 )
    {
        return ret;
    }

    int len = tibetan_send_appframe_->frame_length_;

    //阻塞发送所有的数据
    tibetan_send_appframe_->framehead_encode();

    //
    ssize_t socket_ret = lolo_datagram_.sendto((void *)(tibetan_send_appframe_),
                                               len,
                                               &lolo_svc_ip_,
                                               NULL
                                              );

    //ZCE_LOG(RS_DEBUG,"[framework] SEND %u BYTES  Frame To Svr Succ. ",ret);
    if (socket_ret <= 0 )
    {
        ZCE_LOG(RS_ERROR, "[framework]UDP Send Zerg_App_Frame head len error ,frame len:%d,error[%u|%s].",
                tibetan_recv_appframe_->frame_length_,
                ZCE_LIB::last_error(),
                strerror(ZCE_LIB::last_error()) );
        return SOAR_RET::ERROR_ZULU_SEND_PACKAGE_FAIL;
    }

    return 0;
}

//发送和接收数据，一起一锅搞掂的方式
template< class T1, class T2>
int Lolo_SendRecv_Package::send_recv_package(unsigned int snd_cmd,
                                             unsigned int user_id,
                                             const T1 &send_info,
                                             ZCE_Time_Value *time_wait,
                                             bool if_recv,
                                             unsigned int rcv_cmd,
                                             T2 &recv_info,
                                             unsigned int app_id,
                                             unsigned int backfill_trans_id)
{
    int ret = 0;

    //发送数据
    ret = send_svc_package(user_id,
                           snd_cmd,
                           send_info,
                           app_id,
                           backfill_trans_id);

    if (ret != 0)
    {
        return ret;
    }

    if (false == if_recv)
    {
        return 0;
    }

    //收取数据，
    ret = receive_svc_package(rcv_cmd,
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

