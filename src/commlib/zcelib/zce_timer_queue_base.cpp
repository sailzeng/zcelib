#include "zce_predefine.h"
#include "zce_os_adapt_time.h"
#include "zce_trace_log_debug.h"
#include "zce_timer_handler_base.h"
#include "zce_timer_queue_base.h"

//使用较高的定时器精度，多消耗CPU一点，但定时器的精度会好一些
//#define MORE_HIGH_TIMER_PRECISION 1

ZCE_Timer_Queue *ZCE_Timer_Queue::instance_ = NULL;
/******************************************************************************************
ZCE_Timer_Queue ，定时器的基类
******************************************************************************************/
//构造函数
ZCE_Timer_Queue::ZCE_Timer_Queue(size_t num_timer_node,
                                 unsigned int timer_precision_mesc,
                                 TRIGGER_MODE trigger_mode,
                                 bool dynamic_expand_node):
    num_timer_node_(0),
    timer_precision_mesc_(DEFAULT_TIMER_PRECISION_MSEC),
    num_use_node_(0),
    dynamic_expand_node_(dynamic_expand_node),
    free_node_id_head_(INVALID_TIMER_ID),
    prev_trigger_msec_(0),
    timer_refer_pointer_(0)
{
    int ret = initialize(num_timer_node,
                         timer_precision_mesc,
                         trigger_mode,
                         dynamic_expand_node);
    assert(ret == 0);
    ZCE_UNUSED_ARG(ret);
}

ZCE_Timer_Queue::ZCE_Timer_Queue() :
    num_timer_node_(0),
    timer_precision_mesc_(DEFAULT_TIMER_PRECISION_MSEC),
    num_use_node_(0),
    dynamic_expand_node_(false),
    free_node_id_head_(INVALID_TIMER_ID)
{
}

//析构函数
ZCE_Timer_Queue::~ZCE_Timer_Queue()
{
    //使用vector的好处就是自己不用管理内存了
}

//初始化
int ZCE_Timer_Queue::initialize(size_t num_timer_node,
                                unsigned int timer_precision_mesc,
                                TRIGGER_MODE trigger_mode ,
                                bool dynamic_expand_node)
{

    //时间精度问题
    timer_precision_mesc_ = timer_precision_mesc;
    //目前的NODE数量还是0
    num_timer_node_ = 0;

    num_use_node_ = 0;
    dynamic_expand_node_ = dynamic_expand_node;

    trigger_mode_ = trigger_mode;

    //
    size_t old_num_node = 0;
    extend_node(num_timer_node, old_num_node);

    //触发模式
    if (trigger_mode_ == TRIGGER_MODE_SYSTEM_CLOCK)
    {
        //得到一个时间长度，用于比较衡量
        ZCE_Time_Value  now_time(ZCE_LIB::gettimeofday());

        //取得毫秒
        prev_trigger_msec_ = now_time.total_msec();
    }
    //
    else if (trigger_mode_ == TRIGGER_MODE_CPU_TICK)
    {
        //其实这不是真正的CPU Tick
        ZCE_Time_Value cpu_tick(ZCE_LIB::get_uptime());
        prev_trigger_msec_ = cpu_tick.total_msec();
    }
    else
    {
        ZCE_ASSERT(false);
    }

    timer_refer_pointer_ = prev_trigger_msec_;

    return 0;
}


//关闭
int ZCE_Timer_Queue::close()
{
    //将所有定时器的TIME ID处理
    for (size_t i = 0; i < num_timer_node_; ++i)
    {
        //提醒一下自己，time_node_ary_是记录所有NODE的队列，包括使用的，FREE的，
        if (time_node_ary_[i].timer_handle_)
        {
            //为什么要赋值再使用呢，我担心timer_close会被你调用来清理
            ZCE_Timer_Handler *time_hdl = time_node_ary_[i].timer_handle_;
            time_hdl->timer_close();
        }
    }

    return 0;
}

//扩张的NODE的数量，
int ZCE_Timer_Queue::extend_node(size_t num_timer_node,
                                 size_t &old_num_node)
{
    //总不能比原来还小吧
    assert(num_timer_node > num_timer_node_);

    old_num_node = num_timer_node_;
    num_timer_node_ = num_timer_node;

    //我没打算让你玩20亿个定时器，谢谢，我蛋疼
    ZCE_ASSERT(num_timer_node_ > 0 && num_timer_node_ < 0x7FFFFFFF);

    //
    if (num_timer_node_  > 0x7FFFFFFF)
    {
        return -1;
    }

    //
    time_node_ary_.resize(num_timer_node_);
    free_node_id_list_.resize(num_timer_node_);

    //将所有定时器的TIME ID处理
    for (size_t i = old_num_node; i < num_timer_node_; ++i)
    {
        time_node_ary_[i].time_id_ = static_cast<int>(i);
    }

    //生成空闲队列的数值（属猪）

    //初始化队列,形成一个空闲NODE的对象
    for (size_t i = old_num_node; i < num_timer_node_ - 1; ++i)
    {
        free_node_id_list_[i] = static_cast<int>(i + 1);
    }

    //最后一个表示无效数据,
    free_node_id_list_[num_timer_node_ - 1] = INVALID_TIMER_ID;

    free_node_id_head_ = static_cast<int>(old_num_node);

    return 0;
}

//取消定时器，
int ZCE_Timer_Queue::cancel_timer(int timer_id)
{
    return free_timernode(timer_id);
}

//取消定时器，超级超级，超级慢的函数，平均时间复杂度O(N),N是队列的长度
int ZCE_Timer_Queue::cancel_timer(const ZCE_Timer_Handler *timer_hdl)
{
    assert(timer_hdl);

    //如果后面可以优化一下，用一个使用的HANDLE队列保存使用过的队列，处理这个事情,
    //我考虑了一下非要用双向链表，我放弃了

    bool cancel_time = false;

    for (size_t i = 0; i < num_timer_node_; ++i)
    {
        if (time_node_ary_[i].timer_handle_ == timer_hdl )
        {
            uint32_t time_id = static_cast<uint32_t > (i);
            cancel_timer(time_id);
            cancel_time = false;

            //不能break,因为HANDLE还有对应多个NODE,
        }
    }

    //如果取消定时器
    if (!cancel_time)
    {
        return -1;
    }

    //成功
    return 0;
}

//分配Timer Node
int ZCE_Timer_Queue::alloc_timernode(ZCE_Timer_Handler *timer_hdl,
                                     const void *action,
                                     const ZCE_Time_Value &delay_time,
                                     const ZCE_Time_Value &interval_time,
                                     int &time_node_id,
                                     ZCE_TIMER_NODE *&alloc_time_node)
{
    //TIME HANDLE不能为NULL
    assert(timer_hdl != NULL);

    int ret = 0;
    //默认没有分配到，避免你搞错情况
    time_node_id = INVALID_TIMER_ID;
    alloc_time_node = NULL;

    //如果已经没有了NODE
    if (free_node_id_head_ == INVALID_TIMER_ID)
    {
        //如果不支持动态扩张，返回错误
        if (!dynamic_expand_node_)
        {
            return -1;
        }
        else
        {
            //如果希望动态扩张，我帮你扩张
            size_t old_num_node = 0;
            ret = extend_node(num_timer_node_ + ONCE_EXTEND_NODE_NUMBER, old_num_node);

            if (ret != 0)
            {
                return -1;
            }
        }
    }

    //得到时间NODE
    time_node_id = free_node_id_head_;
    free_node_id_head_ = free_node_id_list_[free_node_id_head_];
    alloc_time_node = &(time_node_ary_[time_node_id]);

    //对时间NODE进行赋值
    alloc_time_node->timer_handle_ =  timer_hdl;
    alloc_time_node->interval_time_ =  interval_time;
    alloc_time_node->action_ =  action;

    //如果你追求高进度，定义这个宏，默认我关闭了
#if defined(MORE_HIGH_TIMER_PRECISION)

    uint64_t now_point = 0;

    //默认你的调用次数不频繁，提供更加安全的时间给你
    if (trigger_mode_ == TRIGGER_MODE_SYSTEM_CLOCK)
    {
        ZCE_Time_Value now_trigger_time;
        now_trigger_time.gettimeofday();
        now_point = now_trigger_time.total_msec();
    }
    else if (trigger_mode_ == TRIGGER_MODE_CPU_TICK)
    {
        //其实这不是真正的CPU Tick
        ZCE_Time_Value cpu_tick(ZCE_LIB::get_uptime());
        now_point = cpu_tick.total_msec();
    }

    alloc_time_node->next_trigger_point_ = now_point + delay_time.total_msec();

#else
    //需要注意的，这个地方不是非常精确，我为了加快执行速度偷懒了，
    //当然问题是，你调用的dispatch_timer精度有多大，调用的越多，timer_refer_pointer_就越准确，
    //如果你调用频度低的吓人（<1s）,我，我，

    //得到要触发的那个TICK点，
    alloc_time_node->next_trigger_point_ = timer_refer_pointer_ + delay_time.total_msec();

#endif

    ++num_use_node_;

    //调试代码，测试时打开
    //ZCE_LOG(RS_DEBUG,"[zcelib] ZCE_Timer_Queue alloc_timernode.time_node_id[%d] "
    //  "next_trigger_point_[%llu], prev_trigger_msec_[%llu],timer_refer_pointer_[%llu].",
    //  time_node_id,
    //  alloc_time_node->next_trigger_point_,
    //  prev_trigger_msec_,
    //  timer_refer_pointer_,
    //  );

    return 0;
}

//计算下一个触发点，
void ZCE_Timer_Queue::calc_next_trigger(int time_node_id,
                                        uint64_t now_trigger_msec,
                                        bool &continue_trigger)
{
    continue_trigger = false;

    ZCE_TIMER_NODE *prc_time_node = &(time_node_ary_[time_node_id]);

    //如果间隔时间为0，且这个定时器还没有触发过，already_trigger_ 判定其实是检查定时器是否被重新设置过
    if (prc_time_node->interval_time_  == ZCE_Time_Value::ZERO_TIME_VALUE )
    {
        //
        continue_trigger = false;
        return;
    }

    //这儿要不要用当前真实时间，还是用自己记录的触发点作为计算点，这个的确是一个问题
    //我目前还是用自己记录的触发点作为计算点，这样精度不好，但不会丢触发点
    continue_trigger = true;

    //计算下一个触发点，，
    prc_time_node->next_trigger_point_ = now_trigger_msec + prc_time_node->interval_time_.total_msec();

    return;
}

//释放Timer Node
int ZCE_Timer_Queue::free_timernode(int time_node_id)
{
    //考虑了一下还是用断言了，避免你写错代码祸国殃民
    ZCE_ASSERT(time_node_ary_[time_node_id].timer_handle_ != NULL &&
               static_cast<size_t>( time_node_id) < num_timer_node_);

    //如果NODE内部的指针为NULL,表示这个NODE没有使用
    if (!time_node_ary_[time_node_id].timer_handle_)
    {
        return -1;
    }

    //不可能超过NODE的数量
    if (static_cast<size_t>(time_node_id) >= num_timer_node_)
    {
        return -1;
    }

    //归还到FREE队列中
    free_node_id_list_[ time_node_id ] = free_node_id_head_;
    free_node_id_head_ = time_node_id;
    //清理数据去
    time_node_ary_[time_node_id].clear();
    --num_use_node_;

    return 0;
}

//得到最快将在多少时间后触发
int ZCE_Timer_Queue::get_first_timeout(ZCE_Time_Value *first_timeout)
{
    int ret = 0;
    int time_node_id = INVALID_TIMER_ID;
    ret = get_frist_nodeid(time_node_id);

    if (INVALID_TIMER_ID == time_node_id || -1 == ret )
    {
        return -1;
    }

    ZCE_Time_Value now_time(ZCE_LIB::gettimeofday());
    uint64_t now_trigger_msec = 0;

    //触发模式
    if (trigger_mode_ == TRIGGER_MODE_SYSTEM_CLOCK)
    {
        //取得毫秒
        now_trigger_msec = now_time.total_msec();
    }
    //
    else if (trigger_mode_ == TRIGGER_MODE_CPU_TICK)
    {
        //其实这不是真正的CPU Tick
        ZCE_Time_Value cpu_tick(ZCE_LIB::get_uptime());
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

size_t ZCE_Timer_Queue::expire()
{
    ZCE_Time_Value now_time(ZCE_LIB::gettimeofday());

    uint64_t now_trigger_msec = 0;

    //触发模式
    if (trigger_mode_ == TRIGGER_MODE_SYSTEM_CLOCK)
    {
        //取得毫秒
        now_trigger_msec = now_time.total_msec();
    }
    //
    else if (trigger_mode_ == TRIGGER_MODE_CPU_TICK)
    {
        //其实这不是真正的CPU Tick
        ZCE_Time_Value cpu_tick(ZCE_LIB::get_uptime());
        now_trigger_msec = cpu_tick.total_msec();
    }
    else
    {
        ZCE_ASSERT(false);
    }

    return dispatch_timer(now_time, now_trigger_msec);
}

//得到唯一的单子实例
ZCE_Timer_Queue *ZCE_Timer_Queue::instance()
{
    return instance_;
}

//赋值唯一的单子实例
void ZCE_Timer_Queue::instance(ZCE_Timer_Queue *pinstatnce)
{
    clean_instance();
    instance_ = pinstatnce;
    return;
}

//清除单子实例
void ZCE_Timer_Queue::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}

