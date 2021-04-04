/*!
* @copyright  2004-2015  Apache License, Version 2.0 FULLSAIL
* @filename   soar_zerg_sndrcv_lolo.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006��9��8��
* @brief
*
*
* @details
*
*
*
* @note       ��������ʵ���˼��ʵ�Ǻ����˶������������ƣ���Ȼ������������а��
*             ����������ֻ��Ϊ�˼���һ�����ϵĸ���
*             ���������������ˣ�LOLO
*
*             �������ϵĶ���
*             ����
*             �������ȥ��ǰ��ס���Ĵ�ׯ
*             ������һ���������ϵ��Ǹ�����
*             ��һ�ȷ�β������ʽ��ˮ��
*             ��һ��ֻ����˵����������ɽ
*             �������ȥ����ǰ��ס���Ĵ�ׯ
*             ������һ���������ϵ��Ǹ�����
*             ��һ�ȷ�β������ʽ��ˮ��
*             ��һ��ֻ����˵����������ɽ
*             ��˵���ɽ�ὥ������
*             ��˵���ˮ��ԴԶ����
*             ��˵����������Ŷ���
*             ��˵��Ĵ�ׯû�б���
*             ���Ҹ���ȥ���㲻�ϵĴ�ׯ
*             һ�����ܲ�֪�������Ƿݹµ�
*             �������������¿���������
*             �����ں����߰��������ˮ�ȸ�
*             ���Ҹ���ס���㲻�ϵĴ�ׯ
*             һ��ȹ����Բ�����Ƕ�ʱ��
*             ������ˮ���������¹���ת
*             ��������������̫��¶��ɽ
*             ��~��Ĵ�ׯҲ�и�����
*             ��~��Ĵ�ׯҲû�б���
*             ��~��Ĵ�ׯҲ�и�����
*             ��~��Ĵ�ׯҲû�б���
*/


#ifndef COMM_SNDRCV_UDP_NET_LOLO_H_
#define COMM_SNDRCV_UDP_NET_LOLO_H_

#include "soar_zerg_sndrcv_base.h"

/******************************************************************************************
class Lolo_SendRecv_Package
******************************************************************************************/
class Lolo_SendRecv_Package  : public SendRecv_Package_Base
{
protected:

    //ZULU���ӵ�IP��ַ
    ZCE_Sockaddr_In          lolo_svc_ip_;

    //�յ����ݵĶԶ˵�ַ��99.99999�����������һ��
    ZCE_Sockaddr_In          lolo_recvdata_ip_;

    //ʹ�õ�SOCKET��װ
    ZCE_Socket_DataGram      lolo_datagram_;

public:
    //
    Lolo_SendRecv_Package();
    virtual ~Lolo_SendRecv_Package();

    //������Ӧ��SVC INFO,
    int set_lolo_svcinfo(const char *svc_ip,
                         unsigned short svc_port,
                         const SERVICES_ID &recv_service,
                         const SERVICES_ID &send_service,
                         const SERVICES_ID &proxy_service,
                         size_t frame_len = Zerg_App_Frame::MAX_LEN_OF_APPFRAME,
                         bool is_check_conn_info = false);

    //��������
    template< class T1>
    int send_svc_package(unsigned int user_id,
                         unsigned int cmd,
                         const T1 &snd_info,
                         unsigned int app_id = 0,
                         unsigned int backfill_trans_id = 0);

    //��������
    template< class T2>
    int receive_svc_package(unsigned int cmd,
                            T2 &rcv_info,
                            ZCE_Time_Value *time_out = NULL);

    //���ͺͽ������ݣ�һ��һ�����ķ�ʽ
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

//������
template<class T>
int Lolo_SendRecv_Package::receive_svc_package(unsigned int cmd,
                                               T &info,
                                               ZCE_Time_Value *time_wait)
{
    int ret = 0;
    ssize_t socket_ret  = 0;
    int data_len = 0;

    // ��������
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
                zce::last_error(),
                strerror(zce::last_error()));
        return SOAR_RET::ERROR_ZULU_LINK_DISCONNECT;
    }
    //С��0һ���ʾ����,�������������Ҳ��ʾһ������,�Ҹ�����ȴ�ʱ����Ϊѡ��
    else if (socket_ret < 0)
    {
        //����������źŵ��µ�����
        int last_error =  zce::last_error();

        ZCE_LOG(RS_ERROR, "[framework] RECV Zerg_App_Frame head error or time out. Ret:%d, error[%u|%s].",
                socket_ret,
                last_error,
                strerror(last_error));

        return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
    }

    //������յ�������ID
    recv_trans_id_ = tibetan_recv_appframe_->transaction_id_;
    //���ݰ��ĳ���
    data_len = tibetan_recv_appframe_->frame_length_ - Zerg_App_Frame::LEN_OF_APPFRAME_HEAD;

    if (data_len < 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] UDP Receive Zerg_App_Frame head len error ,frame len:%d,error[%u|%s].",
                tibetan_recv_appframe_->frame_length_,
                zce::last_error(),
                strerror(zce::last_error()) );
        return SOAR_RET::ERROR_ZULU_RECEIVE_PACKAGE_FAIL;
    }

    //ֻ�����ͷ��Ϣ
    if (data_len == 0)
    {
        return 0;
    }

    //ZCE_LOG(RS_DEBUG,"[framework] Recv %u BYTES  Frame From Svr Succ. ",tibetan_recv_appframe_->frame_length_);

    if (data_len > 0)
    {
        //����
        ret = tibetan_recv_appframe_->appdata_decode(info);

        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
}

//��������
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

    //��֤����ID����0
    ++trans_id_builder_;

    if (++trans_id_builder_ == 0)
    {
        ++trans_id_builder_;
    }

    tibetan_send_appframe_->transaction_id_ = trans_id_builder_;

    //����л�������
    tibetan_send_appframe_->backfill_trans_id_ = backfill_trans_id;
    tibetan_send_appframe_->frame_uid_ = user_id;

    //��дGAME ID
    tibetan_send_appframe_->app_id_ = app_id;

    ret = tibetan_send_appframe_->appdata_encode(Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

    if (ret != 0 )
    {
        return ret;
    }

    int len = tibetan_send_appframe_->frame_length_;

    //�����������е�����
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
                zce::last_error(),
                strerror(zce::last_error()) );
        return SOAR_RET::ERROR_ZULU_SEND_PACKAGE_FAIL;
    }

    return 0;
}

//���ͺͽ������ݣ�һ��һ�����ķ�ʽ
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

    //��������
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

    //��ȡ���ݣ�
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

