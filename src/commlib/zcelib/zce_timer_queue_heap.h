/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_timer_queue_heap.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年10月3日
* @brief      堆的定时器队列，所有的超时时间用堆进行排序，这样判断是否有定时器要触发就很简单，
*             只用在堆顶判断就OK。
*             先说优点，效率也还行，一般群众的需求他都满足，而且即使相差1ms，堆定时器队列能保证
*             触发的顺序，
*             另外，他的空间表WHEEL要小。
*             缺点是，每次发生超时（其实是每个NODE发生超时），设置定时器，取消定时器，就要发生
*             调整，调整的消耗级别都是O(log2N),N为当前有多少定时器。
*             分发定时器的消耗来自于分发后的调整，
*
* @details    大致的性能数据，我们在ACE中，用HEAP的定时器模型，测试10W个定时器，每个定时器大致10s
*             （还是5分钟）触发，压力都不大，CPU.5%以下
*             我们的实现肯定小于ACE的消耗，
*
* @note
*
*/
#ifndef ZCE_LIB_TIMER_QUEUE_HEAP_H_
#define ZCE_LIB_TIMER_QUEUE_HEAP_H_

#include "zce_timer_queue_base.h"
#include "zce_time_value.h"

//

/*!
*@brief     堆的定时器队列，所有的超时时间用堆进行排序，这样判断是否有定时器要触发就很简单，
*           只用在堆顶判断就OK。
*           其各种操作的时间复杂度都基本是O(log(2N)) 这个级别。
*
*/
class ZCE_Timer_Heap : public ZCE_Timer_Queue
{

public:
    ///构造函数
    ZCE_Timer_Heap(size_t num_timer_node,
                   unsigned int timer_length_mesc = 1000,
                   TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                   bool dynamic_expand_node = true );
    ///默认构造函数
    ZCE_Timer_Heap();

    ///析构函数
    virtual ~ZCE_Timer_Heap();
public:

    /*!
    * @brief      初始化
    * @return     int                   返回0标识初始化成功吗，否则失败
    * @param[in]  num_timer_node        定时器NODE的数量，初始化的时候会分配好，
    * @param[in]  timer_precision_mesc  定时器的进度，多少msec，如果设置过大，有些定时器会超时后才触发，如果太小，需要更多的轮的槽位空间，
    * @param[in]  trigger_mode          触发模式是用墙上时钟还是CPU TICK，参考 @ref ZCE_Timer_Queue::TRIGGER_MODE
    * @param[in]  dynamic_expand_node   如果初始化的NODE节点数量不够，是否自动扩展
    */
    int initialize(size_t num_timer_node ,
                   unsigned int timer_precision_mesc = 1000,
                   TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                   bool dynamic_expand_node = true );

    /*!
    * @brief      设置第一个定时器，接口参考了ACE的设计，这个设计其实算很完整了，你扩展的类，必须实现这个接口
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
    * @brief      取消定时器，你继承后必须实现这个接口
    * @return     int      0标识成功，否则失败
    * @param[in]  timer_id 定时器ID
    */
    virtual int cancel_timer(int timer_id);

    //dump数据
    void dump();

    //扩张相关十字链表的NODE的数量，也调用底层的extend_node函数
    virtual int extend_node(size_t num_timer_node,
                            size_t &old_num_node);

protected:

    /*!
    * @brief      在触发一次后，要对定时器进行重新计算,你必须实现的虚函数
    * @return     int              0标识成功，否则失败
    * @param      timer_id         定时器ID
    * @param      now_trigger_msec 当前的触发点的毫秒数，不同模式下表达不太一样
    */
    virtual int reschedule_timer(int timer_id, uint64_t now_trigger_msec);

    /*!
    * @brief      分发定时器
    * @return     size_t            返回分发的触发的定时器的数量
    * @param      now_time          当前的时间，墙上时钟
    * @param      now_trigger_msec  当前触发的点的毫秒数，根据触发模式，表达意义不一样
    */
    virtual size_t dispatch_timer(const ZCE_Time_Value &now_time,
                                  uint64_t now_trigger_msec);

    /*!
    * @brief      取得第一个要触发的定时器NODE，也就是，最近的触发定时器，你继承后必须实现这个接口
    * @return     int   0标识成功，否则失败
    * @param[out] timer_node_id
    */
    virtual int get_frist_nodeid(int &timer_node_id);

    //下面是堆处理的函数----------------------------------------------------------

    /*!
    * @brief      向上寻找合适的位置
    * @return     bool    是否发生了调整
    * @param      heap_id 调整的元素在堆上的ID
    */
    bool reheap_up(size_t heap_id);

    /*!
    * @brief      向下寻找合适的位置
    * @return     bool     是否发生了调整
    * @param      heap_id  调整的元素在堆上的ID
    */
    bool reheap_down(size_t heap_id);

    /*!
    * @brief      增加一个Timer Node元素到堆上
    * @return     int           0表示成功，否则失败
    * @param      timer_node_id 添加的定时器Timer Node ID
    */
    int add_nodeid(int timer_node_id);
    //
    /*!
    * @brief      从堆上删除这个Timer Node元素，
    * @return     int           0表示成功，否则失败
    * @param      timer_node_id 删除的定时器Timer Node ID
    * @note
    */
    int remove_nodeid(int timer_node_id);

protected:

    ///堆的大小，和TIMER NODE的大小应该一样
    size_t               size_heap_;

    ///本质是一个最小堆，里面存放Timer NODE的ID
    std::vector <int>    timer_node_heap_;

    ///TIMER NODE到HEAPID的对应表，他设计成和TIMER NODE一样多，
    ///TIMER NODE的ID（也就是TIMER ID），就能下标定位找到堆的位置
    std::vector <int>    note_to_heapid_;

};

#endif //#ifndef ZCE_LIB_TIMER_QUEUE_HEAP_H_

