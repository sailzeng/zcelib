#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_event_handle_base.h"
#include "zce_os_adapt_socket.h"
#include "zce_trace_log_debug.h"
#include "zce_event_reactor_wfmo.h"

//
ZCE_WFMO_Reactor::ZCE_WFMO_Reactor() :
{
    initialize(MAXIMUM_WAIT_OBJECTS + 16);
}



ZCE_Select_Reactor::~ZCE_Select_Reactor()
{
}

