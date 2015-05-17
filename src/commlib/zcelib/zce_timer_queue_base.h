/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_timer_queue_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2008年10月6日
* @brief      Timer队列处理的基类
*
* @details
*
* @note
*
*/

#ifndef ZCE_TIMER_QUEUE_BASE_H_
#define ZCE_TIMER_QUEUE_BASE_H_

#include "zce_boost_non_copyable.h"
#include "zce_time_value.h"

class ZCE_Timer_Handler;
/******************************************************************************************
ZCE_Timer_Queue
******************************************************************************************/
class ZCE_Timer_Queue : public ZCE_NON_Copyable
{

public:
    //无效的时间ID
    static const int  INVALID_TIMER_ID = -1;

    ///触发模式，
    enum TRIGGER_MODE
    {

        ///根据系统时钟进行触发，缺点是，在系统时间被调整的情况下，会丢失定时器
        TRIGGER_MODE_SYSTEM_CLOCK = 1,

        ///CPU，TICK，好处绝不丢失，坏处保存超时时间不是墙上时钟，
        TRIGGER_MODE_CPU_TICK     = 2,
    };

    //默认的时间精度
    static const unsigned int DEFAULT_TIMER_PRECISION_MSEC = 100;

    //gethrtime返回的原来是纳秒，我只需要毫秒
    static const uint64_t MSEC_PER_NSEC = 1000000ll;

    //如果自动扩张，一次扩张的NODE数量
    static const size_t     ONCE_EXTEND_NODE_NUMBER = 1024 * 8;

protected:
    /******************************************************************************************
    ZCE_TIMER_NODE 定时器的节点，用于定时器的反应器内部保存定时器的信息
    ******************************************************************************************/
    struct ZCE_TIMER_NODE
    {
    public:

        ///定时器的ID,不好意思，我不打算支持>22亿个定时器，我蛋疼，
        int                         time_id_;

        ///以后每次间隔的触发等待时间
        ZCE_Time_Value              interval_time_;

        ///回调的时候返回的指针，我只是保存他，给你用的。你自己把握好
        const void                 *action_;

        ///对应的时间句柄的的指针
        ZCE_Timer_Handler          *timer_handle_;

        ///下一次触发点，可能是一个绝度时间，也可能是一个CPU TICK的计数,但都是一个绝对值
        uint64_t                    next_trigger_point_;

        ///是否已经触发过了，
        ///yunfeiyang帮忙发现了一个bug，为了解决这个bug，我们增加了这个字段。出错过程是这样的，
        ///1. dispatch_timer在调用timer_timeout时，如果timer_timeout中删除了自己的定时器，又增加了自己的定时器，此时增加的定时器，
        ///   time_node_id与当前的在dispatch_timer中处理的time_node_id是一样的,(因为分配队列的原因)
        ///2. dispatch调用完timer_timeout后，会reschedule_timer, reschedule_timer发现这个timer是一次性的，删除，因此time_node_id
        ///   在定时器内部已经失效，（但外部并不知道）
        ///3. 在事务处理中（外部）释放时，其还会错误time_node_id有效，所以又会调用cancel_timer函数，但此时这个time_node_id在定时器内
        ///   部会认为已经释放了，导致出错
        bool                        already_trigger_;

    public:
        //构造函数和析构函数
        ZCE_TIMER_NODE():
            time_id_(INVALID_TIMER_ID),
            interval_time_(ZCE_Time_Value::ZERO_TIME_VALUE),
            action_(NULL),
            timer_handle_(NULL),
            next_trigger_point_(0),
            already_trigger_(false)
        {
        };

        ~ZCE_TIMER_NODE()
        {
        };

        void clear()
        {
            interval_time_.set(0, 0);

            action_ = NULL;
            timer_handle_ = NULL;
            next_trigger_point_ = 0;
            already_trigger_ = false;
        }
    };

protected:
    //构造函数
    ZCE_Timer_Queue(size_t num_timer_node,
                    unsigned int timer_precision_mesc = DEFAULT_TIMER_PRECISION_MSEC,
                    TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                    bool dynamic_expand_node = true);
    ZCE_Timer_Queue();
public:
    virtual ~ZCE_Timer_Queue();

public:

    /*!
    @brief      设置第一个定时器，接口参考了ACE的设计，这个设计其实算很完整了，你扩展的类，必须实现这个接口
    @return     int           返回定时器ID，>=0标识成功，-1标识失败
    @param      timer_hdl     放入的定时器句柄，触发后回调的对象
    @param[in]  action        一个指针，在定时器触发会用参数还给你，
    @param[in]  delay_time    第一次触发的时间，为什么要这样设计参数？你自己考虑一下，想想如何在10:00播放第6套广播体操
    @param[in]  interval_time 第一次触发后，后续间隔 @a interval_time 的时间进行一次触发
                              如果参数等于ZCE_Time_Value::ZERO_TIME_VALUE，标识不需要后续触发，
    */
    virtual int schedule_timer(ZCE_Timer_Handler *timer_hdl,
                               const void *action,
                               const ZCE_Time_Value &delay_time,
                               const ZCE_Time_Value &interval_time = ZCE_Time_Value::ZERO_TIME_VALUE) = 0;

    /*!
    @brief      取消定时器，你继承后必须实现这个接口
    @return     int      0标识成功，否则失败
    @param[in]  timer_id 定时器ID
    */
    virtual int cancel_timer(int timer_id) = 0;

protected:

    /*!
    @brief      取得第一个要触发的定时器NODE，也就是，最近的触发定时器，你继承后必须实现这个接口
    @return     int   0标识成功，否则失败
    @param[out] timer_node_id
    */
    virtual int get_frist_nodeid(int &timer_node_id) = 0;

    /*!
    @brief      分发定时器
    @return     size_t            返回分发的触发的定时器的数量
    @param      now_time          当前的时间，墙上时钟
    @param      now_trigger_msec  当前触发的点的毫秒数，根据触发模式，表达意义不一样
                                  触发模式是 @a TRIGGER_MODE_SYSTEM_CLOCK 那么表示墙上时钟的mesc数量
                                  触发模式是 @a TRIGGER_MODE_CPU_TICK 那么标识CPU Tick的毫秒数量
    */
    virtual size_t dispatch_timer(const ZCE_Time_Value &now_time,
                                  uint64_t now_trigger_msec) = 0;

    /*!
    @brief      在触发一次后，要对定时器进行重新计算，重排，你必须实现的虚函数
    @return     int              0标识成功，否则失败
    @param      timer_id         定时器ID
    @param      now_trigger_msec 当前的触发点的毫秒数，不同模式下表达不太一样
    @note
    */
    virtual int reschedule_timer(int timer_id, uint64_t now_trigger_msec) = 0;

    //---------------------------------------------------------------------------------------
public:

    //下面这些已经实现，你可以重载，或者直接使用而已

    /*!
    @brief      进行超时处理，你需要调用的函数，分发,直接调用函数取得时间然后进行分发
    @return     size_t 返回分发的触发的定时器的数量
    */
    virtual size_t expire();

    /*!
    @brief      使用 ZCE_Timer_Handler的指针 @a timer_hdl 取消定时器方式，超级
                超级，超级慢的函数，(当然使用起来可能比较方便)，你可以继承,
                一般情况下，推荐用time id 取消定时器
    @return     int       返回0表示成功，否则失败
    @param      timer_hdl 定时器句柄的指针
    */
    virtual int cancel_timer(const ZCE_Timer_Handler *timer_hdl);

    /*!
    @brief      扩张相关定时器的NODE的数量，
                由于子类还有和NOE相关的数据结构，在扩展是也要扩展，所以是visual
    @return     int 返回0标识成功
    @param[in]  num_timer_node 要设置的定时器NODE数量
    @param[out] old_num_node   返回原来的定时器NODE数量
    */
    virtual int extend_node(size_t num_timer_node,
                            size_t &old_num_node) = 0;

    /*!
    @brief      关闭定时器队列
    @return     int 0表示成，否则失败
    */
    virtual int close();

protected:

    //--------------------------------------------------------------------------------------

    /*!
    @brief      初始化
    @return     int                   返回0标识初始化成功吗，否则失败
    @param[in]  num_timer_node        定时器NODE的数量，初始化的时候会分配好，
    @param[in]  timer_precision_mesc  定时器的进度，多少msec，如果设置过大，有些定时器会超时后才触发，如果太小，需要更多的轮的槽位空间，
    @param[in]  trigger_mode          触发模式是用墙上时钟还是CPU TICK，参考 @ref ZCE_Timer_Queue::TRIGGER_MODE
    @param[in]  dynamic_expand_node   如果初始化的NODE节点数量不够，是否自动扩展
    */
    int initialize(size_t num_timer_node,
                   unsigned int timer_precision_mesc = DEFAULT_TIMER_PRECISION_MSEC,
                   TRIGGER_MODE trigger_mode = TRIGGER_MODE_SYSTEM_CLOCK,
                   bool dynamic_expand_node = true);

    /*!
    @brief      分配一个崭新的Timer Node
    @return     int             返回0标识分配成功
    @param[in]  timer_hdl       Timer Handler的指针，TIMER NODE中需要记录的，
    @param[in]  action          schedule_timer的参数，在回调timer_timeout的时候会回填回去
    @param[in]  delay_time_     第一次触发的时间
    @param[in]  interval_time_  后续持续触发的间隔时间
    @param[out] time_node_id    返回的分配的ID
    @param[out] alloc_time_node 返回的分配的TIMER NODE的指针
    */
    int alloc_timernode(ZCE_Timer_Handler *timer_hdl,
                        const void *action,
                        const ZCE_Time_Value &delay_time_,
                        const ZCE_Time_Value  &interval_time_,
                        int &time_node_id,
                        ZCE_TIMER_NODE *&alloc_time_node);

    /*!
    @brief      计算下一个触发点，当一个定时器到期触发后，需要计算她下一次的触发点，
    @param[in]  time_node_id     TIME ID,如果还有触发，会修改对应的TIMER NODE下一次触发时间点
    @param[in]  now_trigger_msec 当前的触发时间点，
    @param[out] continue_trigger 如果有触发点返回0，如果没有返回
    */
    void calc_next_trigger(int time_node_id,
                           uint64_t now_trigger_msec,
                           bool &continue_trigger);

    /*!
    @brief      释放Timer Node
    @return     int          0标识成功释放，否则表示参数有问题，如果你的参数正确，都会成功释放
    @param[in]  time_node_id 返回的TIMER NODE指针
    */
    int free_timernode(int time_node_id);

    /*!
    @brief      得到最快将在多少时间后（注意是时间长度）触发
    @return     int           0表示成，否则失败
    @param      first_timeout 最快的时间长度，（不是时间点）
    */
    int get_first_timeout(ZCE_Time_Value *first_timeout);

public:
    //这个地方的单子使用，和其他地方略有不同，要先调用赋值的函数，将子类指针付给这个函数

    //
    static ZCE_Timer_Queue *instance();
    //
    static void instance(ZCE_Timer_Queue *pinstatnce);
    //
    static void clean_instance();

    //数据成员
protected:

    ///TIMER NODE(也就是HANDLER)的数量，
    size_t                      num_timer_node_;

    ///时间精度,以毫秒为单位，想要微秒得定时器，你在做梦呢，
    ///你给出时间进度后，必须在这个精度内调用dispatch_timer 函数
    ///普通服务器我建议你用s的级别，也就是1000ms
    ///要求精度比较高的服务器，我建议你用100ms
    ///要求不高的服务器，建议你用Ns，N小于10s
    unsigned int                timer_precision_mesc_;

    ///已经使用的节点的数量，也就是分配了多少个定时器
    size_t                      num_use_node_;

    ///触发模式
    TRIGGER_MODE                trigger_mode_;

    ///是否支持动态扩张NODE
    bool                        dynamic_expand_node_;

    ///TIMER NODE的队列,本来用指针自己管理的，但是考虑再三，扩张的代码我要写好不如用resize
    ///算了，不要自己造这种轮子
    std::vector<ZCE_TIMER_NODE> time_node_ary_;

    ///空闲TIMER NODE队列的头下标，单向链表
    int                         free_node_id_head_;
    ///空闲队列的下标列表的数组
    std::vector<int>            free_node_id_list_;
    ///本来想蛋疼的写一个使用列表，后来发现如果要用，就必须用双向列表，我蛋疼，饶了我

    ///上一次的CPU TICK的触发点，
    uint64_t                    prev_trigger_msec_;

    ///定时器计算的参考时间点，每次定时器触发的时候改变
    uint64_t                    timer_refer_pointer_;

protected:

    ///单子实例指针
    static ZCE_Timer_Queue     *instance_;
};

#endif //# ZCE_LIB_TIMER_QUEUE_BASE_H_

