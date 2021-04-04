#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_event_handle_base.h"
#include "zce_os_adapt_socket.h"
#include "zce_log_logging.h"
#include "zce_event_reactor_select.h"

//WINDOWS�Ĵ����Լ�����FD_SET�����һ���澯
#if defined (ZCE_OS_WINDOWS)
#pragma warning(disable : 4127)
#endif

//
ZCE_Select_Reactor::ZCE_Select_Reactor():
    read_fd_set_{0},
    write_fd_set_{0},
    exception_fd_set_{0}
{
    initialize(FD_SETSIZE);
}

ZCE_Select_Reactor::ZCE_Select_Reactor(size_t max_event_number):
    read_fd_set_{0},
    write_fd_set_{0},
    exception_fd_set_{0}
{
    initialize(max_event_number);
}

ZCE_Select_Reactor::~ZCE_Select_Reactor()
{
}

//��ʼ��
int ZCE_Select_Reactor::initialize(size_t max_event_number)
{

    //��0
    FD_ZERO(&read_fd_set_);
    FD_ZERO(&write_fd_set_);
    FD_ZERO(&exception_fd_set_);

    return ZCE_Reactor::initialize(max_event_number);
}

//��ĳЩmask��־��
int ZCE_Select_Reactor::schedule_wakeup(ZCE_Event_Handler *event_handler, int event_mask)
{
    int ret = 0;

    ZCE_SOCKET socket_hd = (ZCE_SOCKET) (event_handler->get_handle());
    ZCE_Event_Handler *tmp_handler = NULL;

    //����Ѿ����ڣ����ܼ���ע��
    ret = find_event_handler((ZCE_HANDLE)socket_hd, tmp_handler);
    if (ret != 0)
    {
        ZCE_LOG(RS_INFO, "[zcelib] [%s] fail find handle [%lu],maybe one handle is close previous.",
                __ZCE_FUNC__,
                socket_hd);
        return ret;
    }


    //��Ϊ��Щ��־����һ��ע�ᣬ����������ж��ǲ��еģ����������ͳһ���Ĵ��������Ƕ���д���쳣

    //ע��connect��ʧ�ܣ��ᴥ����д�¼�����Ҫע��,�Ҽǵú����Լ�����������ˡ�
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::READ_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::ACCEPT_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::CONNECT_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::INOTIFY_MASK) )
    {
        FD_SET(socket_hd, &read_fd_set_);
    }

    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::WRITE_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::CONNECT_MASK))
    {
        FD_SET(socket_hd, &write_fd_set_);
    }

    //��WINDOWS�£�����Ƿ��������ӣ��������ʧ�ܷ��ص����¼��ǳ�ʱ
#if defined (ZCE_OS_WINDOWS)

    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::EXCEPT_MASK)
        || ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::CONNECT_MASK))
#elif defined (ZCE_OS_LINUX)
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::EXCEPT_MASK))
#endif
    {
        FD_SET(socket_hd, &exception_fd_set_);
    }



    //Windows��select�������ֵû��Ҫ��,���Ұ�CE����Windows 64λ�İ汾select������һ���������봫��0
#if defined ZCE_OS_LINUX

    //ע�⣬��Ҫ+1��
    if (max_fd_plus_one_ < socket_hd + 1)
    {
        max_fd_plus_one_ = socket_hd + 1;
    }

#endif

    return ZCE_Reactor::schedule_wakeup(event_handler, event_mask);
}

//ȡ��ĳЩmask��־����
int ZCE_Select_Reactor::cancel_wakeup(ZCE_Event_Handler *event_handler, int cancel_mask)
{
    int ret = 0;

    ZCE_SOCKET socket_hd = (ZCE_SOCKET)event_handler->get_handle();
    ZCE_Event_Handler *tmp_handler = NULL;

    //����Ѿ����ڣ����ܼ���ע��
    ret = find_event_handler((ZCE_HANDLE)socket_hd, tmp_handler);
    if (ret != 0)
    {
        ZCE_LOG(RS_INFO, "[zcelib] [%s] fail find handle [%lu],maybe one handle is close previous.",
                __ZCE_FUNC__,
                socket_hd);
        return ret;
    }

    //��Ϊ��Щ��־����һ��ע�ᣬ����������ж��ǲ��е�
    if (ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::READ_MASK)
        || ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::ACCEPT_MASK)
        || ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::CONNECT_MASK)
        || ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::INOTIFY_MASK))
    {
        FD_CLR(socket_hd, &read_fd_set_);
    }

    if (ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::WRITE_MASK)
        || ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::CONNECT_MASK))
    {
        FD_CLR(socket_hd, &write_fd_set_);
    }

#if defined (ZCE_OS_WINDOWS)

    if (ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::EXCEPT_MASK)
        || ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::CONNECT_MASK))
#elif defined (ZCE_OS_LINUX)
    if (ZCE_BIT_IS_SET(cancel_mask, ZCE_Event_Handler::EXCEPT_MASK))
#endif
    {
        FD_CLR(socket_hd, &exception_fd_set_);
    }

    ret = ZCE_Reactor::cancel_wakeup(event_handler, cancel_mask);

    if (0 != ret )
    {
        return ret;
    }

#if defined ZCE_OS_LINUX

    //������µ�maskֵ�����Ϊ0���͸���max_fd_plus_one_
    int new_event_mask =  event_handler->get_mask();

    //����ط���ʵ�ǱȽϺ�ʱ������BOOST������ȡ����ʱ������û�м��٣�
    //�Ҳ���ά��һ��MAP��Ҳ����ACE�����Լ���װһ��fd_set�࣬����ֻ�г����²ߣ�����Ĵ�����˵Ļ�ɬ����Ҳ������
    //�����õ�select�ġ�����+1������-1�ģ�Ҫ��������

    //�����Ҫɾ�����������ļ����ֵ����ô�͵������ֵ
    if (new_event_mask == 0 &&  max_fd_plus_one_ == socket_hd + 1)
    {
        //��Ϊ���ֵ�Ѿ�ɾ������--
        --max_fd_plus_one_;

        while (max_fd_plus_one_ > 0)
        {
            //Ѱ���Ƿ������FD��ע�����Ҫmax_fd_plus_one_-1
            if (FD_ISSET(max_fd_plus_one_ - 1, &read_fd_set_))
            {
                break;
            }

            if (FD_ISSET(max_fd_plus_one_ - 1, &write_fd_set_))
            {
                break;
            }

            if (FD_ISSET(max_fd_plus_one_ - 1, &exception_fd_set_))
            {
                break;
            }

            //
            --max_fd_plus_one_;
        }
    }

#endif

    return 0;
}

//�¼�����
int ZCE_Select_Reactor::handle_events(ZCE_Time_Value *max_wait_time,
                                      size_t *size_event)
{
    //
    *size_event = 0;

    //�����������Ϊselect����������������������Ա��뱣���ˣ���ʱ���鷳ѽ
    para_read_fd_set_ = read_fd_set_;
    para_write_fd_set_ = write_fd_set_;
    para_exception_fd_set_ = exception_fd_set_;

    //
    int const nfds = zce::select (max_fd_plus_one_,
                                  &para_read_fd_set_,
                                  &para_write_fd_set_,
                                  &para_exception_fd_set_,
                                  max_wait_time);

    if (nfds == 0)
    {
        return 0;
    }

    if (nfds == -1)
    {
        // TODO: ������
        return 0;
    }

    //�ϸ����ص���˳�򣬶�ȡ��д���쳣����3��������ɣ�

    //������¼�
    process_ready(&para_read_fd_set_, ZCE_Event_Handler::READ_MASK);

    //����д�¼�
    process_ready(&para_write_fd_set_, ZCE_Event_Handler::WRITE_MASK);

    //�����쳣�¼�
    process_ready(&para_exception_fd_set_, ZCE_Event_Handler::EXCEPT_MASK);

    *size_event = nfds;

    return 0;
}

//����ready��FD��������Ӧ���麯��
void ZCE_Select_Reactor::process_ready(const fd_set *out_fds,
                                       ZCE_Event_Handler::EVENT_MASK proc_mask)
{
    int ret = 0, hdl_ret = 0;
    //
    int max_process = max_fd_plus_one_;

    //������������ζ��ǳ����ƣ�Ҳ���װ�����������ÿ�һ�㣬����...
    //������¼�

#if defined ZCE_OS_WINDOWS
    //WINDOWS��ʵ�ֻ������в�ͬ��������ӿ��ٶ�
    max_process = out_fds->fd_count;
#endif

    for (int i = 0; i < max_process; i++)
    {
        ZCE_SOCKET socket_handle;
        bool hd_ready = zce::is_ready_fds(i, out_fds, &socket_handle);

        if (!hd_ready)
        {
            continue;
        }

        ZCE_Event_Handler *event_hdl = NULL;
        ret =  find_event_handler((ZCE_HANDLE)socket_handle, event_hdl);

        //������ط��������Ǵ���������(���������˶��̣߳�)��Ҳ���ܲ��ǣ���Ϊһ���¼�����󣬿��ܾͱ��ر��ˣ�
        if (0 != ret)
        {
            return;
        }

        //���ݲ�ͬ���¼����е���������

        //READ��ACCEPT�¼�������handle_input��ACCEPT_MASK,INOTIFY_MASK,��д��ԭ���ǣ�������������ڲ����õģ���ֻ����3������
        if (proc_mask == ZCE_Event_Handler::READ_MASK)
        {
            hdl_ret = event_hdl->handle_input();
        }
        //WRITE��CONNECT�¼�������handle_output,CONNECT_MASK,��д��ԭ���ǣ�������������ڲ����õģ���ֻ����3������
        else if (proc_mask == ZCE_Event_Handler::WRITE_MASK)
        {
            hdl_ret = event_hdl->handle_output();
        }
        //�쳣�¼�����ʵ��Ҳ��֪����ʲô���쳣
        else if (proc_mask == ZCE_Event_Handler::EXCEPT_MASK)
        {
#if defined ZCE_OS_WINDOWS
            //����Ƿ��������ӣ�����ʧ�ܺ�ᴥ���쳣�¼���Ϊ�˺�LINUX����ͳһ�����Ǵ���handle_input
            int register_mask = event_hdl->get_mask();

            if (ZCE_BIT_IS_SET(register_mask, ZCE_Event_Handler::CONNECT_MASK) )
            {
                hdl_ret = event_hdl->handle_input();
            }
            else
            {
                hdl_ret = event_hdl->handle_exception();
            }

#elif defined ZCE_OS_LINUX
            hdl_ret = event_hdl->handle_exception();
#endif

        }

        else
        {
            ZCE_ASSERT(false);
        }

        //����-1��ʾ handle_xxxxxϣ������handle_close�˳�
        if (hdl_ret == -1)
        {
            event_hdl->handle_close();
        }
    }
}


#if defined (ZCE_OS_WINDOWS)
#pragma warning(default : 4127)
#endif
