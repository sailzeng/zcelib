#include "zce_predefine.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_predefine.h"
#include "zce_event_handle_base.h"
#include "zce_trace_log_debug.h"
#include "zce_time_value.h"
#include "zce_event_reactor_epoll.h"

//构造函数和析构函数
ZCE_Epoll_Reactor::ZCE_Epoll_Reactor():
    ZCE_Reactor(FD_SETSIZE),
    epoll_fd_(-1),
    edge_triggered_(false),
    once_max_events_(DEFAULT_ONCE_TRIGGER_MAX_EVENT),
    once_events_ary_(NULL)
{
}

//构造函数
ZCE_Epoll_Reactor::ZCE_Epoll_Reactor(size_t max_event_number,
                                     bool edge_triggered,
                                     int once_max_events):
    ZCE_Reactor(max_event_number),
    edge_triggered_(edge_triggered),
    once_max_events_(once_max_events),
    once_events_ary_(NULL)
{
    initialize(max_event_number, edge_triggered, once_max_events);
}

ZCE_Epoll_Reactor::~ZCE_Epoll_Reactor()
{
    ::close(epoll_fd_);

    //释放内存
    if (once_events_ary_)
    {
        delete [] once_events_ary_;
    }
}

//初始化
int ZCE_Epoll_Reactor::initialize(size_t max_event_number,
                                  bool edge_triggered,
                                  int once_max_events )
{
    //如果是非Linux操作系统，直接完蛋算了。
#if !defined (ZCE_OS_LINUX)
    ZCE_ASSERT(false);
#endif

    int ret = 0;
    ret = ZCE_Reactor::initialize(max_event_number);

    if (0  != ret)
    {
        ZLOG_ERROR("[zcelib] Epoll reactor ZCE_Reactor::initialize fail.please check code. ret = %u.", ret);
        return ret;
    }

    edge_triggered_ = edge_triggered;

    //这个只有LINUX下才有
#if defined (ZCE_OS_LINUX)
    epoll_fd_ = ::epoll_create(max_event_number_ + 64);
#endif

    if (epoll_fd_ < 0)
    {
        ZLOG_ERROR("[zcelib] Epoll reactor ::epoll_create fail.please check code. error = [%u|%u]",
                   ZCE_LIB::last_error(),
                   strerror(ZCE_LIB::last_error()));
        return -1;
    }

    once_max_events_ = once_max_events;

    once_events_ary_ = new epoll_event [once_max_events_];

    return 0;
}

//注册一个ZCE_Event_Handler到反应器
int ZCE_Epoll_Reactor::register_handler(ZCE_Event_Handler *event_handler, int event_mask)
{
    int ret = 0;
    //注意第二个参数是0，因为第一要先ADD，第二避免两次调用这个,这个代码放前面是因为回滚麻烦
    ret = ZCE_Reactor::register_handler(event_handler, 0);

    if (0 != ret)
    {
        ZLOG_ERROR("[zcelib] ZCE_Reactor::register_handler fail. please check you code .ret =%d", ret);
        return -1;
    }

    event_handler->set_mask(event_mask);

    struct epoll_event ep_event;
    make_epoll_event(&ep_event, event_handler);

    //EPOLL 在LINUX才有，WINDOWS也不可能模拟出来，难道让我用个SELECT模拟？
#if defined (ZCE_OS_LINUX)
    ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event_handler->get_handle(), &ep_event);
#endif

    if (0 != ret)
    {
        //回滚
        ret = ZCE_Reactor::remove_handler(event_handler, false);

        ZLOG_ERROR("[zcelib] [%s] Epoll reactor ::epoll_ctl fail.please check code. ret =%d error = [%u|%s]",
                   __ZCE_FUNCTION__,
                   ret,
                   ZCE_LIB::last_error(),
                   strerror(ZCE_LIB::last_error()));
        return -1;
    }

    return 0;
}

//从反应器注销一个ZCE_Event_Handler，同事取消他所有的mask
int ZCE_Epoll_Reactor::remove_handler(ZCE_Event_Handler *event_handler, bool call_handle_close)
{

    int ret = 0;

    struct epoll_event event;
    event.events = 0;
#if defined (ZCE_OS_LINUX)
    //调用epoll_ctl EPOLL_CTL_DEL 删除注册对象
    ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, event_handler->get_handle(), &event);
#endif

    if (0 != ret)
    {
        ZLOG_ERROR("[zcelib] [%s] epoll reactor ::epoll_ctl fail.please check code. ret =%d error = [%u|%s]",
                   __ZCE_FUNCTION__,
                   ret,
                   ZCE_LIB::last_error(),
                   strerror(ZCE_LIB::last_error()));
        //这个不返回好还是返回好呢？这真是一个问题
        //return -1;
    }

    //取消掉所有的事件，前面已经删除了，避免里面重复调用
    event_handler->set_mask(0);

    ret = ZCE_Reactor::remove_handler(event_handler, call_handle_close);

    if (0 != ret)
    {
        ZLOG_ERROR("[zcelib] ZCE_Reactor::remove_handler fail. please check you code .ret =%u", ret);
        return -1;
    }

    return 0;
}

//取消某些mask标志，，
int ZCE_Epoll_Reactor::cancel_wakeup(ZCE_Event_Handler *event_handler, int cancel_mask)
{
    int ret = 0;

    ret = ZCE_Reactor::cancel_wakeup(event_handler, cancel_mask);

    //其实ZCE_Reactor::cancel_wakeup不可能失败,
    if (0 != ret)
    {
        return -1;
    }

    struct epoll_event ep_event;

    make_epoll_event(&ep_event, event_handler);

#if defined (ZCE_OS_LINUX)
    //EPOLL_CTL_MOD用于修改
    ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD , event_handler->get_handle(), &ep_event);

#endif

    if (0 != ret)
    {
        //回滚不改动标志位
        ZCE_Reactor::schedule_wakeup(event_handler, cancel_mask);
        ZLOG_ERROR("[zcelib] [%s] epoll reactor ::epoll_ctl fail.please check code. ret =%d error = [%u|%s]",
                   __ZCE_FUNCTION__,
                   ret,
                   ZCE_LIB::last_error(),
                   strerror(ZCE_LIB::last_error()));
        return -1;
    }

    return 0;
}

//打开某些mask标志，
int ZCE_Epoll_Reactor::schedule_wakeup(ZCE_Event_Handler *event_handler, int event_mask)
{
    int ret = 0;
    ret = ZCE_Reactor::schedule_wakeup(event_handler, event_mask);

    //其实ZCE_Reactor::cancel_wakeup不可能失败
    if (0 != ret)
    {
        return -1;
    }

    struct epoll_event ep_event;

    make_epoll_event(&ep_event, event_handler);

#if defined (ZCE_OS_LINUX)
    //EPOLL_CTL_MOD用于修改
    ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD , event_handler->get_handle(), &ep_event);

#endif

    if (0 != ret)
    {
        //回滚，修复标志位
        ZCE_Reactor::cancel_wakeup(event_handler, event_mask);
        ZLOG_ERROR("[zcelib] [%s] epoll reactor ::epoll_ctl fail.please check code. ret =%d error = [%u|%s].",
                   __ZCE_FUNCTION__,
                   ret,
                   ZCE_LIB::last_error(),
                   strerror(ZCE_LIB::last_error()));
        return -1;
    }

    return 0;
}

//时间触发
int ZCE_Epoll_Reactor::handle_events(ZCE_Time_Value *time_out, size_t *size_event)
{

    //默认一直阻塞
    int msec_timeout = -1;

    if (time_out)
    {
        //根据bluehu 提醒修改了下面这段，（不过小伙子们，你们改代码要认真一点喔）
        //由于select的超时参数可以精确到微秒，而epoll_wait的参数只精确到毫秒
        //当超时时间小于1000微秒时，比如20微秒，将时间转换成毫秒会变成0毫秒
        //所以如果用epoll_wait的话，超时时间大于0并且小于1毫秒的情况下统一改为1毫秒

        msec_timeout = static_cast<int>( time_out->total_msec_round());
    }

    int event_happen = 0;

#if defined (ZCE_OS_LINUX)
    //EPOLL等待事件触发，
    event_happen = ::epoll_wait(epoll_fd_, once_events_ary_, once_max_events_, msec_timeout);
#endif

    if (event_happen <= 0)
    {
        return event_happen;
    }

    *size_event = event_happen;

    for (int i = 0; i < event_happen; ++i)
    {
        process_ready_event(once_events_ary_ + i);
    }

    return 0;

}

void ZCE_Epoll_Reactor::process_ready_event(struct epoll_event *ep_event)
{
    int ret = 0;
    ZCE_Event_Handler *event_hdl = NULL;
    bool event_in_happen = false, event_out_happen = false;

    ret =  find_event_handler(ep_event->data.fd, event_hdl);

    //到这个地方，可能是代码有问题，也可能不是，因为一个事件处理后，可能就被关闭了？
    if (0 != ret)
    {
        return;
    }

    //根据不同的事件进行调动，触发
    int hdl_ret = 0;

    //【注意】所有的reactor 都要保证各种触发的顺序，必须保证，否则就有不兼容的问题，正确的顺序应该是读,写,异常处理

    //修正connect发生连接错误，内部希望停止处理，但epoll会返回3个事件的修正,
    //先说明一下这个问题，当内部处理的事件发生错误后，上层可能就会删除掉对应的ZCE_Event_Handler，但底层可能还有事件要触发。
    //由于该代码的时候面临抉择，我必须写一段说明，我在改这段代码的时候有几种种方式,
    //1.整体写成do { break} while()的方式，每个IO事件都作为触发方式，这个好处是简单，但对于epoll,如果是水平触发，那么你可能丢失事件
    //2.继续使用return -1作为一个判断，ACE就是这样的，原来曾经觉得ACE这个设计有点冗余，但自己设计看来，还是有好处的，
    //3.每次调用后，都检查一下event_hdl是否还存在，
    //最后我采用了兼容2，3的方法，2是为了和ACE兼容，3是为了保证就是你TMD天翻地覆，我也能应付,

    //代码有点怪，部分目的是加快速度,避免每个调用都要检查，

    //WRITE和CONNECT事件都调用handle_output
    if (ep_event->events &  EPOLLIN )
    {
        event_in_happen = true;
        hdl_ret = event_hdl->handle_input();

        //返回-1表示 handle_xxxxx希望调用handle_close退出
        if (hdl_ret == -1)
        {
            event_hdl->handle_close();
        }
    }

    //READ和ACCEPT事件都调用handle_input
    if (ep_event->events & EPOLLOUT )
    {
        //如果写事件触发了，那么里面可能调用handle_close,再查询一次，double check.
        if (event_in_happen)
        {
            ret =  find_event_handler(ep_event->data.fd, event_hdl);

            //到这个地方，可能是代码有问题，也可能不是，因为一个事件处理后，可能就被关闭了？
            if (0 != ret)
            {
                return;
            }
        }

        event_out_happen = true;
        hdl_ret = event_hdl->handle_output();

        //返回-1表示 handle_xxxxx希望调用handle_close退出
        if (hdl_ret == -1)
        {
            event_hdl->handle_close();
        }

    }

    //异常事件，其实我也不知道，什么算异常
    if (ep_event->events & EPOLLERR )
    {
        //如果读取或者写事件触发了，那么里面可能调用handle_close,再查询一次，double check.
        if (event_out_happen || event_in_happen)
        {
            ret =  find_event_handler(ep_event->data.fd, event_hdl);

            //到这个地方，可能是代码有问题，也可能不是，因为一个事件处理后，可能就被关闭了？
            if (0 != ret)
            {
                return;
            }
        }

        hdl_ret = event_hdl->handle_exception();

        //返回-1表示 handle_xxxxx希望调用handle_close退出
        if (hdl_ret == -1)
        {
            event_hdl->handle_close();
        }
    }
}

