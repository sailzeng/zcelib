#include "zce_predefine.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_predefine.h"
#include "zce_event_handle_base.h"
#include "zce_log_logging.h"
#include "zce_time_value.h"
#include "zce_event_reactor_epoll.h"

//���캯������������
ZCE_Epoll_Reactor::ZCE_Epoll_Reactor():
    ZCE_Reactor(FD_SETSIZE),
    epoll_fd_(-1),
    edge_triggered_(false),
    once_max_events_(DEFAULT_ONCE_TRIGGER_MAX_EVENT),
    once_events_ary_(NULL)
{
}

//���캯��
ZCE_Epoll_Reactor::ZCE_Epoll_Reactor(size_t max_event_number,
                                     bool edge_triggered,
                                     int once_max_events):
    ZCE_Reactor(max_event_number),
    epoll_fd_(-1),
    edge_triggered_(edge_triggered),
    once_max_events_(once_max_events),
    once_events_ary_(NULL)
{
    initialize(max_event_number, edge_triggered, once_max_events);
}

ZCE_Epoll_Reactor::~ZCE_Epoll_Reactor()
{
    ::close(epoll_fd_);

    //�ͷ��ڴ�
    if (once_events_ary_)
    {
        delete [] once_events_ary_;
    }
}

//��ʼ��
int ZCE_Epoll_Reactor::initialize(size_t max_event_number,
                                  bool edge_triggered,
                                  int once_max_events )
{
    //����Ƿ�Linux����ϵͳ��ֱ���군���ˡ�
#if !defined (ZCE_OS_LINUX)
    ZCE_ASSERT(false);
#endif

    int ret = 0;
    ret = ZCE_Reactor::initialize(max_event_number);

    if (0  != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] Epoll reactor ZCE_Reactor::initialize fail.please check code. ret = %u.", ret);
        return ret;
    }

    edge_triggered_ = edge_triggered;

    //���ֻ��LINUX�²���
#if defined (ZCE_OS_LINUX)
    epoll_fd_ = ::epoll_create(max_event_number_ + 64);
#endif

    if (epoll_fd_ < 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] Epoll reactor ::epoll_create fail.please check code. error = [%u|%u]",
                zce::last_error(),
                strerror(zce::last_error()));
        return -1;
    }

    once_max_events_ = once_max_events;

    once_events_ary_ = new epoll_event [once_max_events_];

    return 0;
}

//ע��һ��ZCE_Event_Handler����Ӧ��
int ZCE_Epoll_Reactor::register_handler(ZCE_Event_Handler *event_handler, int event_mask)
{
    int ret = 0;
    //ע��ڶ���������0����Ϊ��һҪ��ADD���ڶ��������ε������,��������ǰ������Ϊ�ع��鷳
    ret = ZCE_Reactor::register_handler(event_handler, 0);

    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] ZCE_Reactor::register_handler fail. please check you code .ret =%d", ret);
        return -1;
    }

    event_handler->set_mask(event_mask);

    struct epoll_event ep_event;
    make_epoll_event(&ep_event, event_handler);

    //EPOLL ��LINUX���У�WINDOWSҲ������ģ��������ѵ������ø�SELECTģ�⣿
#if defined (ZCE_OS_LINUX)
    ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event_handler->get_handle(), &ep_event);
#endif

    if (0 != ret)
    {
        //�ع�
        ret = ZCE_Reactor::remove_handler(event_handler, false);

        ZCE_LOG(RS_ERROR, "[zcelib] [%s] Epoll reactor ::epoll_ctl fail.please check code. ret =%d error = [%u|%s]",
                __ZCE_FUNC__,
                ret,
                zce::last_error(),
                strerror(zce::last_error()));
        return -1;
    }

    return 0;
}

//�ӷ�Ӧ��ע��һ��ZCE_Event_Handler��ͬ��ȡ�������е�mask
int ZCE_Epoll_Reactor::remove_handler(ZCE_Event_Handler *event_handler, bool call_handle_close)
{

    int ret = 0;

    struct epoll_event event;
    event.events = 0;
#if defined (ZCE_OS_LINUX)
    //����epoll_ctl EPOLL_CTL_DEL ɾ��ע�����
    ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, event_handler->get_handle(), &event);
#endif

    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] [%s] epoll reactor ::epoll_ctl fail.please check code. ret =%d error = [%u|%s]",
                __ZCE_FUNC__,
                ret,
                zce::last_error(),
                strerror(zce::last_error()));
        //��������غû��Ƿ��غ��أ�������һ������
        //return -1;
    }

    //ȡ�������е��¼���ǰ���Ѿ�ɾ���ˣ����������ظ�����
    event_handler->set_mask(0);

    ret = ZCE_Reactor::remove_handler(event_handler, call_handle_close);

    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] ZCE_Reactor::remove_handler fail. please check you code .ret =%u", ret);
        return -1;
    }

    return 0;
}

//ȡ��ĳЩmask��־����
int ZCE_Epoll_Reactor::cancel_wakeup(ZCE_Event_Handler *event_handler, int cancel_mask)
{
    int ret = 0;

    ret = ZCE_Reactor::cancel_wakeup(event_handler, cancel_mask);

    //��ʵZCE_Reactor::cancel_wakeup������ʧ��,
    if (0 != ret)
    {
        return -1;
    }

    struct epoll_event ep_event;

    make_epoll_event(&ep_event, event_handler);

#if defined (ZCE_OS_LINUX)
    //EPOLL_CTL_MOD�����޸�
    ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, event_handler->get_handle(), &ep_event);

#endif

    if (0 != ret)
    {
        //�ع����Ķ���־λ
        ZCE_Reactor::schedule_wakeup(event_handler, cancel_mask);
        ZCE_LOG(RS_ERROR, "[zcelib] [%s] epoll reactor ::epoll_ctl fail.please check code. ret =%d error = [%u|%s]",
                __ZCE_FUNC__,
                ret,
                zce::last_error(),
                strerror(zce::last_error()));
        return -1;
    }

    return 0;
}

//��ĳЩmask��־��
int ZCE_Epoll_Reactor::schedule_wakeup(ZCE_Event_Handler *event_handler, int event_mask)
{
    int ret = 0;
    ret = ZCE_Reactor::schedule_wakeup(event_handler, event_mask);

    //��ʵZCE_Reactor::cancel_wakeup������ʧ��
    if (0 != ret)
    {
        return -1;
    }

    struct epoll_event ep_event;

    make_epoll_event(&ep_event, event_handler);

#if defined (ZCE_OS_LINUX)
    //EPOLL_CTL_MOD�����޸�
    ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, event_handler->get_handle(), &ep_event);

#endif

    if (0 != ret)
    {
        //�ع����޸���־λ
        ZCE_Reactor::cancel_wakeup(event_handler, event_mask);
        ZCE_LOG(RS_ERROR, "[zcelib] [%s] epoll reactor ::epoll_ctl fail.please check code. ret =%d error = [%u|%s].",
                __ZCE_FUNC__,
                ret,
                zce::last_error(),
                strerror(zce::last_error()));
        return -1;
    }

    return 0;
}

//ʱ�䴥��
int ZCE_Epoll_Reactor::handle_events(ZCE_Time_Value *time_out, size_t *size_event)
{

    //Ĭ��һֱ����
    int msec_timeout = -1;

    if (time_out)
    {
        //����bluehu �����޸���������Σ�������С�����ǣ����ǸĴ���Ҫ����һ��ร�
        //����select�ĳ�ʱ�������Ծ�ȷ��΢�룬��epoll_wait�Ĳ���ֻ��ȷ������
        //����ʱʱ��С��1000΢��ʱ������20΢�룬��ʱ��ת���ɺ������0����
        //���������epoll_wait�Ļ�����ʱʱ�����0����С��1����������ͳһ��Ϊ1����

        msec_timeout = static_cast<int>( time_out->total_msec_round());
    }

    int event_happen = 0;

#if defined (ZCE_OS_LINUX)
    //EPOLL�ȴ��¼�������
    event_happen = ::epoll_wait(epoll_fd_, once_events_ary_, once_max_events_, msec_timeout);
#endif

    if (event_happen <= 0)
    {
        return event_happen;
    }

    *size_event = event_happen;

    for (int i = 0; i < event_happen; ++i)
    {
        process_ready_event(once_events_ary_ + i);
    }

    return 0;

}

void ZCE_Epoll_Reactor::process_ready_event(struct epoll_event *ep_event)
{
    int ret = 0;
    ZCE_Event_Handler *event_hdl = NULL;
    bool event_in_happen = false, event_out_happen = false;

    ret =  find_event_handler(ep_event->data.fd, event_hdl);

    //������ط��������Ǵ��������⣬Ҳ���ܲ��ǣ���Ϊһ���¼�����󣬿��ܾͱ��ر��ˣ�
    if (0 != ret)
    {
        return;
    }

    //���ݲ�ͬ���¼����е���������
    int hdl_ret = 0;

    //��ע�⡿���е�reactor ��Ҫ��֤���ִ�����˳�򣬱��뱣֤��������в����ݵ����⣬��ȷ��˳��Ӧ���Ƕ�,д,�쳣����

    //����connect�������Ӵ����ڲ�ϣ��ֹͣ������epoll�᷵��3���¼�������,
    //��˵��һ��������⣬���ڲ�������¼�����������ϲ���ܾͻ�ɾ������Ӧ��ZCE_Event_Handler�����ײ���ܻ����¼�Ҫ������
    //���ڸô����ʱ�����پ����ұ���дһ��˵�������ڸ���δ����ʱ���м����ַ�ʽ,
    //1.����д��do { break} while()�ķ�ʽ��ÿ��IO�¼�����Ϊ������ʽ������ô��Ǽ򵥣�������epoll,�����ˮƽ��������ô����ܶ�ʧ�¼�
    //2.����ʹ��return -1��Ϊһ���жϣ�ACE���������ģ�ԭ����������ACE�������е����࣬���Լ���ƿ����������кô��ģ�
    //3.ÿ�ε��ú󣬶����һ��event_hdl�Ƿ񻹴��ڣ�
    //����Ҳ����˼���2��3�ķ�����2��Ϊ�˺�ACE���ݣ�3��Ϊ�˱�֤������TMD�췭�ظ�����Ҳ��Ӧ��,

    //�����е�֣�����Ŀ���Ǽӿ��ٶ�,����ÿ�����ö�Ҫ��飬

    //READ��CONNECT�¼�������handle_input
    if (ep_event->events &  EPOLLIN )
    {
        event_in_happen = true;
        hdl_ret = event_hdl->handle_input();

        //����-1��ʾ handle_xxxxxϣ������handle_close�˳�
        if (hdl_ret == -1)
        {
            event_hdl->handle_close();
        }
    }

    //READ��ACCEPT�¼�������handle_input
    if (ep_event->events & EPOLLOUT )
    {
        //���д�¼������ˣ���ô������ܵ���handle_close,�ٲ�ѯһ�Σ�double check.
        if (event_in_happen)
        {
            ret =  find_event_handler(ep_event->data.fd, event_hdl);

            //������ط��������Ǵ��������⣬Ҳ���ܲ��ǣ���Ϊһ���¼�����󣬿��ܾͱ��ر��ˣ�
            if (0 != ret)
            {
                return;
            }
        }

        event_out_happen = true;
        hdl_ret = event_hdl->handle_output();

        //����-1��ʾ handle_xxxxxϣ������handle_close�˳�
        if (hdl_ret == -1)
        {
            event_hdl->handle_close();
        }

    }

    //�쳣�¼�����ʵ��Ҳ��֪����ʲô���쳣
    if (ep_event->events & EPOLLERR )
    {
        //�����ȡ����д�¼������ˣ���ô������ܵ���handle_close,�ٲ�ѯһ�Σ�double check.
        if (event_out_happen || event_in_happen)
        {
            ret =  find_event_handler(ep_event->data.fd, event_hdl);

            //������ط��������Ǵ��������⣬Ҳ���ܲ��ǣ���Ϊһ���¼�����󣬿��ܾͱ��ر��ˣ�
            if (0 != ret)
            {
                return;
            }
        }

        hdl_ret = event_hdl->handle_exception();

        //����-1��ʾ handle_xxxxxϣ������handle_close�˳�
        if (hdl_ret == -1)
        {
            event_hdl->handle_close();
        }
    }
}

