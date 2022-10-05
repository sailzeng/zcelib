#include "zce/predefine.h"
#include "zce/os_adapt/common.h"
#include "zce/os_adapt/socket.h"
#include "zce/event/handle_base.h"
#include "zce/event/reactor_base.h"

namespace zce
{
//构造函数和析构函数
event_handler::event_handler() :
    zce_reactor_(NULL),
    event_mask_(static_cast<int>(EVENT_MASK::NULL_MASK))
{
}
event_handler::event_handler(zce::reactor* reactor) :
    zce_reactor_(reactor),
    event_mask_(static_cast<int>(EVENT_MASK::NULL_MASK))
{
}

event_handler::~event_handler()
{
}

//
int event_handler::read_event()
{
    return 0;
}

//
int event_handler::write_event()
{
    return 0;
}
//异常事件调用函数
int event_handler::exception_event()
{
    return 0;
}

//发生了链接的事件
int event_handler::connect_event(bool /*success*/)
{
    return 0;
}

//发生了accept的事件是调用
int event_handler::accept_event()
{
    return 0;
}

//发生了inotify的事件是调用
int event_handler::inotify_event()
{
    return 0;
}

//
int event_handler::event_close()
{
    //移除自己，但是不判断返回，因为如果代码考虑的不细致，可能出现两次remove_handler
    reactor()->remove_handler(this, false);
    return 0;
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