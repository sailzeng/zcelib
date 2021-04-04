#include "zce_predefine.h"
#include "zce_os_adapt_time.h"
#include "zce_log_logging.h"
#include "zce_timer_handler_base.h"
#include "zce_timer_queue_base.h"

//ʹ�ýϸߵĶ�ʱ�����ȣ�������CPUһ�㣬����ʱ���ľ��Ȼ��һЩ
//#define MORE_HIGH_TIMER_PRECISION 1

ZCE_Timer_Queue_Base *ZCE_Timer_Queue_Base::instance_ = NULL;
/******************************************************************************************
ZCE_Timer_Queue ����ʱ���Ļ���
******************************************************************************************/
//���캯��
ZCE_Timer_Queue_Base::ZCE_Timer_Queue_Base(size_t num_timer_node,
                                           unsigned int timer_precision_mesc,
                                           TRIGGER_MODE trigger_mode,
                                           bool dynamic_expand_node):
    dynamic_expand_node_(dynamic_expand_node),
    free_node_id_head_(INVALID_TIMER_ID)
{
    int ret = initialize(num_timer_node,
                         timer_precision_mesc,
                         trigger_mode,
                         dynamic_expand_node);
    assert(ret == 0);
    ZCE_UNUSED_ARG(ret);
}

ZCE_Timer_Queue_Base::ZCE_Timer_Queue_Base()
{
}

//��������
ZCE_Timer_Queue_Base::~ZCE_Timer_Queue_Base()
{
    //ʹ��vector�ĺô������Լ����ù����ڴ���
}

//��ʼ��
int ZCE_Timer_Queue_Base::initialize(size_t num_timer_node,
                                     unsigned int timer_precision_mesc,
                                     TRIGGER_MODE trigger_mode,
                                     bool dynamic_expand_node)
{

    //ʱ�侫������
    timer_precision_mesc_ = timer_precision_mesc;
    //Ŀǰ��NODE��������0
    num_timer_node_ = 0;

    num_use_node_ = 0;
    dynamic_expand_node_ = dynamic_expand_node;

    trigger_mode_ = trigger_mode;

    //
    size_t old_num_node = 0;
    extend_node(num_timer_node, old_num_node);

    //����ģʽ
    if (trigger_mode_ == TRIGGER_MODE_SYSTEM_CLOCK)
    {
        //�õ�һ��ʱ�䳤�ȣ����ڱȽϺ���
        ZCE_Time_Value  now_time(zce::gettimeofday());

        //ȡ�ú���
        prev_trigger_msec_ = now_time.total_msec();
    }
    //
    else if (trigger_mode_ == TRIGGER_MODE_CPU_TICK)
    {
        //��ʵ�ⲻ��������CPU Tick
        ZCE_Time_Value cpu_tick(zce::get_uptime());
        prev_trigger_msec_ = cpu_tick.total_msec();
    }
    else
    {
        ZCE_ASSERT(false);
    }

    timer_refer_pointer_ = prev_trigger_msec_;

    return 0;
}


//�ر�
int ZCE_Timer_Queue_Base::close()
{
    //�����ж�ʱ����TIME ID����
    for (size_t i = 0; i < num_timer_node_; ++i)
    {
        //����һ���Լ���time_node_ary_�Ǽ�¼����NODE�Ķ��У�����ʹ�õģ�FREE�ģ�
        if (time_node_ary_[i].timer_handle_)
        {
            //ΪʲôҪ��ֵ��ʹ���أ��ҵ���timer_close�ᱻ�����������
            ZCE_Timer_Handler *time_hdl = time_node_ary_[i].timer_handle_;
            time_hdl->timer_close();
        }
    }

    return 0;
}

//���ŵ�NODE��������
int ZCE_Timer_Queue_Base::extend_node(size_t num_timer_node,
                                      size_t &old_num_node)
{
    //�ܲ��ܱ�ԭ����С��
    assert(num_timer_node > num_timer_node_);

    old_num_node = num_timer_node_;
    num_timer_node_ = num_timer_node;

    //��û����������20�ڸ���ʱ����лл���ҵ���
    ZCE_ASSERT(num_timer_node_ > 0 && num_timer_node_ < 0x7FFFFFFF);

    //
    if (num_timer_node_  > 0x7FFFFFFF)
    {
        return -1;
    }

    //
    time_node_ary_.resize(num_timer_node_);
    free_node_id_list_.resize(num_timer_node_);

    //�����ж�ʱ����TIME ID����
    for (size_t i = old_num_node; i < num_timer_node_; ++i)
    {
        time_node_ary_[i].time_id_ = static_cast<int>(i);
    }

    //���ɿ��ж��е���ֵ������

    //��ʼ������,�γ�һ������NODE�Ķ���
    for (size_t i = old_num_node; i < num_timer_node_ - 1; ++i)
    {
        free_node_id_list_[i] = static_cast<int>(i + 1);
    }

    //���һ����ʾ��Ч����,
    free_node_id_list_[num_timer_node_ - 1] = INVALID_TIMER_ID;

    free_node_id_head_ = static_cast<int>(old_num_node);

    return 0;
}

//ȡ����ʱ����
int ZCE_Timer_Queue_Base::cancel_timer(int timer_id)
{
    return free_timernode(timer_id);
}

//ȡ����ʱ���������������������ĺ�����ƽ��ʱ�临�Ӷ�O(N),N�Ƕ��еĳ���
int ZCE_Timer_Queue_Base::cancel_timer(const ZCE_Timer_Handler *timer_hdl)
{
    assert(timer_hdl);

    //�����������Ż�һ�£���һ��ʹ�õ�HANDLE���б���ʹ�ù��Ķ��У������������,
    //�ҿ�����һ�·�Ҫ��˫�������ҷ�����

    bool cancel_time = false;

    for (size_t i = 0; i < num_timer_node_; ++i)
    {
        if (time_node_ary_[i].timer_handle_ == timer_hdl )
        {
            uint32_t time_id = static_cast<uint32_t > (i);
            cancel_timer(time_id);
            cancel_time = true;

            //����break,��ΪHANDLE���ж�Ӧ���NODE,
        }
    }

    //���ȡ����ʱ��
    if (!cancel_time)
    {
        return -1;
    }

    //�ɹ�
    return 0;
}

//����Timer Node
int ZCE_Timer_Queue_Base::alloc_timernode(ZCE_Timer_Handler *timer_hdl,
                                          const void *action,
                                          const ZCE_Time_Value &delay_time,
                                          const ZCE_Time_Value &interval_time,
                                          int &time_node_id,
                                          ZCE_TIMER_NODE *&alloc_time_node)
{
    //TIME HANDLE����ΪNULL
    assert(timer_hdl != NULL);

    int ret = 0;
    //Ĭ��û�з��䵽�������������
    time_node_id = INVALID_TIMER_ID;
    alloc_time_node = NULL;

    //����Ѿ�û����NODE
    if (free_node_id_head_ == INVALID_TIMER_ID)
    {
        //�����֧�ֶ�̬���ţ����ش���
        if (!dynamic_expand_node_)
        {
            return -1;
        }
        else
        {
            //���ϣ����̬���ţ��Ұ�������
            size_t old_num_node = 0;
            ret = extend_node(num_timer_node_ + ONCE_EXTEND_NODE_NUMBER, old_num_node);

            if (ret != 0)
            {
                return -1;
            }
        }
    }

    //�õ�ʱ��NODE
    time_node_id = free_node_id_head_;
    free_node_id_head_ = free_node_id_list_[free_node_id_head_];
    alloc_time_node = &(time_node_ary_[time_node_id]);

    //��ʱ��NODE���и�ֵ
    alloc_time_node->timer_handle_ =  timer_hdl;
    alloc_time_node->interval_time_ =  interval_time;
    alloc_time_node->action_ =  action;

    //�����׷��߽��ȣ���������꣬Ĭ���ҹر���
#if defined(MORE_HIGH_TIMER_PRECISION)

    uint64_t now_point = 0;

    //Ĭ����ĵ��ô�����Ƶ�����ṩ���Ӱ�ȫ��ʱ�����
    if (trigger_mode_ == TRIGGER_MODE_SYSTEM_CLOCK)
    {
        ZCE_Time_Value now_trigger_time;
        now_trigger_time.gettimeofday();
        now_point = now_trigger_time.total_msec();
    }
    else if (trigger_mode_ == TRIGGER_MODE_CPU_TICK)
    {
        //��ʵ�ⲻ��������CPU Tick
        ZCE_Time_Value cpu_tick(zce::get_uptime());
        now_point = cpu_tick.total_msec();
    }

    alloc_time_node->next_trigger_point_ = now_point + delay_time.total_msec();

#else
    //��Ҫע��ģ�����ط����Ƿǳ���ȷ����Ϊ�˼ӿ�ִ���ٶ�͵���ˣ�
    //��Ȼ�����ǣ�����õ�dispatch_timer�����ж�󣬵��õ�Խ�࣬timer_refer_pointer_��Խ׼ȷ��
    //��������Ƶ�ȵ͵����ˣ�<1s��,�ң��ң�

    //�õ�Ҫ�������Ǹ�TICK�㣬
    alloc_time_node->next_trigger_point_ = timer_refer_pointer_ + delay_time.total_msec();

#endif

    ++num_use_node_;

    //���Դ��룬����ʱ��
    //ZCE_LOG(RS_DEBUG,"[zcelib] ZCE_Timer_Queue alloc_timernode.time_node_id[%d] "
    //  "next_trigger_point_[%llu], prev_trigger_msec_[%llu],timer_refer_pointer_[%llu].",
    //  time_node_id,
    //  alloc_time_node->next_trigger_point_,
    //  prev_trigger_msec_,
    //  timer_refer_pointer_,
    //  );

    return 0;
}

//������һ�������㣬
void ZCE_Timer_Queue_Base::calc_next_trigger(int time_node_id,
                                             uint64_t now_trigger_msec,
                                             bool &continue_trigger)
{
    continue_trigger = false;

    ZCE_TIMER_NODE *prc_time_node = &(time_node_ary_[time_node_id]);

    //������ʱ��Ϊ0���������ʱ����û�д�������already_trigger_ �ж���ʵ�Ǽ�鶨ʱ���Ƿ��������ù�
    if (prc_time_node->interval_time_  == ZCE_Time_Value::ZERO_TIME_VALUE )
    {
        //
        continue_trigger = false;
        return;
    }

    //���Ҫ��Ҫ�õ�ǰ��ʵʱ�䣬�������Լ���¼�Ĵ�������Ϊ����㣬�����ȷ��һ������
    //��Ŀǰ�������Լ���¼�Ĵ�������Ϊ����㣬�������Ȳ��ã������ᶪ������
    continue_trigger = true;

    //������һ�������㣬��
    prc_time_node->next_trigger_point_ = now_trigger_msec + prc_time_node->interval_time_.total_msec();

    return;
}

//�ͷ�Timer Node
int ZCE_Timer_Queue_Base::free_timernode(int time_node_id)
{
    //������һ�»����ö����ˣ�������д������������
    ZCE_ASSERT(time_node_ary_[time_node_id].timer_handle_ != NULL &&
               static_cast<size_t>( time_node_id) < num_timer_node_);

    //���NODE�ڲ���ָ��ΪNULL,��ʾ���NODEû��ʹ��
    if (!time_node_ary_[time_node_id].timer_handle_)
    {
        return -1;
    }

    //�����ܳ���NODE������
    if (static_cast<size_t>(time_node_id) >= num_timer_node_)
    {
        return -1;
    }

    //�黹��FREE������
    free_node_id_list_[ time_node_id ] = free_node_id_head_;
    free_node_id_head_ = time_node_id;
    //��������ȥ
    time_node_ary_[time_node_id].clear();
    --num_use_node_;

    return 0;
}

//�õ���콫�ڶ���ʱ��󴥷�
int ZCE_Timer_Queue_Base::get_first_timeout(ZCE_Time_Value *first_timeout)
{
    int ret = 0;
    int time_node_id = INVALID_TIMER_ID;
    ret = get_frist_nodeid(time_node_id);

    if (INVALID_TIMER_ID == time_node_id || -1 == ret )
    {
        return -1;
    }

    ZCE_Time_Value now_time(zce::gettimeofday());
    uint64_t now_trigger_msec = 0;

    //����ģʽ
    if (trigger_mode_ == TRIGGER_MODE_SYSTEM_CLOCK)
    {
        //ȡ�ú���
        now_trigger_msec = now_time.total_msec();
    }
    //
    else if (trigger_mode_ == TRIGGER_MODE_CPU_TICK)
    {
        //��ʵ�ⲻ��������CPU Tick
        ZCE_Time_Value cpu_tick(zce::get_uptime());
        now_trigger_msec = cpu_tick.total_msec();
    }
    else
    {
        ZCE_ASSERT(false);
    }

    uint64_t first_timeout_msec = 0;

    //
    if ( time_node_ary_[time_node_id].next_trigger_point_ > now_trigger_msec )
    {
        first_timeout_msec = time_node_ary_[time_node_id].next_trigger_point_ - now_trigger_msec;
    }

    first_timeout->total_msec(first_timeout_msec);

    return 0;
}

size_t ZCE_Timer_Queue_Base::expire()
{
    ZCE_Time_Value now_time(zce::gettimeofday());

    uint64_t now_trigger_msec = 0;

    //����ģʽ
    if (trigger_mode_ == TRIGGER_MODE_SYSTEM_CLOCK)
    {
        //ȡ�ú���
        now_trigger_msec = now_time.total_msec();
    }
    //
    else if (trigger_mode_ == TRIGGER_MODE_CPU_TICK)
    {
        //��ʵ�ⲻ��������CPU Tick
        ZCE_Time_Value cpu_tick(zce::get_uptime());
        now_trigger_msec = cpu_tick.total_msec();
    }
    else
    {
        ZCE_ASSERT(false);
    }

    return dispatch_timer(now_time, now_trigger_msec);
}

//�õ�Ψһ�ĵ���ʵ��
ZCE_Timer_Queue_Base *ZCE_Timer_Queue_Base::instance()
{
    return instance_;
}

//��ֵΨһ�ĵ���ʵ��
void ZCE_Timer_Queue_Base::instance(ZCE_Timer_Queue_Base *pinstatnce)
{
    clean_instance();
    instance_ = pinstatnce;
    return;
}

//�������ʵ��
void ZCE_Timer_Queue_Base::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}

