

#ifndef OGRE_TCP_CONTROL_SERVICE_H_
#define OGRE_TCP_CONTROL_SERVICE_H_

#include "ogre_svrpeer_info_set.h"
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

    //服务的模式
    enum OGRE_HANDLER_MODE
    {
        HANDLER_MODE_CONNECT,          //主动连接的Handler
        HANDLER_MODE_ACCEPTED,         //被动接受某个端口的Handler
    };

protected:
    typedef ZCE_LIB::lordrings<Ogre_TCP_Svc_Handler *> POOL_OF_TCP_HANDLER;

protected:

    //
    enum PEER_STATUS
    {
        PEER_STATUS_NOACTIVE,        //PEER 没有连接上
        PEER_STATUS_JUST_CONNECT,    //PEER 刚刚连接上,但是没有发送或者受到任何数据
        PEER_STATUS_ACTIVE,          //PEER 已经处于激活状态,
    };

public:

    //构造函数
    explicit Ogre_TCP_Svc_Handler(Ogre_TCP_Svc_Handler::OGRE_HANDLER_MODE hdl_mode);
    //为了让你无法在堆以外使用Ogre_TCP_Svc_Handler
protected:
    virtual ~Ogre_TCP_Svc_Handler();

public:

    //初始化对象，
    void init_tcp_svc_handler(const ZCE_Socket_Stream &sockstream,
        FP_JudgeRecv_WholeFrame fp_judge_whole);
    //
    void init_tcp_svc_handler(const ZCE_Socket_Stream &sockstream, 
        const ZCE_Sockaddr_In &socketaddr,
        FP_JudgeRecv_WholeFrame fp_judge_whole);

    //ZEN的一组要求自己继承的函数.
    //
    virtual ZCE_HANDLE get_handle(void) const;
    //
    virtual int handle_input(ZCE_HANDLE);
    //
    virtual int handle_output(ZCE_HANDLE);
    //
    virtual int timer_timeout(const ZCE_Time_Value &time, const void *arg);
    //
    virtual int handle_close ();

    //得到Handle对应PEER的端口
    unsigned short get_peer_port();
    //得到Handle对应PEER的IP地址
    const char *get_peer_address();

protected:

    //从PEER读取数据
    int read_data_from_peer(size_t &szrevc);
    //将数据写入PEER
    int write_data_to_peer(size_t &szsend, bool &bfull);
    //将所有可以发送的数据都发送出去
    int write_all_aata_to_peer();

    //预处理,检查数据,接收的REGISTER数据,根据第一个报决定对应关系
    int  preprocess_recv_frame();

    //处理发送的REGISTER数据,连接后发送第一个数据
    int  process_connect_register();

    //处理发送错误
    int process_senderror(Ogre4a_App_Frame *tmpbuf);

    //得到一个PEER的状态
    PEER_STATUS  get_peer_status();

    //将数据帧放入管道,参数为要放入的数据长度
    int push_frame_to_recvpipe(unsigned int sz_data);

    //将一个发送的帧放入等待发送队列
    int put_frame_to_sendlist(Ogre4a_App_Frame *ogre_frame);

    //合并发送的帧数据
    void unite_frame_sendlist();

public:

    ///读取配置文件
    static int get_config(const Ogre_Server_Config *config);

    ///初始化静态参数
    static int init_all_static_data();
    
    ///注销静态参数
    static int unInit_all_static_data();

    //从池子分配一个Handler
    static Ogre_TCP_Svc_Handler *alloc_svchandler_from_pool(OGRE_HANDLER_MODE handler_mode);

    //取得配置的最大PEER数量
    static void get_maxpeer_num(size_t &maxaccept, size_t &maxconnect);

    //将数据从端口发送数据
    static int process_send_data(Ogre4a_App_Frame *tmpbuf );



protected:


    //定时器ID,避免New传递,回收,我讨厌这个想法,ACE timer_timeout为什么不直接使用TIMEID
    static const  int      TCPCTRL_TIME_ID[];

    //一个未能连接上的Connect 最大可以等待发送的FRAME数量
    static const size_t   MAX_LEN_OF_SEND_LIST = 8;

    //ACCEPT PEER最大可以等待发送的FRAME数量
    static const size_t   MAX_OF_ACCEPT_PEER_SEND_DEQUE = 64;
    //CONNECT PEER最大可以等待发送的FRAME数量,
    static const size_t   MAX_OF_CONNECT_PEER_SEND_DEQUE = 256;

protected:

    //最大能够Accept的PEER数量,
    static size_t                  max_accept_svr_;
    //最大能够Connect的PEER数量
    static size_t                  max_connect_svr_;

    //Connect后等待动作的时长,Connect超时
    static unsigned int           connect_timeout_;

    //等待接受一个完整数据的超时时间,为0表示不限制
    static unsigned int           receive_timeout_;

    //对于错误的数据,尝试发送的次数,只是了保证一定的网络瞬断
    static unsigned int           error_try_num_;

    //SVRINFO对应的PEER的HASHMAP
    static PeerInfoSetToTCPHdlMap svr_peer_info_set_;
    //
    //要自动链接的服务器
    static Ogre_Connect_Server    zerg_auto_connect_;

    //已经Accept的PEER数量
    static size_t                 num_accept_peer_;
    //已经Connect的PEER数量
    static size_t                 num_connect_peer_;

    //ACCEPT SVC handler的池子
    static POOL_OF_TCP_HANDLER    pool_of_acpthdl_;

    //CONNECT svc handler的池子
    static POOL_OF_TCP_HANDLER    pool_of_cnthdl_;

protected:
    //服务模式
    OGRE_HANDLER_MODE             handler_mode_;

    //接收数据的缓冲
    Ogre4a_App_Frame              *rcv_buffer_;

    //发送的数据可能要排队
    ZCE_LIB::lordrings<Ogre4a_App_Frame *>  \
        snd_buffer_deque_;

    //这个PEER接受数据
    size_t                        recieve_bytes_;
    //这个PEER发送数据
    size_t                        send_bytes_;

    //ACE Socket Stream,
    ZCE_Socket_Stream             socket_peer_;

    //PEER连接的远端IP地址信息,有他不过是为了加快速度.
    ZCE_Sockaddr_In               remote_address_;
    //PEER连接的本地IP地址信息,有他不过是为了加快速度.
    ZCE_Sockaddr_In               local_address_;

    //是否处于活动状态
    PEER_STATUS                   peer_status_;

    //连接后无反应超时的TimeID,
    long                          timeout_time_id_;
    //一个时间间隔内接受数据的次数
    unsigned int                  receive_times_;

    //是否被强制要求在发送完成后关闭端口
    bool                          if_force_close_;

    //判断接收到的数据是否是完整的处理函数指针
    FP_JudgeRecv_WholeFrame       fp_judge_whole_frame_;

};

#endif //_OGRE_TCP_CONTROL_SERVICE_H_

