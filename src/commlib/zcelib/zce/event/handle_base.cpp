#include "zce/predefine.h"
#include "zce/os_adapt/define.h"
#include "zce/os_adapt/socket.h"
#include "zce/event/handle_base.h"
#include "zce/event/reactor_base.h"

namespace zce
{
//构造函数和析构函数
event_handler::event_handler() :
    zce_reactor_(nullptr),
    event_mask_(static_cast<int>(RECTOR_EVENT::NULL_MASK))
{
}
event_handler::event_handler(zce::reactor* reactor) :
    zce_reactor_(reactor),
    event_mask_(static_cast<int>(RECTOR_EVENT::NULL_MASK))
{
}

event_handler::~event_handler()
{
}

//读事件
void event_handler::read_event()
{
    return;
}

//写事件
void event_handler::write_event()
{
    return;
}
//异常事件调用函数
void event_handler::exception_event()
{
    return;
}

//发生了链接的事件
void event_handler::connect_event(bool /*success*/)
{
    return;
}

//发生了accept的事件是调用
void event_handler::accept_event()
{
    return;
}

//发生了inotify的事件是调用
void event_handler::inotify_event()
{
    return;
}

//
void event_handler::close_handle()
{
    //移除自己，但是不判断返回，因为如果代码考虑的不细致，可能出现两次remove_handler
    reactor()->remove_handler(this, false);
    return;
}

void event_handler::reactor(zce::reactor* reactor)
{
    zce_reactor_ = reactor;
}

/// Get the event demultiplexors.
zce::reactor* event_handler::reactor(void) const
{
    return zce_reactor_;
}
}