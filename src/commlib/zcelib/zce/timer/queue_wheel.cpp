#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/timer/timer_handler.h"
#include "zce/timer/queue_wheel.h"

namespace zce
{
//构造函数
timer_wheel::timer_wheel(size_t num_timer_node,
                         unsigned int timer_length_mesc,
                         unsigned int timer_precision_mesc,
                         TRIGGER_MODE trigger_mode,
                         bool dynamic_expand_node
) :
    timer_length_mesc_(0),
    num_wheel_point_(0),
    proc_wheel_start_(0)
{
    int ret = 0;
    ret = initialize(num_timer_node,
                     timer_length_mesc,
                     timer_precision_mesc,
                     trigger_mode,
                     dynamic_expand_node);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] Timer_Wheel::initialize fail.");
    }
}

//构造函数
timer_wheel::timer_wheel() :
    timer_length_mesc_(0),
    num_wheel_point_(0),
    proc_wheel_start_(0)
{
}

//析构函数
timer_wheel::~timer_wheel()
{
}

int timer_wheel::initialize(size_t num_timer_node,
                            unsigned int timer_length_mesc,
                            unsigned int timer_precision_mesc,
                            TRIGGER_MODE trigger_mode,
                            bool dynamic_expand_node)
{
    int ret = 0;
    assert(timer_length_mesc > 0);

    if (timer_length_mesc < MIN_TIMER_LENGTH_MESC)
    {
        timer_length_mesc = MIN_TIMER_LENGTH_MESC;
    }

    //记录最大能接受的毫秒数量
    timer_length_mesc_ = timer_length_mesc;

    ret = zce::timer_queue::initialize(num_timer_node,
                                       timer_precision_mesc,
                                       trigger_mode,
                                       dynamic_expand_node);

    if (ret != 0)
    {
        return ret;
    }

    //需要处理多少个时间点
    num_wheel_point_ = static_cast<size_t>(timer_length_mesc) / timer_precision_mesc + 1;
    timer_wheel_point_.resize(num_wheel_point_);

    //将QUEUE上的所点处理成无效，表示没有挂NODE
    for (size_t i = 0; i < num_wheel_point_; ++i)
    {
        timer_wheel_point_[i] = INVALID_TIMER_ID;
    }

    proc_wheel_start_ = 0;
    future_trigger_tid_.reserve(ARRAY_FUTURE_TRIGGER_TIMERID_SIZE);
    return 0;
}

//扩张相关十字链表的NODE的数量，也调用底层的extend_node函数
int timer_wheel::extend_node(size_t num_timer_node,
                             size_t& old_num_node)
{
    int ret = 0;
    ret = zce::timer_queue::extend_node(num_timer_node, old_num_node);

    if (ret != 0)
    {
        return ret;
    }

    //
    wheel_node_list_.resize(num_timer_node_);

    //构造函数其实都写了
    //让所有的节点都为nullptr
    //for (size_t i=old_num_node;i<num_timer_node_;++i)
    //{
    //    wheel_node_list_[i].wheel_point_id_ = INVALID_TIMER_ID;
    //    wheel_node_list_[i].list_prev_ = INVALID_TIMER_ID;
    //    wheel_node_list_[i].list_next_ = INVALID_TIMER_ID;
    //}

    return 0;
}

//将Queue和TimerNode绑定
void timer_wheel::bind_wheel_listnode(int time_node_id)
{
    //前进了多少时间点
    size_t front_num = static_cast<size_t>
        ((time_node_ary_[time_node_id].next_trigger_point_
         - prev_trigger_msec_) / timer_precision_mesc_);

    //调试代码，暂时屏蔽
    //ZCE_LOG(RS_DEBUG,"[zcelib] Timer_Wheel::bind_wheel_listnode next_trigger_point_[%llu] prev_trigger_msec_ [%llu] front_num [%lu]",
    //  time_node_ary_[time_node_id].next_trigger_point_,
    //  prev_trigger_msec_,
    //  front_num);

    //找到轮子的位置
    size_t wheel_point_id = (proc_wheel_start_ + front_num) % num_wheel_point_;

    //比较难看的双向链表，没法像std那样折腾，只能用这种有一个头指针的难看的写法
    int old_node_id = timer_wheel_point_[wheel_point_id];
    timer_wheel_point_[wheel_point_id] = time_node_id;

    //要反向记录，才能在删除时查询到
    wheel_node_list_[time_node_id].wheel_point_id_ = static_cast<int>(wheel_point_id);

    //如果原来头指针后面一个数据，要联入，建立双向链表
    if (INVALID_TIMER_ID != old_node_id)
    {
        wheel_node_list_[old_node_id].list_prev_ = time_node_id;
        wheel_node_list_[time_node_id].list_next_ = old_node_id;
    }
}

//将Queue和TimerNode解除绑定
void timer_wheel::unbind_wheel_listnode(int time_node_id)
{
    int wheel_point_id = wheel_node_list_[time_node_id].wheel_point_id_;

    //如果WHEEL上第一个元素记录的也是这个NODE
    if (timer_wheel_point_[wheel_point_id] == time_node_id)
    {
        timer_wheel_point_[wheel_point_id] = wheel_node_list_[time_node_id].list_next_;
    }
    else
    {
        int prev = wheel_node_list_[time_node_id].list_prev_;
        int next = wheel_node_list_[time_node_id].list_next_;

        //如果有前,前面和后面接起来
        if (prev != INVALID_TIMER_ID)
        {
            wheel_node_list_[prev].list_next_ = next;
        }

        //如果有后，后面找到前面的宗
        if (next != INVALID_TIMER_ID)
        {
            //数据的后指针等于后指针
            wheel_node_list_[next].list_prev_ = prev;
        }
    }

    //清理为nullptr，避免麻烦，
    wheel_node_list_[time_node_id].list_prev_ = INVALID_TIMER_ID;
    wheel_node_list_[time_node_id].list_next_ = INVALID_TIMER_ID;
    wheel_node_list_[time_node_id].wheel_point_id_ = INVALID_TIMER_ID;
}

//设置定时器
int timer_wheel::schedule_timer_i(zce::timer_handler* timer_hdl,
                                  std::function<int(const zce::time_value &, int) > &call_fun,
                                  int& time_id,
                                  const zce::time_value& delay_time,
                                  const zce::time_value& interval_time)
{
    int ret = 0;
    time_id = INVALID_TIMER_ID;

#if defined(ZCE_HAS_DEBUG)

#endif //

    ZCE_TIMER_NODE* alloc_time_node = nullptr;
    ret = alloc_timernode(timer_hdl,
                          call_fun,
                          delay_time,
                          interval_time,
                          time_id,
                          alloc_time_node);

    //注意，这个地方返回INVALID_TIMER_ID表示无效，其实也许参数不应该这样设计，但为了兼容ACE的代码
    if (ret != 0)
    {
        return INVALID_TIMER_ID;
    }

    bind_wheel_listnode(time_id);

    return 0;
}

//取消定时器
int timer_wheel::cancel_timer(int timer_id)
{
    //
    int ret = 0;

    //回收
    unbind_wheel_listnode(timer_id);

    //回收这个TIMER NODE
    ret = zce::timer_queue::cancel_timer(timer_id);

    if (ret != 0)
    {
        return ret;
    }

    //
    return 0;
}

//在触发一次后，要对定时器进行重新计算
int timer_wheel::reschedule_timer(int timer_id,
                                  uint64_t now_trigger_msec)
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

    //这个TIMER NODE还有要继续从事触发的工作，继续折腾
    //先从轮上解除绑定
    unbind_wheel_listnode(timer_id);

    //再重新绑上去
    bind_wheel_listnode(timer_id);

    return 0;
}

//取得第一个元素，也就是，最小的时间,Timer_Wheel如果里面的的NODE很少，可能有点慢,
int timer_wheel::get_frist_nodeid(int& first_node_id)
{
    first_node_id = INVALID_TIMER_ID;

    int timer_node_id = INVALID_TIMER_ID;

    //如果没有定时器，就别搞了，WHEEL找first不快
    if (num_use_node_ == 0)
    {
        return -1;
    }

    for (size_t i = 0; i < num_wheel_point_; ++i)
    {
        //检查这个WHEEL NODE后面是否还有TIMER NODE的链表
        size_t wheel_node_id = (proc_wheel_start_ + i) % num_wheel_point_;
        timer_node_id = timer_wheel_point_[wheel_node_id];

        if (INVALID_TIMER_ID != timer_node_id)
        {
            break;
        }
    }

    if (INVALID_TIMER_ID != timer_node_id)
    {
        first_node_id = timer_node_id;
        return 0;
    }

    //Never goto here.?
    return -1;
}

//分发定时器，返回分发的数量
size_t timer_wheel::dispatch_timer(const zce::time_value& now_time,
                                   uint64_t now_trigger_msec)
{
    //分派了多少个定时器计数
    size_t num_dispatch = 0;

    //先调整计时的参考点，因为在处理定时器的过程，回调timer_timeout时，有人会在timer_timeout重新设置定时器，
    //如果不调整timer_refer_pointer_，那么这是设置的时间可能出现问题，
    //如果中间等了一个很长时间，如果timer_refer_pointer_没有先调整，回调timer_timeout时设置的定时器就错误了。（可能会造成死循环）
    timer_refer_pointer_ = now_trigger_msec;

    //如果代码进入下面的if，表示时间同步调整，导致时间跳变到过去的某个时间上(pasttime)去了,
    if (now_trigger_msec < prev_trigger_msec_)
    {
        //这儿用错误是因为有必要提醒一下你，定时器被调整了，
        ZCE_LOG(RS_ERROR, "[zcelib] Timer_Wheel error. now_trigger_msec[%llu] < prev_trigger_msec_[%llu],may be you adjust systime time to past time.",
                now_trigger_msec,
                prev_trigger_msec_);
        prev_trigger_msec_ = now_trigger_msec;
        return 0;
    }

    uint64_t elapsed_msec = now_trigger_msec - prev_trigger_msec_;

    //进入这个条件，要不就是你一直没有调用Dispatch，要不就是你的时间轮设计的过小,或者时间向未来调整了很多
    if (elapsed_msec > timer_length_mesc_)
    {
        //注释掉断言，yunfei说明了一种情况，如果时间向前跳变超过了循环周期.虽然我认为这个情况暂时不会发生，就是将定时器调整成未来的某个时间，
        //assert (elapsed_msec < timer_length_mesc_);

        ZCE_LOG(RS_ALERT, "[zcelib] Timer_Wheel alert. now_trigger_msec[%llu], prev_trigger_msec_[%llu],elapsed_msec[%llu],timer_length_mesc_[%llu],"
                "may be you adjust systime time to future time or you ,or wheel is too little ,or dispatch_timer or expire invoke too little.",
                now_trigger_msec,
                prev_trigger_msec_,
                elapsed_msec,
                timer_length_mesc_);

        //先用安全的方式?
        prev_trigger_msec_ = now_trigger_msec;
        return 0;

        //如果时间向前跳变超过了循环周期, 则遍历一下整个列表
        //而yunfei的倾向，如果讲定时器猛然向前调整，就让所有的定时器都触发，这个是不是有更高的风险？或者又其实，这种也应该用断言？
        //elapsed_msec = timer_length_mesc_;
    }

    size_t passing_wheel_node = static_cast<size_t>(elapsed_msec / timer_precision_mesc_);

    //调试代码，暂时关闭
    //ZCE_LOG(RS_DEBUG,"[zcelib] Timer_Wheel . now_trigger_msec[%llu], prev_trigger_msec_[%llu],elapsed_msec[%llu],timer_length_mesc_[%u],passing_wheel_node[%lu]",
    //  now_trigger_msec,
    //  prev_trigger_msec_,
    //  elapsed_msec,
    //  timer_length_mesc_,
    //  passing_wheel_node);

    //处理N个轮子节点，在0和1,<=和<上面又犯了2次糊涂,0是必须，因为有人希望马上触发，=也是必须的，因为……要处理到当前时间。
    for (size_t i = 0; i <= passing_wheel_node; ++i)
    {
        //检查这个WHEEL NODE后面是否还有TIMER NODE的链表
        size_t wheel_node_id = (proc_wheel_start_ + i) % num_wheel_point_;
        int timer_node_id = timer_wheel_point_[wheel_node_id];

        if (INVALID_TIMER_ID == timer_node_id)
        {
            continue;
        }

        //判定的精度,直接使用timer_precision_mesc_也应该可以，但因为轮子其实很大，所以 * 10感觉安全一点
        const uint64_t JUDGE_PRECISION_MESC = 10 * timer_precision_mesc_;

        //
        while (INVALID_TIMER_ID != timer_node_id)
        {
            //这个地方改了一下，因为按照原来的写法，如果放入的定时器的触发时间超过轮子长度，那么会被提前触发。
            //因为进度问题，所以必须 + JUDGE_PRECISION_MESC,
            if (time_node_ary_[timer_node_id].next_trigger_point_ <= now_trigger_msec + JUDGE_PRECISION_MESC)
            {
                //标记这个定时器已经触发过，详细见already_trigger_的解释
                time_node_ary_[timer_node_id].already_trigger_ = true;
                //时钟触发
                if (time_node_ary_[timer_node_id].timer_handle_)
                {
                    time_node_ary_[timer_node_id].timer_handle_->timer_timeout(now_time,
                                                                               time_node_ary_[timer_node_id].time_id_);
                }
                else
                {
                    time_node_ary_[timer_node_id].timer_call_(now_time, time_node_ary_[timer_node_id].time_id_);
                }
                ++num_dispatch;

                //因为timer_timeout其实可能取消了这个定时器，所以在调用之后，要进行一下检查
                if (time_node_ary_[timer_node_id].timer_handle_ && time_node_ary_[timer_node_id].already_trigger_ == true)
                {
                    //重新规划这个定时器是否要安排，如果不用内部会取消他，理论不可能失败，暂时不加跟踪
                    reschedule_timer(timer_node_id, now_trigger_msec);
                }
            }
            //到else 有2个可能，
            //一个是原来的定时触发时间是一个超长的时间，超过了轮子的周期
            //一个是时间周期被调整过，（比如向后调整）
            else
            {
                //先保存着
                future_trigger_tid_.push_back(timer_node_id);
                //先从轮上解除绑定
                unbind_wheel_listnode(timer_node_id);
            }

            //继续从头部开始找
            timer_node_id = timer_wheel_point_[wheel_node_id];
        }

        //如果本槽位有未来触发的Timer ID，
        if (future_trigger_tid_.size() > 0)
        {
            for (size_t x = 0; x < future_trigger_tid_.size(); ++x)
            {
                //重新计算相关的未来的触发点，然后加入轮子
                bind_wheel_listnode(future_trigger_tid_[x]);
            }
            //clear应该不会回收空间
            future_trigger_tid_.clear();
        }
    }

    //如果处理了NODE借点，当前时间，向前进，否则不动，
    if (passing_wheel_node)
    {
        //下面注释行代码是错误的，因为如果每次都这样调整，每次前进的步长都会丢掉精度后面部分，积少成多，会让定时器的触发延迟
        //prev_trigger_msec_ = now_trigger_msec;

        //所以修正为：确保每次只前进N个步长，而且不会丢失精度部分
        prev_trigger_msec_ += (passing_wheel_node * timer_precision_mesc_);

        //开始处理的点以环形队列的方式前进
        proc_wheel_start_ = (proc_wheel_start_ + passing_wheel_node) % num_wheel_point_;
    }

    //返回数量
    return num_dispatch;
}
}