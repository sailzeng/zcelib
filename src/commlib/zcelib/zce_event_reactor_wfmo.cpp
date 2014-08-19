#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_event_handle_base.h"
#include "zce_os_adapt_socket.h"
#include "zce_trace_log_debug.h"
#include "zce_event_reactor_wfmo.h"




//
ZCE_WFMO_Reactor::ZCE_WFMO_Reactor()
{
    initialize();
}



ZCE_WFMO_Reactor::~ZCE_WFMO_Reactor()
{
}


//初始化
int ZCE_WFMO_Reactor::initialize()
{
    //全部处理为无效
    for (size_t i = 0; i < MAXIMUM_WAIT_OBJECTS; ++i)
    {
        watch_handle_ary_[i] = INVALID_HANDLE_VALUE;
    }

    return ZCE_Reactor::initialize(MAXIMUM_WAIT_OBJECTS);
}


int ZCE_WFMO_Reactor::handle_events(ZCE_Time_Value *time_out, size_t *size_event)
{
    int ret = 0;
    *size_event = 0;

    DWORD wait_msec = static_cast<DWORD>(time_out->total_msec());
    wait_msec = 0;
    DWORD watch_ary_size = static_cast<DWORD>(handler_map_.size());
    DWORD wait_status = WaitForMultipleObjects(watch_ary_size,
        watch_handle_ary_,
        FALSE,
        wait_msec);

    if (wait_status == WAIT_FAILED)
    {
        ZLOG_ERROR("[%s] ::WaitForMultipleObjects fail,error [%u].",
            __ZCE_FUNCTION__,
            ZCE_OS::last_error());
        return -1;
    }

    //如果是超时
    if (wait_status == WAIT_TIMEOUT)
    {
        errno = ETIMEDOUT;
        return -1;
    }

    *size_event = 1;
    size_t activate_id = wait_status - WAIT_OBJECT_0;

    ZCE_Event_Handler *event_hdl = NULL;
    ret = find_event_handler(watch_handle_ary_[activate_id], event_hdl);
    if (0 != ret)
    {
        return -1;
    }

    int event_mask = event_hdl->get_mask();
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::INOTIFY_MASK))
    {
        event_hdl->handle_input();
    }


    return 0;
}



