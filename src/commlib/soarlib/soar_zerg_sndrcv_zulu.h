/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_zerg_sndrcv_zulu.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Friday, September 05, 2014
* @brief      2006��9��8��,ԭ����Scottxu ����ֲ,Sailzeng
*
*
* @details
*
*
*
* @note       ����δ����ƶ���COMM,ģ�廯
*
*/

#ifndef SOARING_LIB_SNDRCV_TCP_ZULU_H_
#define SOARING_LIB_SNDRCV_TCP_ZULU_H_

#include "soar_zerg_sndrcv_base.h"

/******************************************************************************************
class Zulu_SendRecv_Package
******************************************************************************************/
class Zulu_SendRecv_Package : public SendRecv_Package_Base
{

protected:

    //ZULU �Ƿ�����
    bool                   zulu_connected_;

    //ZULU���ӵ�IP��ַ
    ZCE_Sockaddr_In        zulu_svc_ip_;

    //ʹ�õ�SOCKET��װ
    ZCE_Socket_Stream      zulu_stream_;

public:

    //���캯������������
    Zulu_SendRecv_Package();
    ~Zulu_SendRecv_Package();

protected:

    //����һ��APPFRAME�������ڲ���tibetan_recv_appframe_
    int receive_svc_package(ZCE_Time_Value *time_wait = NULL);

    //���ڲ���tibetan_send_appframe_���ͳ�ȥ
    int send_svc_package(ZCE_Time_Value *time_wait = NULL);

public:

    //������Ӧ��SVC INFO,
    int set_zulu_svcinfo(const char *svc_ip,
                         unsigned short svc_port,
                         const SERVICES_ID &recv_service,
                         const SERVICES_ID &send_service,
                         const SERVICES_ID &proxy_service,
                         size_t frame_len = Zerg_App_Frame::MAX_LEN_OF_APPFRAME);

    //���ӷ�����,time_wait������const����ԭ��ģ���Ϊ����select��ZCE_Time_Value�Ƿ���ʣ��ʱ���
    int connect_zulu_server(ZCE_Time_Value *time_wait);

    //�ر��ڲ������ӡ�
    void close();

    //ȡ�ñ��صĵ�ַ��Ϣ
    int getsockname (ZCE_Sockaddr *addr)  const;

    /*!
    * @brief      ��������
    * @tparam     T1
    * @return     int
    * @param      user_id  USER ID
    * @param      cmd      ���͵�����
    * @param      snd_info ����ķ��ͽṹ
    * @param      time_out ���͵ĳ�ʱʱ��
    * @param      app_id   Ӧ��ID
    * @param      backfill_trans_id ���������ID��Ĭ��Ϊ0����ʾ������
    */
    template< class T1>
    int send_svc_package(unsigned int user_id,
                         unsigned int cmd,
                         const T1 &snd_info,
                         ZCE_Time_Value *time_out = NULL,
                         unsigned int app_id = 0,
                         unsigned int backfill_trans_id = 0);

    /*!
    * @brief      �������ݣ������Ľ���һ��APPFRAME����
    * @tparam     T2
    * @return     int       OK
    * @param      cmd       Ԥ�ƽ��ܵĵ�������
    * @param      rcv_info  ���յ���Ϣ����
    * @param      error_continue ����յ������ݲ��������ģ��ͼ����ȴ���ֱ���ȴ���Ӧ������
    * @param      time_out  ��ʱʱ�������Ҫһֱ����������NULL
    */
    template< class T2>
    int receive_svc_package(unsigned int cmd,
                            T2 &rcv_info,
                            bool error_continue = true,
                            ZCE_Time_Value *time_out = NULL);

    //����һ�����ݰ����õ������֣�����Ե���get_recv_appframe���к����Ĵ���
    int receive_svc_package(unsigned int &recv_cmd,
                            ZCE_Time_Value *time_out = NULL);


    /*!
    * @brief      ���ͺͽ������ݣ�����ǰ�������ӵġ�
    * @note       ������ο�send_svc_package,receive_svc_package
    */
    template< class T1, class T2>
    int send_recv_package(unsigned int snd_cmd,
                          unsigned int user_id,
                          const T1 &send_info,
                          ZCE_Time_Value *time_wait,
                          unsigned int rcv_cmd,
                          T2 &recv_info,
                          bool error_continue = true,
                          unsigned int app_id = 0,
                          unsigned int backfill_trans_id = 0);

};

//�����Ľ���һ��APPFRAME����
template<class T2>
int Zulu_SendRecv_Package::receive_svc_package(unsigned int cmd,
                                               T2 &info,
                                               bool error_continue,
                                               ZCE_Time_Value *time_wait)
{
    int ret = 0;

    //����ѭ����֤��������������Դͳһ�ͷ�
    for (;;)
    {
        ret = receive_svc_package(time_wait);

        if ( 0 != ret )
        {
            break;
        }

        //����յ�������֡���ǰ��ڴ���
        if (cmd != tibetan_recv_appframe_->frame_command_ )
        {
            //�����������,��������,���������
            if (error_continue )
            {
                continue;
            }
            else
            {
                ZCE_LOG(RS_ERROR, "[framework] recv a error or unexpect frame,expect cmd =%u,recv cmd =%u.",
                        cmd,
                        tibetan_recv_appframe_->frame_command_);
                ret =  SOAR_RET::ERROR_ZULU_RECEIVE_OTHERS_COMMAND;
                break;
            }
        }
        else
        {
            //�������data�����ݣ����н���
            if (tibetan_recv_appframe_->frame_length_ > Zerg_App_Frame::LEN_OF_APPFRAME_HEAD )
            {
                ret = tibetan_recv_appframe_->appdata_decode(info);

                if (ret != 0)
                {
                    return ret;
                }
            }
        }

        //����ѭ��
        break;
    }

    //�����д��󣬽������⴦��
    if (ret != 0)
    {
        zulu_stream_.close();
        zulu_connected_ = false;
        return ret;
    }

    return 0;
}

//����һ�����ݰ�
template< class T1>
int Zulu_SendRecv_Package::send_svc_package(unsigned int user_id,
                                            unsigned int cmd,
                                            const T1 &info,
                                            ZCE_Time_Value *time_wait,
                                            unsigned int app_id,
                                            unsigned int backfill_trans_id)
{
    int ret = 0;

    //��û�����ӵ���������ӷ�����
    if ( false == zulu_connected_ )
    {
        ret = connect_zulu_server(time_wait);

        if (0 != ret)
        {
            return ret;
        }
    }

    tibetan_send_appframe_->frame_command_ = cmd;
    tibetan_send_appframe_->backfill_trans_id_ = backfill_trans_id;
    tibetan_send_appframe_->app_id_ = app_id;
    tibetan_send_appframe_->frame_uid_ = user_id;

    //����
    ret = tibetan_send_appframe_->appdata_encode(Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

    if (ret != 0 )
    {
        return ret;
    }

    ret = send_svc_package(time_wait);

    if (ret != 0)
    {
        zulu_stream_.close();
        zulu_connected_ = false;
        return ret;
    }

    return 0;
}

//����һ�����ݰ������ҽ���һ�����ݰ�
template< class T1, class T2>
int Zulu_SendRecv_Package::send_recv_package(unsigned int snd_cmd,
                                             unsigned int user_id,
                                             const T1 &send_info,
                                             ZCE_Time_Value *time_wait,
                                             unsigned int rcv_cmd,
                                             T2 &recv_info,
                                             bool error_continue,
                                             unsigned int app_id,
                                             unsigned int backfill_trans_id)
{
    int ret = 0;

    //��������
    ret = send_svc_package(user_id,
                           snd_cmd,
                           send_info,
                           time_wait,
                           app_id,
                           backfill_trans_id);

    if (ret != 0)
    {
        return ret;
    }

    //��ȡ���ݣ�
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

