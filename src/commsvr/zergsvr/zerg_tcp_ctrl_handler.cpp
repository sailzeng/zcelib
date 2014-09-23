
#include "zerg_predefine.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_comm_manager.h"
#include "zerg_app_timer.h"
#include "zerg_stat_define.h"

/****************************************************************************************************
class  TCP_Svc_Handler
****************************************************************************************************/
//CONNECT后等待数据的超时时间
unsigned int   TCP_Svc_Handler::accepted_timeout_ = 3;
//接受数据的超时时间
unsigned int   TCP_Svc_Handler::receive_timeout_ = 5;

//TIME ID
const int      TCP_Svc_Handler::TCPCTRL_TIME_ID[] = { 1, 2 };


//
Active_SvcHandle_Set TCP_Svc_Handler::svr_peer_info_set_;



//这儿没有使用Singlton的方式的原因如下：
//1.速度第一的思想
//2.原来没有用instance

//
ZBuffer_Storage  *TCP_Svc_Handler::zbuffer_storage_ = NULL;
//通信管理器
Zerg_Comm_Manager *TCP_Svc_Handler::zerg_comm_mgr_ = NULL;
//
Comm_Stat_Monitor *TCP_Svc_Handler::server_status_ = NULL;

//自己是否是代理
bool           TCP_Svc_Handler::if_proxy_ = false;

//
size_t         TCP_Svc_Handler::num_accept_peer_ = 0;
//
size_t         TCP_Svc_Handler::num_connect_peer_ = 0;


//最大可以接受的接受数量
size_t         TCP_Svc_Handler::max_accept_svr_ = 0;
//最大可以接受的连接数量
size_t         TCP_Svc_Handler::max_connect_svr_ = 0;

//容量告警阈值
size_t         TCP_Svc_Handler::accpet_threshold_warn_ = 0;
//已经超过告警阈值的次数
size_t         TCP_Svc_Handler::threshold_warn_number_ = 0;

//
Zerg_Auto_Connector TCP_Svc_Handler::zerg_auto_connect_;


//svc handler的池子
TCP_Svc_Handler::POOL_OF_TCP_HANDLER TCP_Svc_Handler::pool_of_acpthdl_;
//svc handler的池子
TCP_Svc_Handler::POOL_OF_TCP_HANDLER TCP_Svc_Handler::pool_of_cnthdl_;

//发送缓冲区的最大frame数，从配置读取
size_t         TCP_Svc_Handler::accept_send_deque_size_ = 0;

//主动连接的发送队列长度
size_t  TCP_Svc_Handler::connect_send_deque_size_ = 0;

unsigned int  TCP_Svc_Handler::handler_id_builder_ = 0;


//构造函数
TCP_Svc_Handler::TCP_Svc_Handler(TCP_Svc_Handler::HANDLER_MODE hdl_mode) :
    ZCE_Event_Handler(ZCE_Reactor::instance()),
    ZCE_Timer_Handler(ZCE_Timer_Queue::instance()),
    handler_mode_(hdl_mode),
    my_svc_id_(0, 0),
    peer_svr_id_(0, 0),
    rcv_buffer_(NULL),
    recieve_counter_(0),
    send_counter_(0),
    recieve_bytes_(0),
    send_bytes_(0),
    peer_status_(PEER_STATUS_NOACTIVE),
    timeout_time_id_(-1),
    receive_times_(0),
    if_force_close_(false),
    start_live_time_(0),
    tptoid_table_id_(size_t(-1))
{

    if (HANDLER_MODE_CONNECT == hdl_mode)
    {
        snd_buffer_deque_.initialize(connect_send_deque_size_);
    }
    else if (HANDLER_MODE_ACCEPTED == hdl_mode)
    {
        snd_buffer_deque_.initialize(accept_send_deque_size_);
    }
    else
    {
        ZCE_ASSERT(false);
    }
}


//用于Accept的端口的处理Event Handle初始化处理.
void TCP_Svc_Handler::init_tcpsvr_handler(const SERVICES_ID &my_svcinfo,
                                          const ZCE_Socket_Stream &sockstream,
                                          const ZCE_Sockaddr_In     &socketaddr)
{
    handler_mode_ = HANDLER_MODE_ACCEPTED;
    my_svc_id_ = my_svcinfo;
    peer_svr_id_.set_svcid(0, 0);
    rcv_buffer_ = NULL;
    recieve_counter_ = 0;
    send_counter_ = 0;
    recieve_bytes_ = 0;
    send_bytes_ = 0;
    socket_peer_ = sockstream;
    peer_address_ = socketaddr;
    peer_status_ = PEER_STATUS_JUST_ACCEPT;
    timeout_time_id_ = -1;
    receive_times_ = 0;
    if_force_close_ = false;
    start_live_time_ = 0;
    tptoid_table_id_ = size_t(-1);

    ////调整Socket 为O_NONBLOCK
    int ret = socket_peer_.sock_enable(O_NONBLOCK);


    ZCE_LOGMSG(RS_INFO, "[zergsvr] Accept peer socket IP Address:[%s|%u] Success. Set O_NONBLOCK ret =%d.",
               peer_address_.get_host_addr(),
               peer_address_.get_port_number(),
               ret);

    //先增加计数器。避免等下关闭的时候又--了。
    ++num_accept_peer_;

    //检查最大链接数量,REACTOR自己其实有控制,但是我又要控制ACCEPT又要控制CONNECT.
    //所以只好,头XX大, 另外这个事情其实从理论上来讲,几乎不会发生,
    if (num_accept_peer_ <= max_accept_svr_)
    {
        //继续检查是否有告警阈值
        if (num_accept_peer_ > accpet_threshold_warn_)
        {
            const size_t WARNNING_TIMES = 5;

            if ((threshold_warn_number_ % (WARNNING_TIMES)) == 0)
            {
                ZLOG_ALERT("[zergsvr] Great than threshold_warn_number_ Reject! num_accept_peer_:%u,threshold_warn_number_:%u,accpet_threshold_warn_:%u,max_accept_svr_:%u .",
                           num_accept_peer_,
                           threshold_warn_number_,
                           accpet_threshold_warn_,
                           max_accept_svr_);
            }

            //记录告警总次数的增加
            ++threshold_warn_number_;
        }

        //注册读写事件
        ret = reactor()->register_handler(this,
                                          ZCE_Event_Handler::READ_MASK | ZCE_Event_Handler::WRITE_MASK);

        //
        if (ret != 0)
        {
            ZCE_LOGMSG(RS_ERROR, "[zergsvr] Register accept [%s|%u] handler fail! ret =%u  errno=%u|%s .",
                       peer_address_.get_host_addr(),
                       peer_address_.get_port_number(),
                       ret,
                       ZCE_LIB::last_error(),
                       strerror(ZCE_LIB::last_error()));

            handle_close();
            return;
        }

        reactor()->cancel_wakeup(this, ZCE_Event_Handler::WRITE_MASK);

        //统计
        server_status_->set_by_statid(ZERG_ACCEPT_PEER_NUMBER, 0, 0, static_cast<int>(num_accept_peer_));
        server_status_->increase_by_statid(ZERG_ACCEPT_PEER_COUNTER, 0, 0, 1);
    }
    //要测试检查一下,
    else
    {
        ZCE_LOGMSG(RS_ERROR, "[zergsvr] Peer [%s|%u] great than max_accept_svr_ Reject! num_accept_peer_:%u,max_accept_svr_:%u .",
                   peer_address_.get_host_addr(),
                   peer_address_.get_port_number(),
                   num_accept_peer_,
                   max_accept_svr_);
        handle_close();
        return;
    }

    //如果配置了超时出来,N秒必须收到一个包

    ZCE_Time_Value delay(0, 0);
    ZCE_Time_Value interval(0, 0);

    //
    (accepted_timeout_ > 0) ? delay.sec(accepted_timeout_) : delay.sec(STAT_TIMER_INTERVAL_SEC);
    (receive_timeout_ > 0) ? interval.sec(receive_timeout_) : interval.sec(STAT_TIMER_INTERVAL_SEC);

    timeout_time_id_ = timer_queue()->schedule_timer(this, &TCPCTRL_TIME_ID[0], delay, interval);


    //保活
    int keep_alive = 1;
    socklen_t opvallen = sizeof(int);
    socket_peer_.setsockopt(SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<void *>(&keep_alive), opvallen);

    //这儿地方导致过一个BUG，就是客户端收不到数据，所以这个选项还不适合在这个服务器使用。呵呵。
    //这个选项是保证主动关闭的时候，不用等待将数据发送给对方,
    //这段东西开过两次，但也关闭了两次，我还是估计有一些细节我没有弄明白。
    //struct linger sock_linger = {1, 30};
    //sock_linger.l_onoff = 1;
    //sock_linger.l_linger = 30;
    //opvallen = sizeof(linger);
    //socket_peer_.set_option(SOL_SOCKET,SO_LINGER,reinterpret_cast<void *>(&sock_linger),opvallen);

#if defined _DEBUG || defined DEBUG
    socklen_t sndbuflen, rcvbuflen;
    opvallen = sizeof(socklen_t);
    socket_peer_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&rcvbuflen), &opvallen);
    socket_peer_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&sndbuflen), &opvallen);
    ZCE_LOGMSG(RS_DEBUG, "[zergsvr] Accept peer SO_RCVBUF:%u SO_SNDBUF %u.", rcvbuflen, sndbuflen);

#endif
}



//主动CONNET链接出去的HANDLER，对应Event Handle的初始化.
void TCP_Svc_Handler::init_tcpsvr_handler(const SERVICES_ID &my_svcinfo,
                                          const SERVICES_ID &peer_svrinfo,
                                          const ZCE_Socket_Stream &sockstream,
                                          const ZCE_Sockaddr_In     &socketaddr)
{
    handler_mode_ = HANDLER_MODE_CONNECT;
    my_svc_id_ = my_svcinfo;
    peer_svr_id_ = peer_svrinfo;
    rcv_buffer_ = NULL;
    recieve_counter_ = 0;
    send_counter_ = 0;
    recieve_bytes_ = 0;
    send_bytes_ = 0;
    socket_peer_ = sockstream;
    peer_address_ = socketaddr;
    peer_status_ = PEER_STATUS_NOACTIVE;
    timeout_time_id_ = -1;
    receive_times_ = 0;
    if_force_close_ = false;
    start_live_time_ = 0;
    tptoid_table_id_ = size_t(-1);

    //调整Socket 为ACE_NONBLOCK
    int ret = socket_peer_.sock_enable(O_NONBLOCK);

    ZCE_LOGMSG(RS_INFO, "[zergsvr] Connect peer socket Services ID[%u|%u] IP Address:[%s|%u] Success. Set O_NONBLOCK ret =%d.",
               peer_svr_id_.services_type_,
               peer_svr_id_.services_id_,
               peer_address_.get_host_addr(),
               peer_address_.get_port_number(),
               ret);

    snd_buffer_deque_.initialize(connect_send_deque_size_);

    //注册到
    ret = reactor()->register_handler(this, ZCE_Event_Handler::CONNECT_MASK);

    //我几乎没有见过register_handler失败,
    if (ret != 0)
    {
        ZCE_LOGMSG(RS_ERROR, "[zergsvr] Register services [%u|%u] IP[%s|%u]  connect handler fail! ret =%d  errno=%d|%s .",
                   peer_svr_id_.services_type_,
                   peer_svr_id_.services_id_,
                   peer_address_.get_host_addr(),
                   peer_address_.get_port_number(),
                   ret,
                   ZCE_LIB::last_error(),
                   strerror(ZCE_LIB::last_error()));
        handle_close();
        return;
    }

    //放入连接处理的MAP
    ret = svr_peer_info_set_.add_services_peerinfo(peer_svr_id_, this);

    //在这儿自杀是不是危险了一点
    if (ret != 0)
    {
        handle_close();
        return;
    }

    ++num_connect_peer_;

    ZCE_Time_Value delay(STAT_TIMER_INTERVAL_SEC, 0);
    ZCE_Time_Value interval(STAT_TIMER_INTERVAL_SEC, 0);

    timeout_time_id_ = timer_queue()->schedule_timer(this, &TCPCTRL_TIME_ID[0], delay, interval);

    //统计
    server_status_->set_by_statid(ZERG_CONNECT_PEER_NUMBER, 0, 0, num_connect_peer_);
    server_status_->increase_by_statid(ZERG_CONNECT_PEER_COUNTER, 0, 0, 1);

    //SO_RCVBUF，SO_SNDBUF，按照UNPv1的讲解，应该在connect之前设置，虽然我的测试证明，放在这儿设置也可以。

    int keep_alive = 1;
    socklen_t opvallen = sizeof(int);
    socket_peer_.setsockopt(SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<void *>(&keep_alive), opvallen);

    //Win32下没有这个选项
#ifndef ZCE_OS_WINDOWS
    //避免DELAY发送这种情况
    int NODELAY = 1;
    opvallen = sizeof(int);
    socket_peer_.setsockopt(SOL_TCP, TCP_NODELAY, reinterpret_cast<void *>(&NODELAY), opvallen);
#endif

#if defined _DEBUG || defined DEBUG
    socklen_t sndbuflen = 0, rcvbuflen = 0;
    opvallen = sizeof(socklen_t);
    socket_peer_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&rcvbuflen), &opvallen);
    socket_peer_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&sndbuflen), &opvallen);
    ZCE_LOGMSG(RS_DEBUG, "[zergsvr] Set Connect Peer SO_RCVBUF:%u SO_SNDBUF %u.", rcvbuflen, sndbuflen);
#endif

}

TCP_Svc_Handler::~TCP_Svc_Handler()
{
}


//从配置文件读取配置信息
int TCP_Svc_Handler::get_config(const Zerg_Server_Config *config)
{
    int ret = 0;

    //unsigned int tmp_uint = 0 ;
    //从CONNECT到收到数据的时长
    accepted_timeout_ = config->zerg_cfg_data_.accepted_timeout_;


    //RECEIVE一个数据的超时时间,为0表示不限制
    receive_timeout_ = config->zerg_cfg_data_.receive_timeout_;


    //是否是一个代理服务,代理的发送行为和普通服务器不一样.
    if_proxy_ = config->zerg_cfg_data_.is_proxy_;


    //最大的链接我的服务器个数
    max_accept_svr_ = config->zerg_cfg_data_.max_accept_svr_;


    //容量告警阈值
    accpet_threshold_warn_ = static_cast<size_t> (max_accept_svr_ * 0.8);
    ZCE_LOGMSG(RS_INFO, "[zergsvr] Max accept svr number :%u,accept warn threshold number:%u. ",
               max_accept_svr_,
               accpet_threshold_warn_);


    //发送缓冲区的最大frame数
    accept_send_deque_size_ = config->zerg_cfg_data_.acpt_send_deque_size_;

    //主动连接的发送队列长度
    connect_send_deque_size_ = config->zerg_cfg_data_.cnnt_send_deque_size_;
    ZCE_LOGMSG(RS_INFO, "[zergsvr] conncet send deque size :%u ,accept send deque size :%u",
               connect_send_deque_size_, accept_send_deque_size_);

    //得到连接的SERVER的配置
    ret = zerg_auto_connect_.get_config(config);

    if (ret != 0)
    {
        return ret;
    }

    //
    return 0;
}



//将需要初始化的静态参数初始化
//一些参数从配置类读取,避免后面的操作还要访问配置类
int TCP_Svc_Handler::init_all_static_data()
{
    //
    //int ret = 0;
    //
    zerg_comm_mgr_ = Zerg_Comm_Manager::instance();
    //自己的服务的类型,服务编号,APPID
    //
    zbuffer_storage_ = ZBuffer_Storage::instance();

    //服务器的统计操作实例
    server_status_ = Comm_Stat_Monitor::instance();


    //最大要链接数量等于自动链接服务的数量,增加16个
    max_connect_svr_ = zerg_auto_connect_.numsvr_connect() + 16;

    ZCE_LOGMSG(RS_INFO, "[zergsvr] MaxAcceptSvr:%u MaxConnectSvr:%u.", max_accept_svr_, max_connect_svr_);

    //为CONNECT的HDL预先分配内存，成为一个池子
    ZCE_LOGMSG(RS_INFO, "[zergsvr] Connet Hanlder:size of TCP_Svc_Handler [%u],one connect handler have deqeue length [%u],number of connect handler [%u]."
               "About need  memory [%u] bytes.",
               sizeof(TCP_Svc_Handler),
               MAX_OF_CONNECT_PEER_SEND_DEQUE,
               max_connect_svr_,
               (max_connect_svr_ * (sizeof(TCP_Svc_Handler) + MAX_OF_CONNECT_PEER_SEND_DEQUE * sizeof(size_t)))
              );
    pool_of_cnthdl_.initialize(max_connect_svr_);

    for (size_t i = 0; i < max_connect_svr_; ++i)
    {
        TCP_Svc_Handler *p_handler = new TCP_Svc_Handler(HANDLER_MODE_CONNECT);
        pool_of_cnthdl_.push_back(p_handler);
    }

    //为ACCEPT的HDL预先分配内存，成为一个池子
    ZCE_LOGMSG(RS_INFO, "[zergsvr] Accept Hanlder:size of TCP_Svc_Handler [%u],one accept handler have deqeue length [%u],number of accept handler [%u]."
               "About need  memory [%u] bytes.",
               sizeof(TCP_Svc_Handler),
               accept_send_deque_size_,
               max_accept_svr_,
               (max_accept_svr_ * (sizeof(TCP_Svc_Handler) + accept_send_deque_size_ * sizeof(size_t)))
              );
    pool_of_acpthdl_.initialize(max_accept_svr_);

    for (size_t i = 0; i < max_accept_svr_; ++i)
    {
        TCP_Svc_Handler *p_handler = new TCP_Svc_Handler(HANDLER_MODE_ACCEPTED);
        pool_of_acpthdl_.push_back(p_handler);
    }

    //也许把这个扩展一些比较效率好。
    svr_peer_info_set_.initialize(max_accept_svr_ + max_connect_svr_ + 1024);

    //连接所有的SERVER
    size_t szsucc = 0, szfail = 0, szvalid = 0;
    zerg_auto_connect_.reconnect_allserver(szvalid, szsucc, szfail);

    return 0;
}

//取得句柄
ZCE_HANDLE TCP_Svc_Handler::get_handle(void) const
{
    return (ZCE_HANDLE)socket_peer_.get_handle();
}

//获得一个整型的handle
unsigned int TCP_Svc_Handler::get_handle_id()
{
#if defined ZCE_OS_WINDOWS

    ++handler_id_builder_;

    if (handler_id_builder_ == 0)
    {
        ++handler_id_builder_;
    }

    return  handler_id_builder_;

#elif defined ZCE_OS_LINUX
    return get_handle();
#endif
}


//读取,断连的事件触发处理函数
int TCP_Svc_Handler::handle_input()
{
    //读取数据
    size_t szrecv;

    int ret = read_data_from_peer(szrecv);

    ZCE_LOGMSG_DBG(RS_DEBUG, "Read event ,svcinfo[%u|%u] IP[%s|%u], handle input event triggered. ret:%d,szrecv:%u.",
                   peer_svr_id_.services_type_,
                   peer_svr_id_.services_id_,
                   peer_address_.get_host_addr(),
                   peer_address_.get_port_number(),
                   ret,
                   szrecv);

    //这儿任何错误都关闭,
    if (ret != 0)
    {
        return -1;
    }

    //将数据放入接收的管道,这儿返回的错误应该都是预处理的错误,放入管道的错误应该不会上升
    ret = push_frame_to_comm_mgr();

    //这儿任何错误都关闭,
    if (ret != 0)
    {
        return -1;
    }


    return 0;
}



//读取,断连的事件触发处理函数
int TCP_Svc_Handler::handle_output()
{

    //如果NON BLOCK Connect成功,也会调用handle_output
    if (PEER_STATUS_NOACTIVE == peer_status_)
    {
        //处理连接后的事宜
        process_connect_register();

        return 0;
    }

    int ret = 0;
    ret = write_all_data_to_peer();

    if (0 != ret)
    {
        //
        //为什么我不处理错误呢,不return -1,因为如果错误会关闭Socket,handle_input将被调用,这儿不重复处理
        //如果是中断等错误,程序可以继续的.
        //后来为啥又改成了return -1,加快处理?忘记忘记了。应该写注释呀。
        return -1;
    }

    return 0;
}



//定时器触发
int TCP_Svc_Handler::timer_timeout(const ZCE_Time_Value &now_time, const void *arg)
{
    const int timeid = *(static_cast<const int *>(arg));

    //连接上N秒，或者接受了M秒
    if (TCPCTRL_TIME_ID[0] == timeid)
    {
        //如果有受到数据，那么什么也不做
        if (receive_times_ > 0)
        {
            receive_times_ = 0;
        }
        //如果没有收到数据,跳楼自杀
        else
        {
            //如果是监听的端口，而且有相应的超时判断
            if (HANDLER_MODE_ACCEPTED == handler_mode_ &&
                ((0 == start_live_time_ && 0 < accepted_timeout_) ||
                 (0 < start_live_time_ && 0 < receive_timeout_)))
            {

                ZCE_LOGMSG(RS_ERROR, "[zergsvr] Connect or receive expire event,peer services [%u|%u] IP[%s|%u] "
                           "want to close handle. live time %lu. recieve times=%u.",
                           peer_svr_id_.services_type_,
                           peer_svr_id_.services_id_,
                           peer_address_.get_host_addr(),
                           peer_address_.get_port_number(),
                           now_time.sec() - start_live_time_,
                           receive_times_);

                //在这儿直接调用handle_close
                handle_close();
                return 0;
            }
        }

        //第一次使用的是accepted_timeout_
        if (0 == start_live_time_)
        {
            start_live_time_ = now_time.sec();
        }

        //打印一下各个端口的生存信息
        ZCE_LOGMSG(RS_DEBUG, "[zergsvr] Connect or receive expire event,peer services [%u|%u] IP[%s|%u] live "
                   "time %lu. recieve times=%u.",
                   peer_svr_id_.services_type_,
                   peer_svr_id_.services_id_,
                   peer_address_.get_host_addr(),
                   peer_address_.get_port_number(),
                   now_time.sec() - start_live_time_,
                   receive_times_);

        //这类统计如果过于频繁影响程序的运行,所以放入定时器作,虽然会感觉不是太准确,但是性能优先
        server_status_->increase_by_statid(ZERG_RECV_SUCC_COUNTER, 0, 0, recieve_counter_);
        server_status_->increase_by_statid(ZERG_SEND_SUCC_COUNTER, 0, 0, send_counter_);
        server_status_->increase_by_statid(ZERG_SEND_BYTES_COUNTER, 0, 0, send_bytes_);
        server_status_->increase_by_statid(ZERG_RECV_BYTES_COUNTER, 0, 0, recieve_bytes_);

        recieve_counter_ = 0;
        recieve_bytes_ = 0;
        send_counter_ = 0;
        send_bytes_ = 0;
    }
    else if (TCPCTRL_TIME_ID[1] == timeid)
    {
    }

    return 0;
}


//PEER Event Handler关闭的处理
int TCP_Svc_Handler::handle_close()
{
    ZCE_LOGMSG(RS_DEBUG, "[zergsvr] TCP_Svc_Handler::handle_close : %u.%u.",
               peer_svr_id_.services_type_, peer_svr_id_.services_id_);

    //不要使用cancel_timer(this),其繁琐,而且慢,好要new,而且有一个不知名的死机

    //取消此Event Handler相关的定时器
    if (-1 != timeout_time_id_)
    {
        timer_queue()->cancel_timer(timeout_time_id_);
        timeout_time_id_ = -1;
    }


    //取消MASK,最后阶段,避免调用handle_close,
    //内部会进行remove_handler
    ZCE_Event_Handler::handle_close();

    //关闭端口,
    socket_peer_.close();

    //释放接收数据缓冲区
    if (rcv_buffer_)
    {
        zbuffer_storage_->free_byte_buffer(rcv_buffer_);
        rcv_buffer_ = NULL;
    }

    //处理发送数据缓冲区
    size_t sz_of_deque = snd_buffer_deque_.size();

    for (size_t i = 0; i < sz_of_deque; i++)
    {
        //处理发送错误队列,同时进行回收
        process_send_error(snd_buffer_deque_[i], true);
        snd_buffer_deque_[i] = NULL;
    }

    snd_buffer_deque_.clear();

    //如果服务是激活状态，或者是主动连接的服务.
    if (peer_status_ == PEER_STATUS_ACTIVE || handler_mode_ == HANDLER_MODE_CONNECT)
    {
        //注销这些信息
        svr_peer_info_set_.del_services_peerInfo(peer_svr_id_);

        //如果是非后端业务进程强制关闭，则发送一个通知给业务进程，否则不做通知
        if (false == if_force_close_)
        {
            //通知后面的服务器
#ifdef ZCE_OS_WINDOWS
#pragma warning ( disable : 4815)
#endif
            //数据区的长度
            Zerg_App_Frame appframe;
#ifdef ZCE_OS_WINDOWS
#pragma warning ( default : 4815)
#endif
            appframe.init_framehead(Zerg_App_Frame::LEN_OF_APPFRAME_HEAD, 0, INNER_REG_SOCKET_CLOSED);
            appframe.send_service_ = peer_svr_id_;

            zerg_comm_mgr_->pushback_recvpipe(&appframe);
        }
    }

    //这类统计如果过于频繁影响程序的运行,所以放入最后作,虽然会感觉不是太准确,但是性能优先
    server_status_->increase_by_statid(ZERG_RECV_SUCC_COUNTER, 0, 0, recieve_counter_);
    server_status_->increase_by_statid(ZERG_SEND_SUCC_COUNTER, 0, 0, send_counter_);
    server_status_->increase_by_statid(ZERG_SEND_BYTES_COUNTER, 0, 0, send_bytes_);
    server_status_->increase_by_statid(ZERG_RECV_BYTES_COUNTER, 0, 0, recieve_bytes_);

    recieve_counter_ = 0;
    recieve_bytes_ = 0;
    send_counter_ = 0;
    send_bytes_ = 0;


    peer_status_ = PEER_STATUS_NOACTIVE;

    //根据不同的类型计数,

    //不进行主动重新连接,如果有一个新的数据要发送时主动重新连接
    if (handler_mode_ == HANDLER_MODE_CONNECT)
    {
        ZCE_LOGMSG(RS_INFO, "[zergsvr] Connect peer close, services[%u|%u] socket IP|Port :[%s|%u].",
                   peer_svr_id_.services_type_,
                   peer_svr_id_.services_id_,
                   peer_address_.get_host_addr(),
                   peer_address_.get_port_number()
                  );

        --num_connect_peer_;
        server_status_->set_by_statid(ZERG_CONNECT_PEER_NUMBER, 0, 0,
                                      num_connect_peer_);
        //将指针归还到池子中间去
        pool_of_cnthdl_.push_back(this);
    }
    else if (handler_mode_ == HANDLER_MODE_ACCEPTED)
    {
        ZCE_LOGMSG(RS_INFO, "[zergsvr] Accept peer close, services[%u|%u] socket IP|Port :[%s|%u].",
                   peer_svr_id_.services_type_,
                   peer_svr_id_.services_id_,
                   peer_address_.get_host_addr(),
                   peer_address_.get_port_number()
                  );

        --num_accept_peer_;
        server_status_->set_by_statid(ZERG_ACCEPT_PEER_NUMBER, 0, 0, num_accept_peer_);
        //将指针归还到池子中间去
        pool_of_acpthdl_.push_back(this);
    }

    return 0;
}



//收到一个完整的帧后的预处理工作
//合并发送队列
int TCP_Svc_Handler::preprocess_recvframe(Zerg_App_Frame *proc_frame)
{
    int ret = 0;
    //Zerg_App_Frame *proc_frame = reinterpret_cast<Zerg_App_Frame *>( rcv_buffer_->buffer_data_);

    //必须解码,否则后面的管道无法识别
    proc_frame->framehead_decode();
    DEBUGDUMP_FRAME_HEAD(proc_frame, "preprocess_recvframe After framehead_decode:", RS_DEBUG);

    //清理内部选项,避免被错误数据或者其他人整蛊
    proc_frame->clear_inner_option();

    //检查这个帧是否是发送给这个SVR,

    //如果是代理,检查帧的代理部分数据
    if (proc_frame->proxy_service_.services_type_ != SERVICES_ID::INVALID_SERVICES_TYPE && if_proxy_ == true)
    {

        if (my_svc_id_ != proc_frame->proxy_service_)
        {
            return SOAR_RET::ERR_ZERG_APPFRAME_ERROR;
        }
        //
        proc_frame->proxy_service_.services_id_ = my_svc_id_.services_id_;
    }
    //检查帧的接受部分
    else
    {
        if (my_svc_id_ != proc_frame->recv_service_)
        {
            return SOAR_RET::ERR_ZERG_APPFRAME_ERROR;
        }
        //对于某些情况(主要是客户端),对方不知道ServicesID
        else
        {
            proc_frame->recv_service_ = my_svc_id_;
        }

    }

    //如果端口仅仅刚刚ACCEPT上来，还没有收到数据
    if (PEER_STATUS_JUST_ACCEPT == peer_status_)
    {
        //记录Service Info,用于后面的处理,(发送的时候)
        if (proc_frame->proxy_service_.services_type_ != SERVICES_ID::INVALID_SERVICES_TYPE && if_proxy_ == false)
        {
            peer_svr_id_ = proc_frame->proxy_service_;
        }
        else
        {
            //如果是特殊的不加密的客户端，那么直接拿handle生成ID，因为这个时候没有qq号。
            if (SERVICES_ID::DYNAMIC_ALLOC_SERVICES_ID == proc_frame->send_service_.services_id_)
            {
                //分配一个ID给你。但你要记得回填喔
                proc_frame->send_service_.services_id_ = get_handle_id();
                peer_svr_id_ = proc_frame->send_service_;

                //proc_frame->frame_uid_   = proc_frame->send_service_.services_id_;

                //这种情况，可以先返回一个ID给对方
                //send_simple_zerg_cmd(,peer_svr_info_);
            }
            else
            {
                peer_svr_id_ = proc_frame->send_service_;
            }
        }

        //注册,如果原来有响应的链接,会返回原有的链接.replace_services_peerInfo,必然成功
        TCP_Svc_Handler *old_hdl = NULL;
        ret = svr_peer_info_set_.replace_services_peerInfo(peer_svr_id_, this, old_hdl);

        //如果有原有的链接,则找到原来的那个踢下去.
        if (old_hdl != NULL)
        {
            //而且修改原有链接的状态,避免重复从SET删除
            old_hdl->peer_status_ = PEER_STATUS_JUST_ACCEPT;
            old_hdl->send_simple_zerg_cmd(ZERG_REPEAT_LOGIN_KICK_OLD_RSP,
                                          peer_svr_id_,
                                          Zerg_App_Frame::DESC_SNDPRC_CLOSE_PEER);

            //不直接关闭了，而是先把命令发送完成了，再关闭
            //old_hdl->handle_close(ACE_INVALID_HANDLE, 0);
        }

        //最后调整自己PEER的状态
        peer_status_ = PEER_STATUS_ACTIVE;

        ZCE_LOGMSG(RS_INFO, "[zergsvr] Accept peer services[%u|%u],IP|Prot[%s|%u] regist success.",
                   peer_svr_id_.services_type_,
                   peer_svr_id_.services_id_,
                   peer_address_.get_host_addr(),
                   peer_address_.get_port_number()
                  );
    }
    //如果端口仅仅刚刚ACCEPT上去，还没有收到数据
    else if (PEER_STATUS_JUST_CONNECT == peer_status_)
    {
        //最后调整自己PEER的状态
        peer_status_ = PEER_STATUS_ACTIVE;

        ZCE_LOGMSG(RS_INFO, "[zergsvr] Connect peer services[%u|%u],IP|Prot[%s|%u] active success.",
                   peer_svr_id_.services_type_,
                   peer_svr_id_.services_id_,
                   peer_address_.get_host_addr(),
                   peer_address_.get_port_number()
                  );

    }
    else
    {
        if (SERVICES_ID::DYNAMIC_ALLOC_SERVICES_ID != proc_frame->send_service_.services_id_)
        {
            //否则检测发送者还是不是原来的发送者，是否被篡改
            if ((peer_svr_id_ != proc_frame->send_service_) && (peer_svr_id_ != proc_frame->proxy_service_))
            {
                return SOAR_RET::ERR_ZERG_APPFRAME_ERROR;
            }
        }
    }

    //如果是链接注册的请求，回答一个应答
    //如果想做复杂的交互控制，应该在这个地方加代码。
    if (ZERG_CONNECT_REGISTER_REQ == proc_frame->frame_command_)
    {
        send_simple_zerg_cmd(ZERG_CONNECT_REGISTER_RSP, peer_svr_id_);
    }

    //记录接受了多少次数据
    receive_times_++;

    if (receive_times_ == 0)
    {
        ++receive_times_;
    }

    //
    //填写IP地址和端口号
    proc_frame->send_ip_address_ = peer_address_.get_ip_address();


    return 0;
}

//返回端口的状态,
TCP_Svc_Handler::PEER_STATUS  TCP_Svc_Handler::get_peer_status()
{
    return peer_status_;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月27日
Function        : TCP_Svc_Handler::process_connect_register
Return          : int
Parameter List  : NULL
Description     : 处理注册发送,
Calls           :
Called By       :
Other           : 刚刚连接上对方,发送一个注册信息给对方.如果有命令发送命令
Modify Record   :
******************************************************************************************/
int TCP_Svc_Handler::process_connect_register()
{

    peer_status_ = PEER_STATUS_JUST_CONNECT;

    //主动发射一个注册CMD，如果有要发射的数据正好.
    send_simple_zerg_cmd(ZERG_CONNECT_REGISTER_REQ, peer_svr_id_);

    //再折腾了我至少3天以后，终于发现了EPOLL反复触发写事件的原因是没有取消CONNECT_MASK
    reactor()->cancel_wakeup(this, ZCE_Event_Handler::CONNECT_MASK);

    //注册读取的MASK
    reactor()->schedule_wakeup(this, ZCE_Event_Handler::READ_MASK);


    //打印信息
    ZCE_Sockaddr_In      peeraddr;
    socket_peer_.getpeername(&peeraddr);
    ZCE_LOGMSG(RS_INFO, "[zergsvr] Connect services[%u|%u] peer socket IP|Port :[%s|%u] Success.",
               peer_svr_id_.services_type_,
               peer_svr_id_.services_id_,
               peeraddr.get_host_addr(),
               peeraddr.get_port_number());

    return 0;
}




//从PEER读取数据
int TCP_Svc_Handler::read_data_from_peer(size_t &szrevc)
{

    szrevc = 0;
    ssize_t recvret = 0;

    //申请分配一个内存
    if (rcv_buffer_ == NULL)
    {
        rcv_buffer_ = zbuffer_storage_->allocate_buffer();
    }

    //ZCE_LOGMSG(RS_INFO,"[zergsvr] read_data_from_peer %d .", get_handle());

    //充分利用缓冲区去接收
    recvret = socket_peer_.recv(rcv_buffer_->buffer_data_ + rcv_buffer_->size_of_buffer_,
                                Zerg_Buffer::CAPACITY_OF_BUFFER - rcv_buffer_->size_of_buffer_,
                                0);

    //表示被关闭或者出现错误
    if (recvret < 0)
    {
        //我只使用EWOULDBLOCK 但是要注意EAGAIN, ZCE_LIB::last_error() != EWOULDBLOCK && ZCE_LIB::last_error() != EAGAIN
        if (ZCE_LIB::last_error() != EWOULDBLOCK)
        {
            szrevc = 0;

            //遇到中断,等待重入
            if (ZCE_LIB::last_error() == EINTR)
            {
                return 0;
            }

            //统计接收错误
            server_status_->increase_by_statid(ZERG_RECV_FAIL_COUNTER, 0, 0, 1);

            //记录错误,返回错误
            ZCE_LOGMSG(RS_ERROR, "[zergsvr] Receive data error ,services[%u|%u],IP[%s|%u] peer:%u,ZCE_LIB::last_error()=%d|%s.",
                       peer_svr_id_.services_type_,
                       peer_svr_id_.services_id_,
                       peer_address_.get_host_addr(),
                       peer_address_.get_port_number(),
                       socket_peer_.get_handle(),
                       ZCE_LIB::last_error(),
                       strerror(ZCE_LIB::last_error()));
            return SOAR_RET::ERR_ZERG_FAIL_SOCKET_OP_ERROR;
        }

        //统计接收阻塞的错误
        server_status_->increase_by_statid(ZERG_RECV_BLOCK_COUNTER, 0, 0, 1);

        //如果错误是阻塞,什么都不作
        return 0;
    }

    //Socket被关闭，也返回错误标示
    if (recvret == 0)
    {
        return SOAR_RET::ERR_ZERG_SOCKET_CLOSE;
    }

    //此时RET应该> 0
    szrevc = recvret;

    //接收了N个字符
    rcv_buffer_->size_of_buffer_ += static_cast<size_t>(szrevc);
    recieve_bytes_ += static_cast<size_t>(szrevc);

    return 0;
}




//检查是否收到了一个完整的帧,
//这儿还存在一种可能,一次收取了多个帧的可能,
int TCP_Svc_Handler::check_recv_full_frame(bool &bfull,
                                           unsigned int &whole_frame_len)
{
    whole_frame_len = 0;
    bfull = false;

    //如果连4个字节都没有收集齐,不用处理下面这段
    //注意这儿是在32位环境考虑
    if (rcv_buffer_->size_of_buffer_ - rcv_buffer_->size_of_use_ >= sizeof(unsigned int))
    {
        //如果有4个字节,检查帧的长度
        whole_frame_len = *(reinterpret_cast<unsigned int *>(rcv_buffer_->buffer_data_ + rcv_buffer_->size_of_use_));
        whole_frame_len = ntohl(whole_frame_len);

        //如果包的长度大于定义的最大长度,小于最小长度,见鬼去,出现做个错误不是代码错误，就是被人整蛊
        if (whole_frame_len > Zerg_App_Frame::MAX_LEN_OF_APPFRAME || whole_frame_len < Zerg_App_Frame::LEN_OF_APPFRAME_HEAD)
        {
            ZCE_LOGMSG(RS_ERROR, "[zergsvr] Recieve error frame,services[%u|%u],IP[%s|%u], famelen %u , MAX_LEN_OF_APPFRAME:%u ,recv and use len:%u|%u.",
                       peer_svr_id_.services_type_,
                       peer_svr_id_.services_id_,
                       peer_address_.get_host_addr(),
                       peer_address_.get_port_number(),
                       whole_frame_len,
                       Zerg_App_Frame::MAX_LEN_OF_APPFRAME,
                       rcv_buffer_->size_of_buffer_,
                       rcv_buffer_->size_of_use_);
            //
            DEBUGDUMP_FRAME_HEAD(reinterpret_cast<Zerg_App_Frame *>(rcv_buffer_->buffer_data_), "Error frame before framehead_decode,", RS_DEBUG);
            return SOAR_RET::ERR_ZERG_GREATER_MAX_LEN_FRAME;
        }
    }


    //如果接受的数据已经完整,(至少有一个完整)
    if (rcv_buffer_->size_of_buffer_ - rcv_buffer_->size_of_use_ >= whole_frame_len && whole_frame_len > 0)
    {
        bfull = true;
        ++recieve_counter_;
        ZCE_LOGMSG_DBG(RS_DEBUG, "Receive a whole frame from services[%u|%u] IP|Port [%s|%u] FrameLen:%u.",
                       peer_svr_id_.services_type_,
                       peer_svr_id_.services_id_,
                       peer_address_.get_host_addr(),
                       peer_address_.get_port_number(),
                       whole_frame_len);
    }

    return 0;
}


//将数据写入PEER，同时处理周边的事情，包括写事件注册,如果发送队列还有数据，继续发送等
int TCP_Svc_Handler::write_all_data_to_peer()
{
    int ret = 0;

    for (;;)
    {
        //发送一个数据包
        size_t szsend;
        bool   bfull = false;
        int ret = write_data_to_peer(szsend, bfull);

        //出现错误,
        if (ret != 0)
        {
            return ret;
        }

        //如果数据报已经完整发送
        if (true == bfull)
        {
            //成功，释放申请的空间
            zbuffer_storage_->free_byte_buffer(snd_buffer_deque_[0]);
            snd_buffer_deque_[0] = NULL;
            snd_buffer_deque_.pop_front();
        }
        //如果没有全部发送出去，等待下一次写时间的触发
        else
        {
            break;
        }

        //如果已经没有数据可以发送了
        if (snd_buffer_deque_.size() == 0)
        {
            break;
        }
    }

    //取得当前的MASK值
    int  handle_mask = get_mask();


    //如果队列中没有可以写的数据
    if (snd_buffer_deque_.size() == 0)
    {
        //
        if (handle_mask & ZCE_Event_Handler::WRITE_MASK)
        {
            //取消可写的MASK值,
            ret = reactor()->cancel_wakeup(this, ZCE_Event_Handler::WRITE_MASK);

            //return -1表示错误，正确返回的是old mask值
            if (-1 == ret)
            {
                ZCE_LOGMSG(RS_ERROR, "[zergsvr] TNNND cancel_wakeup return(%d) == -1 errno=%d|%s. ",
                           ret,
                           ZCE_LIB::last_error(),
                           strerror(ZCE_LIB::last_error()));
            }

        }

        //如果将要关闭
        if (true == if_force_close_)
        {
            ZCE_LOGMSG(RS_INFO, "[zergsvr] Send to peer services [%u|%u] IP|Port :[%s|%u] complete ,want to close peer on account of frame option.",
                       peer_svr_id_.services_type_,
                       peer_svr_id_.services_id_,
                       peer_address_.get_host_addr(),
                       peer_address_.get_port_number());
            //让上层去关闭，要小心，小心，很麻烦，很多生命周期的问题
            return SOAR_RET::ERR_ZERG_SOCKET_CLOSE;
        }
    }
    //如过没有发送成功，全部发送出去，则准备进行写事件
    else
    {
        //没有WRITE MASK，准备增加写标志
        if (!(handle_mask & ZCE_Event_Handler::WRITE_MASK))
        {
            ret = reactor()->schedule_wakeup(this, ZCE_Event_Handler::WRITE_MASK);

            //schedule_wakeup 返回return -1表示错误，再次BS ACE一次，正确返回的是old mask值
            if (-1 == ret)
            {
                ZCE_LOGMSG(RS_ERROR, "[zergsvr] TNNND schedule_wakeup return (%d)== -1 errno=%d|%s. ",
                           ret,
                           ZCE_LIB::last_error(),
                           strerror(ZCE_LIB::last_error()));
            }
        }
    }

    return 0;
}



//将数据写入PEER
int TCP_Svc_Handler::write_data_to_peer(size_t &szsend, bool &bfull)
{
    bfull = false;
    szsend = 0;

    //如果没有数据要发送, 到这儿应该是有问题
    //#if defined DEBUG || defined _DEBUG
    if (snd_buffer_deque_.empty() == true)
    {
        ZCE_LOGMSG(RS_ERROR, "[zergsvr] Goto handle_output|write_data_to_peer ,but not data to send. Please check,buffer deque size=%u.",
                   snd_buffer_deque_.size());
        ZCE_BACKTRACE_STACK(RS_ERROR);
        reactor()->cancel_wakeup(this, ZCE_Event_Handler::WRITE_MASK);
        ZCE_ASSERT(false);
        return 0;
    }

    //#endif //#if defined DEBUG || defined _DEBUG



    //前面有检查,不会越界
    Zerg_Buffer *sndbuffer = snd_buffer_deque_[0];

    ssize_t sendret = socket_peer_.send(sndbuffer->buffer_data_ + sndbuffer->size_of_buffer_,
                                        sndbuffer->size_of_use_ - sndbuffer->size_of_buffer_,
                                        0);

    if (sendret <= 0)
    {

        //遇到中断,等待重入的判断是if (ZCE_LIB::last_error() == EINVAL),但这儿不仔细检查错误,一视同仁,上层回忽视所有错误,如果错误致命,还会有handle_input反射
        //我只使用EWOULDBLOCK 但是要注意EAGAIN ZCE_LIB::last_error() != EWOULDBLOCK && ZCE_LIB::last_error() != EAGAIN
        if (ZCE_LIB::last_error() != EWOULDBLOCK)
        {
            //后面应该会打印方的IP，这儿不重复
            ZCE_LOGMSG(RS_ERROR, "[zergsvr] Send data error,services[%u|%u] IP|Port [%s|%u],Peer:%d errno=%d|%s .",
                       peer_svr_id_.services_type_,
                       peer_svr_id_.services_id_,
                       peer_address_.get_host_addr(),
                       peer_address_.get_port_number(),
                       socket_peer_.get_handle(),
                       ZCE_LIB::last_error(),
                       strerror(ZCE_LIB::last_error()));
            server_status_->increase_by_statid(ZERG_SEND_FAIL_COUNTER, 0, 0, 1);

            return SOAR_RET::ERR_ZERG_FAIL_SOCKET_OP_ERROR;
        }

        //统计发送阻塞的错误
        server_status_->increase_by_statid(ZERG_SEND_BLOCK_COUNTER, 0, 0, 1);

        //如果错误是阻塞,什么都不作
        return 0;
    }

    szsend = sendret;

    //发送了N个字符
    sndbuffer->size_of_buffer_ += static_cast<size_t>(szsend);
    send_bytes_ += static_cast<size_t>(szsend);

    //如果数据已经全部发送了
    if (sndbuffer->size_of_use_ == sndbuffer->size_of_buffer_)
    {
        bfull = true;
        ++send_counter_;
        //ZCE_LOGMSG_DBG(RS_DEBUG,"Send a few(n>=1) whole frame To  IP|Port :%s|%u FrameLen:%u.",
        //    peer_address_.get_host_addr(),
        //    peer_address_.get_port_number(),
        //    sndbuffer->size_of_buffer_);
    }

    return 0;
}

//处理发送错误.
int TCP_Svc_Handler::process_send_error(Zerg_Buffer *tmpbuf, bool frame_encode)
{
    //记录已经使用到的位置
    size_t use_start = tmpbuf->size_of_buffer_;
    tmpbuf->size_of_buffer_ = 0;

    //一个队列中间可能有多个FRAME，要对头部进行解码，所以必须一个个弄出来
    while (tmpbuf->size_of_buffer_ != tmpbuf->size_of_use_)
    {
        Zerg_App_Frame *proc_frame = reinterpret_cast<Zerg_App_Frame *>(tmpbuf->buffer_data_ + tmpbuf->size_of_buffer_);

        //如果FRAME已经编码
        if (frame_encode)
        {
            proc_frame->framehead_decode();
        }

        //检查已经使用的地址表示这个帧是否发送了,如果已经发送了，这个帧就不要处理

        //如果没有发送完成，记录下来，进行处理
        if (use_start < tmpbuf->size_of_buffer_ + proc_frame->frame_length_)
        {

            //如果是要记录的命令，记录下来，可以帮忙回溯一些问题
            if (proc_frame->frame_option_ & Zerg_App_Frame::DESC_SEND_FAIL_RECORD)
            {
                ZCE_LOGMSG(RS_ERROR, "[zergsvr] Connect peer ,send frame fail.frame len[%u] frame command[%u] frame uin[%u] snd svcid[%u|%u] proxy svc [%u|%u] recv[%u|%u] address[%s|%u],peer status[%u]. ",
                           proc_frame->frame_length_,
                           proc_frame->frame_command_,
                           proc_frame->frame_uid_,
                           proc_frame->send_service_.services_type_,
                           proc_frame->send_service_.services_id_,
                           proc_frame->proxy_service_.services_type_,
                           proc_frame->proxy_service_.services_id_,
                           proc_frame->recv_service_.services_type_,
                           proc_frame->recv_service_.services_id_,
                           peer_address_.get_host_addr(),
                           peer_address_.get_port_number(),
                           peer_status_
                          );
            }
        }

        //增加错误发送的处理
        server_status_->increase_by_statid(ZERG_SEND_FAIL_COUNTER, 0, 0, 1);

        //
        tmpbuf->size_of_buffer_ += proc_frame->frame_length_;
    }

    //归还到POOL中间。
    zbuffer_storage_->free_byte_buffer(tmpbuf);

    return 0;


}



// 从池子里面得到一个Handler给大家使用
TCP_Svc_Handler *TCP_Svc_Handler::alloce_hdl_from_pool(HANDLER_MODE handler_mode)
{
    //
    if (handler_mode == HANDLER_MODE_ACCEPTED)
    {
        if (pool_of_acpthdl_.size() == 0)
        {
            ZCE_LOGMSG(RS_INFO, "[zergsvr] Pool is too small to process accept handler,please notice.Pool size:%u,capacity:%u.",
                       pool_of_acpthdl_.size(),
                       pool_of_acpthdl_.capacity()
                      );
            return NULL;
        }

        TCP_Svc_Handler *p_handler = NULL;
        pool_of_acpthdl_.pop_front(p_handler);
        return p_handler;
    }
    //Connect的端口应该永远不发生取不到Hanler的事情
    else if (HANDLER_MODE_CONNECT == handler_mode)
    {
        ZCE_ASSERT(pool_of_cnthdl_.size() > 0);
        TCP_Svc_Handler *p_handler = NULL;
        pool_of_cnthdl_.pop_front(p_handler);
        return p_handler;
    }
    //Never go here.
    else
    {
        ZCE_ASSERT(false);
        return NULL;
    }
}


//链接所有的要自动链接的服务器,这个事避免服务器的链接断口后。又没有数据发送的情况
void TCP_Svc_Handler::reconnect_allserver(size_t szsucc,
                                          size_t szfail,
                                          size_t szvalid)
{
    //连接所有的SERVER
    zerg_auto_connect_.reconnect_allserver(szvalid, szsucc, szfail);
}
//
int TCP_Svc_Handler::uninit_all_staticdata()
{
    //
    svr_peer_info_set_.clear_and_closeall();

    //
    pool_of_acpthdl_.clear();

    return 0;
}

//处理发送一个数据
int TCP_Svc_Handler::process_send_data(Zerg_Buffer *tmpbuf)
{
    int ret = 0;
    server_status_->increase_by_statid(ZERG_SEND_FRAME_COUNTER, 0, 0, 1);
    //
    Zerg_App_Frame *proc_frame = reinterpret_cast<Zerg_App_Frame *>(tmpbuf->buffer_data_);
    DEBUGDUMP_FRAME_HEAD(proc_frame, "process_send_data Before framehead_encode:", RS_DEBUG);

    SERVICES_ID *p_sendto_svrinfo = NULL;

    //发送给代理，发送给接受者
    if (proc_frame->proxy_service_.services_type_ != SERVICES_ID::INVALID_SERVICES_TYPE && if_proxy_ == false)
    {
        p_sendto_svrinfo = &(proc_frame->proxy_service_);
    }
    else
    {
        p_sendto_svrinfo = &(proc_frame->recv_service_);
    }

    //广播
    if (p_sendto_svrinfo->services_id_ == SERVICES_ID::BROADCAST_SERVICES_ID)
    {
        std::vector<uint32_t> *id_ary;
        ret = svr_peer_info_set_.find_hdlary_by_type(p_sendto_svrinfo->services_type_, id_ary);
        if (0 != ret )
        {
            ZCE_LOGMSG(RS_ERROR, "process_send_data: service_id==BROADCAST_SERVICES_ID but cant't find has service_type=%d svrinfo",
                p_sendto_svrinfo->services_type_);
            return SOAR_RET::ERR_ZERG_SEND_FRAME_FAIL;
        }

        size_t ary_size = id_ary->size();
        TCP_Svc_Handler *svchanle = NULL;
        for (size_t i = 0; i < ary_size; ++i)
        {
            SERVICES_ID bc_svc_id(p_sendto_svrinfo->services_type_, (*id_ary)[i]);
            ret = svr_peer_info_set_.find_handle_by_svcid(bc_svc_id, svchanle);

            //理论上不可能找不到
            ZCE_ASSERT(ret == 0);
            if (ret != 0)
            {
            }

            p_sendto_svrinfo->services_id_ = bc_svc_id.services_id_;
            svchanle->put_frame_to_sendlist(tmpbuf);
        }
        
    }
    //给一个人
    else
    {
        uint32_t services_id = SERVICES_ID::INVALID_SERVICES_ID;
        TCP_Svc_Handler *svchanle = NULL;

        //对一些动态的SVC ID进行处理
        //负载均衡的方式
        if (p_sendto_svrinfo->services_id_ == SERVICES_ID::LOAD_BALANCE_DYNAMIC_ID)
        {

            ret = svr_peer_info_set_.find_lbseqhdl_by_type(p_sendto_svrinfo->services_type_, services_id, svchanle);
            if (ret != 0)
            {
                ZCE_LOGMSG(RS_ERROR, "process_send_data: service_id==LOAD_BALANCE_DYNAMIC_ID but cant't find has service_type=%d svrinfo",
                    p_sendto_svrinfo->services_type_);
            }

            // 修改一下要发送的svrinfo的id
            p_sendto_svrinfo->services_id_ = services_id;
            ZCE_LOGMSG(RS_DEBUG, "process_send_data: service_type=%d service_id= LOAD_BALANCE_DYNAMIC_ID,"
                " change service id to %u",
                p_sendto_svrinfo->services_type_,
                p_sendto_svrinfo->services_id_);
        }
        //这种情况，配置的服务器数量不能太多
        //负载均衡的方式
        else if (p_sendto_svrinfo->services_id_ == SERVICES_ID::MAIN_STANDBY_DYNAMIC_ID)
        {
            ret = svr_peer_info_set_.find_mshdl_by_type(p_sendto_svrinfo->services_type_, services_id, svchanle);
            if (ret != 0)
            {
                ZCE_LOGMSG(RS_ERROR, "process_send_data: service_id==MAIN_STANDBY_DYNAMIC_ID but cant't find has service_type=%d svrinfo",
                    p_sendto_svrinfo->services_type_);
            }
            // 修改一下要发送的svrinfo的id
            p_sendto_svrinfo->services_id_ = services_id;
            ZCE_LOGMSG(RS_DEBUG, "process_send_data: service_type=%d service_id= LOAD_BALANCE_DYNAMIC_ID,"
                " change service id to %u",
                p_sendto_svrinfo->services_type_,
                p_sendto_svrinfo->services_id_);
            //到达这儿应该有几种情况,
            //主路由OK，处于ACTIVE状态，使用主路由发送
            //主路由不处于ACTIVE状态，但是备份路由处于ACTIVE状态，使用备份路由发送
            //主路由存在，但是不处于ACTIVE状态，备份路由也不处于ACTIVE状态，将数据交给一个人，缓冲到发送队列
        }
        else
        {
            ret = svr_peer_info_set_.find_handle_by_svcid(*p_sendto_svrinfo, svchanle);
            //如果是要主动连接出去的服务器
            if (0 != ret )
            {
                ZCE_LOGMSG(RS_ERROR, "process_send_data: but cant't find has svc id=%u.%u svrinfo",
                    p_sendto_svrinfo->services_type_,
                    p_sendto_svrinfo->services_id_);

                //if (zerg_auto_connect_.is_auto_connect_svcid(*p_sendto_svrinfo))
                //{
                //    //不检查是否成功，异步连接，99.99999%是成功的,
                //    zerg_auto_connect_.reconnect_server(*p_sendto_svrinfo);
                //}
            }
        }

        //Double Check方法
        //如果SVCHANDLE为空,表示没有相关的连接,进行错误处理
        if (svchanle == NULL)
        {
            //这儿还没有编码
            ZCE_LOGMSG(RS_ERROR, "[zergsvr] [SEND TO NO EXIST HANDLE] ,send to a no exist handle[%u|%u],it could "
                "have been existed. frame command[%u]. uin[%u] frame length[%u].",
                p_sendto_svrinfo->services_type_,
                p_sendto_svrinfo->services_id_,
                proc_frame->frame_command_,
                proc_frame->frame_uid_,
                proc_frame->frame_length_
                );
            DEBUGDUMP_FRAME_HEAD(proc_frame, "[SEND TO NO EXIST HANDLE]", RS_ERROR);
            return SOAR_RET::ERR_ZERG_SEND_FRAME_FAIL;
        }


        //将发送的FRAME给HANDLE对象，当然这个地方未必一定放的进去，因为有几种情况,
        //1.就是一个关闭指令,
        //2.HANDLE内部的队列满了,

        //这儿不进行错误处理，因为put_frame_to_sendlist内部进行了错误处理，回收等操作
        //到这儿为止，认为成功
        svchanle->put_frame_to_sendlist(tmpbuf);
    }

    return 0;
}


///根据services_type查询对应的配置主备服务器列表数组 MS（主备）,
///请参考 @ref Zerg_Auto_Connector
int TCP_Svc_Handler::find_conf_ms_svcid_ary(uint16_t services_type,
                                            std::vector<uint32_t> *& ms_svcid_ary)
{
    return zerg_auto_connect_.find_conf_ms_svcid_ary(services_type, ms_svcid_ary);
}


//发送简单的的ZERG命令,用于某些特殊命令的处理
int TCP_Svc_Handler::send_simple_zerg_cmd(unsigned int cmd,
                                          const SERVICES_ID &recv_services_info,
                                          unsigned int option)
{
    //ZCE_LOGMSG_DBG(RS_DEBUG,"Send simple command to services[%u|%u] IP[%s|%u],Cmd %u.",
    //    peer_svr_info_.services_type_,
    //    peer_svr_info_.services_id_,
    //    peer_address_.get_host_addr(),
    //    peer_address_.get_port_number(),
    //    cmd);
    //向对方发送一个心跳包
    Zerg_Buffer *tmpbuf = zbuffer_storage_->allocate_buffer();
    Zerg_App_Frame *proc_frame = reinterpret_cast<Zerg_App_Frame *>(tmpbuf->buffer_data_);

    proc_frame->init_framehead(Zerg_App_Frame::LEN_OF_APPFRAME_HEAD, option, cmd);
    //注册命令
    proc_frame->send_service_ = my_svc_id_;

    //如果自己是代理服务器,填写代理服务器信息,保证发送,
    if (if_proxy_)
    {
        proc_frame->proxy_service_ = my_svc_id_;
    }

    //
    proc_frame->recv_service_ = recv_services_info;

    //
    tmpbuf->size_of_use_ = Zerg_App_Frame::LEN_OF_APPFRAME_HEAD;

    //
    return put_frame_to_sendlist(tmpbuf);
}

//发送心跳
int TCP_Svc_Handler::send_zergheatbeat_reg()
{
    //
    return send_simple_zerg_cmd(ZERG_HEART_BEAT_REQ, peer_svr_id_);
}


//将发送数据放入发送队列中
//如果一个PEER没有连接上,等待发送的数据不能多于PEER_STATUS_NOACTIVE个
//put_frame_to_sendlist内部进行了错误处理，回收等操作
int TCP_Svc_Handler::put_frame_to_sendlist(Zerg_Buffer *tmpbuf)
{
    int ret = 0;

    Zerg_App_Frame *proc_frame = reinterpret_cast<Zerg_App_Frame *>(tmpbuf->buffer_data_);

    //如果是通知关闭端口
    if (proc_frame->frame_command_ == INNER_RSP_CLOSE_SOCKET)
    {
        ZCE_LOGMSG(RS_INFO, "[zergsvr] Recvice CMD_RSP_CLOSE_SOCKET,services[%u|%u] IP[%s|%u] Svchanle will close.",
                   peer_svr_id_.services_type_,
                   peer_svr_id_.services_id_,
                   peer_address_.get_host_addr(),
                   peer_address_.get_port_number()
                  );
        if_force_close_ = true;
        //回收帧
        process_send_error(tmpbuf, false);
        //如果不是UDP的处理,关闭端口,UDP的东西没有链接的概念,
        handle_close();

        //返回一个错误，让上层回收
        return SOAR_RET::ERR_ZERG_SOCKET_CLOSE;
    }

    //如果发送完成,并且后台业务要求关闭端口,注意必须转换网络序
    if (proc_frame->frame_option_ & Zerg_App_Frame::DESC_SNDPRC_CLOSE_PEER)
    {
        ZCE_LOGMSG(RS_INFO, "[zergsvr] This Peer Services[%u|%u] IP|Port :[%s|%u] will close when all frame send complete ,because send frame has option Zerg_App_Frame::DESC_SNDPRC_CLOSE_PEER.",
                   peer_svr_id_.services_type_,
                   peer_svr_id_.services_id_,
                   peer_address_.get_host_addr(),
                   peer_address_.get_port_number());
        if_force_close_ = true;
    }

    //注意这个地方，我们会调整这个Services ID，保证发送出去的数据都是用自己的SVCID标示的.
    if (!if_proxy_)
    {
        proc_frame->send_service_ = my_svc_id_;
    }

    //对头部进行编码
    proc_frame->framehead_encode();

    //放入发送队列,并注册标志位
    bool bret = snd_buffer_deque_.push_back(tmpbuf);

    if (!bret)
    {
        server_status_->increase_by_statid(ZERG_SEND_LIST_FULL_COUNTER, 0, 0, 1);
        //丢弃或者错误处理那个数据比较好呢?这儿值得商榷, 我这儿进行错误处理(可能丢弃)的是最新的.
        //我的考虑是如果命令有先后性.而且可以避免内存操作.
        ZCE_LOGMSG(RS_ERROR, "[zergsvr] Services [%u|%u] IP|Port[%s|%u] send buffer cycle deque is full,this data must throw away,Send deque capacity =%u,may be extend it.",
                   peer_svr_id_.services_type_,
                   peer_svr_id_.services_id_,
                   peer_address_.get_host_addr(),
                   peer_address_.get_port_number(),
                   snd_buffer_deque_.capacity());

        //回收帧
        process_send_error(tmpbuf, true);
        //返回一个错误
        return SOAR_RET::ERR_ZERG_SEND_FRAME_FAIL;
    }

    //------------------------------------------------------------------
    //这儿开始，数据已经放入发送队列，回收可以再handle_close自己回收了.

    if (peer_status_ != PEER_STATUS_NOACTIVE)
    {
        ret = write_all_data_to_peer();

        //出现错误,
        if (ret != 0)
        {
            //为什么我不处理错误呢,不return -1,因为如果错误会关闭Socket,handle_input将被调用,这儿不重复处理
            //如果是中断等错误,程序可以继续的.
            handle_close();

            //发送数据已经放入队列，返回OK
            return 0;
        }

        //合并
        unite_frame_sendlist();
    }

    //只有放入发送队列才算成功.
    return 0;
}


//合并发送队列
void TCP_Svc_Handler::unite_frame_sendlist()
{
    //如果有2个以上的的发送队列，则可以考虑合并处理
    size_t sz_deque = snd_buffer_deque_.size();

    if (sz_deque <= 1)
    {
        return;
    }

    //如果倒数第2个桶有能力放下倒数第1个桶的FRAME数据，则进行合并操作。
    if (Zerg_App_Frame::MAX_LEN_OF_APPFRAME - snd_buffer_deque_[sz_deque - 2]->size_of_use_ > snd_buffer_deque_[sz_deque - 1]->size_of_use_)
    {
        //将倒数第1个节点的数据放入倒数第2个节点中间。所以实际的Cache能力是非常强的，
        //空间利用率也很高。越发佩服我自己了。
        memcpy(snd_buffer_deque_[sz_deque - 2]->buffer_data_ + snd_buffer_deque_[sz_deque - 2]->size_of_use_,
               snd_buffer_deque_[sz_deque - 1]->buffer_data_,
               snd_buffer_deque_[sz_deque - 1]->size_of_use_);
        snd_buffer_deque_[sz_deque - 2]->size_of_use_ += snd_buffer_deque_[sz_deque - 1]->size_of_use_;

        //将倒数第一个施放掉
        zbuffer_storage_->free_byte_buffer(snd_buffer_deque_[sz_deque - 1]);
        snd_buffer_deque_[sz_deque - 1] = NULL;
        snd_buffer_deque_.pop_back();
    }

    ////下面的代码用于合并的测试，平常会注释掉
    //else
    //{
    //    ZCE_LOGMSG_DBG(RS_DEBUG,"Goto unite_frame_sendlist sz_deque=%u,Zerg_App_Frame::MAX_LEN_OF_APPFRAME=%u,"
    //        "snd_buffer_deque_[sz_deque-2]->size_of_use_=%u,"
    //        "snd_buffer_deque_[sz_deque-1]->size_of_use_=%u.",
    //        sz_deque,
    //        Zerg_App_Frame::MAX_LEN_OF_APPFRAME,
    //        snd_buffer_deque_[sz_deque-2]->size_of_use_,
    //        snd_buffer_deque_[sz_deque-1]->size_of_use_);
    //}

}



//将数据帧交给通信管理器，放入管道
int TCP_Svc_Handler::push_frame_to_comm_mgr()
{

    int ret = 0;

    //
    rcv_buffer_->size_of_use_ = 0;

    //
    while (rcv_buffer_)
    {
        unsigned int whole_frame_len = 0;
        bool bfull = false;
        ret = check_recv_full_frame(bfull, whole_frame_len);

        if (ret != 0)
        {
            return -1;
        }

        //如果没有受到
        if (false == bfull)
        {
            if (rcv_buffer_->size_of_use_ > 0)
            {
                //拷贝的内存可能交错,所以不用memcpy
                memmove(rcv_buffer_->buffer_data_,
                        rcv_buffer_->buffer_data_ + rcv_buffer_->size_of_use_,
                        rcv_buffer_->size_of_buffer_ - rcv_buffer_->size_of_use_);

                //改变buffer长度
                rcv_buffer_->size_of_buffer_ = rcv_buffer_->size_of_buffer_ - rcv_buffer_->size_of_use_;
                rcv_buffer_->size_of_use_ = 0;
            }

            break;
        }

        Zerg_App_Frame *proc_frame = reinterpret_cast<Zerg_App_Frame *>(rcv_buffer_->buffer_data_ + rcv_buffer_->size_of_use_);

        //如果已经收集了一个数据
        ret = preprocess_recvframe(proc_frame);

        //已经有相同的ID连接了,或者帧检查错误
        if (ret != 0)
        {
            //迫不得已,把 错误答应的函数放到这个地方,放到后面的函数,要输出的种类过多,违背我的代码美学.
            if (SOAR_RET::ERR_ZERG_APPFRAME_ERROR == ret || SOAR_RET::ERR_ZERG_SERVER_ALREADY_LONGIN == ret)
            {
                //
                ZCE_LOGMSG(RS_ERROR, "[zergsvr] Peer services[%u|%u] IP[%s|%u] appFrame Error,Frame Len:%u,Command:%u,Uin:%u "
                           "Peer SvrType|SvrID:%u|%u,"
                           "Self SvrType|SvrID:%u|%u,"
                           "Send SvrType|SvrID:%u|%u,"
                           "Recv SvrType|SvrID:%u|%u,"
                           "Proxy SvrType|SvrID:%u|%u.",
                           peer_svr_id_.services_type_,
                           peer_svr_id_.services_id_,
                           peer_address_.get_host_addr(),
                           peer_address_.get_port_number(),
                           proc_frame->frame_length_,
                           proc_frame->frame_command_,
                           proc_frame->frame_uid_,
                           peer_svr_id_.services_type_, peer_svr_id_.services_id_,
                           my_svc_id_.services_type_, my_svc_id_.services_id_,
                           proc_frame->send_service_.services_type_,
                           proc_frame->send_service_.services_id_,
                           proc_frame->recv_service_.services_type_,
                           proc_frame->recv_service_.services_id_,
                           proc_frame->proxy_service_.services_type_,
                           proc_frame->proxy_service_.services_id_
                          );
            }
            else
            {
                ZCE_LOGMSG(RS_ERROR, "[zergsvr] Peer services [%u|%u] IP[%s|%u] preprocess_recvframe Ret =%d.",
                           peer_svr_id_.services_type_,
                           peer_svr_id_.services_id_,
                           peer_address_.get_host_addr(),
                           peer_address_.get_port_number(),
                           ret);
            }

            //统计接收错误
            server_status_->increase_by_statid(ZERG_RECV_FAIL_COUNTER, 0, 0, 1);
            return -1;
        }

        //将数据放入接收的管道,不检测错误,因为错误会记录日志,而且有错误，也无法处理

        zerg_comm_mgr_->pushback_recvpipe(proc_frame);

        //接收一个完整的数据
        rcv_buffer_->size_of_use_ += whole_frame_len;

        if (rcv_buffer_->size_of_buffer_ == rcv_buffer_->size_of_use_)
        {
            //无论处理正确与否,都释放缓冲区的空间
            zbuffer_storage_->free_byte_buffer(rcv_buffer_);
            rcv_buffer_ = NULL;
        }
        //如果第一个包的收到数据已经大于这个长度.那么就会出现下面的情况，
        //如果这儿想避免复杂的判断，可以限定收到的第一个数据包的最大长度为帧头的长度，但是这样会降低效率。
        else if (rcv_buffer_->size_of_buffer_ > rcv_buffer_->size_of_use_)
        {

        }
    }

    return 0;
}


void TCP_Svc_Handler::get_max_peer_num(size_t &maxaccept, size_t &maxconnect)
{
    maxaccept = max_accept_svr_;
    maxconnect = max_connect_svr_;
}


//得到Handle对应PEER的IP地址#端口信息
const char *TCP_Svc_Handler::get_peer_address(char *addr_buf, int buf_size)
{
    return peer_address_.get_host_addr_port(addr_buf, buf_size);
}

void TCP_Svc_Handler::dump_status_staticinfo(ZCE_LOG_PRIORITY out_lvl)
{
    ZCE_LOGMSG(out_lvl, "Dump TCP_Svc_Handler Static Info:");
    ZCE_LOGMSG(out_lvl, "max_accept_svr_=%lu" , max_accept_svr_);
    ZCE_LOGMSG(out_lvl, "max_connect_svr_=%lu" , max_connect_svr_);
    ZCE_LOGMSG(out_lvl, "if_proxy_= %s", if_proxy_ ? "True" : "False");
    ZCE_LOGMSG(out_lvl, "accepted_timeout_=%u:", accepted_timeout_);
    ZCE_LOGMSG(out_lvl, "receive_timeout_=%u", receive_timeout_);
    ZCE_LOGMSG(out_lvl, "num_accept_peer_=%u", num_accept_peer_);
    ZCE_LOGMSG(out_lvl, "num_connect_peer_=%lu", num_connect_peer_);
    ZCE_LOGMSG(out_lvl, "NUM CONNECT PEER=%lu" , num_connect_peer_);
}



//
void TCP_Svc_Handler::dump_status_info(ZCE_LOG_PRIORITY out_lvl)
{
    const size_t OUT_BUF_LEN = 64;
    char out_buf[OUT_BUF_LEN];
    out_buf[OUT_BUF_LEN] = '\0';
    ZCE_LOGMSG(out_lvl, "my_svc_id_=[%hu.%u]", my_svc_id_.services_type_ , my_svc_id_.services_id_);
    ZCE_LOGMSG(out_lvl, "peer_svr_id_=[%hu.%u]", peer_svr_id_.services_type_, peer_svr_id_.services_id_);
    ZCE_LOGMSG(out_lvl, "peer_address_=%s", peer_address_.to_string(out_buf, OUT_BUF_LEN - 1));
    ZCE_LOGMSG(out_lvl, "peer_status_=%d", peer_status_);
#if defined (ZCE_OS_WINDOWS)
    ZCE_LOGMSG(out_lvl, "get_handle=%p", get_handle());
#elif defined (ZCE_OS_LINUX)
    ZCE_LOGMSG(out_lvl, "get_handle=%d", get_handle());
#endif
    ZCE_LOGMSG(out_lvl, "recieve_bytes_ =%lu,rcv_buffer_ =%d", recieve_bytes_, ((rcv_buffer_ != NULL) ? 1 : 0));
    ZCE_LOGMSG(out_lvl, "send_bytes_=%lu snd_buffer_deque_.size=%lu", send_bytes_, snd_buffer_deque_.size());
}

//Dump 所有的PEER信息
void TCP_Svc_Handler::dump_svcpeer_info(ZCE_LOG_PRIORITY out_lvl)
{
    ZCE_LOGMSG(out_lvl, "Services Peer Size =%lu", svr_peer_info_set_.get_services_peersize());
    svr_peer_info_set_.dump_svr_peerinfo(out_lvl);
}

//关闭相应的连接
int TCP_Svc_Handler::close_services_peer(const SERVICES_ID &svr_info)
{
    int ret = 0;
    TCP_Svc_Handler *svchanle = NULL;
    ret = svr_peer_info_set_.find_handle_by_svcid(svr_info, svchanle);

    //如果是要重新进行连接的服务器主动主动连接,
    if (ret != 0)
    {
        return ret;
    }

    svchanle->handle_close();
    return 0;
}

//根据有的SVR INFO，查询相应的HDL
int TCP_Svc_Handler::find_services_peer(const SERVICES_ID &svr_info, TCP_Svc_Handler *&svchanle)
{
    int ret = 0;
    ret = svr_peer_info_set_.find_handle_by_svcid(svr_info, svchanle);

    //如果是要重新进行连接的服务器主动主动连接,
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

const ZCE_Sockaddr_In &TCP_Svc_Handler::get_peer_sockaddr() const
{
    return peer_address_;
}

///取得tptoid_table_id_
size_t TCP_Svc_Handler::get_tptoid_table_id()
{
    return tptoid_table_id_;
}
///设置tptoid_table_id_
void TCP_Svc_Handler::set_tptoid_table_id(size_t ary_id)
{
    tptoid_table_id_ = ary_id;
}
