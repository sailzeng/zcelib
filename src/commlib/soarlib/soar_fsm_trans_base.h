/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_fsm_trans_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006年3月29日
* @brief
*
*
* @details
*
*
*
* @note       为了让TRANSACTION自动注册,自动销毁,让TRANSACTION保留了MANAGER的指针
*             而且是静态的,这个想法其实限制了部分行为.好不好?
*             2010年3月22日
*             这个类也写了4年了,能坚持4年，不容易，而且今天是Google离开中国的第3天，无奈，无语中
*/

#ifndef SOARING_LIB_TRANSACTION_BASE_H_
#define SOARING_LIB_TRANSACTION_BASE_H_

#include "soar_zerg_frame.h"
#include "soar_fsm_trans_mgr.h"
#include "soar_error_code.h"

class Zerg_App_Frame;
class Soar_MMAP_BusPipe;

//本来打算封装几个静态变量为STATIC的,但是如果这样其实限制了整体

class SOARING_EXPORT Transaction_Base : public ZCE_Async_FSM
{
    friend class Transaction_Manager;
protected:


    enum WAIT_TIMEOUT_VALUE
    {
        TOO_SHORT_SOARING_LIB_TIME_OUT         = 5,
        SHORT_SOARING_LIB_TIME_OUT             = 10,
        NORMAL_SOARING_LIB_TIME_OUT            = 15,
        MIDDING_SOARING_LIB_TIME_OUT           = 30,
        LONG_SOARING_LIB_TIME_OUT              = 60,
        LONG_LONG_SOARING_LIB_TIME_OUT         = 120,
        TOO_LONG_SOARING_LIB_TIME_OUT          = 240,
        TOO_TOO_LONG_TIME_OUT               = 480,
    };



protected:

    //私有的构造函数,你必须使用NEW得到
    virtual ~Transaction_Base();

public:
    //构造函数
    Transaction_Base(Transaction_Manager *pmngr,
                     bool trans_locker = false);

    //每次重新进行初始化时候的事情，等等，尽量保证基类的这个函数优先调用，类似构造函数
    virtual void on_start() = 0;
    //回收后的处理，用于资源的释放，等等，尽量保证基类的这个函数最后调用，类似析构函数。
    virtual void finish();

    //退出的调用的函数.
    virtual int on_exit();

    inline unsigned int get_wait_cmd() const;

    //得到请求的IP地址。本地序号
    inline unsigned int get_req_ipaddress() const;
    //得到请求的命令
    inline unsigned int get_req_cmd() const;

    //得到管理器
    inline  Transaction_Manager *get_trans_mgr() const;

    //发送命令接口暴露出来,主要是考虑到万一有你有其他地方要保存Trans指针，然后使用
public:

    //--------------------------------------------------------------------
    //下面这组函数务必不要混合使用,回送就是用于回送,发送给其他服务器就用send_msg_to,request_send_to_proxy

    //请求发送数据到SERVICES_ID服务器(不能是Proxy),使用请求的UIN，
    template< class T> int request_send_to_peer(unsigned int cmd,
                                                const SERVICES_ID &rcv_svc,
                                                const T &info,
                                                unsigned int app_id = 0,
                                                unsigned int option = 0);

    //请求发送数据到SERVICES_ID服务器(不能是Proxy),指定特定UIN,赠送这些命令使用
    template< class T> int request_send_to_peer(unsigned int cmd,
                                                unsigned int qquin,
                                                const SERVICES_ID &rcv_svc,
                                                const T &info,
                                                unsigned int app_id = 0,
                                                unsigned int option = 0 );

    //请求发送数据到SERVICES_ID服务器,可以指定UIN和，回填的事务ID
    template <class T> int request_send_to_peer(unsigned int cmd,
                                                unsigned int qquin,
                                                unsigned int backfill_trans_id,
                                                const SERVICES_ID &rcv_svc,
                                                const T &info,
                                                unsigned int app_id = 0,
                                                unsigned int option = 0
                                               );

    // 请求发送数据到SERVICES_ID服务器(不能是Proxy),指定特定UIN,赠送这些命令使用
    int request_send_buf_to_peer(unsigned int cmd,
                                 unsigned int qquin,
                                 const SERVICES_ID &rcv_svc,
                                 const unsigned char *buf,
                                 unsigned int buf_len,
                                 unsigned int app_id = 0,
                                 unsigned int option = 0
                                );

    // 请求发送数据到proxy服务器, 指定特定UIN,命令字
    int request_send_buf_to_proxy(unsigned int cmd,
                                  unsigned int qquin,
                                  const SERVICES_ID &proxy_svc,
                                  const SERVICES_ID &recv_svc,
                                  const unsigned char *buf,
                                  unsigned int buf_len,
                                  unsigned int app_id = 0,
                                  unsigned int option = 0
                                 );

    //--------------------------------------------------------------------------------------
    //请求发送数据到代理服务器,用请求的QQUIN发送
    template< class T> int request_send_to_proxy(unsigned int cmd,
                                                 const SERVICES_ID &proxy_svc,
                                                 unsigned short rcvtype,
                                                 const T &info,
                                                 unsigned int app_id = 0,
                                                 unsigned int option = 0 );

    //请求发送数据到代理服务器,用指定UIN发送
    template< class T> int request_send_to_proxy(unsigned int cmd,
                                                 unsigned int qquin,
                                                 const SERVICES_ID &proxy_svc,
                                                 unsigned short rcvtype,
                                                 const T &info,
                                                 unsigned int app_id = 0,
                                                 unsigned int option = 0);

    //请求发送数据到代理服务器,,使用请求的QQUIN,RCV SERVICESID(比如用代理中转到指定服务器),
    template< class T> int request_send_to_proxy(unsigned int cmd,
                                                 unsigned int qquin,
                                                 const SERVICES_ID &proxy_svc,
                                                 const SERVICES_ID &recvsvc,
                                                 const T &info,
                                                 unsigned int app_id = 0,
                                                 unsigned int option = 0);

    //请求发送数据到代理服务器,,使用请求的QQUIN,,RCV SERVICESID(比如用代理中转到指定服务器),回填的事务ID
    template< class T> int request_send_to_proxy(unsigned int cmd,
                                                 unsigned int qquin,
                                                 unsigned int backfill_trans_id,
                                                 const SERVICES_ID &proxy_svc,
                                                 const SERVICES_ID &recvsvc,
                                                 const T &info,
                                                 unsigned int app_id = 0,
                                                 unsigned int option = 0);

    //--------------------------------------------------------------------------------------
    //应答，向回发送数据,只能是除了接受命令的时候才可以调用这个函数,否则....
    template< class T> int response_sendback(unsigned int cmd,
                                             const T &info,
                                             unsigned int option = 0);

    //回送信息,应答一个请求,但是可以指定回应的QQUIN,只能是除了接受命令的时候才可以调用这个函数,否则....
    //比如发送邮件，FRAME中的主人是收件人,但是返回命令应该是发件人
    //为啥要用2作为函数名称而不重载呢，你可以考虑一下,我不作答
    template< class T> int response_sendback2(unsigned int cmd,
                                              unsigned int qquin,
                                              const T &info,
                                              unsigned int option = 0);

    //应答，向回发送数据,数据是一段buf,这段buf须是一个完整的框架协议包
    // 这个函数与response_sendback2的区别是info被打包成了buf,
    // 一般只在你要回的这个buf,你自己解不出来时才使用，慎用
    int response_buf_sendback(unsigned int cmd,
                              unsigned int uin,
                              const unsigned char *buf,
                              unsigned int buf_len,
                              unsigned int option = 0);

    //--------------------------------------------------------------------------------------
    //奉劝大家不要用这个，最大集合,除了了dir的版本升级以外
    template< class T> int sendmsg_to_service(unsigned int cmd,
                                              unsigned int qquin,
                                              unsigned int transaction_id,
                                              unsigned int backfill_trans_id,
                                              const SERVICES_ID &rcv_svc,
                                              const SERVICES_ID &proxy_svc,
                                              const SERVICES_ID &snd_svc,
                                              const T &tmpvalue,
                                              unsigned int app_id,
                                              unsigned int option );

    // 发送数据到SERVICES_ID服务器,最大集合, 一般用封装过的函数
    int sendbuf_to_service(unsigned int cmd,
                           unsigned int qquin,
                           unsigned int transaction_id,
                           unsigned int backfill_trans_id,
                           const SERVICES_ID &rcv_svc,
                           const SERVICES_ID &proxy_svc,
                           const SERVICES_ID &snd_svc,
                           const unsigned char *buf,
                           size_t buf_len,
                           unsigned int app_id,
                           unsigned int option);

protected:

    //初始化
    int initialize_trans(Zerg_App_Frame *proc_frame);

    //关闭
    virtual int handle_close ();

    //处理事务的事件
    int process_trans_event();

    //lock其实不是真正的锁，目的是保证在同一时刻，只处理一个用户的一个请求。
    //对当前用户的，当前事务命令字进行加锁
    int lock_qquin_cmd();
    //对当前用户的，当前事务命令字进行解锁
    void unlock_qquin_cmd();

    //对当前用户的一个锁ID进行加锁
    int lock_qquin_key(unsigned int trnas_lock_id);
    //对当前用户的一个锁ID进行解锁
    void unlock_qquin_key(unsigned int trnas_lock_id);

    //设置事务阶段
    inline void set_phase(int transphase);


    //设置事务需要检查的命令(也就是明确他要等待的命令是什么)
    inline void set_wait_cmd(unsigned int need_cmd = CMD_INVALID_CMD);
    //检查接受到的FRAME的数据和命令
    virtual int check_receive_frame(const Zerg_App_Frame *recv_frame);
    //收到一个数据后
    virtual int receive_trans_msg(Zerg_App_Frame *proc_frame);

protected:

    //关闭请求的的Service
    int close_request_service() const;

    //检查用户的数据帧头部的UIN和数据体的UIN是否一致,避免用户作弊
    inline int check_req_qquin(unsigned int qq_uin) const;

    //用于检查请求的IP地址是否是内部IP地址
    inline int check_request_internal() const;

    //DUMP所有的事物的信息
    void dump_transa_info(std::ostringstream &strstream) const;
    //DUMP输出事务的所有信息
    void output_trans_info(const char *outstr = "Output Transaction Info") const;

public:
    //无效的阶段ID
    static const int INVALID_PHASE_ID = 0;

    //TIME ID
    static const int TRANSACTION_TIME_ID[];

    //这儿对封装要求很严格，但是我感觉对外暴露的接口足够了。
protected:

    //事件管理器
    Transaction_Manager     *trans_manager_;

    //是否加事务锁
    bool                    trans_locker_;

    //请求的事务UID
    unsigned int            req_user_id_;
    //这个事务的命令字
    unsigned int            trans_command_;


    //请求事务的发送者
    SERVICES_ID             req_snd_service_;
    //请求事务的代理
    SERVICES_ID             req_rcv_service_;
    //请求事务的代理
    SERVICES_ID             req_proxy_service_;

    //请求的事务ID
    unsigned int            req_trans_id_;
    //请求的的会话ID:这里是回填事务id，意思变了,其实没怎么使用
    unsigned int            req_session_id_;
    //请求的app_id
    unsigned int            req_game_app_id_;

    //请求的IP地址是什么.
    unsigned int            req_ip_address_;
    //请求的FRAME的的选项
    unsigned int            req_frame_option_;

    //事务超时的定时器ID
    int                     trans_timeout_id_;
    //发生active后，是否自动停止time out定时器
    bool                    active_auto_stop_;
    //事务的定时器ID
    int                     trans_touchtimer_id_;

    //需要检查的命令
    unsigned int            wait_cmd_;
    //事务的创建时间
    time_t                  trans_create_time_;

    //日志跟踪的优先级
    ZCE_LOG_PRIORITY        trace_log_pri_;

    // 事务处理的错误码，当事务处理返回EXIT_PROCESS_FAIL时使用
    int                     process_errno_;
};

//得到CMD
inline unsigned int Transaction_Base::get_req_cmd() const
{
    return trans_command_;
}
//得到需要的CMD
inline unsigned int Transaction_Base::get_wait_cmd() const
{
    return wait_cmd_;
}



//设置事务需要检查的命令
//need_cmd == 0表示不进行命令
inline void Transaction_Base::set_wait_cmd(unsigned int need_cmd)
{
    wait_cmd_ = need_cmd;
}

//得到管理器
inline  Transaction_Manager *Transaction_Base::get_trans_mgr()  const
{
    return trans_manager_;
}

//检测包头和包体的qq_uin以及发送的service_id是否一致,
inline int Transaction_Base::check_req_qquin(unsigned int qq_uin) const
{
    if (qq_uin != req_user_id_)
    {
        ZCE_LOG(RS_ERROR, "[framework] QQuin in header , body and snd_service is diffrent.body: %u| request appframe header: %u.",
                qq_uin, req_user_id_);
        return SOAR_RET::ERROR_QQUIN_INCERTITUDE;
    }

    return 0;
}

//取得请求命令的IP地址信息
inline unsigned int Transaction_Base::get_req_ipaddress() const
{
    return req_ip_address_;
}

//用于检查请求的IP地址是否是内部IP地址,是返回0，不是返回非0
inline int Transaction_Base::check_request_internal() const
{
    return (ZCE_LIB::is_internal(req_ip_address_)) ? 0 : -1;
}

//请求发送消息的Service,
template <class T>
int Transaction_Base::request_send_to_peer(unsigned int cmd,
                                           const SERVICES_ID &rcv_svc,
                                           const T &info,
                                           unsigned int app_id,
                                           unsigned int option)
{
    SERVICES_ID proxy_svc(0, 0);
    return sendmsg_to_service(cmd,
                              req_user_id_,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//请求发送消息的Service,
template <class T>
int Transaction_Base::request_send_to_peer(unsigned int cmd,
                                           unsigned int qquin,
                                           unsigned int backfill_trans_id,
                                           const SERVICES_ID &rcv_svc,
                                           const T &info,
                                           unsigned int app_id,
                                           unsigned int option)
{
    SERVICES_ID proxy_svc(0, 0);
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              backfill_trans_id,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//向某个Service发送数据,同时使用UIN
template< class T>
int Transaction_Base::request_send_to_peer(unsigned int cmd,
                                           unsigned int qquin,
                                           const SERVICES_ID &rcv_svc,
                                           const T &info,
                                           unsigned int app_id,
                                           unsigned int option )
{
    SERVICES_ID proxy_svc(0, 0);
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);

}

//--------------------------------------------------------------------------------------------------------------------------
//请求发送数据到代理服务器,用请求的QQUIN发送
template< class T>
int Transaction_Base::request_send_to_proxy(unsigned int cmd,
                                            const SERVICES_ID &proxy_svc,
                                            unsigned short rcvtype,
                                            const T &info,
                                            unsigned int app_id,
                                            unsigned int option)
{
    SERVICES_ID rcv_svc(rcvtype, 0);
    return sendmsg_to_service(cmd,
                              this->req_user_id_,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//请求发送数据到代理服务器,用参数指定UIN发送
template< class T>
int Transaction_Base::request_send_to_proxy(unsigned int cmd,
                                            unsigned int qquin,
                                            const SERVICES_ID &proxy_svc,
                                            unsigned short rcvtype,
                                            const T &info,
                                            unsigned int app_id,
                                            unsigned int option)
{
    SERVICES_ID rcv_svc(rcvtype, 0);
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//请求发送数据到代理服务器,,使用请求的QQUIN,RCV SERVICESID(比如用代理中转到指定服务器),
//注意RECV SVC 和PROXY SVC参数的顺序,主要事务和
template< class T>
int Transaction_Base::request_send_to_proxy(unsigned int cmd,
                                            unsigned int qquin,
                                            const SERVICES_ID &proxy_svc,
                                            const SERVICES_ID &recvsvc,
                                            const T &info,
                                            unsigned int app_id,
                                            unsigned int option)
{
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              0,
                              recvsvc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//请求发送数据到代理服务器,使用请求的QQUIN,,RCV SERVICESID(比如用代理中转到指定服务器),回填的事务ID
//注意RECV SVC 和PROXY SVC参数的顺序
template< class T>
int Transaction_Base::request_send_to_proxy(unsigned int cmd,
                                            unsigned int qquin,
                                            unsigned int backfill_trans_id,
                                            const SERVICES_ID &recvsvc,
                                            const SERVICES_ID &proxy_svc,
                                            const T &info,
                                            unsigned int app_id,
                                            unsigned int option)
{
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              backfill_trans_id,
                              recvsvc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              app_id,
                              option);
}

//--------------------------------------------------------------------------------------------------------------------------
//回送信息,应答一个请求,只能是除了接受命令的时候才可以调用这个函数,否则....
template< class T>
int Transaction_Base::response_sendback(unsigned int cmd,
                                        const T &info,
                                        unsigned int option)
{

    //加入UDP返回的代码部分
    if (req_frame_option_ & Zerg_App_Frame::DESC_UDP_FRAME)
    {
        option |= Zerg_App_Frame::DESC_UDP_FRAME;
    }

    //
    return sendmsg_to_service(cmd,
                              this->req_user_id_,
                              this->asyncobj_id_,
                              this->req_trans_id_,
                              this->req_snd_service_,
                              this->req_proxy_service_,
                              this->req_rcv_service_,
                              info,
                              this->req_game_app_id_,
                              option);
}

//回送信息,应答一个请求,但是可以指定回应的QQUIN,只能是除了接受命令的时候才可以调用这个函数,否则....
//比如发送邮件，FRAME中的主人是收件人,但是返回命令应该是发件人
//为啥要用2作为函数名称而不重载呢，你可以考虑一下,我不作答
template< class T>
int Transaction_Base::response_sendback2(unsigned int cmd,
                                         unsigned int qquin,
                                         const T &info,
                                         unsigned int option)
{
    //加入UDP返回的代码部分
    if (req_frame_option_ & Zerg_App_Frame::DESC_UDP_FRAME)
    {
        option |= Zerg_App_Frame::DESC_UDP_FRAME;
    }

    //
    return sendmsg_to_service(cmd,
                              qquin,
                              this->asyncobj_id_,
                              this->req_trans_id_,
                              this->req_snd_service_,
                              this->req_proxy_service_,
                              this->req_rcv_service_,
                              info,
                              this->req_game_app_id_,
                              option);
}

//--------------------------------------------------------------------------------------------------------------------------
//最完整的填写发送消息函数，
template< class T>
int Transaction_Base::sendmsg_to_service(unsigned int cmd,
                                         unsigned int qquin,
                                         unsigned int transaction_id,
                                         unsigned int backfill_trans_id,
                                         const SERVICES_ID &rcv_svc,
                                         const SERVICES_ID &proxy_svc,
                                         const SERVICES_ID &snd_svc,
                                         const T &info,
                                         unsigned int app_id,
                                         unsigned int option)
{
    //如果请求的命令要求要监控，后面的处理进行监控
    if (req_frame_option_ & Zerg_App_Frame::DESC_MONITOR_TRACK)
    {
        option |= Zerg_App_Frame::DESC_MONITOR_TRACK;
    }

    //如果发送的
    if (app_id == 0)
    {
        app_id = req_game_app_id_;
    }

    //条用管理器的发送函数
    return trans_manager_->mgr_sendmsg_to_service(cmd,
                                                  qquin,
                                                  transaction_id,
                                                  backfill_trans_id,
                                                  rcv_svc,
                                                  proxy_svc,
                                                  snd_svc,
                                                  info,
                                                  app_id,
                                                  option);
}

#endif //SOARING_LIB_TRANSACTION_MANAGER_H_

