/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_sndrcv_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��11��30��
* @brief      ��ӦZERG�ģ����ԣ����߿ͻ��˽ӿڣ�
*             ���ڽ�TCP��UDP�ķ��ʹ��벿����һ�����ϣ����һ��base�࣬����һЩ��������
*
* @details
*
*
*
* @note
*
*/


#ifndef SOARING_LIB_SND_RCV_BASE_H_
#define SOARING_LIB_SND_RCV_BASE_H_

/******************************************************************************************
class SendRecv_Package_Base base ��
******************************************************************************************/
class SendRecv_Package_Base
{

public:
    //
    SendRecv_Package_Base();
    virtual ~SendRecv_Package_Base();

public:

    //������Ӧ��SVC INFO,
    void set_services_id(const SERVICES_ID &recv_service,
                         const SERVICES_ID &send_service,
                         const SERVICES_ID &proxy_service,
                         size_t frame_len = Zerg_App_Frame::MAX_LEN_OF_APPFRAME);

    //ȡ���յ�������ID
    void get_recv_transid(unsigned int &trans_id);

    unsigned int get_send_transid();
    unsigned int get_backfill_transid();

    //ȡ�ò��Ե�APPFRAME
    Zerg_App_Frame *get_send_appframe();
    //ȡ�ý��յ�APPFRAME
    Zerg_App_Frame *get_recv_appframe();

protected:

    //�����ߵ�SERVICES_ID
    SERVICES_ID              tibetan_recv_service_;
    //�����ߵ�tibetan_send_service_��
    SERVICES_ID              tibetan_send_service_;
    //�������������Ϣ
    SERVICES_ID              tibetan_proxy_service_;

    //��������֡�ĳ���,��������ý������ݵĴ�С���������64K,̫�����Ӱ��������������,��Ŀǰ�о����ⲻ��
    size_t                    test_frame_len_;

    //����ID������������һ��������
    unsigned int              trans_id_builder_;

    //Ϊ�˶��߳̽�ԭ����test_appframe_��Ϊ��2����һ������һ�����ͣ������ڶ��̴߳��������£�
    //�Ͳ�����ֹ���һ��BUFFER������

    //���ͻ�������֡
    Zerg_App_Frame *tibetan_send_appframe_;
    //���ջ�����
    Zerg_App_Frame *tibetan_recv_appframe_;

    //�յ�������ID
    unsigned int              recv_trans_id_;

    //���������ID
    unsigned int              backfill_trans_id_;

};

#endif //SOARING_LIB_SND_RCV_BASE_H_

