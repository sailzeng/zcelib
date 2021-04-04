#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_event_handle_base.h"
#include "zce_os_adapt_socket.h"
#include "zce_log_logging.h"
#include "zce_event_reactor_wfmo.h"


#if defined ZCE_OS_WINDOWS

//���캯��
ZCE_WFMO_Reactor::ZCE_WFMO_Reactor():
    watch_handle_ary_{ ZCE_INVALID_HANDLE }
{
    initialize();

#if defined ZCE_OS_LINUX
    ZCE_ASSERT(false);
#endif

}

ZCE_WFMO_Reactor::~ZCE_WFMO_Reactor()
{
}


//��ʼ��
int ZCE_WFMO_Reactor::initialize()
{
    //ȫ������Ϊ��Ч
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


//ע��һ��������Լ������ĵ��¼�
int ZCE_WFMO_Reactor::register_handler(ZCE_Event_Handler *event_handler,
                                       int event_mask)
{
    int ret = 0;
    //ע��ڶ���������0����Ϊ��һҪ��ADD���ڶ��������ε������,��������ǰ������Ϊ�ع��鷳
    ret = ZCE_Reactor::register_handler(event_handler, 0);
    if (0 != ret)
    {
        return -1;
    }
    size_t watch_size = handler_map_.size();

    //�����SOCKET���粿��
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
                    zce::last_error()
                   );
            return -1;
        }
        ret = wfmo_socket_event(event_handler, socket_event, event_mask);
        if (0 != ret)
        {
            //�����ع�
            ZCE_Reactor::remove_handler(event_handler, false);
            return ret;
        }

        //WaitForMultipleObjects
        watch_handle_ary_[watch_size - 1] = socket_event;
        watch_socket_ary_[watch_size - 1] = (SOCKET) event_handler->get_handle();
    }

    //�����INOTIFY ���¼���ֱ��ע������ȥ
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::INOTIFY_MASK))
    {
        watch_handle_ary_[watch_size - 1] = event_handler->get_handle();
    }

    event_handler->set_mask(event_mask);

    return 0;
}

//�ӷ�Ӧ��ע��һ��ZCE_Event_Handler��ͬʱȡ�������е�mask
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


            //�����һ�������Ա�ƶ�������ط�
            watch_socket_ary_[i] = watch_socket_ary_[watch_size - 1];
            watch_handle_ary_[i] = watch_handle_ary_[watch_size - 1];

            //�����һ����������Ϊ��Ч
            watch_socket_ary_[watch_size - 1] = ZCE_INVALID_SOCKET;
            watch_handle_ary_[watch_size - 1] = ZCE_INVALID_HANDLE;

            break;
        }
    }

    return ZCE_Reactor::remove_handler(event_handler, call_handle_close);
}


//��һ�����Ѿ�ע��ģ���������������ĵ��¼�
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
            //ע�⣬������ǹ�����event��Ҳ����watch_handle_ary_[i]
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



//��һ�����Ѿ�ע��ģ������ȡ�������ĵ��¼�
int ZCE_WFMO_Reactor::cancel_wakeup(ZCE_Event_Handler *event_handler,
                                    int cancel_mask)
{
    int ret = 0;
    //�õ�ȡ�����maskֵ
    int event_mask = event_handler->get_mask();
    event_mask &= (~cancel_mask);

    size_t watch_size = handler_map_.size();
    SOCKET socket_handle = (SOCKET) event_handler->get_handle();
    //
    for (size_t i = 0; i < watch_size; ++i)
    {
        if (watch_socket_ary_[i] == socket_handle)
        {
            //ע�⣬������ǹ�����event��Ҳ����watch_handle_ary_[i]
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

//Windows �� ��Socket ����EVENT_MASK�������Ӧ�������¼������Ұ󶨵��¼���
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
                zce::last_error()
               );
        return -1;
    }

    return 0;
}

//����IO��������
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
                zce::last_error());
        return -1;
    }

    //����ǳ�ʱ
    if (wait_status == WAIT_TIMEOUT)
    {
        errno = ETIMEDOUT;
        return -1;
    }

    *size_event = 1;
    size_t activate_id = wait_status - WAIT_OBJECT_0;

    ZCE_Event_Handler *event_hdl = NULL;

    //��ΪSocket ���¼���������ķ�Ӧ�������¼����������event handle�ڲ���socket�������
    //����event handle�� map����socket�������key�����������µĴ���
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
            //��Ϊ�ܶ��¼��Ტ�д�������ǰ��һ���¼��������ܻ�ر�event_hdl,���Ա���double check
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
                //ͳһ���첽CONNECT���ʧ�ܣ�����handle_input,�ɹ�����handle_output
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
