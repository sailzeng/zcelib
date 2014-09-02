

#ifndef ZERG_TCP_CONTROL_SERVICE_H_
#define ZERG_TCP_CONTROL_SERVICE_H_

#include "zerg_buf_storage.h"
#include "zerg_auto_connect.h"
#include "zerg_service_info_set.h"

class Zerg_Buffer;
class mmap_dequechunk;
class Zerg_Auto_Connector;
class Zerg_Comm_Manager;
class TcpHandlerImpl;

/****************************************************************************************************
class  TCP_Svc_Handler
****************************************************************************************************/
class TCP_Svc_Handler : public ZCE_Event_Handler,
    public ZCE_Timer_Handler
{
public:
    //服务的模式
    enum HANDLER_MODE
    {
        HANDLER_MODE_INVALID,          //无效的链接
        HANDLER_MODE_CONNECT,          //主动连接的Handler
        HANDLER_MODE_CONNECT_EXTERNAL, //主动连接的Handler,使用外部协议
        HANDLER_MODE_ACCEPTED,         //被动接受某个端口的Handler
        HANDLER_MODE_ACCEPTED_EXTERNAL,//被动接收某个端口的handler，使用非框架协议
    };

    //
    enum PEER_STATUS
    {
        PEER_STATUS_NOACTIVE,        //PEER 没有连接上
        PEER_STATUS_JUST_ACCEPT,     //PEER 刚刚ACCEPT上,但是没有发送或者受到任何数据
        PEER_STATUS_JUST_CONNECT,    //PEER 刚刚CONNECT上,但是没有收到任何数据
        PEER_STATUS_ACTIVE,          //PEER 已经处于激活状态,
    };

protected:

    typedef ZCE_LIB::lordrings<TCP_Svc_Handler *> POOL_OF_TCP_HANDLER;



    //为了让你无法在堆以外使用TCP_Svc_Handler
protected:
    //
    TCP_Svc_Handler(HANDLER_MODE hdl_mode);
    //
    virtual ~TCP_Svc_Handler();

public:

    inline static Zerg_Auto_Connector& get_zerg_auto_connect()
    {
        return zerg_auto_connect_;
    }

    //
    void init_tcpsvr_handler(const SERVICES_ID &my_svcinfo,
                             const ZCE_Socket_Stream &sockstream,
                             const ZCE_Sockaddr_In     &socketaddr,
                             bool sessionkey_verify);

    //主动CONNET链接出去的HANDLER
    void init_tcpsvr_handler(const SERVICES_ID &my_svcinfo,
                             const SERVICES_ID &svrinfo,
                             const ZCE_Socket_Stream &sockstream,
                             const ZCE_Sockaddr_In     &socketaddr);

    // 设置外部协议处理类
    void change_impl(TcpHandlerImpl *impl);

    //ZEN的一组要求自己继承的函数.
    //ZCE_Event_Handler必须重载的函数，取得SOCKET句柄
    virtual ZCE_HANDLE get_handle(void) const;

    //读事件触发
    virtual int handle_input();

    //写事件触发
    virtual int handle_output();

    //超时事件触发
    virtual int handle_timeout(const ZCE_Time_Value &time, const void *arg);

    //关闭事件触发
    virtual int handle_close ();

    //得到Handle对应PEER的端口
    unsigned short get_peer_port();

    //得到Handle对应PEER的IP地址
    const char *get_peer_address();

    //得到连接模式
    HANDLER_MODE get_handle_mode()
    {
        return handler_mode_;
    }

    //得到每个PEER状态信息
    void dump_status_info(std::ostringstream &ostr_stream );

    //发送简单的ZERG命令给对方
    int send_simple_zerg_cmd(unsigned int cmd,
                             const SERVICES_ID &recv_services_info,
                             unsigned int option = 0);

    //发送心跳
    int send_zerg_heart_beat_reg();

    //得到一个PEER的状态
    PEER_STATUS  get_peer_status();

    //获得一个整型的handle
    unsigned int get_handle_id();

    const ZCE_Sockaddr_In &get_peer_sockaddr() const;

    bool if_socket_block()
    {
        return if_socket_block_;
    }

protected:

    //从PEER读取数据
    int read_data_from_peer(size_t &szrevc);

    //检查收到的数据是否含有一个完整的数据包.
    int check_recv_full_frame(bool &bfull, unsigned int &whole_frame_len);

    //将数据写入PEER
    int write_data_to_peer(size_t &szsend, bool &bfull);
    //将数据写入PEER，同时处理周边的事情，包括写事件注册等
    int write_all_data_to_peer();

    //预处理,检查数据,接收的REGISTER数据,根据第一个报决定对应关系
    int  preprocess_recvframe(Zerg_App_Frame *proc_frame);

    //处理发送的REGISTER数据,连接后发送第一个数据
    int  process_connect_register();

    //将数据帧放入管道
    int push_frame_to_comm_mgr();

    //将一个发送的帧放入等待发送队列
    int put_frame_to_sendlist(Zerg_Buffer *tmpbuf);
    //合并发送的数据
    void unite_frame_sendlist();

public:
    //初始化静态参数
    static int init_all_static_data();

    // 调整连接池
    static int adjust_svc_handler_pool();

    //读取配置文件
    static int get_config(Zerg_Server_Config *config);

    //reload主动连接
    static int reload_config(Zerg_Server_Config *config);


    //注销静态参数
    static int uninit_all_staticdata();

    //从SEND管道找到所有的数据去发送,
    static int popall_sendpipe_write(size_t &procframe);

    //得到最大的
    static void get_max_peer_num(size_t &maxaccept, size_t &maxconnect);

    //关闭svr_info相应的PEER
    static int close_connect_services_peer(const SERVICES_ID &svr_info);

    //根据有的SVR INFO，查询相应的HDL
    static int find_connect_services_peer(const SERVICES_ID &svr_info, TCP_Svc_Handler *&svchanle);

    //链接所有的要自动链接的服务器,这个事避免服务器的链接断口后
    static void auto_connect_allserver(size_t &szvalid, size_t &szsucc, size_t &szfail);



    //从池子里面得到一个Handler给大家使用
    static TCP_Svc_Handler *AllocSvcHandlerFromPool(HANDLER_MODE handler_mode);

    //Dump所有的STATIC变量的信息
    static void dump_status_staticinfo(std::ostringstream &ostr_stream );

    //Dump 所有的PEER信息
    static void dump_svcpeer_info(std::ostringstream &ostr_stream, size_t startno, size_t numquery);

protected:

    //处理发送错误
    int process_send_error(Zerg_Buffer *tmpbuf, bool frame_encode);

public:
    //处理发送一个数据
    static int process_send_data(Zerg_Buffer *tmpbuf );


protected:

    //定时器ID,避免New传递,回收,我讨厌这个想法,ACE handle_timeout为什么不直接使用TIMEID
    static const  int         TCPCTRL_TIME_ID[];

    //ACCEPT PEER最大可以等待发送的FRAME数量
    static const size_t       MAX_OF_ACCEPT_PEER_SEND_DEQUE = 32;
    //CONNECT PEER最大可以等待发送的FRAME数量,
    static const size_t       MAX_OF_CONNECT_PEER_SEND_DEQUE = 256;

    //特殊处理的最大的不用加密处理的命令的数量
    static const size_t       MAX_OF_SPEC_NO_ENCRYPT_CMD = 32;

    //默认句柄处理的超时时间
    static const unsigned int DEFAULT_TIME_OUT_SEC = 8 * 60;

    // 定时上报统计数据的时间, 每30秒上报一次数据，
    // 间隔设置短点可以避免两个5分钟统计的数据与实际数据的误差太大
    static const unsigned int STAT_TIMER_INTERVAL_SEC = 30;

    //SessionKey
    static const size_t   MAX_SESSION_KEY_LEN = 32;

    //主动连接预留大小
    static const size_t CONNECT_HANDLE_RESERVER_NUM = 16;

protected:

    //通讯管理器,保存是为了加快速度
    static  Zerg_Comm_Manager   *zerg_comm_mgr_;

    //存储缓存,全局唯一,保存是为了加快速度
    static ZBuffer_Storage     *zbuffer_storage_;

    //统计，使用单子类的指针
    static Comm_Stat_Monitor    *server_status_;

    //最大能够Accept的PEER数量,
    static size_t              max_accept_svr_;
    //最大能够Connect的PEER数量
    static size_t              max_connect_svr_;

    //容量告警阈值,
    static size_t              accpet_threshold_warn_;
    //已经超过告警阈值的次数
    static size_t              threshold_warn_number_;

    //服务示是代理服务器
    static bool                if_proxy_;

    //允许的最大的帧的长度
    static unsigned int        max_frame_len_;

    //Connect后等待动作的时长,Connect超时
    static unsigned int        connect_timeout_;

    //等待接受一个完整数据的超时时间,为0表示不限制
    static unsigned int        receive_timeout_;

    //要自动链接的服务器
    static Zerg_Auto_Connector    zerg_auto_connect_;

    //SVRINFO对应的PEER的HASHMAP
    static Service_Info_Set svr_peer_info_set_;

    //已经Accept的PEER数量
    static size_t              num_accept_peer_;
    //已经Connect的PEER数量
    static size_t              num_connect_peer_;

    //是否检查帧的数据
    static bool                if_check_frame_;

    // 是否允许多开连接
    static bool                multi_con_flag_;

    //ACCEPT SVC handler的池子
    static POOL_OF_TCP_HANDLER pool_of_acpthdl_;

    //CONNECT svc handler的池子
    static POOL_OF_TCP_HANDLER pool_of_cnthdl_;

    //external CONNECT svc handler的池子
    static POOL_OF_TCP_HANDLER pool_of_external_cnthdl_;

    //发送缓冲区的最大frame数
    static size_t              snd_buf_size_;

    //主动连接的发送队列长度
    static size_t              connect_send_deque_size_;

    //
    static unsigned int        handler_id_builder_;

    // gameid
    static unsigned int        game_id_;

protected:
    //服务模式
    HANDLER_MODE               handler_mode_;

    //自己的服务的标示
    SERVICES_ID                my_svc_info_;

    //PEER的ServiceInfo
    SERVICES_ID                peer_svr_info_;

    //接收数据的缓冲
    Zerg_Buffer               *rcv_buffer_;

    //发送的数据可能要排队
    ZCE_LIB::lordrings<Zerg_Buffer *>  snd_buffer_deque_;

    // 内外协议不同处理逻辑实现类
    TcpHandlerImpl *impl_;

    //-------------------------------------------------------------------------------------
    //下面这4个字段其实是记录一个时间段内的接受和发送的数据总数
    //接收的次数计数器
    size_t                    recieve_counter_;
    //发送的次数计数器
    size_t                    send_counter_;

    //这个PEER接受数据
    size_t                    recieve_bytes_;
    //这个PEER发送数据
    size_t                    send_bytes_;
    //---------------------------------------------------------------------------------------

    //ACE Socket Stream,
    ZCE_Socket_Stream         socket_peer_;

    //PEER连接的IP地址信息
    ZCE_Sockaddr_In           peer_address_;

    //是否处于活动状态
    PEER_STATUS               peer_status_;

    //连接后无反应超时的TimeID,
    int                       timeout_time_id_;
    //一个时间间隔内接受数据的次数
    unsigned int              receive_times_;

    //是否验证用户的sessionkey_verify
    bool                      sessionkey_verify_;

    //是否是后端服务器强制关闭这个PEER
    bool                      if_force_close_;

    //开始的事件，其实是第一次进入定时器的事件,
    time_t                    start_live_time_;

    //连接是否有阻塞
    bool                      if_socket_block_;
};

#endif //_ZERG_TCP_CONTROL_SERVICE_H_

