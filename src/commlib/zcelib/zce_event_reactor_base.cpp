#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_log_logging.h"
#include "zce_event_handle_base.h"
#include "zce_event_reactor_base.h"

//
ZCE_Reactor   *ZCE_Reactor::instance_ = NULL;

ZCE_Reactor::ZCE_Reactor():
    max_event_number_(FD_SETSIZE)
{
    initialize(max_event_number_);
}

ZCE_Reactor::ZCE_Reactor(size_t max_event_number):
    max_event_number_(max_event_number)
{
    initialize(max_event_number_);
}

ZCE_Reactor::~ZCE_Reactor()
{
}

//��ǰ��Ӧ�������ľ������
size_t ZCE_Reactor::size()
{
    return handler_map_.size();
}

//��ǰ��Ӧ�����������������������
size_t ZCE_Reactor::max_size()
{
    return max_event_number_;
}

//
int ZCE_Reactor::initialize(size_t max_event_number)
{
    max_event_number_ = max_event_number;
    handler_map_.rehash(max_event_number_ + 16);

    return 0;
}

//�رշ�Ӧ����������ע���EVENT HANDLER ע����
int ZCE_Reactor::close()
{
    //������HASH MAP�ٶ��е���
    MAP_OF_HANDLER_TO_EVENT::iterator iter_temp =  handler_map_.begin();

    //
    for (; iter_temp != handler_map_.end();)
    {
        //�ر�֮
        ZCE_Event_Handler *event_handler = (iter_temp->second);

        //��handle_close,
        event_handler->handle_close();

        //�õ������������ʼ����
        iter_temp = handler_map_.begin();
    }

    handler_map_.clear();

    return 0;
}

//ע��һ��ZCE_Event_Handler����Ӧ��
int ZCE_Reactor::register_handler(ZCE_Event_Handler *event_handler,
                                  int event_mask )
{

    int ret = 0;

    //����Ѿ�����������������ش���
    if (handler_map_.size() >= max_event_number_)
    {
        return -1;
    }

    ZCE_HANDLE socket_hd = event_handler->get_handle();
    ZCE_Event_Handler *tmp_handler = NULL;

    //����Ѿ����ڣ����ܼ���ע��
    ret = find_event_handler(socket_hd, tmp_handler);
    if (ret == 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] [%s] find_event_handler eaqul handle [%lu]. please check you code .",
                __ZCE_FUNC__,
                tmp_handler);
        return -1;
    }

    //������ˣ�ʧ���˾���������
    handler_map_.insert(std::make_pair(socket_hd, event_handler));

    if (event_mask != 0)
    {
        schedule_wakeup(event_handler, event_mask);
    }

    return 0;
}

//�ӷ�Ӧ��ע��һ��ZCE_Event_Handler��ͬ��ȡ�������е�mask
//event_mask��ʵֻ�ж������DONT_CALL
int ZCE_Reactor::remove_handler(ZCE_Event_Handler *event_handler,
                                bool call_handle_close)
{
    int ret = 0;

    ZCE_HANDLE ev_hd = event_handler->get_handle();
    ZCE_Event_Handler *tmp_handler = NULL;

    //remove_handler���ܻ�������ε��õ���������Ƽ���ֱ�ӵ���handle_close
    ret = find_event_handler(ev_hd, tmp_handler);
    if (ret != 0)
    {
        // δ�ҵ�
        ZCE_LOG(RS_INFO, "[zcelib][%s] find handle [%lu] fail. my be reclose ?",
                __ZCE_FUNC__,
                ev_hd);
        return -1;
    }

    //������������Ա�������Ϊ���ܳ���һЩ�ظ����õ�����
    //ZCE_ASSERT_DGB(tmp_handler == event_handler);

    int event_mask = event_handler->get_mask();

    //�����mask��ȡ����
    if (event_mask != 0)
    {
        cancel_wakeup(event_handler, event_mask);
    }

    //������ˣ�ʧ���˾���������
    handler_map_.erase(event_handler->get_handle());

    //
    if (call_handle_close)
    {
        //����handle_close
        event_handler->handle_close();
    }

    return 0;
}

//
int ZCE_Reactor::cancel_wakeup(ZCE_Event_Handler *event_handler, int cancel_mask)
{
    event_handler->disable_mask(cancel_mask);
    return 0;
}

//
int ZCE_Reactor::schedule_wakeup(ZCE_Event_Handler *event_handler, int event_mask)
{
    event_handler->enable_mask(event_mask);
    return 0;
}

//�õ�Ψһ�ĵ���ʵ��
ZCE_Reactor *ZCE_Reactor::instance()
{
    //����ط����������Ӻ�����ͬ��Ҫ�ȸ�ֵ
    return instance_;
}

//��ֵΨһ�ĵ���ʵ��
void ZCE_Reactor::instance(ZCE_Reactor *pinstatnce)
{
    clean_instance();
    instance_ = pinstatnce;
    return;
}

//�������ʵ��
void ZCE_Reactor::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}

