#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/timer/timer_handler.h"
#include "zce/timer/queue_heap.h"

namespace zce
{
//注意由于C/C++的下标变化是从0开始的，
#define ZCE_TIMER_HEAP_PARENT(x)  ((x) == 0 ? 0 : (((x) - 1) / 2))
//左子树的下标
#define ZCE_TIMER_HEAP_LCHILD(x)  ((x) + (x) + 1)
//左子树的下标
#define ZCE_TIMER_HEAP_RCHILD(x)  ((x) + (x) + 2)

//
Timer_Heap::Timer_Heap(size_t num_timer_node,
                       unsigned int timer_precision_mesc,
                       TRIGGER_MODE trigger_mode,
                       bool dynamic_expand_node)
{
    int ret = Timer_Heap::initialize(num_timer_node,
                                     timer_precision_mesc,
                                     trigger_mode,
                                     dynamic_expand_node);

    //
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR,"[zcelib] Timer_Heap::initialize fail.");
    }
}

//构造函数
Timer_Heap::Timer_Heap():
    size_heap_(0)
{
}

Timer_Heap::~Timer_Heap()
{
}

int Timer_Heap::initialize(size_t num_timer_node,
                           unsigned int timer_precision_mesc,
                           TRIGGER_MODE trigger_mode,
                           bool dynamic_expand_node)
{
    //在基类进行初始化
    int ret = 0;
    ret = zce::Timer_Queue::initialize(num_timer_node,
                                       timer_precision_mesc,
                                       trigger_mode,
                                       dynamic_expand_node);

    if (ret != 0)
    {
        return ret;
    }

    //开始阶段没有任何数据
    size_heap_ = 0;

    return 0;
}

//扩张相关十字链表的NODE的数量，也调用底层的extend_node函数
int Timer_Heap::extend_node(size_t num_timer_node,
                            size_t &old_num_node)
{
    int ret = 0;
    ret = zce::Timer_Queue::extend_node(num_timer_node,old_num_node);

    if (ret != 0)
    {
        return ret;
    }

    timer_node_heap_.resize(num_timer_node_);

    //将QUEUE上的所点处理成无效，表示没有挂NODE
    for (size_t i = old_num_node; i < num_timer_node_; ++i)
    {
        timer_node_heap_[i] = INVALID_TIMER_ID;
    }

    note_to_heapid_.resize(num_timer_node_);

    //将QUEUE上的所点处理成无效，表示没有对应关系
    for (size_t i = old_num_node; i < num_timer_node_; ++i)
    {
        note_to_heapid_[i] = INVALID_TIMER_ID;
    }

    return 0;
}

//分发定时器
size_t Timer_Heap::dispatch_timer(const zce::Time_Value &now_time,
                                  uint64_t now_trigger_msec)
{
    int ret = 0;
    //分发的数量
    size_t num_dispatch = 0;

    int timer_node_id = INVALID_TIMER_ID;
    ret = get_frist_nodeid(timer_node_id);

    while (timer_node_id != INVALID_TIMER_ID)
    {
        //如果已经超时，进行触发
        if (time_node_ary_[timer_node_id].next_trigger_point_ <= now_trigger_msec)
        {
            ++num_dispatch;
            //标记这个定时器已经触发过，详细见already_trigger_的解释
            time_node_ary_[timer_node_id].already_trigger_ = true;
            //时钟触发
            time_node_ary_[timer_node_id].timer_handle_->timer_timeout(now_time,
                                                                       time_node_ary_[timer_node_id].action_);
        }
        else
        {
            break;
        }

        //因为timer_timeout其实可能取消了这个定时器，所以在调用之后，要进行一下检查
        if (time_node_ary_[timer_node_id].timer_handle_ && time_node_ary_[timer_node_id].already_trigger_ == true)
        {
            //重新规划这个TIME NODE的位置等,如果不需要触发了则取消定时器
            reschedule_timer(timer_node_id,now_trigger_msec);
        }

        //
        ret = get_frist_nodeid(timer_node_id);

        if (0 != ret)
        {
            break;
        }
    }

    prev_trigger_msec_ = now_trigger_msec;

    //返回数量
    return num_dispatch;
}

//设置定时器
int Timer_Heap::schedule_timer(zce::Timer_Handler *timer_hdl,
                               const void *action,
                               const zce::Time_Value &delay_time,
                               const zce::Time_Value &interval_time)
{
    int ret = 0;
    int time_node_id = INVALID_TIMER_ID;

    //看能否分配一个TIME NODE
    ZCE_TIMER_NODE *alloc_time_node = NULL;
    ret = alloc_timernode(timer_hdl,
                          action,
                          delay_time,
                          interval_time,
                          time_node_id,
                          alloc_time_node
    );

    //注意，这个地方返回INVALID_TIMER_ID表示无效，其实也许参数不应该这样设计，但为了兼容ACE的代码
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

//取消定时器
int Timer_Heap::cancel_timer(int timer_id)
{
    //
    int ret = 0;

    //先从堆上上删除这个节点
    ret = remove_nodeid(timer_id);

    if (ret != 0)
    {
        return ret;
    }

    //回收TIME NODE
    ret = zce::Timer_Queue::cancel_timer(timer_id);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int Timer_Heap::reschedule_timer(int timer_id,uint64_t now_trigger_msec)
{
    bool contiue_trigger = false;

    //计算下一次触发的点
    calc_next_trigger(timer_id,
                      now_trigger_msec,
                      contiue_trigger);

    //如果不需要继续触发定时器了，取消
    if (!contiue_trigger)
    {
        return cancel_timer(timer_id);
    }

    //这个TIMER NODE 仍然要触发

    //先从堆上上删除这个节点
    int ret = remove_nodeid(timer_id);

    if (ret != 0)
    {
        return ret;
    }

    //然后重新放入对中间去
    ret = add_nodeid(timer_id);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//向上寻找合适的位置
bool Timer_Heap::reheap_up(size_t  heap_id)
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
        //最小堆，发现父节点大于子节点，就调整
        if (time_node_ary_[timer_node_heap_[parent_id]].next_trigger_point_ >
            time_node_ary_[timer_node_heap_[child_id]].next_trigger_point_)
        {
            //
            already_up = true;

            //交换
            int old_heap_parent = timer_node_heap_[parent_id];
            int old_heap_child = timer_node_heap_[child_id];

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
    } while (child_id > 0);

    return already_up;
}

//向下寻找合适的位置
bool Timer_Heap::reheap_down(size_t heap_id)
{
    //是否进行了下降操作
    bool already_down = false;
    //
    size_t parent_id = heap_id;
    size_t child_id = ZCE_TIMER_HEAP_LCHILD(parent_id);

    if (child_id > size_heap_)
    {
        return already_down;
    }

    //和左右节点比较，然后向下旋转
    do
    {
        //如果左子节点大于右子节点，那么，向右边进发
        if (((child_id + 1) < size_heap_) &&
            (time_node_ary_[timer_node_heap_[child_id]].next_trigger_point_ > time_node_ary_[timer_node_heap_[child_id + 1]].next_trigger_point_))
        {
            ++child_id;
        }

        //dump();

        //父节点大于子节点，
        if (time_node_ary_[timer_node_heap_[parent_id]].next_trigger_point_ > time_node_ary_[timer_node_heap_[child_id]].next_trigger_point_)
        {
            already_down = true;

            //交换
            int old_heap_parent = timer_node_heap_[parent_id];
            int old_heap_child = timer_node_heap_[child_id];

            timer_node_heap_[parent_id] = old_heap_child;
            timer_node_heap_[child_id] = old_heap_parent;

            int swap_data = note_to_heapid_[old_heap_parent];
            note_to_heapid_[old_heap_parent] = note_to_heapid_[old_heap_child];
            note_to_heapid_[old_heap_child] = swap_data;

            parent_id = child_id;
            child_id = ZCE_TIMER_HEAP_LCHILD(parent_id);
        }
        //表示左右子树都大于父节点,不用向下寻找了
        else
        {
            return already_down;
        }
    } while (child_id < size_heap_);

    //返回
    return already_down;
}

int Timer_Heap::get_frist_nodeid(int &timer_node_id)
{
    //先附一个无效值
    timer_node_id = INVALID_TIMER_ID;

    if (size_heap_ == 0)
    {
        return -1;
    }

    timer_node_id = timer_node_heap_[0];
    return 0;
};

//增加一个Timer Node
int Timer_Heap::add_nodeid(int add_node_id)
{
    //放在堆的最后一个，
    timer_node_heap_[size_heap_] = add_node_id;
    note_to_heapid_[add_node_id] = static_cast<int>(size_heap_);

    ++size_heap_;

    reheap_up(size_heap_ - 1);

    return 0;
}

int Timer_Heap::remove_nodeid(int timer_node_id)
{
    assert(size_heap_ > 0);

    if (size_heap_ == 0)
    {
        //看你妹呀，瞎填参数
        return -1;
    }

    size_t delete_heap_id = note_to_heapid_[timer_node_id];

    //
    note_to_heapid_[timer_node_heap_[delete_heap_id]] = INVALID_TIMER_ID;
    timer_node_heap_[delete_heap_id] = INVALID_TIMER_ID;

    --size_heap_;

    //数量为0不旋转
    if (size_heap_ == 0)
    {
        return 0;
    }

    //将最后一个堆放到删除的位置，进行调整
    timer_node_heap_[delete_heap_id] = timer_node_heap_[size_heap_];
    note_to_heapid_[timer_node_heap_[delete_heap_id]] = static_cast<int>(delete_heap_id);
    timer_node_heap_[size_heap_] = INVALID_TIMER_ID;

    //看是否需要向上调整
    bool alread_up = reheap_up(delete_heap_id);

    //如果不需要向上调整，看是否需要向下调整
    if (!alread_up)
    {
        reheap_down(delete_heap_id);
    }

    return 0;
}

//dump数据
void Timer_Heap::dump()
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
}