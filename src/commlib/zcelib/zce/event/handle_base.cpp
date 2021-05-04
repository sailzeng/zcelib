#include "zce/predefine.h"
#include "zce/os_adapt/common.h"
#include "zce/os_adapt/socket.h"
#include "zce/event/handle_base.h"
#include "zce/event/reactor_base.h"

//构造函数和析构函数
ZCE_Event_Handler::ZCE_Event_Handler():
    zce_reactor_(NULL),
    event_mask_(static_cast<int>(EVENT_MASK::NULL_MASK))
{
}
ZCE_Event_Handler::ZCE_Event_Handler(ZCE_Reactor *reactor):
    zce_reactor_(reactor),
    event_mask_(static_cast<int>(EVENT_MASK::NULL_MASK))
{
}

ZCE_Event_Handler::~ZCE_Event_Handler()
{
}

////
int ZCE_Event_Handler::handle_input()
{
    return 0;
}

//
int ZCE_Event_Handler::handle_output()
{
    return 0;
}
////
int ZCE_Event_Handler::handle_exception()
{
    return 0;
}

int ZCE_Event_Handler::handle_close()
{
    //移除自己，但是不判断返回，因为如果代码考虑的不细致，可能出现两次remove_handler
    reactor()->remove_handler(this,false);
    return 0;
}

void ZCE_Event_Handler::reactor(ZCE_Reactor *reactor)
{
    zce_reactor_ = reactor;
}

/// Get the event demultiplexors.
ZCE_Reactor *ZCE_Event_Handler::reactor(void) const
{
    return zce_reactor_;
}