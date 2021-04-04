
#ifndef ZERG_COMMUNICATION_MANAGER_H_
#define ZERG_COMMUNICATION_MANAGER_H_

#include "zerg_configure.h"
#include "zerg_stat_define.h"

//��С���䣺�����ǰ�ǹ�ɣ����߽�����ëǮ
//��������¼һ�£�Ϊʲôÿ���˿�Ҫ����һ��SVC ID��������⣬
//�Ҹ�������ͻῼ��һ�ߡ�����ÿ�ζ��ῼ�ǵ�UDP��͸���������,̫TMD��������ˡ�
//����̫���Ľ������ʵ�ܼ�
//���һ��SVCID��Ӧ����˿ڣ��㲻���ֱ�ʶ������ô����������֪�����ӵ�SVCID�������Ǹ��˿���

class TCP_Accept_Handler;
class UDP_Svc_Handler;
class Soar_MMAP_BusPipe;

#include "zerg_buf_storage.h"

/****************************************************************************************************
class  Zerg_Comm_Manager
****************************************************************************************************/
class Zerg_Comm_Manager
{

    //
    typedef std::vector<TCP_Accept_Handler *> TCPACCEPT_HANDLER_LIST;
    //
    typedef std::vector<UDP_Svc_Handler *> UDPSVC_HANDLER_LIST;

protected:

    //���캯������������
    Zerg_Comm_Manager();
    ~Zerg_Comm_Manager();

public:


    /*!
    * @brief      ��ʼ��,�������ļ���ȡ����
    * @return     int
    * @param      config
    */
    int get_config(const Zerg_Server_Config *config);

    /*!
    * @brief      ��ʼ�����еļ�����UDP�˿ڣ�
    * @return     int
    */
    int init_allpeer();

    /*!
    * @brief      ����SVC INFO ��ʼ��Socket,
    * @return     int
    * @param      init_svcid ��ʼ�������ݵ�SVC INFO
    */
    int init_socketpeer(const SERVICES_ID &init_svcid);

    /*!
    * @brief      ���˿��Ƿ�ȫ,��ȫ�˿ڱ��벻ʹ�ñ���(FALSE)
    * @return     int
    * @param      inetadd ���ĵĵ�ַ��Ϣ
    */
    int check_safeport(const ZCE_Sockaddr_In &inetadd);


    /*!
    * @brief      ȡ�÷������ݽ��з���
    * @return     int
    * @param      want_send_frame  ϣ�����͵������������룬���Ǽ���һ����෢�͵�֡���޶�
    * @param      proc_frame_num   ʵ�ʴ��������
    * @note
    */
    int popall_sendpipe_write(size_t want_send_frame, size_t &proc_frame_num);

    //
    void pushback_recvpipe(Zerg_App_Frame *recv_frame);

    //��鷢��Ƶ��
    void check_freamcount(unsigned int now);

    //
    int send_single_buf(Zerg_Buffer *tmpbuf);

public:

    //����ʵ������
    static Zerg_Comm_Manager *instance();
    //������ʵ��
    static void clean_instance();

protected:

    ///һ����෢��2048֡
    static const unsigned int MAX_ONCE_SEND_FRAME = 4096;

    ///�������澯ֵ
    static const unsigned int SEND_FRAME_ALERT_VALUE = 40000;

protected:
    //����ʵ��
    static Zerg_Comm_Manager  *instance_;


protected:

    ///ACCEPET��HANDLER����
    TCPACCEPT_HANDLER_LIST zerg_acceptor_;
    ///UPD��HANDLER����
    UDPSVC_HANDLER_LIST zerg_updsvc_;


    ///���ڴ��������,���Է��͵Ĵ���,ֻ���˱�֤һ��������˲��
    unsigned int error_try_num_;

    ///��������������Ϊ�˼ӿ��ٶȣ����ñ�����
    size_t monitor_size_;
    ///��ص�����
    unsigned int monitor_cmd_[ZERG_CONFIG_DATA::MAX_MONITOR_FRAME_NUMBER];

    ///�ڴ�ܵ����ʵ�����󣬱���������Ϊ�˼���
    Soar_MMAP_BusPipe *zerg_mmap_pipe_;
    ///���ͺͽ��ջ����BUFF��ʵ�����󣬱���������Ϊ�˼���
    ZBuffer_Storage *zbuffer_storage_;
    ///ͳ�ƣ�ʹ�õ������ָ�룬����������Ϊ�˼���
    Soar_Stat_Monitor *server_status_;



    ///������ʼʱ��
    unsigned int count_start_time_;
    ///Э������ͼ�����
    unsigned int send_frame_count_;

    ///����ʵ��ָ��
    const Zerg_Server_Config *zerg_config_;
};



#endif //_ZERG_COMMUNICATION_MANAGER_H_

