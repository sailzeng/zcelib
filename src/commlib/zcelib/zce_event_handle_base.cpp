#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_socket.h"
#include "zce_event_handle_base.h"
#include "zce_event_reactor_base.h"

//构造函数和析构函数
ZCE_Event_Handler::ZCE_Event_Handler():
    zce_reactor_(NULL),
    event_mask_(NULL_MASK)
{

}
ZCE_Event_Handler::ZCE_Event_Handler(ZCE_Reactor *reactor):
    zce_reactor_(reactor),
    event_mask_(NULL_MASK)
{

}

ZCE_Event_Handler::~ZCE_Event_Handler()
{
}

////
int ZCE_Event_Handler::handle_input ()
{
    return 0;
}

//
int ZCE_Event_Handler::handle_output ()
{
    return 0;
}
////
int ZCE_Event_Handler::handle_exception()
{
    return 0;
}

int ZCE_Event_Handler::handle_close ()
{
    //移除自己，但是不判断返回，因为如果代码考虑的不细致，可能出现两次remove_handler
    reactor()->remove_handler(this, false);
    return 0;
}

void ZCE_Event_Handler::reactor (ZCE_Reactor *reactor)
{
    zce_reactor_ = reactor;
}

/// Get the event demultiplexors.
ZCE_Reactor *ZCE_Event_Handler::reactor (void) const
{
    return zce_reactor_;
}

