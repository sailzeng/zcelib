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
        watch_handle_ary_[i] = ZCE_INVALID_HANDLE;
    }
    for (size_t i = 0; i < MAXIMUM_WAIT_OBJECTS; ++i)
    {
        watch_socket_ary_[i] = ZCE_INVALID_HANDLE;
    }

    return ZCE_Reactor::initialize(MAXIMUM_WAIT_OBJECTS);
}


int ZCE_WFMO_Reactor::register_handler(ZCE_Event_Handler *event_handler, 
    int event_mask)
{
    int ret = 0;
    //注意第二个参数是0，因为第一要先ADD，第二避免两次调用这个,这个代码放前面是因为回滚麻烦
    ret = ZCE_Reactor::register_handler(event_handler, 0);
    if (0 != ret)
    {
        return -1;
    }
    size_t watch_size = handler_map_.size();
    
    //如果是SOCKET网络部分
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::READ_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::ACCEPT_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::CONNECT_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::EXCEPT_MASK) 
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::WRITE_MASK)  )
    {
        WSAEVENT socket_event = ::WSACreateEvent();
        if (socket_event == WSA_INVALID_EVENT)
        {
            ZCE_LOGMSG(RS_ERROR, "WSACreateEvent return event fail.last error [%d|%s]",
                ZCE_OS::last_error(),
                strerror(ZCE_OS::last_error())
                );
            return -1;
        }
        ret = wfmo_socket_event(event_handler, socket_event);
        if (0 != ret)
        {
            //出错，回滚
            ret = ZCE_Reactor::remove_handler(event_handler, false);
            return ret;
        }

        //WaitForMultipleObjects
        watch_handle_ary_[watch_size - 1] = socket_event;
        watch_socket_ary_[watch_size - 1] = event_handler->get_handle();
    }

    //
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::INOTIFY_MASK))
    {
        watch_handle_ary_[watch_size - 1] = event_handler->get_handle();
    }

    event_handler->set_mask(event_mask);

    return 0;
}

//从反应器注销一个ZCE_Event_Handler，同时取消他所有的mask
int ZCE_WFMO_Reactor::remove_handler(ZCE_Event_Handler *event_handler, 
    bool call_handle_close)
{
    
    size_t watch_size = handler_map_.size();
    ZCE_HANDLE socket_handle = event_handler->get_handle();
    //
    for (size_t i = 0; i < watch_size; ++i)
    {
        if (watch_socket_ary_[i] == socket_handle)
        {
            watch_socket_ary_[i] = ZCE_INVALID_HANDLE;
            watch_handle_ary_[i] = ZCE_INVALID_HANDLE;

            watch_socket_ary_[i] = watch_socket_ary_[watch_size - 1];
            watch_handle_ary_[i] = watch_handle_ary_[watch_size - 1];

            ::WSACloseEvent(watch_handle_ary_[i]);
        }
    }

    return ZCE_Reactor::remove_handler(event_handler, call_handle_close);
}


//
int ZCE_WFMO_Reactor::schedule_wakeup(ZCE_Event_Handler *event_handler, 
    int event_mask)
{
    int ret = 0;
    size_t watch_size = handler_map_.size();
    ZCE_HANDLE socket_handle = event_handler->get_handle();
    //
    for (size_t i = 0; i < watch_size; ++i)
    {
        if (watch_socket_ary_[i] == socket_handle)
        {
            //注意，让如的是关联的event，也就是watch_handle_ary_[i]
            ret = wfmo_socket_event(event_handler, watch_handle_ary_[i]);
            if (0 != ret)
            {
                return ret;
            }
            break;
        }
    }

    return ZCE_Reactor::schedule_wakeup(event_handler, event_mask);
}


//
int ZCE_WFMO_Reactor::cancel_wakeup(ZCE_Event_Handler *event_handler,
    int event_mask)
{
    int ret = 0;
    size_t watch_size = handler_map_.size();
    ZCE_HANDLE socket_handle = event_handler->get_handle();
    //
    for (size_t i = 0; i < watch_size; ++i)
    {
        if (watch_socket_ary_[i] == socket_handle)
        {
            //注意，让如的是关联的event，也就是watch_handle_ary_[i]
            ret = wfmo_socket_event(event_handler, watch_handle_ary_[i]);
            if (0 != ret)
            {
                return ret;
            }
            break;
        }
    }

    return ZCE_Reactor::cancel_wakeup(event_handler, event_mask);
}

int ZCE_WFMO_Reactor::wfmo_socket_event(ZCE_Event_Handler *event_handler,
    WSAEVENT socket_event)
{
    int ret = 0;
    int event_mask = event_handler->get_mask();


    long wmfo_net_event = 0;
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::READ_MASK))
    {
        wmfo_net_event |= FD_READ | FD_CLOSE;
    }
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::WRITE_MASK))
    {
        wmfo_net_event |= FD_WRITE;
    }
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::CONNECT_MASK))
    {
        wmfo_net_event |= FD_CONNECT;
    }
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::ACCEPT_MASK))
    {
        wmfo_net_event |= FD_ACCEPT;
    }
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::EXCEPT_MASK))
    {
        wmfo_net_event |= FD_OOB;
    }
    ret = ::WSAEventSelect((SOCKET)event_handler->get_handle(), 
        socket_event, 
        wmfo_net_event);
    if (ret != 0)
    {
        ZCE_LOGMSG(RS_ERROR, "[zcelib][%s]WSAEventSelect return [%d] fail.last error [%d|%s]",
            __ZCE_FUNCTION__,
            ret,
            ZCE_OS::last_error(),
            strerror(ZCE_OS::last_error())
            );
        return -1;
    }

    return 0;
}

//进行IO触发操作
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