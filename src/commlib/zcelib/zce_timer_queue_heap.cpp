#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_timer_handler_base.h"
#include "zce_timer_queue_heap.h"

//ע������C/C++���±�仯�Ǵ�0��ʼ�ģ�
#define ZCE_TIMER_HEAP_PARENT(x)  ((x) == 0 ? 0 : (((x) - 1) / 2))
//���������±�
#define ZCE_TIMER_HEAP_LCHILD(x)  ((x) + (x) + 1)
//���������±�
#define ZCE_TIMER_HEAP_RCHILD(x)  ((x) + (x) + 2)

//
ZCE_Timer_Heap::ZCE_Timer_Heap(size_t num_timer_node,
                               unsigned int timer_precision_mesc,
                               TRIGGER_MODE trigger_mode,
                               bool dynamic_expand_node)
{
    int ret = ZCE_Timer_Heap::initialize(num_timer_node,
                                         timer_precision_mesc,
                                         trigger_mode,
                                         dynamic_expand_node);

    //
    if ( 0 != ret )
    {
        ZCE_LOG(RS_ERROR, "[zcelib] ZCE_Timer_Heap::initialize fail.");
    }

}

//���캯��
ZCE_Timer_Heap::ZCE_Timer_Heap():
    size_heap_(0)
{
}

ZCE_Timer_Heap::~ZCE_Timer_Heap()
{

}

int ZCE_Timer_Heap::initialize(size_t num_timer_node,
                               unsigned int timer_precision_mesc,
                               TRIGGER_MODE trigger_mode,
                               bool dynamic_expand_node)
{
    //�ڻ�����г�ʼ��
    int ret = 0;
    ret = ZCE_Timer_Queue_Base::initialize(num_timer_node,
                                           timer_precision_mesc,
                                           trigger_mode,
                                           dynamic_expand_node);

    if (ret != 0)
    {
        return ret;
    }

    //��ʼ�׶�û���κ�����
    size_heap_ = 0;

    return 0;
}

//�������ʮ�������NODE��������Ҳ���õײ��extend_node����
int ZCE_Timer_Heap::extend_node(size_t num_timer_node,
                                size_t &old_num_node)
{
    int ret = 0;
    ret = ZCE_Timer_Queue_Base::extend_node(num_timer_node, old_num_node);

    if (ret != 0)
    {
        return ret;
    }

    timer_node_heap_.resize(num_timer_node_);

    //��QUEUE�ϵ����㴦�����Ч����ʾû�й�NODE
    for (size_t i = old_num_node; i < num_timer_node_; ++i)
    {
        timer_node_heap_[i] = INVALID_TIMER_ID;
    }

    note_to_heapid_.resize(num_timer_node_);

    //��QUEUE�ϵ����㴦�����Ч����ʾû�ж�Ӧ��ϵ
    for (size_t i = old_num_node; i < num_timer_node_; ++i)
    {
        note_to_heapid_[i] = INVALID_TIMER_ID;
    }

    return 0;
}

//�ַ���ʱ��
size_t ZCE_Timer_Heap::dispatch_timer(const ZCE_Time_Value &now_time,
                                      uint64_t now_trigger_msec)
{
    int ret = 0;
    //�ַ�������
    size_t num_dispatch = 0;

    int timer_node_id = INVALID_TIMER_ID;
    ret = get_frist_nodeid(timer_node_id);

    while (timer_node_id != INVALID_TIMER_ID )
    {
        //����Ѿ���ʱ�����д���
        if ( time_node_ary_[timer_node_id].next_trigger_point_ <= now_trigger_msec )
        {
            ++num_dispatch;
            //��������ʱ���Ѿ�����������ϸ��already_trigger_�Ľ���
            time_node_ary_[timer_node_id].already_trigger_ = true;
            //ʱ�Ӵ���
            time_node_ary_[timer_node_id].timer_handle_->timer_timeout(now_time,
                                                                       time_node_ary_[timer_node_id].action_);

        }
        else
        {
            break;
        }

        //��Ϊtimer_timeout��ʵ����ȡ���������ʱ���������ڵ���֮��Ҫ����һ�¼��
        if (time_node_ary_[timer_node_id].timer_handle_ && time_node_ary_[timer_node_id].already_trigger_ == true )
        {
            //���¹滮���TIME NODE��λ�õ�,�������Ҫ��������ȡ����ʱ��
            reschedule_timer(timer_node_id, now_trigger_msec);
        }

        //
        ret = get_frist_nodeid(timer_node_id);

        if ( 0 != ret )
        {
            break;
        }
    }

    prev_trigger_msec_ = now_trigger_msec;

    //��������
    return num_dispatch;
}

//���ö�ʱ��
int ZCE_Timer_Heap::schedule_timer(ZCE_Timer_Handler *timer_hdl,
                                   const void *action,
                                   const ZCE_Time_Value &delay_time,
                                   const ZCE_Time_Value &interval_time)
{
    int ret = 0;
    int time_node_id = INVALID_TIMER_ID;

    //���ܷ����һ��TIME NODE
    ZCE_TIMER_NODE *alloc_time_node = NULL;
    ret = alloc_timernode(timer_hdl,
                          action,
                          delay_time,
                          interval_time,
                          time_node_id,
                          alloc_time_node
                         );

    //ע�⣬����ط�����INVALID_TIMER_ID��ʾ��Ч����ʵҲ�������Ӧ��������ƣ���Ϊ�˼���ACE�Ĵ���
    if (ret != 0)
    {
        return INVALID_TIMER_ID;
    }

    ret = add_nodeid(time_node_id);

    if (ret != 0)
    {
        return INVALID_TIMER_ID;
    }

    return time_node_id;
}

//ȡ����ʱ��
int ZCE_Timer_Heap::cancel_timer(int timer_id)
{
    //
    int ret = 0;

    //�ȴӶ�����ɾ������ڵ�
    ret = remove_nodeid(timer_id);

    if (ret != 0)
    {
        return ret;
    }

    //����TIME NODE
    ret = ZCE_Timer_Queue_Base::cancel_timer(timer_id);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int ZCE_Timer_Heap::reschedule_timer(int timer_id, uint64_t now_trigger_msec)
{
    bool contiue_trigger = false;

    //������һ�δ����ĵ�
    calc_next_trigger(timer_id,
                      now_trigger_msec,
                      contiue_trigger);

    //�������Ҫ����������ʱ���ˣ�ȡ��
    if (!contiue_trigger)
    {
        return cancel_timer(timer_id);
    }

    //���TIMER NODE ��ȻҪ����

    //�ȴӶ�����ɾ������ڵ�
    int ret = remove_nodeid(timer_id);

    if (ret != 0)
    {
        return ret;
    }

    //Ȼ�����·�����м�ȥ
    ret = add_nodeid(timer_id);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//����Ѱ�Һ��ʵ�λ��
bool ZCE_Timer_Heap::reheap_up(size_t  heap_id)
{
    bool already_up = false;

    //
    if (heap_id == 0)
    {
        return already_up;
    }

    //
    size_t child_id = heap_id;
    size_t parent_id = ZCE_TIMER_HEAP_PARENT(child_id);

    do
    {
        //��С�ѣ����ָ��ڵ�����ӽڵ㣬�͵���
        if (time_node_ary_[timer_node_heap_[parent_id]].next_trigger_point_ >
            time_node_ary_[timer_node_heap_[child_id]].next_trigger_point_ )
        {
            //
            already_up = true;

            //����
            int old_heap_parent = timer_node_heap_[parent_id];
            int old_heap_child =  timer_node_heap_[child_id];

            timer_node_heap_[parent_id] = old_heap_child;
            timer_node_heap_[child_id] = old_heap_parent;

            int swap_data = note_to_heapid_[old_heap_parent];
            note_to_heapid_[old_heap_parent] = note_to_heapid_[old_heap_child];
            note_to_heapid_[old_heap_child] = swap_data;

            child_id = parent_id;
            parent_id = ZCE_TIMER_HEAP_PARENT(child_id);

        }
        else
        {
            break;
        }

    }
    while (child_id > 0);

    return already_up;
}

//����Ѱ�Һ��ʵ�λ��
bool ZCE_Timer_Heap::reheap_down(size_t heap_id)
{
    //�Ƿ�������½�����
    bool already_down = false;
    //
    size_t parent_id = heap_id;
    size_t child_id = ZCE_TIMER_HEAP_LCHILD(parent_id);

    if (child_id > size_heap_)
    {
        return already_down;
    }

    //�����ҽڵ�Ƚϣ�Ȼ��������ת
    do
    {
        //������ӽڵ�������ӽڵ㣬��ô�����ұ߽���
        if ( ((child_id + 1) <  size_heap_) &&
             (time_node_ary_[timer_node_heap_[child_id]].next_trigger_point_ > time_node_ary_[timer_node_heap_[child_id + 1]].next_trigger_point_) )
        {
            ++child_id;
        }

        //dump();

        //���ڵ�����ӽڵ㣬
        if (time_node_ary_[timer_node_heap_[parent_id]].next_trigger_point_ > time_node_ary_[timer_node_heap_[child_id]].next_trigger_point_ )
        {
            already_down = true;

            //����
            int old_heap_parent = timer_node_heap_[parent_id];
            int old_heap_child =  timer_node_heap_[child_id];

            timer_node_heap_[parent_id] = old_heap_child;
            timer_node_heap_[child_id] = old_heap_parent;

            int swap_data = note_to_heapid_[old_heap_parent];
            note_to_heapid_[old_heap_parent] = note_to_heapid_[old_heap_child];
            note_to_heapid_[old_heap_child] = swap_data;

            parent_id = child_id;
            child_id = ZCE_TIMER_HEAP_LCHILD(parent_id);
        }
        //��ʾ�������������ڸ��ڵ�,��������Ѱ����
        else
        {
            return already_down;
        }
    }
    while (child_id < size_heap_);

    //����
    return already_down;
}

int ZCE_Timer_Heap::get_frist_nodeid(int &timer_node_id)
{
    //�ȸ�һ����Чֵ
    timer_node_id = INVALID_TIMER_ID;

    if (size_heap_ == 0)
    {
        return -1;
    }

    timer_node_id =  timer_node_heap_[0];
    return 0;
};

//����һ��Timer Node
int ZCE_Timer_Heap::add_nodeid(int add_node_id)
{
    //���ڶѵ����һ����
    timer_node_heap_[size_heap_] = add_node_id;
    note_to_heapid_[add_node_id] = static_cast<int>( size_heap_);

    ++size_heap_;

    reheap_up(size_heap_ - 1);

    return 0;
}

int ZCE_Timer_Heap::remove_nodeid(int timer_node_id)
{

    assert(size_heap_ > 0);

    if (size_heap_ == 0)
    {
        //������ѽ��Ϲ�����
        return -1;
    }

    size_t delete_heap_id = note_to_heapid_[timer_node_id];

    //
    note_to_heapid_[timer_node_heap_[delete_heap_id]] = INVALID_TIMER_ID;
    timer_node_heap_[delete_heap_id] = INVALID_TIMER_ID;

    --size_heap_;

    //����Ϊ0����ת
    if (size_heap_ == 0)
    {
        return 0;
    }

    //�����һ���ѷŵ�ɾ����λ�ã����е���
    timer_node_heap_[delete_heap_id] = timer_node_heap_[size_heap_];
    note_to_heapid_[timer_node_heap_[delete_heap_id]] = static_cast<int>( delete_heap_id);
    timer_node_heap_[size_heap_] = INVALID_TIMER_ID;

    //���Ƿ���Ҫ���ϵ���
    bool alread_up = reheap_up(delete_heap_id);

    //�������Ҫ���ϵ��������Ƿ���Ҫ���µ���
    if ( !alread_up )
    {
        reheap_down(delete_heap_id);
    }

    return 0;
}

//dump����
void ZCE_Timer_Heap::dump()
{
    std::cout << "timer_node_heap_:";

    for (size_t i = 0; i < num_timer_node_; ++i)
    {
        std::cout << std::setw(3) << timer_node_heap_[i] << " ";
    }

    std::cout << std::endl;

    std::cout << "note_to_heapid_ :";

    for (size_t i = 0; i < num_timer_node_; ++i)
    {
        std::cout << std::setw(3) << note_to_heapid_[i] << " ";
    }

    std::cout << std::endl;
}

