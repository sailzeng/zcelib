

#include "ogre_predefine.h"
#include "ogre_svrpeer_info_set.h"
#include "ogre_app_handler.h"
#include "ogre_configure.h"
#include "ogre_buf_storage.h"
#include "ogre_auto_connect.h"

#include "ogre_tcp_ctrl_handler.h"

/****************************************************************************************************
class  Ogre_TCP_Svc_Handler
****************************************************************************************************/
//CONNECT后等待数据的超时时间
unsigned int   Ogre_TCP_Svc_Handler::connect_timeout_ = 3;
//接受数据的超时时间
unsigned int   Ogre_TCP_Svc_Handler::receive_timeout_ = 5;

//TIME ID
const int      Ogre_TCP_Svc_Handler::TCPCTRL_TIME_ID[] = {1, 2};

unsigned int   Ogre_TCP_Svc_Handler::error_try_num_ = 3;

//
PeerInfoSetToTCPHdlMap Ogre_TCP_Svc_Handler::svr_peer_info_set_;

Ogre_Connect_Server Ogre_TCP_Svc_Handler::zerg_auto_connect_;

//
size_t         Ogre_TCP_Svc_Handler::num_accept_peer_ = 0;
//
size_t         Ogre_TCP_Svc_Handler::num_connect_peer_ = 0;

//最大可以接受的接受数量
size_t         Ogre_TCP_Svc_Handler::max_accept_svr_ = 0;
//最大可以接受的连接数量
size_t         Ogre_TCP_Svc_Handler::max_connect_svr_ = 0;

//ACCEPT SVC handler的池子
Ogre_TCP_Svc_Handler::POOL_OF_TCP_HANDLER Ogre_TCP_Svc_Handler::pool_of_acpthdl_;

//CONNECT svc handler的池子
Ogre_TCP_Svc_Handler::POOL_OF_TCP_HANDLER Ogre_TCP_Svc_Handler::pool_of_cnthdl_;

//构造函数
Ogre_TCP_Svc_Handler::Ogre_TCP_Svc_Handler(Ogre_TCP_Svc_Handler::OGRE_HANDLER_MODE hdl_mode ):
    ZEN_Event_Handler(ZEN_Reactor::instance()),
    handler_mode_(hdl_mode),
    rcv_buffer_(NULL),
    recieve_bytes_(0),
    send_bytes_(0),
    peer_status_(PEER_STATUS_JUST_CONNECT),
    timeout_time_id_(-1),
    receive_times_(0),
    if_force_close_(false),
    fp_judge_whole_frame_(NULL)
{
    if ( HANDLER_MODE_CONNECT == hdl_mode )
    {
        snd_buffer_deque_.initialize(MAX_OF_CONNECT_PEER_SEND_DEQUE);
    }
    else if (HANDLER_MODE_ACCEPTED == hdl_mode )
    {
        snd_buffer_deque_.initialize(MAX_OF_ACCEPT_PEER_SEND_DEQUE);
    }
    else
    {
        ZEN_ASSERT(false);
    }
}

//析构函数
Ogre_TCP_Svc_Handler::~Ogre_TCP_Svc_Handler()
{

}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2006年3月22日
Function        : Ogre_TCP_Svc_Handler::init_tcp_svc_handler
Return          : void
Parameter List  :
  Param1: const ZEN_Socket_Stream& sockstream
  Param2: const ZEN_Sockaddr_In& socketaddr   Socket的地址，其实可以从sockstream中得到，但为了效率和方便.
Description     : 初始化函数,用于Accept的端口的处理Event Handle构造.
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
void Ogre_TCP_Svc_Handler::init_tcp_svc_handler(const ZEN_Socket_Stream &sockstream,
                                                FPJudgeRecvWholeFrame fp_judge_whole)
{
    handler_mode_ = HANDLER_MODE_ACCEPTED;
    rcv_buffer_ = NULL;
    recieve_bytes_ = 0;
    send_bytes_ = 0;
    socket_peer_ = sockstream;
    peer_status_ = PEER_STATUS_JUST_CONNECT;
    timeout_time_id_ = -1;
    receive_times_  = 0;
    if_force_close_ = false;
    fp_judge_whole_frame_ = fp_judge_whole;

    ////调整Socket 为O_NONBLOCK
    int ret = socket_peer_.sock_enable(O_NONBLOCK);
    socket_peer_.getpeername(&remote_address_);
    socket_peer_.getsockname(&local_address_);

    const size_t TMP_ADDR_LEN = 32;
    char local_addr[TMP_ADDR_LEN], remote_addr[TMP_ADDR_LEN];
    strncpy(local_addr, local_address_.get_host_addr(), TMP_ADDR_LEN);
    strncpy(remote_addr, remote_address_.get_host_addr(), TMP_ADDR_LEN);
    ZLOG_INFO( "Listen peer [%s|%u] accept Socket IP Address: [%s|%u] success,.\n",
               local_addr,
               local_address_.get_port_number(),
               remote_addr,
               remote_address_.get_port_number()
             );

    ++num_accept_peer_;
    //统计

    //检查最大链接数量,REACTOR自己其实有控制,但是我又要控制ACCEPT又要控制CONNECT.
    //所以只好,头XX大, 另外这个事情其实从理论上来讲,几乎不会发生,
    if (num_accept_peer_ < max_accept_svr_)
    {
        //注册读写事件
        ret = reactor()->register_handler(this, ZEN_Event_Handler::READ_MASK | ZEN_Event_Handler::WRITE_MASK);

        //
        if (ret != 0)
        {
            ZLOG_ERROR( "Register Accept handler fail! ret =%d  errno=%u|%s \n", ret, ZEN_OS::last_error(), strerror(ZEN_OS::last_error()));
            handle_close();
            return;
        }

        reactor()->cancel_wakeup(this, ZEN_Event_Handler::WRITE_MASK);
    }
    //要测试检查一下,
    else
    {
        ZLOG_ERROR( "Great than max_accept_svr_ Reject! num_accept_peer_:%u,max_accept_svr_:%u \n",
                    num_accept_peer_,
                    max_accept_svr_);
        handle_close();
        return;
    }

    //如果配置了超时出来,N秒必须收到一个包
    if ( connect_timeout_ > 0 || receive_timeout_ > 0)
    {
        ZEN_Time_Value delay(0, 0);
        ZEN_Time_Value interval(0, 0);

        delay.sec(connect_timeout_);
        interval.sec(receive_timeout_);

        //如果配置了接收数据超时，那么无论如何连接部分都有超时
        if (connect_timeout_ <= 0)
        {
            delay.sec(receive_timeout_);
        }

        timeout_time_id_ = timer_queue()->schedule_timer (this, &TCPCTRL_TIME_ID[0], delay, interval);
    }

    Socket_Peer_Info peer_svr_info(remote_address_);
    //放入连接处理的MAP
    ret = svr_peer_info_set_.add_services_peerinfo(peer_svr_info, this);

    //在这儿自杀是不是危险了一点
    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        handle_close();
        return;
    }

    peer_status_ = PEER_STATUS_ACTIVE;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2006年3月22日
Function        : Ogre_TCP_Svc_Handler::init_tcp_svc_handler
Return          : void
Parameter List  :
  Param1: const Socket_Peer_Info& svrinfo        Socket_Peer_Info信息
  Param2: const ZEN_Socket_Stream& sockstream  Sokcet Peer
  Param3: const ZEN_Sockaddr_In& socketaddr    对应连接的地址信息
Description     : 初始化函数,用于Connect出去的PEER 对应Event Handle构造.
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
void Ogre_TCP_Svc_Handler::init_tcp_svc_handler(const ZEN_Socket_Stream &sockstream,
                                                const ZEN_Sockaddr_In &socketaddr,
                                                FPJudgeRecvWholeFrame fp_judge_whole)
{
    handler_mode_ = HANDLER_MODE_CONNECT;
    rcv_buffer_ = NULL;
    recieve_bytes_ = 0;
    send_bytes_ = 0;
    socket_peer_ = sockstream;
    remote_address_ = socketaddr;
    peer_status_ = PEER_STATUS_NOACTIVE;
    timeout_time_id_ = -1;
    receive_times_ = 0;
    if_force_close_ = false;
    fp_judge_whole_frame_ = fp_judge_whole;

    //调整Socket 为O_NONBLOCK
    int ret = socket_peer_.sock_enable(O_NONBLOCK);
    //
    socket_peer_.getsockname(&local_address_);

    //注册到
    ret = reactor()->register_handler(this, ZEN_Event_Handler::CONNECT_MASK);

    //我几乎没有见过register_handler失败,
    if (ret != 0)
    {

        ZLOG_ERROR("Register Connect handler fail! ret =%  errno=%u|%s \n", ret, ZEN_OS::last_error(), strerror(ZEN_OS::last_error()));
        handle_close();
        return;
    }

    Socket_Peer_Info peer_svr_info(remote_address_);
    //放入连接处理的MAP
    ret = svr_peer_info_set_.add_services_peerinfo(peer_svr_info, this);

    //在这儿自杀是不是危险了一点
    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        handle_close();
        return;
    }

    ++num_connect_peer_;

    socklen_t opval = 32 * 1024;
    int opvallen = sizeof(socklen_t);
    //设置一个SND,RCV BUFFER,
    socket_peer_.setsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&opval), opvallen);
    socket_peer_.setsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&opval), opvallen);
    int keep_alive = 1;
    opvallen = sizeof(int);
    socket_peer_.setsockopt(SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<void *>(&keep_alive), opvallen);

#ifndef WIN32
    //避免DELAY发送这种情况
    int NODELAY = 1;
    opvallen = sizeof(int);
    socket_peer_.setsockopt(SOL_TCP, TCP_NODELAY, reinterpret_cast<void *>(&NODELAY), opvallen);
#endif
}

//取得句柄
ZEN_SOCKET Ogre_TCP_Svc_Handler::get_handle(void) const
{
    return socket_peer_.get_handle();
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月27日
Function        : Ogre_TCP_Svc_Handler::handle_input
Return          : int
Parameter List  :
  Param1: ZEN_HANDLE
Description     : ACE读取,断连的事件触发处理函数
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::handle_input(ZEN_HANDLE)
{
    //读取数据
    size_t szrecv;

    int ret = read_data_from_peer(szrecv);

    ZLOG_DEBUG("Read event,[%s|%u] ,TCP handle input event triggered. ret:%d,szrecv:%u.\n",
               remote_address_.get_host_addr(),
               remote_address_.get_port_number(),
               ret,
               szrecv);

    //这儿任何错误都关闭,
    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return -1;
    }

    //
    while (rcv_buffer_)
    {
        unsigned int size_frame = 0;
        bool if_recv_whole = false;
        //调用模块代码
        ret = fp_judge_whole_frame_(rcv_buffer_->frame_data_,
                                    rcv_buffer_->ogre_frame_len_ - Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD,
                                    static_cast<unsigned int>(Ogre4a_AppFrame::MAX_OF_OGRE_DATA_LEN),
                                    if_recv_whole,
                                    size_frame);

        if (TSS_RET::TSS_RET_SUCC != ret)
        {
            ZLOG_ERROR( "Read data error [%s|%u].Judge whole fale error ret=%u.\n",
                        remote_address_.get_host_addr(),
                        remote_address_.get_port_number(),
                        ret);
            return -1;
        }

        //如果已经收集了一个数据
        if (if_recv_whole == true)
        {
            //将数据放入接收的管道,不检测错误,因为错误会记录日志,而且有错误，也无法处理
            ZEN_LOGMSG_DBG(RS_DEBUG, "Read a whole data [%s|%u] recv buffer len:%u, Frame len:%u.\n",
                           remote_address_.get_host_addr(),
                           remote_address_.get_port_number(),
                           rcv_buffer_->ogre_frame_len_ - Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD,
                           size_frame);

            //记录接受了多少次数据
            receive_times_++;

            if (receive_times_ == 0)
            {
                receive_times_ ++;
            }

            push_frame_to_recvpipe(size_frame);
        }
        else
        {
            //
            break;

        }
    }

    return 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月27日
Function        : Ogre_TCP_Svc_Handler::handle_output
Return          : int
Parameter List  :
  Param1: ZEN_HANDLE
Description     : ACE读取,断连的事件触发处理函数
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::handle_output(ZEN_HANDLE)
{

    //如果NON BLOCK Connect成功,也会调用handle_output
    if ( PEER_STATUS_NOACTIVE == peer_status_)
    {

        //处理连接后的事宜
        process_connect_register();
        return 0;
    }

    //
    int ret =  write_all_aata_to_peer();

    //出现错误,
    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        //为什么我不处理错误呢,不return -1,因为如果错误会关闭Socket,handle_input将被调用,这儿不重复处理
        //如果是中断等错误,程序可以继续的.
        return -1;
    }

    return 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年12月20日
Function        : Ogre_TCP_Svc_Handler::handle_timeout
Return          : int
Parameter List  :
  Param1: const ZEN_Time_Value& * time  时间,
  Param2: const void* arg               唯一标示参数
Description     : 定时触发
Calls           :
Called By       :
Other           : ACE为什么不直接使用TIME ID,
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::handle_timeout(const ZEN_Time_Value &/*time*/, const void *arg)
{
    const int timeid = *(static_cast<const int *>(arg));

    //连接上3秒没有反应
    if (TCPCTRL_TIME_ID[0] == timeid)
    {
        //如果有受到数据，那么什么也不做
        if ( receive_times_ > 0)
        {
            receive_times_ = 0;
        }
        //如果没有收到数据,跳楼自杀
        else
        {
            ZLOG_ERROR( "Timeout event,[%s|%u] connect or Recive expire,want to close handle. recieve_counter=%u\n",
                        remote_address_.get_host_addr(),
                        remote_address_.get_port_number(),
                        receive_times_);
            //原来是在这个地方reutrn -1,现在发现return -1是一个很消耗的事情,(定时器的取消会使用指针的方式,会遍历所有的数据)
            //所以在这儿直接调用handle_close
            handle_close();
            return 0;
        }
    }
    else if (TCPCTRL_TIME_ID[1] == timeid)
    {
    }

    return 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年12月20日
Function        : Ogre_TCP_Svc_Handler::handle_close
Return          : int
Parameter List  :
Description     : PEER Event Handler关闭的处理
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::handle_close ()
{
    //不要使用cancel_timer(this),其繁琐,而且慢,好要new,而且有一个不知名的死机

    //取消此Event Handler相关的定时器
    if ( -1 != timeout_time_id_  )
    {
        timer_queue()->cancel_timer(timeout_time_id_);
        timeout_time_id_ = -1;
    }

    //取消所有的MASK,最后阶段,避免调用handle_close,只调用一次
    reactor()->remove_handler(this, false);

    //关闭端口,
    socket_peer_.close();

    //释放接收数据缓冲区
    if (rcv_buffer_)
    {
        Ogre_Buffer_Storage::instance()->free_byte_buffer(rcv_buffer_);
        rcv_buffer_ = NULL;
    }

    //处理发送数据缓冲区
    size_t sz_of_deque = snd_buffer_deque_.size();

    for (size_t i = 0; i < sz_of_deque; i++)
    {
        //处理发送错误队列,同时进行回收
        process_senderror(snd_buffer_deque_[i]);
        snd_buffer_deque_[i] = NULL;
    }

    snd_buffer_deque_.clear();

    //如果服务是激活状态，或者是主动连接的服务.
    if (peer_status_ == PEER_STATUS_ACTIVE || handler_mode_ == HANDLER_MODE_CONNECT)
    {
        Socket_Peer_Info peer_svr_info(remote_address_);
        //注销这些信息
        svr_peer_info_set_.del_services_peerinfo(peer_svr_info);

    }

    peer_status_ = PEER_STATUS_NOACTIVE;

    //根据不同的类型计数,

    //不进行主动重新连接,如果有一个新的数据要发送时主动重新连接
    if (handler_mode_ == HANDLER_MODE_CONNECT)
    {
        ZLOG_ERROR( "Close event,[%s|%u] connect peer socket close .\n",
                    remote_address_.get_host_addr(),
                    remote_address_.get_port_number());

        //将指针归还到池子中间去
        pool_of_cnthdl_.push_back(this);
        --num_connect_peer_ ;
    }
    else if (handler_mode_ == HANDLER_MODE_ACCEPTED)
    {
        ZLOG_INFO( "Close event,[%s|%u] accept peer socket close.\n",
                   remote_address_.get_host_addr(),
                   remote_address_.get_port_number());

        //将指针归还到池子中间去
        pool_of_acpthdl_.push_back(this);
        --num_accept_peer_;
    }

    return 0;
}

//返回端口的状态,
Ogre_TCP_Svc_Handler::PEER_STATUS  Ogre_TCP_Svc_Handler::get_peer_status()
{
    return peer_status_;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月27日
Function        : Ogre_TCP_Svc_Handler::process_connect_register
Return          : int
Parameter List  : NULL
Description     : 处理注册发送,
Calls           :
Called By       :
Other           : 刚刚连接上对方,发送一个注册信息给对方.如果有命令发送命令
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::process_connect_register()
{

    peer_status_ = PEER_STATUS_ACTIVE;

    //再折腾了我至少3天以后，终于发现了EPOLL反复触发写事件的原因是没有取消CONNECT_MASK
    reactor()->cancel_wakeup(this, ZEN_Event_Handler::CONNECT_MASK);

    //如果有数据要发送
    if (snd_buffer_deque_.empty() != true )
    {
        reactor()->schedule_wakeup(this, ZEN_Event_Handler::WRITE_MASK | ZEN_Event_Handler::READ_MASK);
    }
    //只读取
    else
    {
        reactor()->schedule_wakeup(this, ZEN_Event_Handler::READ_MASK);
    }

    //打印信息
    const size_t TMP_ADDR_LEN = 32;
    char local_addr[TMP_ADDR_LEN], remote_addr[TMP_ADDR_LEN];
    strncpy(local_addr, local_address_.get_host_addr(), TMP_ADDR_LEN);
    strncpy(remote_addr, remote_address_.get_host_addr(), TMP_ADDR_LEN);
    ZLOG_INFO( "Local peer[%s|%u] auto connect [%s|%u] success,.\n",
               local_addr,
               local_address_.get_port_number(),
               remote_addr,
               remote_address_.get_port_number()
             );
    return TSS_RET::TSS_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月15日
Function        : Ogre_TCP_Svc_Handler::read_data_from_peer
Return          : int
Parameter List  :
  Param1: size_t& szrevc
Description     : 从PEER读取数据
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::read_data_from_peer(size_t &szrevc)
{
    szrevc = 0;

    ssize_t recvret = 0;

    //申请分配一个内存
    if (rcv_buffer_ == NULL)
    {
        rcv_buffer_ = Ogre_Buffer_Storage::instance()->allocate_byte_buffer();
        rcv_buffer_->snd_peer_info_.SetSocketPeerInfo(remote_address_);
        rcv_buffer_->rcv_peer_info_.SetSocketPeerInfo(local_address_);
        //
        rcv_buffer_->ogre_frame_option_ = 0;
        rcv_buffer_->ogre_frame_option_ |= Ogre4a_AppFrame::OGREDESC_PEER_TCP;
    }

    //
    size_t data_len = rcv_buffer_->ogre_frame_len_ - Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD ;

    if (data_len < Ogre4a_AppFrame::MAX_OF_OGRE_DATA_LEN)
    {
        recvret = socket_peer_.recv(rcv_buffer_->frame_data_ + data_len,
                                    Ogre4a_AppFrame::MAX_OF_OGRE_FRAME_LEN - rcv_buffer_->ogre_frame_len_,
                                    0);
    }
    //在最大的包内都没有接受完整数据
    else
    {
        ZLOG_ERROR( "Read error[%s|%u],Buffer size is not enought or this is a error (attack) data.\n",
                    remote_address_.get_host_addr(),
                    remote_address_.get_port_number()
                  );
        return TSS_RET::ERR_OGRE_SOCKET_CLOSE;
    }

    //表示被关闭或者出现错误
    if (recvret < 0)
    {
        //我只使用EWOULDBLOCK 但是要注意EAGAIN, ZEN_OS::last_error() != EWOULDBLOCK && ZEN_OS::last_error() != EAGAIN
        if (ZEN_OS::last_error() != EWOULDBLOCK )
        {
            szrevc = 0;

            //遇到中断,等待重入
            if (ZEN_OS::last_error() == EINVAL)
            {
                return TSS_RET::TSS_RET_SUCC;
            }

            //记录错误,返回错误
            ZLOG_ERROR( "Read error,[%s|%u] receive data error peer:%u ZEN_OS::last_error()=%d|%s.\n",
                        remote_address_.get_host_addr(),
                        remote_address_.get_port_number(),
                        socket_peer_.get_handle(),
                        ZEN_OS::last_error(),
                        strerror(ZEN_OS::last_error()));

            return TSS_RET::ERR_OGRE_SOCKET_OP_ERROR;
        }

        //如果错误是阻塞,什么都不作
        return TSS_RET::TSS_RET_SUCC;
    }

    //Socket被关闭，也返回错误标示
    if (recvret == 0)
    {
        return TSS_RET::ERR_OGRE_SOCKET_CLOSE;
    }

    //此时RET应该> 0
    szrevc = recvret;

    //接收了N个字符
    rcv_buffer_->ogre_frame_len_ += static_cast<unsigned int>(szrevc) ;
    recieve_bytes_ +=  static_cast<size_t>(szrevc);

    return TSS_RET::TSS_RET_SUCC;

}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月15日
Function        : Ogre_TCP_Svc_Handler::write_data_to_peer
Return          : int
Parameter List  :
  Param1: size_t& szsend
  Param2: bool& bfull
Description     : 向PEER写数据
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::write_data_to_peer(size_t &szsend, bool &if_full)
{
    if_full = false;
    szsend = 0;

    //为什么加上这段呢，原因是有些时候注销写标志位会失败，这个为什么很难衡量
    //#if defined DEBUG || defined _DEBUG
    //如果没有数据要发送, 到这儿应该是有问题
    if (snd_buffer_deque_.empty() == true)
    {
        ZLOG_ERROR( "Write error,[%s|%u] goto handle_output|write_data_to_peer ,but not data to send. Please check you code.\n",
                    remote_address_.get_host_addr(),
                    remote_address_.get_port_number()
                  );
        reactor()->cancel_wakeup (this, ZEN_Event_Handler::WRITE_MASK);
        return TSS_RET::TSS_RET_SUCC;
    }

    //#endif //#if defined DEBUG || defined _DEBUG

    //前面有检查,不会越界
    Ogre4a_AppFrame *sndbuffer = snd_buffer_deque_[0];

    ssize_t sendret = socket_peer_.send(sndbuffer->frame_data_ + send_bytes_  ,
                                        (sndbuffer->ogre_frame_len_ - send_bytes_ - Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD));

    if (sendret < 0)
    {
        int last_error = ZEN_OS::last_error();

        szsend = 0;
        //后面应该会打印方的IP，这儿不重复
        ZLOG_ERROR( "Write error[%s|%u],send data error. peer:%d errno=%u|%s \n",
                    remote_address_.get_host_addr(),
                    remote_address_.get_port_number(),
                    socket_peer_.get_handle(),
                    last_error,
                    strerror(last_error));

        //EINVAL:遇到中断,等待重入的判断是if (ZEN_OS::last_error() == EINVAL),但这儿不仔细检查错误,一视同仁,上层回忽视所有错误,如果错误致命,还会有handle_input反射
        //EWOULDBLOCK:我只使用EWOULDBLOCK 但是要注意EAGAIN ZEN_OS::last_error() != EWOULDBLOCK && ZEN_OS::last_error() != EAGAIN
        if (  EWOULDBLOCK  != last_error && EINVAL != last_error )
        {
            return TSS_RET::ERR_OGRE_SOCKET_OP_ERROR;
        }

        //如果错误是阻塞,什么都不作
        return TSS_RET::TSS_RET_SUCC;
    }

    szsend  = sendret;
    send_bytes_ += static_cast<size_t>(szsend);

    //如果接受的数据
    if (sndbuffer->ogre_frame_len_ - Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD == send_bytes_ )
    {
        if_full = true;
        ZEN_LOGMSG_DBG(RS_DEBUG, "Send a whole frame To  IP|Port :%s|%u FrameLen:%u.\n", remote_address_.get_host_addr(), remote_address_.get_port_number(), send_bytes_);
        send_bytes_ = 0;
    }

    return TSS_RET::TSS_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2008年2月4日
Function        : Ogre_TCP_Svc_Handler::write_all_aata_to_peer
Return          : int
Parameter List  : NULL
Description     : 给力的发送所有要发送的数据，尽自己最大的努力
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::write_all_aata_to_peer()
{
    int ret = 0;

    //在可以发送的是很一次将所有的数据全部发送完成,
    for (;;)
    {
        //发送一个数据包
        size_t szsend ;
        bool   bfull = false;
        int ret =  write_data_to_peer(szsend, bfull);

        //出现错误,
        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return ret;
        }

        //如果数据报已经完整发送
        if ( true == bfull )
        {
            Ogre_Buffer_Storage::instance()->free_byte_buffer(snd_buffer_deque_[0]);
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
    int handle_mask = get_mask();

    //如果队列中没有可以写的数据
    if (snd_buffer_deque_.size() == 0)
    {
        //如果有写标志，取消之
        if ( handle_mask & ZEN_Event_Handler::WRITE_MASK )
        {
            //取消可写的MASK值,
            ret = reactor()->cancel_wakeup(this, ZEN_Event_Handler::WRITE_MASK);

            //return -1表示错误，正确返回的是old mask值
            if ( -1  ==  ret )
            {
                int last_err = ZEN_OS::last_error();
                ZLOG_ERROR( "TNNND cancel_wakeup return(%d) == -1 errno=%d|%s. \n",
                            ret,
                            last_err,
                            strerror(last_err));
                //认为这种情况是出现了问题，关闭之
                return TSS_RET::ERR_OGRE_SOCKET_CLOSE;
            }
        }

        //如果将要关闭
        if (true == if_force_close_ )
        {
            ZLOG_INFO( "Send to peer handle[%u] IP|Port :[%s|%u] complete ,want to close peer on account of frame option.\n",
                       socket_peer_.get_handle(),
                       remote_address_.get_host_addr(),
                       remote_address_.get_port_number());
            //让上层去关闭，要小心，小心，很麻烦，很多生命周期的问题,因为有两个地方调用这个函数
            return TSS_RET::ERR_OGRE_SOCKET_CLOSE;
        }
    }
    //如过没有发送成功，全部发送出去，则准备进行写事件
    else
    {
        //没有WRITE MASK，准备增加写标志
        if (!(handle_mask & ZEN_Event_Handler::WRITE_MASK))
        {
            ret = reactor()->schedule_wakeup(this, ZEN_Event_Handler::WRITE_MASK);

            //schedule_wakeup 返回return -1表示错误，再次BS ACE一次，正确返回的是old mask值
            if ( -1 == ret)
            {
                int last_err = ZEN_OS::last_error();
                ZLOG_ERROR( "TNNND schedule_wakeup return (%d)== -1 errno=%d|%s. \n",
                            ret,
                            last_err,
                            strerror(last_err));
                //认为这种情况是出现了问题，关闭之
                return TSS_RET::ERR_OGRE_SOCKET_CLOSE;
            }
        }
    }

    return TSS_RET::TSS_RET_SUCC;
}

//处理发送错误.
int Ogre_TCP_Svc_Handler::process_senderror(Ogre4a_AppFrame *inner_frame)
{
    int ret = 0;

    //如果命令帧明确表示要进行错误重试,
    if (inner_frame->ogre_frame_option_ & Ogre4a_AppFrame::OGREDESC_SEND_FAIL_RECORD  )
    {
        ////原来一直考虑使用错误管道进行重新发送等处理，现在想想，实在多余，算了
        //// 如果错误管道不为空
        //if(  Zerg_MMAP_BusPipe::instance()->IsExistZergPipe(Zerg_MMAP_BusPipe::ERROR_PIPE_ID) == true)
        //{
        //}
        ZLOG_ERROR( " Peer handle [%u] ,send frame fail.frame len[%u] frame command[%u] frame uin[%u] address[%s|%u],peer status[%u]. \n",
                    socket_peer_.get_handle(),
                    inner_frame->ogre_frame_len_,
                    remote_address_.get_host_addr(),
                    remote_address_.get_port_number(),
                    peer_status_
                  );
    }
    //如果发送命令明确要通知后面的服务,和RETRY互斥
    else if (inner_frame->ogre_frame_option_  & Ogre4a_AppFrame::OGREDESC_SNDPRC_NOTIFY_APP)
    {

        //标示这个帧是发送错误,回交给后面的应用,
        inner_frame->ogre_frame_option_ |= Ogre4a_AppFrame::OGREDESC_SEND_ERROR;

        //日志在函数中有输出,这儿略.
        ret = Zerg_MMAP_BusPipe::instance()->push_back_bus(Zerg_MMAP_BusPipe::RECV_PIPE_ID,
                                                           reinterpret_cast<const ZEN_LIB::dequechunk_node *>(inner_frame));

        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            return TSS_RET::ERR_OGRE_RECEIVE_PIPE_IS_FULL;
        }
    }

    //归还到POOL中间。
    Ogre_Buffer_Storage::instance()->free_byte_buffer(inner_frame);

    return TSS_RET::TSS_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2006年3月22日
Function        : Ogre_TCP_Svc_Handler::get_configure
Return          : int
Parameter List  :
  Param1: const char* cfgfilename 配置文件名称
Description     : 从配置文件读取配置信息
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::get_configure(Zen_INI_PropertyTree &cfg_file)
{
    int ret = 0;
    ret = Zerg_MMAP_BusPipe::instance()->getpara_from_zergcfg(cfg_file);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    unsigned int tmp_uint = 0 ;
    //从CONNECT到收到数据的时长
    ret  = cfg_file.get_uint32_value("COMMCFG", "CONNECTTIMEOUT", tmp_uint);
    connect_timeout_ = tmp_uint;
    TESTCONFIG((ret == 0 && connect_timeout_ <= 50 ), "COMMCFG|CONNECTTIMEOUT key error.");

    //RECEIVE一个数据的超时时间,为0表示不限制
    ret = cfg_file.get_uint32_value("COMMCFG", "RECEIVETIMEOUT", tmp_uint );
    receive_timeout_ = tmp_uint;
    TESTCONFIG((ret == 0 && receive_timeout_ <= 1800 ), "COMMCFG|RECEIVETIMEOUT key error.");

    //最大的链接我的服务器个数
    ret = cfg_file.get_uint32_value("COMMCFG", "MAXACCEPTSVR", tmp_uint);
    max_accept_svr_ = tmp_uint;
    TESTCONFIG((ret == 0 && max_accept_svr_ <= 409600 && max_accept_svr_ > 0), "COMMCFG|MAXACCEPTSVR key error.");

    //错误发送数据尝试发送次数
    ret = cfg_file.get_uint32_value("COMMCFG", "TRYERROR", tmp_uint );
    error_try_num_ = tmp_uint;
    TESTCONFIG((ret == 0 && error_try_num_ < 10), "COMMCFG|TRYERROR key error.");

    //自动链接部分读取配置
    ret = zerg_auto_connect_.get_configure(cfg_file);

    if (ret != 0)
    {
        return ret;
    }

    //最大要链接数量等于自动链接服务的数量
    max_connect_svr_ = zerg_auto_connect_.num_svr_to_connect() ;
    //
    return TSS_RET::TSS_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2006年3月22日
Function        : Ogre_TCP_Svc_Handler::init_all_static_data
Return          : int
Parameter List  : NULL
Description     : 将配置参数初始化
Calls           :
Called By       :
Other           : 一些参数从配置类读取,避免后面的操作还要访问配置类
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::init_all_static_data()
{
    int ret = 0;

    //都设置一个最小值
    max_accept_svr_ += 8;
    max_connect_svr_ += 8;
    ZLOG_INFO("MaxAcceptSvr:%u MaxConnectSvr:%u.\n", max_accept_svr_, max_connect_svr_);

    //为CONNECT的HDL预先分配内存，成为一个池子
    ZLOG_INFO( "Connet Hanlder:size of Ogre_TCP_Svc_Handler [%u],one connect handler have deqeue length [%u],number of connect handler [%u]."
               "About need  memory [%u] bytes.\n",
               sizeof(Ogre_TCP_Svc_Handler),
               MAX_OF_CONNECT_PEER_SEND_DEQUE,
               max_connect_svr_,
               (max_connect_svr_ * (sizeof(Ogre_TCP_Svc_Handler) + MAX_OF_CONNECT_PEER_SEND_DEQUE * sizeof(size_t)))
             );
    pool_of_cnthdl_.initialize(max_connect_svr_);

    for (size_t i = 0; i < max_connect_svr_; ++i)
    {
        Ogre_TCP_Svc_Handler *p_handler = new Ogre_TCP_Svc_Handler(HANDLER_MODE_CONNECT);
        pool_of_cnthdl_.push_back(p_handler);
    }

    //为ACCEPT的HDL预先分配内存，成为一个池子
    ZLOG_INFO( "Accept Hanlder:size of Ogre_TCP_Svc_Handler [%u],one accept handler have deqeue length [%u],number of accept handler [%u]."
               "About need  memory [%u] bytes.\n",
               sizeof(Ogre_TCP_Svc_Handler),
               MAX_OF_ACCEPT_PEER_SEND_DEQUE,
               max_accept_svr_,
               (max_accept_svr_ * (sizeof(Ogre_TCP_Svc_Handler) + MAX_OF_ACCEPT_PEER_SEND_DEQUE * sizeof(size_t)))
             );
    pool_of_acpthdl_.initialize(max_accept_svr_ );

    for (size_t i = 0; i < max_accept_svr_; ++i)
    {
        Ogre_TCP_Svc_Handler *p_handler = new Ogre_TCP_Svc_Handler(HANDLER_MODE_ACCEPTED);
        pool_of_acpthdl_.push_back(p_handler);
    }

    //初始化所有的端口对应表,
    svr_peer_info_set_.init_services_peerinfo(max_accept_svr_ + max_connect_svr_ + 64);

    //连接所有的SERVER,如果有严重错误退出
    size_t sz_of_svr = 0, sz_of_succ = 0;
    ret = zerg_auto_connect_.connect_all_server(sz_of_svr, sz_of_succ);
    ZLOG_INFO( "Have %u server to auto connect ,success %u ,ret =%d.\n",
               sz_of_svr, sz_of_succ, ret);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    return TSS_RET::TSS_RET_SUCC;
}

//
int Ogre_TCP_Svc_Handler::unInit_all_static_data()
{
    //
    svr_peer_info_set_.clear_and_close();
    return TSS_RET::TSS_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2007年12月24日
Function        : Ogre_TCP_Svc_Handler::alloc_svchandler_from_pool
Return          : Ogre_TCP_Svc_Handler*
Parameter List  :
Param1: OGRE_HANDLER_MODE handler_mode
Description     : 从池子里面得到一个Handler给大家使用
Calls           :
Called By       :
Other           : Connect的端口应该永远不发生取不到Hanler的事情
Modify Record   :
******************************************************************************************/
Ogre_TCP_Svc_Handler *Ogre_TCP_Svc_Handler::alloc_svchandler_from_pool(OGRE_HANDLER_MODE handler_mode)
{
    //
    if (handler_mode == HANDLER_MODE_ACCEPTED )
    {
        if (pool_of_acpthdl_.size() == 0)
        {
            ZLOG_INFO( "Pool is too small to process accept handler,please notice.Pool size:%u,capacity:%u.\n",
                       pool_of_acpthdl_.size(),
                       pool_of_acpthdl_.capacity()
                     );
            return NULL;
        }

        Ogre_TCP_Svc_Handler *p_handler = NULL;
        pool_of_acpthdl_.pop_front(p_handler);
        return p_handler;
    }
    //Connect的端口应该永远不发生取不到Hanler的事情
    else if ( HANDLER_MODE_CONNECT == handler_mode )
    {
        ZEN_ASSERT(pool_of_cnthdl_.size() > 0);
        Ogre_TCP_Svc_Handler *p_handler = NULL;
        pool_of_cnthdl_.pop_front(p_handler);
        return p_handler;
    }
    //Never go here.
    else
    {
        ZEN_ASSERT(false);
        return NULL;
    }
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月15日
Function        : Ogre_TCP_Svc_Handler::process_send_data
Return          : int
Parameter List  :
    Param1: ZByteBuffer* tmpbuf   要得到数据的ZByteBuffer,要求分配好,
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::process_send_data(Ogre4a_AppFrame *ogre_frame )
{

    int ret = 0;

    //由于静态数组的关系，不能做两个参数
    const size_t TMP_IP_ADDRESS_LEN = 32;
    char remote_ip_str[TMP_IP_ADDRESS_LEN], local_ip_str[TMP_IP_ADDRESS_LEN];

    Socket_Peer_Info svrinfo = ogre_frame->rcv_peer_info_;

    Ogre_TCP_Svc_Handler *svchanle = NULL;
    ret = svr_peer_info_set_.find_services_peerinfo(svrinfo, svchanle);

    //如果是要重新进行连接的服务器主动主动连接,
    if ( ret != 0 )
    {
        //为什么不把一个Ogre_TCP_Svc_Handler作为返回,因为在发起Connect过程中,也可能handle_close.
        ret = zerg_auto_connect_.connect_server_by_svrinfo(svrinfo);

        //这个地方是一个Double Check,如果发起连接成功
        if (ret == TSS_RET::TSS_RET_SUCC)
        {
            ret = svr_peer_info_set_.find_services_peerinfo(svrinfo, svchanle);
        }
    }

    //如果SVCHANDLE为空,表示没有相关的连接,进行错误处理
    if (svchanle == NULL)
    {

        //无法发送
        ZLOG_INFO( "Can't find handle remote address[%s|%u],send fail ,local address [%s|%u],frame len[%u].\n",
                   ZEN_OS::inet_ntoa(ogre_frame->rcv_peer_info_.peer_ip_address_, remote_ip_str, TMP_IP_ADDRESS_LEN),
                   ogre_frame->rcv_peer_info_.peer_port_,
                   ZEN_OS::inet_ntoa(ogre_frame->snd_peer_info_.peer_ip_address_, local_ip_str, TMP_IP_ADDRESS_LEN),
                   ogre_frame->snd_peer_info_.peer_port_,
                   ogre_frame->ogre_frame_len_
                 );
        return TSS_RET::ERR_OGRE_SEND_FRAME_FAIL;
    }

    //如果是通知关闭端口
    if (ogre_frame->ogre_frame_option_ & Ogre4a_AppFrame::OGREDESC_CLOSE_PEER )
    {
        ZLOG_INFO( "Recvice DESC_CLOSE_PEER,Svchanle will close, svrinfo [IP|Port:%s|%u ].\n",
                   ZEN_OS::inet_ntoa(svrinfo.peer_ip_address_, local_ip_str, TMP_IP_ADDRESS_LEN),
                   svrinfo.peer_port_);
        //如果不是UDP的处理,关闭端口,UDP的东西没有链接的概念,
        svchanle->handle_close();
        return TSS_RET::ERR_OGRE_SOCKET_CLOSE;
    }

    //将发送的FRAME给HANDLE对象，当然这个地方未必一定放的进去，因为有几种情况,
    //1.就是一个关闭指令,
    //2.HANDLE内部的队列满了,

    svchanle->put_frame_to_sendlist(ogre_frame);

    return TSS_RET::TSS_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2006年3月22日
Function        : Ogre_TCP_Svc_Handler::put_frame_to_sendlist
Return          : int
Parameter List  :
Param1: ZByteBuffer* tmpbuf
Description     : 将发送数据放入发送队列中
Calls           :
Called By       :
Other           : 如果一个PEER没有连接上,等待发送的数据不能多于PEER_STATUS_NOACTIVE个
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::put_frame_to_sendlist(Ogre4a_AppFrame *ogre_frame)
{
    int ret = 0;

    //对于一个没有连接上的PEER,等待连接队列的数据不能过多,
    if (peer_status_ == PEER_STATUS_NOACTIVE &&
        snd_buffer_deque_.size() >= MAX_LEN_OF_SEND_LIST)
    {
        //丢弃或者错误处理那个数据比较好呢?这儿值得商榷, 我这儿进行错误处理(可能丢弃)的是最新的.
        //我的考虑是如果命令有先后性.而且可以避免内存操作.
        process_senderror(ogre_frame);
        return TSS_RET::ERR_OGRE_SEND_FRAME_FAIL;
    }

    //如果发送完成,并且后台业务要求关闭端口,注意必须转换网络序
    if ( ogre_frame->ogre_frame_option_ & Ogre4a_AppFrame::OGREDESC_SNDPRC_CLOSE_PEER)
    {
        ZLOG_INFO( "This Peer handle[%u] IP|Port :[%s|%u] complete ,will close when all frame send complete ,because send frame has option Ogre4a_AppFrame::OGREDESC_SNDPRC_CLOSE_PEER.\n",
                   socket_peer_.get_handle(),
                   remote_address_.get_host_addr(),
                   remote_address_.get_port_number());
        if_force_close_ = true;
    }

    //放入发送队列,
    bool bret = snd_buffer_deque_.push_back(ogre_frame);

    if ( false == bret )
    {
        //丢弃或者错误处理那个数据比较好呢?这儿值得商榷, 我这儿进行错误处理(可能丢弃)的是最新的.
        //我的考虑是如果命令有先后性.而且可以避免内存操作.
        ZLOG_ERROR( "Peer handle [%u] IP|Port[%s|%u] send buffer cycle deque is full,this data must throw away,Send deque capacity =%u,may be extend it.\n",
                    socket_peer_.get_handle(),
                    remote_address_.get_host_addr(),
                    remote_address_.get_port_number(),
                    snd_buffer_deque_.capacity());

        //回收帧
        process_senderror(ogre_frame);
        //返回一个错误
        return TSS_RET::ERR_OGRE_SEND_FRAME_FAIL;
    }

    //尝试发送这个数据包

    if (peer_status_ != PEER_STATUS_NOACTIVE)
    {

        ret = write_all_aata_to_peer();

        //出现错误,
        if (ret != TSS_RET::TSS_RET_SUCC)
        {
            //这儿已经进行了调整，坚决关闭之，对于中断错误在上层已经转化了错误
            handle_close();

            //发送数据已经放入队列，返回OK
            return TSS_RET::TSS_RET_SUCC;
        }
    }

    //合并,合并有利于高效处理，对于端口短暂的阻塞有好处
    unite_frame_sendlist();

    //只有放入发送队列才算成功.
    return TSS_RET::TSS_RET_SUCC;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2008年1月23日
Function        : Ogre_TCP_Svc_Handler::unite_frame_sendlist
Return          : void
Parameter List  : NULL
Description     : 合并发送队列
Calls           :
Called By       :
Other           : 如果有2个以上的的发送队列，则可以考虑合并处理
Modify Record   :
******************************************************************************************/
void Ogre_TCP_Svc_Handler::unite_frame_sendlist()
{
    //如果有2个以上的的发送队列，则可以考虑合并处理
    size_t sz_deque = snd_buffer_deque_.size();

    if ( sz_deque <= 1)
    {
        return;
    }

    //如果倒数第2个桶有能力放下倒数第1个桶的FRAME数据，则进行合并操作。
    if ( (Ogre4a_AppFrame::MAX_OF_OGRE_FRAME_LEN - snd_buffer_deque_[sz_deque - 2]->ogre_frame_len_) >  \
         (snd_buffer_deque_[sz_deque - 1]->ogre_frame_len_ - Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD))
    {
        //将倒数第1个节点的数据放入倒数第2个节点中间。所以实际的Cache能力是非常强的，
        //空间利用率也很高。越发佩服我自己了。
        memcpy((snd_buffer_deque_[sz_deque - 2]->frame_data_ + snd_buffer_deque_[sz_deque - 2]->ogre_frame_len_ - Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD),
               snd_buffer_deque_[sz_deque - 1]->frame_data_,
               (snd_buffer_deque_[sz_deque - 1]->ogre_frame_len_ - Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD));

        snd_buffer_deque_[sz_deque - 2]->ogre_frame_len_ += (snd_buffer_deque_[sz_deque - 1]->ogre_frame_len_ - Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD);

        //将倒数第一个施放掉
        Ogre_Buffer_Storage::instance()->free_byte_buffer(snd_buffer_deque_[sz_deque - 1]);
        snd_buffer_deque_[sz_deque - 1] = NULL;
        snd_buffer_deque_.pop_back();
    }

    ////下面的代码用于合并的测试，平常会注释掉
    //else
    //{
    //    ACE_DEBUGEX((LM_DEBUG,"Goto unite_frame_sendlist sz_deque=%u,Ogre4a_AppFrame::MAX_OF_OGRE_FRAME_LEN=%u,"
    //        "snd_buffer_deque_[sz_deque-2]->ogre_frame_len_=%u,"
    //        "snd_buffer_deque_[sz_deque-1]->ogre_frame_len_=%u\n",
    //        sz_deque,
    //        Ogre4a_AppFrame::MAX_OF_OGRE_FRAME_LEN,
    //        snd_buffer_deque_[sz_deque-2]->ogre_frame_len_,
    //        snd_buffer_deque_[sz_deque-1]->ogre_frame_len_));
    //}

}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2006年3月22日
Function        : Ogre_TCP_Svc_Handler::push_frame_to_recvpipe
Return          : int
Parameter List  : NULL
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_TCP_Svc_Handler::push_frame_to_recvpipe(unsigned int sz_data)
{

    int ret = Zerg_MMAP_BusPipe::instance()->push_back_bus(Zerg_MMAP_BusPipe::RECV_PIPE_ID,
                                                           reinterpret_cast<ZEN_LIB::dequechunk_node *>(rcv_buffer_));

    //还收到了后面一个帧的数据,
    if (rcv_buffer_->ogre_frame_len_ > sz_data + Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD )
    {
        memmove(rcv_buffer_->frame_data_,
                rcv_buffer_->frame_data_ + sz_data ,
                rcv_buffer_->ogre_frame_len_ - sz_data - Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD );

        //改变buffer长度
        rcv_buffer_->ogre_frame_len_ = rcv_buffer_->ogre_frame_len_ - sz_data;

    }
    else if (rcv_buffer_->ogre_frame_len_ == sz_data + Ogre4a_AppFrame::LEN_OF_OGRE_FRAME_HEAD)
    {
        //无论处理正确与否,都释放缓冲区的空间
        Ogre_Buffer_Storage::instance()->free_byte_buffer(rcv_buffer_);
        rcv_buffer_ = NULL;
    }
    //代码错误
    else
    {
        ZEN_ASSERT(false);
    }

    //无论处理正确与否,都释放缓冲区的空间

    //日志在函数中有输出,这儿略.
    if (ret != TSS_RET::TSS_RET_SUCC )
    {
        return TSS_RET::ERR_OGRE_RECEIVE_PIPE_IS_FULL;
    }

    return TSS_RET::TSS_RET_SUCC;
}

//
void Ogre_TCP_Svc_Handler::get_maxpeer_num(size_t &maxaccept, size_t &maxconnect)
{
    maxaccept = max_accept_svr_;
    maxconnect = max_connect_svr_;
}

//得到Handle对应PEER的端口
unsigned short Ogre_TCP_Svc_Handler::get_peer_port()
{
    return remote_address_.get_port_number();
}
//得到Handle对应PEER的IP地址
const char *Ogre_TCP_Svc_Handler::get_peer_address()
{
    return remote_address_.get_host_addr();
}

