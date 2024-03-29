/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar/fsm/fsm_base.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
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

#pragma once

#include "soar/zerg/frame_zerg.h"
#include "soar/fsm/fsm_mgr.h"
#include "soar/enum/error_code.h"

//本来打算封装几个静态变量为STATIC的,但是如果这样其实限制了整体
namespace soar
{
class zerg_frame;
class svrd_buspipe;

class  fsm_base : public zce::async_fsm
{
    friend class fsm_manager;

protected:

    enum WAIT_TIMEOUT_VALUE
    {
        TOO_SHORT_TIME_OUT = 5,
        SHORT_TIME_OUT = 10,
        NORMAL_TIME_OUT = 15,
        MIDDING_TIME_OUT = 30,
        LONG_TIME_OUT = 60,
        LONG_LONG_TIME_OUT = 120,
        TOO_LONG_TIME_OUT = 240,
        TOO_TOO_LONG_TIME_OUT = 480,
    };

public:
    //构造函数
    fsm_base(fsm_manager* pmngr,
             uint32_t create_cmd,
             bool trans_locker = false);
protected:
    //私有的构造函数,你必须使用NEW得到
    virtual ~fsm_base();

public:

    ///初始化，类似构造函数
    virtual void trans_init() = 0;

    /*!
    * @brief      事物运行，必须重载的函数
    * @return     virtual void
    * @param      recv_frame
    * @param      continue_run
    */
    virtual void trans_run(const soar::zerg_frame* recv_frame,
                           bool& continue_run) = 0;

    /*!
    * @brief      ，默认continue_run返回false，让系统回收，
    * @param[in]  now_time  发生超时的时间，
    * @param[out] continue_run 异步对象是否继续运行,
    */
    virtual void trans_timeout(const zce::time_value& now_time,
                               bool& continue_run) = 0;

protected:

    ///每次重新进行初始化时候的事情，等等，尽量保证基类的这个函数优先调用，
    ///这个地方用于恢复很多初始值
    virtual void on_init() final;

    ///事物的on_run函数，重载的FSM的，把调用转向到trans_run
    virtual void on_run(bool& continue_run) final;

    ///状态机对象超时处理，重载的FSM的（异步对象的），把调用转向到trans_timeout
    virtual void on_timeout(const zce::time_value& now_time,
                            bool& continue_run) final;

    /*!
    * @brief      根据Frame初始化得到对方发送的信息
    * @param      recv_frame 初始化接收的FRAME数据,
    */
    void create_init(const soar::zerg_frame* recv_frame);

    //lock其实不是真正的锁，目的是保证在同一时刻，只处理一个用户的一个请求。
    //对当前用户的，当前事务命令字进行加锁
    int lock_cmd_userid();
    //对当前用户的，当前事务命令字进行解锁
    void unlock_cmd_userid();

    //关闭请求的的Service
    int close_request_service() const;

    ///检查用户的数据帧头部的UIN和数据体的UIN是否一致,避免用户作弊
    int check_req_userid(uint32_t user_id) const;
    ///用于检查请求的IP地址是否是内部IP地址
    int check_request_internal() const;
    ///检查接受到的FRAME的数据和命令
    int check_receive_frame(const soar::zerg_frame* recv_frame,
                            uint32_t wait_cmd);

    //!DUMP所有的事物的信息
    void dump(zce::LOG_PRIORITY log_priority, const char* outstr) const;

protected:

    //--------------------------------------------------------------------
    //下面这组函数务必不要混合使用,回送就是用于回送,发送给其他服务器就用send_msg_to,request_proxy

    //请求发送数据到soar::SERVICES_ID服务器(不能是Proxy),使用请求的UIN，
    template< class T>
    int  request_peer(uint32_t cmd,
                      const soar::SERVICES_ID& rcv_svc,
                      const T& msg,
                      uint32_t option = 0);

    //请求发送数据到soar::SERVICES_ID服务器(不能是Proxy),指定特定UIN,赠送这些命令使用
    template< class T>
    int request_peer(uint32_t cmd,
                     uint32_t user_id,
                     const soar::SERVICES_ID& rcv_svc,
                     const T& msg,
                     uint32_t option = 0);

    //请求发送数据到soar::SERVICES_ID服务器,可以指定UIN和，回填的事务ID
    template <class T>
    int request_peer(uint32_t cmd,
                     uint32_t user_id,
                     uint32_t backfill_fsm_id,
                     const soar::SERVICES_ID& rcv_svc,
                     const T& msg,
                     uint32_t option = 0);

    //--------------------------------------------------------------------------------------
    //请求发送数据到代理服务器,用请求的USER ID发送
    template< class T>
    int request_proxy(uint32_t cmd,
                      const soar::SERVICES_ID& proxy_svc,
                      uint16_t rcv_type,
                      const T& msg,
                      uint32_t option = 0);

    //请求发送数据到代理服务器,用指定UIN发送
    template< class T>
    int request_proxy(uint32_t cmd,
                      uint32_t user_id,
                      const soar::SERVICES_ID& proxy_svc,
                      uint16_t rcv_type,
                      const T& msg,
                      uint32_t option = 0);

    //请求发送数据到代理服务器,,使用请求的USER ID,RCV SERVICESID(比如用代理中转到指定服务器),
    template< class T>
    int request_proxy(uint32_t cmd,
                      uint32_t user_id,
                      const soar::SERVICES_ID& proxy_svc,
                      const soar::SERVICES_ID& recvsvc,
                      const T& msg,
                      uint32_t option = 0);

    //请求发送数据到代理服务器,,使用请求的USER ID,,RCV SERVICESID(比如用代理中转到指定服务器),回填的事务ID
    template< class T>
    int request_proxy(uint32_t cmd,
                      uint32_t user_id,
                      uint32_t backfill_fsm_id,
                      const soar::SERVICES_ID& proxy_svc,
                      const soar::SERVICES_ID& recvsvc,
                      const T& msg,
                      uint32_t option = 0);

    //--------------------------------------------------------------------------------------
    //应答，向回发送数据,只能是除了接受命令的时候才可以调用这个函数,否则....
    template< class T>
    int response_sendback(uint32_t cmd,
                          const T& msg,
                          uint32_t option = 0);

    //回送信息,应答一个请求,但是可以指定回应的USER ID,只能是除了接受命令的时候才可以调用这个函数,否则....
    //比如发送邮件，FRAME中的主人是收件人,但是返回命令应该是发件人
    //为啥要用2作为函数名称而不重载呢，你可以考虑一下,我不作答
    template< class T>
    int response_sendback2(uint32_t cmd,
                           uint32_t user_id,
                           const T& msg,
                           uint32_t option = 0);

    //--------------------------------------------------------------------------------------
    //奉劝大家不要用这个，最大集合,
    template< class T>
    int sendmsg_to_service(uint32_t cmd,
                           uint32_t user_id,
                           uint32_t fsm_id,
                           uint32_t backfill_fsm_id,
                           const soar::SERVICES_ID& rcv_svc,
                           const soar::SERVICES_ID& proxy_svc,
                           const soar::SERVICES_ID& snd_svc,
                           const T& msg,
                           uint32_t option);

public:

    //!这儿对封装要求很严格，但是我感觉对外暴露的接口足够了。
protected:

    //!事件管理器
    fsm_manager*            trans_manager_ = nullptr;

    //!是否加事务锁
    bool                    trans_locker_ = false;

    //!事物被创建起来
    bool                    trans_create_ = true;

    //!
    soar::zerg_frame* recv_frame_ = nullptr;
    //!

    //! 请求的状态机的头部
    soar::zerg_head         req_zerg_head_;

    //!事务超时的定时器ID
    int                     trans_timeout_id_ = -1;
    //!发生active后，是否自动停止time out定时器
    bool                    active_auto_stop_ = true;
    //!事务的定时器ID
    int                     trans_touchtimer_id_ = -1;

    //!事务的创建时间
    time_t                  trans_create_time_ = 0;

    //!日志跟踪的优先级
    zce::LOG_PRIORITY       trace_log_pri_ = RS_INFO;
};

//请求发送消息的Service,
template <class T>
int fsm_base::request_peer(uint32_t cmd,
                           const soar::SERVICES_ID& rcv_svc,
                           const T& info,
                           uint32_t option)
{
    soar::SERVICES_ID proxy_svc(0, 0);
    return sendmsg_to_service(cmd,
                              req_zerg_head_.user_id_,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              option);
}

//请求发送消息的Service,
template <class T>
int fsm_base::request_peer(uint32_t cmd,
                           uint32_t user_id,
                           uint32_t backfill_fsm_id,
                           const soar::SERVICES_ID& rcv_svc,
                           const T& info,
                           uint32_t option)
{
    soar::SERVICES_ID proxy_svc(0, 0);
    return sendmsg_to_service(cmd,
                              user_id,
                              this->asyncobj_id_,
                              backfill_fsm_id,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              option);
}

//向某个Service发送数据,同时使用UIN
template< class T>
int fsm_base::request_peer(uint32_t cmd,
                           uint32_t user_id,
                           const soar::SERVICES_ID& rcv_svc,
                           const T& info,
                           uint32_t option)
{
    soar::SERVICES_ID proxy_svc(0, 0);
    return sendmsg_to_service(cmd,
                              user_id,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              option);
}

//--------------------------------------------------------------------------------------------------------------------------
//请求发送数据到代理服务器,用请求的USER ID发送
template< class T>
int fsm_base::request_proxy(uint32_t cmd,
                            const soar::SERVICES_ID& proxy_svc,
                            uint16_t rcv_type,
                            const T& info,
                            uint32_t option)
{
    soar::SERVICES_ID rcv_svc(rcv_type, 0);
    return sendmsg_to_service(cmd,
                              this->req_zerg_head_.user_id_,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              option);
}

//请求发送数据到代理服务器,用参数指定USER ID发送
template< class T>
int fsm_base::request_proxy(uint32_t cmd,
                            uint32_t user_id,
                            const soar::SERVICES_ID& proxy_svc,
                            uint16_t rcv_type,
                            const T& info,
                            uint32_t option)
{
    soar::SERVICES_ID rcv_svc(rcv_type, 0);
    return sendmsg_to_service(cmd,
                              user_id,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              option);
}

//请求发送数据到代理服务器,,使用请求的USER ID,RCV SERVICESID(比如用代理中转到指定服务器),
//注意RECV SVC 和PROXY SVC参数的顺序,主要事务和
template< class T>
int fsm_base::request_proxy(uint32_t cmd,
                            uint32_t user_id,
                            const soar::SERVICES_ID& proxy_svc,
                            const soar::SERVICES_ID& recvsvc,
                            const T& info,
                            uint32_t option)
{
    return sendmsg_to_service(cmd,
                              user_id,
                              this->asyncobj_id_,
                              0,
                              recvsvc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              option);
}

//请求发送数据到代理服务器,使用请求的USER ID,,RCV SERVICESID(比如用代理中转到指定服务器),回填的事务ID
//注意RECV SVC 和PROXY SVC参数的顺序
template< class T>
int fsm_base::request_proxy(uint32_t cmd,
                            uint32_t user_id,
                            uint32_t backfill_fsm_id,
                            const soar::SERVICES_ID& recvsvc,
                            const soar::SERVICES_ID& proxy_svc,
                            const T& info,
                            uint32_t option)
{
    return sendmsg_to_service(cmd,
                              user_id,
                              this->asyncobj_id_,
                              backfill_fsm_id,
                              recvsvc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              info,
                              option);
}

//--------------------------------------------------------------------------------------------------------------------------
//回送信息,应答一个请求,只能是除了接受命令的时候才可以调用这个函数,否则....
template< class T>
int fsm_base::response_sendback(uint32_t cmd,
                                const T& info,
                                uint32_t option)
{
    //加入UDP返回的代码部分
    if (req_zerg_head_.u32_option_ & soar::zerg_frame::DESC_UDP_FRAME)
    {
        option |= soar::zerg_frame::DESC_UDP_FRAME;
    }

    //
    return sendmsg_to_service(cmd,
                              this->req_zerg_head_.user_id_,
                              this->asyncobj_id_,
                              this->req_zerg_head_.fsm_id_,
                              this->req_zerg_head_.recv_service_,
                              this->req_zerg_head_.proxy_service_,
                              this->req_zerg_head_.send_service_,
                              info,
                              option);
}

//回送信息,应答一个请求,但是可以指定回应的USER ID,只能是除了接受命令的时候才可以调用这个函数,否则....
//比如发送邮件，FRAME中的主人是收件人,但是返回命令应该是发件人
//为啥要用2作为函数名称而不重载呢，你可以考虑一下,我不作答
template< class T>
int fsm_base::response_sendback2(uint32_t cmd,
                                 uint32_t user_id,
                                 const T& info,
                                 uint32_t option)
{
    //加入UDP返回的代码部分
    if (req_zerg_head_.u32_option_ & soar::zerg_frame::DESC_UDP_FRAME)
    {
        option |= soar::zerg_frame::DESC_UDP_FRAME;
    }

    //
    return sendmsg_to_service(cmd,
                              user_id,
                              this->asyncobj_id_,
                              this->req_zerg_head_.fsm_id_,
                              this->req_zerg_head_.send_service_,
                              this->req_zerg_head_.proxy_service_,
                              this->req_zerg_head_.recv_service_,
                              info,
                              option);
}

//--------------------------------------------------------------------------------------------------------------------------
//最完整的填写发送消息函数，
template< class T>
int fsm_base::sendmsg_to_service(uint32_t cmd,
                                 uint32_t user_id,
                                 uint32_t fsm_id,
                                 uint32_t backfill_fsm_id,
                                 const soar::SERVICES_ID& rcv_svc,
                                 const soar::SERVICES_ID& proxy_svc,
                                 const soar::SERVICES_ID& snd_svc,
                                 const T& msg,
                                 uint32_t option)
{
    //如果请求的命令要求要监控，后面的处理进行监控
    if (req_zerg_head_.u32_option_ & soar::zerg_frame::DESC_TRACK_MONITOR)
    {
        option |= soar::zerg_frame::DESC_TRACK_MONITOR;
    }
    //条用管理器的发送函数
    return trans_manager_->sendmsg_to_service(cmd,
                                              user_id,
                                              fsm_id,
                                              backfill_fsm_id,
                                              rcv_svc,
                                              proxy_svc,
                                              snd_svc,
                                              msg,
                                              option);
}
}
