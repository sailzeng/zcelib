#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_event_handle_base.h"
#include "zce_os_adapt_socket.h"
#include "zce_trace_debugging.h"
#include "zce_event_reactor_wfmo.h"


#if defined ZCE_OS_WINDOWS

//构造函数
ZCE_WFMO_Reactor::ZCE_WFMO_Reactor()
{
    initialize();

#if defined ZCE_OS_LINUX
    ZCE_ASSERT(false);
#endif

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
        watch_socket_ary_[i] = ZCE_INVALID_SOCKET;
    }

    return ZCE_Reactor::initialize(MAXIMUM_WAIT_OBJECTS);
}


//注册一个句柄，以及他关心的事件
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
            ZCE_LOG(RS_ERROR, "[zcelib][%s]WSACreateEvent return fail.last error [%d]",
                    __ZCE_FUNC__,
                    ZCE_LIB::last_error()
                   );
            return -1;
        }
        ret = wfmo_socket_event(event_handler, socket_event, event_mask);
        if (0 != ret)
        {
            //出错，回滚
            ZCE_Reactor::remove_handler(event_handler, false);
            return ret;
        }

        //WaitForMultipleObjects
        watch_handle_ary_[watch_size - 1] = socket_event;
        watch_socket_ary_[watch_size - 1] = (SOCKET) event_handler->get_handle();
    }

    //如果是INOTIFY 的事件，直接注册句柄进去
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
    ZCE_SOCKET socket_handle = (SOCKET) event_handler->get_handle();
    //
    for (size_t i = 0; i < watch_size; ++i)
    {
        if (watch_socket_ary_[i] == socket_handle)
        {

            if ( ZCE_INVALID_HANDLE != watch_handle_ary_[i])
            {
                ::WSACloseEvent(watch_handle_ary_[i]);
            }


            //将最后一个数组成员移动到这个地方
            watch_socket_ary_[i] = watch_socket_ary_[watch_size - 1];
            watch_handle_ary_[i] = watch_handle_ary_[watch_size - 1];

            //将最后一个数组句柄置为无效
            watch_socket_ary_[watch_size - 1] = ZCE_INVALID_SOCKET;
            watch_handle_ary_[watch_size - 1] = ZCE_INVALID_HANDLE;

            break;
        }
    }

    return ZCE_Reactor::remove_handler(event_handler, call_handle_close);
}


//对一个（已经注册的）句柄，设置他关心的事件
int ZCE_WFMO_Reactor::schedule_wakeup(ZCE_Event_Handler *event_handler,
                                      int event_mask)
{
    int ret = 0;
    size_t watch_size = handler_map_.size();
    SOCKET socket_handle = (SOCKET)event_handler->get_handle();
    //
    for (size_t i = 0; i < watch_size; ++i)
    {
        if (watch_socket_ary_[i] == socket_handle)
        {
            //注意，让如的是关联的event，也就是watch_handle_ary_[i]
            ret = wfmo_socket_event(event_handler, watch_handle_ary_[i], event_mask);
            if (0 != ret)
            {
                return ret;
            }
            break;
        }
    }

    return ZCE_Reactor::schedule_wakeup(event_handler, event_mask);
}



//对一个（已经注册的）句柄，取消他关心的事件
int ZCE_WFMO_Reactor::cancel_wakeup(ZCE_Event_Handler *event_handler,
                                    int cancel_mask)
{
    int ret = 0;
    //得到取消后的mask值
    int event_mask = event_handler->get_mask();
    event_mask &= (~cancel_mask);

    size_t watch_size = handler_map_.size();
    SOCKET socket_handle = (SOCKET) event_handler->get_handle();
    //
    for (size_t i = 0; i < watch_size; ++i)
    {
        if (watch_socket_ary_[i] == socket_handle)
        {
            //注意，让如的是关联的event，也就是watch_handle_ary_[i]
            ret = wfmo_socket_event(event_handler, watch_handle_ary_[i], event_mask);
            if (0 != ret)
            {
                return ret;
            }
            break;
        }
    }

    return ZCE_Reactor::cancel_wakeup(event_handler, cancel_mask);
}

//Windows 下 对Socket 根据EVENT_MASK设置其对应的网络事件，并且绑定到事件上
int ZCE_WFMO_Reactor::wfmo_socket_event(ZCE_Event_Handler *event_handler,
                                        WSAEVENT socket_event,
                                        int event_mask)
{
    int ret = 0;

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
        ZCE_LOG(RS_ERROR, "[zcelib][%s]WSAEventSelect return [%d] fail,event mask [%d],last error [%d]",
                __ZCE_FUNC__,
                ret,
                event_mask,
                ZCE_LIB::last_error()
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
    DWORD watch_ary_size = static_cast<DWORD>(handler_map_.size());

    DWORD wait_status = WaitForMultipleObjects(watch_ary_size,
                                               watch_handle_ary_,
                                               FALSE,
                                               wait_msec);

    if (wait_status == WAIT_FAILED)
    {
        ZCE_LOG(RS_ERROR, "[%s] ::WaitForMultipleObjects fail,error [%u].",
                __ZCE_FUNC__,
                ZCE_LIB::last_error());
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

    //因为Socket 的事件处理，放入的反应器的是事件句柄，但在event handle内部是socket句柄，而
    //保存event handle的 map是用socket句柄做得key，所以有如下的代码
    if (ZCE_INVALID_SOCKET != watch_socket_ary_[activate_id])
    {
        ret = find_event_handler((ZCE_HANDLE)watch_socket_ary_[activate_id],
                                 event_hdl);
    }
    else
    {
        ret = find_event_handler((ZCE_HANDLE)watch_handle_ary_[activate_id],
                                 event_hdl);
    }

    if (0 != ret)
    {
        ZCE_LOG(RS_INFO, "[zcelib] [%s] fail find handle [%lu],maybe one handle is close previous.",
                __ZCE_FUNC__,
                watch_socket_ary_[activate_id]);
        return -1;
    }

    int event_mask = event_hdl->get_mask();


    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::READ_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::ACCEPT_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::CONNECT_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::EXCEPT_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::WRITE_MASK))
    {
        WSANETWORKEVENTS socket_event;
        ::WSAEnumNetworkEvents((SOCKET)watch_socket_ary_[activate_id],
                               watch_handle_ary_[activate_id],
                               &socket_event);
        SOCKET socket_handle = (SOCKET)watch_socket_ary_[activate_id];
        if (ZCE_BIT_IS_SET(socket_event.lNetworkEvents, FD_ACCEPT))
        {
            event_hdl->handle_input();
        }
        if (ZCE_BIT_IS_SET(socket_event.lNetworkEvents, FD_READ))
        {
            //因为很多事件会并列触发，而前面一个事件处理，可能会关闭event_hdl,所以必须double check
            ret = find_event_handler((ZCE_HANDLE)socket_handle, event_hdl);
            if (ret == 0)
            {
                ret = event_hdl->handle_input();
                if (ret == -1)
                {
                    event_hdl->handle_close();
                }
            }
        }
        if (ZCE_BIT_IS_SET(socket_event.lNetworkEvents, FD_CLOSE))
        {
            ret = find_event_handler((ZCE_HANDLE)socket_handle, event_hdl);
            if (ret == 0)
            {
                ret = event_hdl->handle_input();
                if (ret == -1)
                {
                    event_hdl->handle_close();
                }
            }
        }
        if (ZCE_BIT_IS_SET(socket_event.lNetworkEvents, FD_WRITE))
        {
            ret = find_event_handler((ZCE_HANDLE)socket_handle, event_hdl);
            if (ret == 0)
            {
                ret = event_hdl->handle_output();
                if (ret == -1)
                {
                    event_hdl->handle_close();
                }
            }
        }
        if (ZCE_BIT_IS_SET(socket_event.lNetworkEvents, FD_OOB))
        {
            ret = find_event_handler((ZCE_HANDLE)socket_handle, event_hdl);
            if (ret == 0)
            {
                ret = event_hdl->handle_exception();
                if (ret == -1)
                {
                    event_hdl->handle_close();
                }
            }
        }
        if (ZCE_BIT_IS_SET(socket_event.lNetworkEvents, FD_CONNECT))
        {
            ret = find_event_handler((ZCE_HANDLE)socket_handle, event_hdl);
            if (ret == 0)
            {
                //统一，异步CONNECT如果失败，调用handle_input,成功调用handle_output
                if (socket_event.iErrorCode[FD_CONNECT_BIT])
                {
                    ret = event_hdl->handle_input();
                }
                else
                {
                    ret = event_hdl->handle_output();
                }
                if (ret == -1)
                {
                    event_hdl->handle_close();
                }
            }
        }
    }

    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::INOTIFY_MASK))
    {
        ret = event_hdl->handle_input();
        if (ret == -1)
        {
            event_hdl->handle_close();
        }
    }
    return 0;
}


#endif //#if defined ZCE_OS_WINDOWS
