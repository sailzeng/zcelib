

#ifndef OGRE_TCP_CONTROL_SERVICE_H_
#define OGRE_TCP_CONTROL_SERVICE_H_

#include "ogre_tcppeer_id_set.h"
#include "ogre_auto_connect.h"

class Ogre4a_App_Frame;
class mmap_dequechunk;
class Soar_MMAP_BusPipe;

/****************************************************************************************************
class  Ogre_TCP_Svc_Handler
****************************************************************************************************/
class Ogre_TCP_Svc_Handler : public  ZCE_Event_Handler,
    public ZCE_Timer_Handler
{
public:

    //�����ģʽ
    enum OGRE_HANDLER_MODE
    {
        //�������ӵ�Handler
        HANDLER_MODE_CONNECT,
        //��������ĳ���˿ڵ�Handler
        HANDLER_MODE_ACCEPTED,
    };

protected:

    //
    enum PEER_STATUS
    {
        //PEER û��������
        PEER_STATUS_NOACTIVE,
        //PEER �ո�������,����û�з��ͻ����ܵ��κ�����
        PEER_STATUS_JUST_CONNECT,
        //PEER �Ѿ����ڼ���״̬,
        PEER_STATUS_ACTIVE,
    };

public:

    //���캯��
    explicit Ogre_TCP_Svc_Handler(Ogre_TCP_Svc_Handler::OGRE_HANDLER_MODE hdl_mode);
    //Ϊ�������޷��ڶ�����ʹ��Ogre_TCP_Svc_Handler
protected:
    virtual ~Ogre_TCP_Svc_Handler();

public:


    /*!
    * @brief      ��ʼ������
    * @return     void
    * @param      sockstream
    * @param      fp_judge_whole
    */
    void init_tcp_svc_handler(const ZCE_Socket_Stream &sockstream,
                              FP_JudgeRecv_WholeFrame fp_judge_whole);

    /*!
    * @brief
    * @param      sockstream
    * @param      socketaddr
    * @param      fp_judge_whole
    */
    void init_tcp_svc_handler(const ZCE_Socket_Stream &sockstream,
                              const ZCE_Sockaddr_In &socketaddr,
                              FP_JudgeRecv_WholeFrame fp_judge_whole);

    //ZEN��һ��Ҫ���Լ��̳еĺ���.
    virtual ZCE_HANDLE get_handle(void) const;
    //
    virtual int handle_input(ZCE_HANDLE);
    //
    virtual int handle_output(ZCE_HANDLE);
    //
    virtual int timer_timeout(const ZCE_Time_Value &time, const void *arg);
    //
    virtual int handle_close ();

    //�õ�Handle��ӦPEER��IP��ַ
    const ZCE_Sockaddr_In &get_peer();

protected:

    //��PEER��ȡ����
    int read_data_from_peer(size_t &szrevc);
    //������д��PEER
    int write_data_to_peer(size_t &szsend, bool &bfull);
    //�����п��Է��͵����ݶ����ͳ�ȥ
    int write_all_aata_to_peer();

    //Ԥ����,�������,���յ�REGISTER����,���ݵ�һ����������Ӧ��ϵ
    int  preprocess_recv_frame();

    //�����͵�REGISTER����,���Ӻ��͵�һ������
    int  process_connect_register();

    //�����ʹ���
    int process_senderror(Ogre4a_App_Frame *tmpbuf);

    //�õ�һ��PEER��״̬
    PEER_STATUS  get_peer_status();

    //������֡����ܵ�,����ΪҪ��������ݳ���
    int push_frame_to_recvpipe(unsigned int sz_data);

    //��һ�����͵�֡����ȴ����Ͷ���
    int put_frame_to_sendlist(Ogre4a_App_Frame *ogre_frame);

    //�ϲ����͵�֡����
    void unite_frame_sendlist();

public:

    ///��ȡ�����ļ�
    static int get_config(const Ogre_Server_Config *config);

    ///��ʼ����̬����
    static int init_all_static_data();

    ///ע����̬����
    static int unInit_all_static_data();

    //�ӳ��ӷ���һ��Handler
    static Ogre_TCP_Svc_Handler *alloc_svchandler_from_pool(OGRE_HANDLER_MODE handler_mode);

    //ȡ�����õ����PEER����
    static void get_maxpeer_num(size_t &maxaccept, size_t &maxconnect);

    //�����ݴӶ˿ڷ�������
    static int process_send_data(Ogre4a_App_Frame *tmpbuf );


    /*!
    * @brief
    * @return     int == 0��ʾ�ҵ���
    * @param      peer_id  ��ѯ��PEER ID
    * @param      svchanle ���ز�ѯ���ľ��
    */
    static int find_services_peer(const OGRE_PEER_ID &peer_id,
                                  Ogre_TCP_Svc_Handler *&svchanle);

    ///��û�����ӵĵķ�������������
    static int connect_all_server();

protected:

    ///
    typedef zce::lordrings<Ogre_TCP_Svc_Handler *> POOL_OF_TCP_HANDLER;

    ///��ʱ��ID,����New����,����,����������뷨,ACE timer_timeoutΪʲô��ֱ��ʹ��TIMEID
    static const  int      TCPCTRL_TIME_ID[];

    ///һ��δ�������ϵ�Connect �����Եȴ����͵�FRAME����
    static const size_t   MAX_LEN_OF_SEND_LIST = 8;

    ///ACCEPT PEER�����Եȴ����͵�FRAME����
    static const size_t   MAX_OF_ACCEPT_PEER_SEND_DEQUE = 64;
    ///CONNECT PEER�����Եȴ����͵�FRAME����,
    static const size_t   MAX_OF_CONNECT_PEER_SEND_DEQUE = 256;

protected:

    ///����ܹ�Accept��PEER����,
    static size_t                  max_accept_svr_;
    ///����ܹ�Connect��PEER����
    static size_t                  max_connect_svr_;

    ///Connect��ȴ�������ʱ��,Connect��ʱ
    static unsigned int           accept_timeout_;

    ///�ȴ�����һ���������ݵĳ�ʱʱ��,Ϊ0��ʾ������
    static unsigned int           receive_timeout_;

    ///���ڴ��������,���Է��͵Ĵ���,ֻ���˱�֤һ��������˲��
    static unsigned int           error_try_num_;

    ///SVRINFO��Ӧ��PEER��HASHMAP
    static PeerID_To_TCPHdl_Map   svr_peer_hdl_set_;

    ///Ҫ�Զ����ӵķ������Ĺ�����
    static Ogre_Connect_Server    zerg_auto_connect_;

    ///�Ѿ�Accept��PEER����
    static size_t                 num_accept_peer_;
    ///�Ѿ�Connect��PEER����
    static size_t                 num_connect_peer_;

    ///ACCEPT SVC handler�ĳ���
    static POOL_OF_TCP_HANDLER    pool_of_acpthdl_;

    ///CONNECT svc handler�ĳ���
    static POOL_OF_TCP_HANDLER    pool_of_cnthdl_;

protected:

    ///����ģʽ
    OGRE_HANDLER_MODE             handler_mode_;

    ///�������ݵĻ���
    Ogre4a_App_Frame              *rcv_buffer_;


    ///���͵����ݿ���Ҫ�Ŷ�
    zce::lordrings<Ogre4a_App_Frame *>  \
    snd_buffer_deque_;

    ///���PEER��������
    size_t                        recieve_bytes_;

    ///���PEER��������
    size_t                        send_bytes_;

    ///ACE Socket Stream,
    ZCE_Socket_Stream             socket_peer_;

    ///PEER���ӵ�Զ��IP��ַ��Ϣ,����������Ϊ�˼ӿ��ٶ�.
    ZCE_Sockaddr_In               remote_address_;

    ///PEER���ӵı���IP��ַ��Ϣ,����������Ϊ�˼ӿ��ٶ�.
    ZCE_Sockaddr_In               local_address_;

    //�Ƿ��ڻ״̬
    PEER_STATUS                   peer_status_;

    ///���Ӻ��޷�Ӧ��ʱ��TimeID,
    long                          timeout_time_id_;

    ///һ��ʱ�����ڽ������ݵĴ���
    unsigned int                  receive_times_;

    ///�Ƿ�ǿ��Ҫ���ڷ�����ɺ�رն˿�
    bool                          if_force_close_;

    ///�жϽ��յ��������Ƿ��������Ĵ�����ָ��
    FP_JudgeRecv_WholeFrame       fp_judge_whole_frame_;

};

#endif //OGRE_TCP_CONTROL_SERVICE_H_

