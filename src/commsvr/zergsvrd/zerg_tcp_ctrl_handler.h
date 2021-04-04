#ifndef ZERG_TCP_CONTROL_SERVICE_H_
#define ZERG_TCP_CONTROL_SERVICE_H_

#include "zerg_buf_storage.h"
#include "zerg_auto_connect.h"
#include "zerg_active_svchdl_set.h"

class Zerg_Buffer;
class mmap_dequechunk;
class Zerg_Auto_Connector;
class Zerg_Comm_Manager;

/****************************************************************************************************
class  TCP_Svc_Handler
****************************************************************************************************/
class TCP_Svc_Handler : public ZCE_Event_Handler,
    public ZCE_Timer_Handler
{
public:

    ///�����ģʽ
    enum HANDLER_MODE
    {
        ///��Ч������
        HANDLER_MODE_INVALID,
        ///�������ӵ�Handler
        HANDLER_MODE_CONNECT,
        ///��������ĳ���˿ڵ�Handler
        HANDLER_MODE_ACCEPTED,
    };

    //
    enum PEER_STATUS
    {
        ///PEER û��������
        PEER_STATUS_NOACTIVE,
        ///PEER �ո�ACCEPT��,����û�з��ͻ����ܵ��κ�����
        PEER_STATUS_JUST_ACCEPT,
        ///PEER �ո�CONNECT��,����û���յ��κ�����
        PEER_STATUS_JUST_CONNECT,
        ///PEER �Ѿ����ڼ���״̬,
        PEER_STATUS_ACTIVE,
    };


protected:

    ///����ĳ��ӣ�����ÿ�ζ�new����
    typedef zce::lordrings<TCP_Svc_Handler *> POOL_OF_TCP_HANDLER;

    //Ϊ�������޷��ڶ�����ʹ��TCP_Svc_Handler
protected:

    /*!
    * @brief      ���캯��
    * @param      hdl_mode ģʽ���ο�@enum HANDLER_MODE,����ģʽ�Ĳ�ͬ�ڲ����Ͷ��д�С��һ��
    */
    TCP_Svc_Handler(HANDLER_MODE hdl_mode);
    ///��������
    virtual ~TCP_Svc_Handler();

public:


    /*!
    * @brief      ����Accept�Ķ˿ڵĴ���Event Handle��ʼ������.
    * @return     void
    * @param      my_svcinfo  �˾����Ӧ�ı��˵�SVC ID
    * @param      sockstream  SOCKET ���
    * @param      socketaddr  �Զ˵ĵ�ַ����ʵ���Դ�sockstream�еõ�����Ϊ��Ч�ʺͷ���.
    * @note       �Զ˸ոձ�accept��������ʵ��ʱ�޷�ȷ���Զ˵�SVC ID
    */
    void init_tcpsvr_handler(const SERVICES_ID &my_svcinfo,
                             const ZCE_Socket_Stream &sockstream,
                             const ZCE_Sockaddr_In     &socketaddr);


    /*!
    * @brief      ����CONNET���ӳ�ȥ��HANDLER����ӦEvent Handle�ĳ�ʼ��.
    * @return     void
    * @param      my_svcinfo  �˾����Ӧ�ı���SVC ID
    * @param      svrinfo     �Զ˵�SVC ID
    * @param      sockstream  SOCKET ���
    * @param      socketaddr  ��Ӧ���ӵĶԶ˵�ַ��Ϣ
    * @note
    */
    void init_tcpsvr_handler(const SERVICES_ID &my_svcinfo,
                             const SERVICES_ID &svrinfo,
                             const ZCE_Socket_Stream &sockstream,
                             const ZCE_Sockaddr_In     &socketaddr);

    //ZEN��һ��Ҫ���Լ��̳еĺ���.
    //ZCE_Event_Handler�������صĺ�����ȡ��SOCKET���
    virtual ZCE_HANDLE get_handle(void) const;


    /*!
    * @brief      ���¼����� ���첽����ʧ�ܴ���
    * @return     int ����0������
    */
    virtual int handle_input();

    ///д�¼�����
    /*!
    * @brief      ���¼����� ���첽���ӳɹ�����
    * @return     int ����0������
    */
    virtual int handle_output();

    ///�ر��¼�����
    virtual int handle_close();

    /*!
    * @brief      ��ʱ�¼�����
    * @return     virtual int
    * @param      time  ʱ��
    * @param      arg   Ψһ��ʾ����
    */
    virtual int timer_timeout(const ZCE_Time_Value &time, const void *arg);



    ///�õ�Handle��ӦPEER�Ķ˿�
    const ZCE_Sockaddr_In& get_peer();

    //�õ�ÿ��PEER״̬��Ϣ
    void dump_status_info(ZCE_LOG_PRIORITY out_lvl);

    //���ͼ򵥵�ZERG������Է�
    int send_simple_zerg_cmd(unsigned int cmd,
                             const SERVICES_ID &recv_services_info,
                             unsigned int option = 0);

    ///��������
    int send_zergheatbeat_reg();

    ///�õ�һ��PEER��״̬
    PEER_STATUS  get_peer_status();

    ///���һ�����͵�handle
    unsigned int get_handle_id();

    ///�õ��Զ˵�IP��ַ��Ϣ
    const ZCE_Sockaddr_In &get_peer_sockaddr() const;

    ///ȡ��tptoid_table_id_
    size_t get_tptoid_table_id();
    ///����tptoid_table_id_
    void set_tptoid_table_id(size_t ary_id);

protected:

    /*!
    * @brief      ��PEER��ȡ����
    * @return     int
    * @param      szrevc ��ȡ���ֽ�����
    */
    int read_data_from_peer(size_t &szrevc);

    //����յ��������Ƿ���һ�����������ݰ�.
    int check_recv_full_frame(bool &bfull, unsigned int &whole_frame_len);

    /*!
    * @brief      ������д��PEER
    * @return     int �ɹ� ==0
    * @param      szsend ���͵��ֽ�����
    * @param      bfull  �Ƿ������ķ��ͳ�ȥ�ˣ�
    */
    int write_data_to_peer(size_t &szsend, bool &bfull);


    /*!
    * @brief      ������д��PEER��ͬʱ�����ܱߵ����飬����д�¼�ע��,������Ͷ��л������ݣ��������͵�
    * @return     int ==0 ��ʾ�ɹ�
    * @note       ������Ͷ��л������ݣ���������Է���
    */
    int write_all_data_to_peer();


    //Ԥ����,�������,���յ�REGISTER����,���ݵ�һ����������Ӧ��ϵ
    int  preprocess_recvframe(Zerg_App_Frame *proc_frame);

    //�����͵�REGISTER����,���Ӻ��͵�һ������
    int  process_connect_register();


    /*!
    * @brief      ������֡����ͨ�Ź�����������ܵ�
    * @return     int
    * @note       ����������� Zerg_App_Frame�Ѿ�����������.��ע��.
    */
    int push_frame_to_comm_mgr();

    //��һ�����͵�֡����ȴ����Ͷ���
    int put_frame_to_sendlist(Zerg_Buffer *tmpbuf);

    /*!
    * @brief      �ϲ����Ͷ���
    * @note       �����2�����ϵĵķ��Ͷ��У�����Կ��Ǻϲ�����
    */
    void unite_frame_sendlist();


    //�����ʹ���
    int process_send_error(Zerg_Buffer *tmpbuf, bool frame_encode);

public:
    //��ʼ����̬����
    static int init_all_static_data();

    ///��ȡ�����ļ�
    static int get_config(const Zerg_Server_Config *config);

    //ע����̬����
    static int uninit_all_staticdata();


    //�õ�����
    static void get_max_peer_num(size_t &maxaccept, size_t &maxconnect);

    //�ر�svr_info��Ӧ��PEER
    static int close_services_peer(const SERVICES_ID &svr_info);

    //�����е�SVR ID����ѯ��Ӧ��HDL
    static int find_services_peer(const SERVICES_ID &svc_id, TCP_Svc_Handler *&svchanle);

    ///�������е�Ҫ�Զ����ӵķ�����,����±�������������ӶϿں�
    static void reconnect_allserver();


    /*!
    * @brief      �ӳ�������õ�һ��Handler�����ʹ��
    * @return     TCP_Svc_Handler* ���صķ���ľ��
    * @param      handler_mode     ����ľ����ģʽ����accept ����connect��
    & @note       Connect�Ķ˿�Ӧ����Զ������ȡ����Hanler������
    */
    static TCP_Svc_Handler *alloce_hdl_from_pool(HANDLER_MODE handler_mode);

    ///Dump���е�STATIC��������Ϣ
    static void dump_status_staticinfo(ZCE_LOG_PRIORITY out_lvl);

    ///Dump ���е�PEER��Ϣ
    static void dump_svcpeer_info(ZCE_LOG_PRIORITY out_lvl);

    ///������һ������
    static int process_send_data(Zerg_Buffer *tmpbuf);

    ///����services_type��ѯ��Ӧ�����������������б����� MS��������,
    ///��ο� @ref Zerg_Auto_Connector
    static int find_conf_ms_svcid_ary(uint16_t services_type,
                                      std::vector<uint32_t> *&ms_svcid_ary);
protected:

    //��ʱ��ID,����New����,����,����������뷨,ACE timer_timeoutΪʲô��ֱ��ʹ��TIMEID
    static const  int         TCPCTRL_TIME_ID[];


    //ACCEPT PEER�����Եȴ����͵�FRAME����
    static const size_t       MAX_OF_ACCEPT_PEER_SEND_DEQUE = 32;
    //CONNECT PEER�����Եȴ����͵�FRAME����,
    static const size_t       MAX_OF_CONNECT_PEER_SEND_DEQUE = 256;

    //���⴦������Ĳ��ü��ܴ�������������
    static const size_t       MAX_OF_SPEC_NO_ENCRYPT_CMD = 32;

    //Ĭ�Ͼ������ĳ�ʱʱ��
    static const unsigned int DEFAULT_TIME_OUT_SEC = 8 * 60;

    // ��ʱ�ϱ�ͳ�����ݵ�ʱ��, ÿ30���ϱ�һ�����ݣ�
    // ������ö̵���Ա�������5����ͳ�Ƶ�������ʵ�����ݵ����̫��
    static const unsigned int STAT_TIMER_INTERVAL_SEC = 60;

    //SessionKey
    static const size_t   MAX_SESSION_KEY_LEN = 32;




protected:

    ///ͨѶ������,������Ϊ�˼ӿ��ٶ�
    static  Zerg_Comm_Manager *zerg_comm_mgr_;

    ///�洢����,ȫ��Ψһ,������Ϊ�˼ӿ��ٶ�
    static ZBuffer_Storage *zbuffer_storage_;

    ///ͳ�ƣ�ʹ�õ������ָ��
    static Soar_Stat_Monitor *server_status_;

    ///����ܹ�Accept��PEER����,
    static size_t max_accept_svr_;
    ///����ܹ�Connect��PEER����
    static size_t max_connect_svr_;

    ///�����澯��ֵ,
    static size_t accpet_threshold_warn_;
    ///�Ѿ������澯��ֵ�Ĵ���
    static size_t threshold_warn_number_;

    ///����ʾ�Ǵ��������
    static bool if_proxy_;

    ///ACCEPT�˿ڵĺ�ȴ�������ʱ��,���֮��һ��ʱ��û�ж�������Ϊ������KO
    static unsigned int accepted_timeout_;

    ///�ȴ�����һ���������ݵĳ�ʱʱ��,Ϊ0��ʾ������,��Ϊ������KO
    static unsigned int receive_timeout_;



    ///Ҫ�Զ����ӵķ�����
    static Zerg_Auto_Connector zerg_auto_connect_;

    ///SVRINFO��Ӧ��PEER��HASHMAP
    static Active_SvcHandle_Set svr_peer_info_set_;


    ///�Ѿ�Accept��PEER����
    static size_t num_accept_peer_;
    ///�Ѿ�Connect��PEER����
    static size_t num_connect_peer_;


    ///ACCEPT SVC handler�ĳ���
    static POOL_OF_TCP_HANDLER  pool_of_acpthdl_;

    ///CONNECT svc handler�ĳ���
    static POOL_OF_TCP_HANDLER  pool_of_cnthdl_;


    ///���ͻ����������frame��
    static size_t accept_send_deque_size_;

    ///�������ӵķ��Ͷ��г���
    static size_t connect_send_deque_size_;

    ///Handle ID�����������������Ҫ������Ҫ��������������SERVICES ID�ĵط�
    static unsigned int         handler_id_builder_;

protected:
    ///����ģʽ
    HANDLER_MODE               handler_mode_;

    ///�Լ��ķ���ı�ʾ
    SERVICES_ID                my_svc_id_;

    ///PEER��ServiceInfo
    SERVICES_ID                peer_svr_id_;


    ///�������ݵĻ���
    Zerg_Buffer               *rcv_buffer_;

    ///���Ͷ��еĴ�С�����һ���˿ڽ������ݱȽϻ���������ܻ��ȷ��뷢�Ͷ��У��ȶ˿ڱ�Ϊ��д���ܷ��͹�ȥ��
    ///��ô���Ͷ��о�Ҫ������������Σ�������񣬷����ܻ��峤��ʵ�ʵ��� = ���Ͷ��еĳ���*ÿ�����г�ԱBUFFER�Ĵ�С(64K)��
    ///�����һ����Ҫ�ܶ�������ģ��������������Ŀͻ��˷�������ACPT_SEND_DEQUE_SIZE ��Ҫ���ù��󣬽���32��
    ///����������������������ޣ���ô���ó�128, 256Ҳ�ǿ��Խ��ܵģ�����ʵ�������д��۲�

    ///���͵����ݿ���Ҫ�Ŷ�
    zce::lordrings<Zerg_Buffer *>  snd_buffer_deque_;



    ///������4���ֶ���ʵ�Ǽ�¼һ��ʱ����ڵĽ��ܺͷ��͵���������
    ///���յĴ���������
    size_t                    recieve_counter_;
    ///���͵Ĵ���������
    size_t                    send_counter_;

    ///���PEER��������
    size_t                    recieve_bytes_;
    ///���PEER��������
    size_t                    send_bytes_;


    //ZCE Socket Stream,
    ZCE_Socket_Stream         socket_peer_;

    //PEER���ӵ�IP��ַ��Ϣ
    ZCE_Sockaddr_In           peer_address_;

    //�Ƿ��ڻ״̬
    PEER_STATUS               peer_status_;

    //���Ӻ��޷�Ӧ��ʱ��TimeID,
    int                       timeout_time_id_;
    //һ��ʱ�����ڽ������ݵĴ���
    unsigned int              receive_times_;

    //�Ƿ��Ǻ�˷�����ǿ�ƹر����PEER
    bool                      if_force_close_;

    //��ʼ���¼�����ʵ�ǵ�һ�ν��붨ʱ�����¼�,
    time_t                    start_live_time_;

    ///��¼my_svc_id_�е�services_id_��Active_SvcHandle_Set�е�
    ///type_to_idtable_�������±�ID
    size_t                    tptoid_table_id_;

};




#endif //_ZERG_TCP_CONTROL_SERVICE_H_


