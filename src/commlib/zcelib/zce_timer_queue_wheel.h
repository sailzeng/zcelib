/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_timer_queue_wheel.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2008年10月6日
* @brief      时间轮的定时器队列，速度飞飞飞飞飞飞飞快，但是有精度限制的定时器处理，
*             用一个循环链表记录时间，
*             他的放置，取消定时器的时间销毁都是O(1)级别，快的如飞，
*
* @details    这个WHEEL和ACE的WHEEL不是一个东西，我的整体思路是将一段时间做成一个
*             轮子，WHELL,轮子上有N个时间槽，轮子上的每个槽放置一个时间的超时定时器，
*             这个点都是代表一个固定精度，一般是0.1s，（我觉得这对服务器一般足够了）
*
*             当有一个超时时间的Timer要放入轮子中时，会根据当前的时间，和他超时的时间
*             放在一个槽点上，如果有多个定时器，会形成一个链表，
*             链表上的超时数据不会再做任何排序处理，
*             当时间前进，跨越N个槽位是，这个N个槽位上所有的定时器将被触发，
*
*             在设计这个定时器时，我参考过LINUX内核的实现，和ACE的4种实现，
*             在这个实现上，我强调了速度，而不考虑完全的准确性（同一个槽位的定时器不做
*             优先级排序，后放入的定时器会先触发）
*
*             我估计10万的定时器（10s）对于这个实现，估计CPU连1%都不用。
*
*/

#ifndef ZCE_TIMER_QUEUE_WHEEL_H_
#define ZCE_TIMER_QUEUE_WHEEL_H_

#include "zce_timer_queue_base.h"
#include "zce_time_value.h"

/*!
* @brief      时间轮定时器，有人也成为十字链表（循环队列），
*             Timer触发器,高效率，高性能的带名称，
*             主要设计思路是只能保存一定时长的触发定时器，用一个WHEEL(循环的队列表示时间点
*             这个时间点有要触发的定时器就放在这个点后面，用双向链表存放时间NODE，每个NODE
*             内部也指向有WHEEL
*             设置定时器，取消定时器，分发定时器的消耗级别都是O(1)
*/
class ZCE_Timer_Wheel : public ZCE_Timer_Queue
{

protected:

    ///这个只封装链表数据结构，用于在横轴上找一个相同时间上有几个定时器
    ///本质上是一个双向链表，同时也记录头也就是WHEEL的点，这样设计的主要是为了方便删除
    struct ZCE_WHEEL_TIMER_NODE
    {

        ///构造函数
        ZCE_WHEEL_TIMER_NODE():
            list_prev_(ZCE_Timer_Queue::INVALID_TIMER_ID),
            list_next_(ZCE_Timer_Queue::INVALID_TIMER_ID),
            wheel_point_id_(ZCE_Timer_Queue::INVALID_TIMER_ID)
        {
        }
        ///析构函数
        ~ZCE_WHEEL_TIMER_NODE()
        {
        }

        ///双向链表的前驱
        int    list_prev_;
        ///双向链表的后置，
        int    list_next_;

        ///表示当前TIMER NODE所在WHEEL上的那个点的标示,这儿理论用size_t合理一点，但是为了空间和处理方便，用了int存储
        int    wheel_point_id_;
    };

    ///存放所有NODE的数组，会在初始化时分配好空间，避免后面分配，
    typedef   std::vector<ZCE_WHEEL_TIMER_NODE>  ARRAY_WHEEL_TIMER_NODE;

protected:

    ///默认定时器支持的时间长度MESC,(并不是需要这么多个槽位，内部精度可以自己定义)
    static const unsigned int DEFAULT_TIMER_LENGTH_MESC   = 72 * 3600 * 1000;

    ///最小的周期轮子长度
    static const unsigned int MIN_TIMER_LENGTH_MESC       = 1 * 3600 * 1000;

    ///未来触发的时间ID的数组的初始化大小，
    static const size_t ARRAY_FUTURE_TRIGGER_TIMERID_SIZE = 512;

public:

    /*!
    * @brief      构造函数
    * @param[in]  num_timer_node        定时器NODE的数量，初始化的时候会分配好，
    * @param[in]  timer_length_mesc     轮子标识的时间长度，msec，轮子上的槽位数量= timer_length_mesc/timer_precision_mesc
    * @param[in]  timer_precision_mesc  定时器的进度，多少msec，如果设置过大，有些定时器会超时后才触发，如果太小，需要更多的轮的槽位空间，
    * @param[in]  trigger_mode          触发模式是用墙上时钟还是CPU TICK，参考 @ref ZCE_Timer_Queue::TRIGGER_MODE
    * @param[in]  dynamic_expand_node   如果初始化的NODE节点数量不够，是否自动扩展
    */
    ZCE_Timer_Wheel(size_t num_timer_node,
                    unsigned int timer_length_mesc = DEFAULT_TIMER_LENGTH_MESC,
                    unsigned int timer_precision_mesc = DEFAULT_TIMER_PRECISION_MSEC,
                    TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                    bool dynamic_expand_node = true );
    ///构造函数
    ZCE_Timer_Wheel();

    ///析构函数
    virtual ~ZCE_Timer_Wheel();
public:

    /*!
    * @brief      初始化
    * @return     int                   返回0表示成功，否则表示失败
    * @param[in]  num_timer_node        定时器NODE的数量，初始化的时候会分配好，
    * @param[in]  timer_length_mesc     轮子标识的时间长度，msec，轮子上的槽位数量= timer_length_mesc/timer_precision_mesc
    * @param[in]  timer_precision_mesc  定时器的进度，多少msec，如果设置过大，有些定时器会超时后才触发，如果太小，需要更多的轮的槽位空间，
    * @param[in]  trigger_mode          触发模式是用墙上时钟还是CPU TICK，参考 @ref ZCE_Timer_Queue::TRIGGER_MODE
    * @param[in]  dynamic_expand_node   如果初始化的NODE节点数量不够，是否自动扩展
    */
    int initialize(size_t num_timer_node,
                   unsigned int timer_length_mesc = DEFAULT_TIMER_LENGTH_MESC,
                   unsigned int timer_precision_mesc = DEFAULT_TIMER_PRECISION_MSEC,
                   TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                   bool dynamic_expand_node = true );

    /*!
    * @brief      设置第一个定时器，接口参考了ACE的设计，这个设计其实算很完整了
    * @return     int           返回定时器ID，>=0标识成功，-1标识失败
    * @param      timer_hdl     放入的定时器句柄，触发后回调的对象
    * @param[in]  action        一个指针，在定时器触发会用参数打给你，
    * @param[in]  delay_time    第一次触发的时间，为什么要这样设计参数？你自己考虑一下，想想如何在10:00播放第6套广播体操
    * @param[in]  interval_time 第一次触发后，后续间隔 @a interval_time 的时间进行一次触发
    *                           如果参数等于ZCE_Time_Value::ZERO_TIME_VALUE，标识不需要后续触发，
    */
    virtual int schedule_timer(ZCE_Timer_Handler *timer_hdl,
                               const void *action,
                               const ZCE_Time_Value &delay_time,
                               const ZCE_Time_Value &interval_time = ZCE_Time_Value::ZERO_TIME_VALUE) ;

    /*!
    * @brief      取消定时器
    * @return     int      0标识成功，否则失败
    * @param      timer_id 定时器ID
    */
    virtual int cancel_timer(int timer_id);

    /*!
    * @brief      扩张相关定时器的NODE的数量，也调用底层的extend_node函数
    * @return     int 返回0标识成功
    * @param[in]  num_timer_node 要设置的定时器NODE数量
    * @param[out] old_num_node   返回原来的定时器NODE数量
    */
    virtual int extend_node(size_t num_timer_node,
                            size_t &old_num_node);

protected:

    ///在触发一次后，要对定时器进行重新计算
    virtual int reschedule_timer(int timer_id, uint64_t now_trigger_msec);

    ///取得第一个元素，也就是，最小的时间
    virtual int get_frist_nodeid(int &first_node_id);

    ///分发定时器
    size_t dispatch_timer(const ZCE_Time_Value &now_time,
                          uint64_t now_trigger_msec);

    ///将Queue和TimerNode绑定
    void bind_wheel_listnode(int time_node_id);

    ///将Queue和TimerNode解除绑定
    void unbind_wheel_listnode(int time_node_id);

protected:
    ///定时器能处理的毫秒数
    unsigned int            timer_length_mesc_;

    ///定时器点的数量，决定可以设置多长时间的定时器
    size_t                  num_wheel_point_;
    ///
    std::vector<int>        timer_wheel_point_;

    ///当前处理的时间点，因为是一个轮子，这个表示轮子开始的地方，每次触发后前进一点
    size_t                  proc_wheel_start_;

    ///时间数组队列
    ARRAY_WHEEL_TIMER_NODE  wheel_node_list_;

    ///未来触发的时间ID，用于在dispatch_timer 函数中记录，本次不触发的timer id，
    ///因为是一个环（轮子），上面可能记录的时间点是超出轮子一周表示的时间的，
    ///你可以理解为绕几圈放上去（模除），但如果发生这种情况，在处理定时器分发时就要特殊一点
    std::vector<int>        future_trigger_tid_;
};

#endif //#ifndef ZCE_LIB_TIMER_QUEUE_WHEEL_H_

