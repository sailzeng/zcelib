/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/thread/msgque_condi.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年6月17日
* @brief      多线程使用的Message Queue，内部的同步对象使用了条件变量
*             这个类暂时不要使用，他和整个体系是脱节的，完全是为了测试条件变量写的类
*             同时支持list，deque,和自己的一个容器zce::lord_rings 环形队列
* @details
*
*
*
* @note
*
*/

#pragma once

#include "zce/util/non_copyable.h"
#include "zce/lock/thread_mutex.h"
#include "zce/container/lord_rings.h"
#include "zce/lock/thread_condi.h"
#include "zce/comm/common.h"

namespace zce
{
/*!
* @brief      用条件变量+容器实现的消息队列，对于我个人来说，条件变量有点怪，
*             装B？请问condi传入Mutex的目的是？
*             请问最后应该是notiy_one,notify_all?
* @tparam     T 消息队列放入的数据类型
* @tparam     C 消息队列内部容器类型
* note
*/
template <typename T, typename C = std::deque<T> >
class msgqueue_condi
{
public:

    //构造函数和析构函数
    msgqueue_condi(size_t queue_max_size) :
        queue_max_size_(queue_max_size),
        queue_cur_size_(0)
    {
    }

    ~msgqueue_condi() = default;

    //QUEUE是否为NULL
    inline bool empty()
    {
        std::lock_guard<std::mutex> guard(queue_lock_);
        if (queue_cur_size_ == 0)
        {
            return true;
        }

        return false;
    }

    //QUEUE是否为满
    inline bool full()
    {
        std::lock_guard<std::mutex> guard(queue_lock_);
        if (queue_cur_size_ == queue_max_size_)
        {
            return true;
        }
        return false;
    }

    //放入，一直等待
    bool enqueue(const T& value_data)
    {
        std::chrono::microseconds nouse_timeout;
        return enqueue_i(value_data,
                         MQW_WAIT_FOREVER,
                         nouse_timeout);
    }

    //有超时放入
    template<class Rep, class Period>
    bool enqueue_wait(const T& value_data,
                      const std::chrono::duration<Rep, Period>& wait_time)
    {
        return enqueue_i(value_data,
                         MQW_WAIT_TIMEOUT,
                         wait_time);
    }

    //尝试放入，立即返回
    bool try_enqueue(const T& value_data)
    {
        std::chrono::microseconds nouse_timeout;
        return enqueue_i(value_data,
                         MQW_NO_WAIT,
                         nouse_timeout);
    }

    //取出
    bool dequeue(T& value_data)
    {
        std::chrono::microseconds nouse_timeout;
        return dequeue_i(value_data,
                         MQW_WAIT_FOREVER,
                         nouse_timeout);
    }

    //有超时处理的取出
    template<class Rep, class Period>
    bool dequeue_wait(T& value_data,
                      const std::chrono::duration<Rep, Period>& wait_time)
    {
        return dequeue_i(value_data,
                         MQW_WAIT_TIMEOUT,
                         wait_time);
    }
    bool dequeue_wait(T& value_data,
                      const zce::time_value& wait_time)
    {
        std::chrono::microseconds wait_mircosec;
        wait_time.to(wait_mircosec);
        return dequeue_i(value_data,
                         MQW_WAIT_TIMEOUT,
                         wait_mircosec);
    }
    //尝试取出，立即返回
    bool try_dequeue(T& value_data)
    {
        std::chrono::microseconds nouse_timeout;
        return dequeue_i(value_data,
                         MQW_NO_WAIT,
                         nouse_timeout);
    }

    void clear()
    {
        std::lock_guard<std::mutex> guard(queue_lock_);
        message_queue_.clear();
        queue_cur_size_ = 0;
    }

    size_t size()
    {
        std::lock_guard<std::mutex> guard(queue_lock_);
        return queue_cur_size_;
    }

protected:

    //放入一个数据，根据参数确定是否等待一个相对时间
    template<class Rep, class Period>
    bool enqueue_i(const T& value_data,
                   MQW_WAIT_MODEL wait_model,
                   const std::chrono::duration<Rep, Period>& wait_time)
    {
        //注意这段代码必须用{}保护，因为你必须先保证数据放入，再触发条件，
        //而条件触发其实内部是解开了保护的
        {
            std::unique_lock<std::mutex> guard(queue_lock_);

            //cond的语意是非常含混的，讨厌的，这个地方必须用while，必须重入检查
            //详细见pthread_condi的说明，
            while (queue_cur_size_ == queue_max_size_)
            {
                if (wait_model == MQW_WAIT_TIMEOUT)
                {
                    //timed_wait里面放入锁的目的是为了解开（退出的时候加上），不是加锁，
                    //所以含义很含混,WINDOWS下的实现应该是用信号灯模拟的
                    auto status = cv_en_.wait_for(guard, wait_time);

                    //如果超时了，返回false
                    if (status == std::cv_status::timeout)
                    {
                        return false;
                    }
                }
                else if (wait_model == MQW_WAIT_FOREVER)
                {
                    cv_en_.wait(guard);
                }
                else if (wait_model == MQW_NO_WAIT)
                {
                    return false;
                }
            }

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }

        //通知一个等待的人
        cv_de_.notify_one();

        return true;
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    template<class Rep, class Period>
    bool dequeue_i(T& value_data,
                   MQW_WAIT_MODEL wait_model,
                   const std::chrono::duration<Rep, Period>& wait_time)
    {
        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            std::unique_lock<std::mutex> guard(queue_lock_);

            //cond的语意是非常含混的，讨厌的，这个地方必须用while，
            //详细见pthread_condi的说明，
            while (queue_cur_size_ == 0)
            {
                //判断是否要进行超时等待
                if (wait_model == MQW_WAIT_MODEL::MQW_WAIT_TIMEOUT)
                {
                    //timed_wait里面放入锁的目的是为了解开（退出的时候加上），不是加锁，
                    //所以含义很含混
                    auto status = cv_de_.wait_for(guard, wait_time);

                    //如果超时了，返回false
                    if (status == std::cv_status::timeout)
                    {
                        return false;
                    }
                }
                else if (wait_model == MQW_WAIT_MODEL::MQW_WAIT_FOREVER)
                {
                    cv_de_.wait(guard);
                }
                else if (wait_model == MQW_WAIT_MODEL::MQW_NO_WAIT)
                {
                    return false;
                }
            }

            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }

        //通知一个等待的人
        cv_en_.notify_one();

        return true;
    }

protected:

    //QUEUE的最大尺寸
    std::size_t                  queue_max_size_;

    //由于LIST的size()函数比较耗时(过时理论)，所以这儿还是用了几个计数器
    std::size_t                  queue_cur_size_;

    //队列的LOCK,用于读写操作的同步控制
    std::mutex                   queue_lock_;
    //插入保护的条件变量
    std::condition_variable      cv_en_;
    //取出进行保护的条件变量
    std::condition_variable      cv_de_;

    //容器类型，可以是list,dequeue,
    C                            message_queue_;
};

/*!
* @brief      内部用list,deque,lord_rings实现的消息队列，
*             list msglist_condi<T> 性能低,边界保护用的条件变量。但一开始占用内存不多
*             deque msgdeque_condi<T> 性能较好
*             lord_rings msgring_condi<T> 内部用lord_rings实现的消息队列，性能非常好
* @tparam     T 消息队列保存的数据类型
* note        主要就是为了给你一些语法糖
*/
template <typename T >
class msglist_condi : public msgqueue_condi<T, std::list<T> >
{
public:
    explicit msglist_condi(size_t queue_max_size) :
        msgqueue_condi<T, std::list<T> >(queue_max_size)
    {
    }
    ~msglist_condi() = default;
};

template <class T >
class msgdeque_condi : public msgqueue_condi<T, std::deque<T> >
{
public:
    explicit msgdeque_condi(size_t queue_max_size) :
        msgqueue_condi<T, std::deque<T> >(queue_max_size)
    {
    }
    ~msgdeque_condi() = default;
};
template <class T >
class msgring_condi : public msgqueue_condi<T, zce::lord_rings<T> >
{
public:
    explicit msgring_condi(size_t queue_max_size) :
        msgqueue_condi<T, zce::lord_rings<T> >(queue_max_size)
    {
        msgqueue_condi<T,
            zce::lord_rings<T> >::message_queue_.resize(queue_max_size);
    }
    ~msgring_condi() = default;
};

//======================================================================================
/*!
* @brief      用内部条件变量+容器实现的消息队列，对于我个人来说，条件变量有点怪，
*             装B？请问condi传入Mutex的目的是？
*
* @tparam     T 消息队列放入的数据类型
* @tparam     C 消息队列内部容器类型
* note        内置条件变量是根据系统API实现的，Windows下是模拟的，在有C++14的环境下
*             不建议使用
*/
template <typename T, typename C = std::deque<T> >
class MsgQueue_Condi
{
public:

    //构造函数和析构函数
    MsgQueue_Condi(size_t queue_max_size) :
        queue_max_size_(queue_max_size),
        queue_cur_size_(0)
    {
    }

    ~MsgQueue_Condi() = default;

    //QUEUE是否为NULL
    inline bool empty()
    {
        zce::Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);

        if (queue_cur_size_ == 0)
        {
            return true;
        }

        return false;
    }

    //QUEUE是否为满
    inline bool full()
    {
        zce::Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);

        if (queue_cur_size_ == queue_max_size_)
        {
            return true;
        }

        return false;
    }

    //放入，一直等待
    bool enqueue(const T& value_data)
    {
        zce::time_value  nouse_timeout;
        return enqueue_interior(value_data,
                                MQW_WAIT_FOREVER,
                                nouse_timeout);
    }

    //有超时放入
    bool enqueue(const T& value_data,
                 const zce::time_value& wait_time)
    {
        return enqueue_interior(value_data,
                                MQW_WAIT_TIMEOUT,
                                wait_time);
    }

    //尝试放入，立即返回
    bool try_enqueue(const T& value_data)
    {
        zce::time_value  nouse_timeout;
        return enqueue_interior(value_data,
                                MQW_NO_WAIT,
                                nouse_timeout);
    }

    //取出
    bool dequeue(T& value_data)
    {
        zce::time_value  nouse_timeout;
        return dequeue_interior(value_data,
                                MQW_WAIT_FOREVER,
                                nouse_timeout);
    }

    //有超时处理的取出
    bool dequeue(T& value_data,
                 const zce::time_value& wait_time)
    {
        return dequeue_interior(value_data,
                                MQW_WAIT_TIMEOUT,
                                wait_time);
    }

    //尝试取出，立即返回
    bool try_dequeue(T& value_data)
    {
        zce::time_value  nouse_timeout;
        return dequeue_interior(value_data,
                                MQW_NO_WAIT,
                                nouse_timeout);
    }

    void clear()
    {
        zce::Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
        message_queue_.clear();
        queue_cur_size_ = 0;
    }

    size_t size()
    {
        zce::Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
        return queue_cur_size_;
    }

protected:

    //放入一个数据，根据参数确定是否等待一个相对时间
    bool enqueue_interior(const T& value_data,
                          MQW_WAIT_MODEL wait_model,
                          const timeval& wait_time)
    {
        //注意这段代码必须用{}保护，因为你必须先保证数据放入，再触发条件，
        //而条件触发其实内部是解开了保护的
        {
            zce::Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
            bool bret = false;

            //cond的语意是非常含混的，讨厌的，这个地方必须用while，必须重入检查
            //详细见pthread_condi的说明，
            while (queue_cur_size_ == queue_max_size_)
            {
                if (wait_model == MQW_WAIT_TIMEOUT)
                {
                    //timed_wait里面放入锁的目的是为了解开（退出的时候加上），不是加锁，
                    //所以含义很含混,WINDOWS下的实现应该是用信号灯模拟的
                    bret = cond_enqueue_.wait_for(&queue_lock_, wait_time);

                    //如果超时了，返回false
                    if (!bret)
                    {
                        return false;
                    }
                }
                else if (wait_model == MQW_WAIT_FOREVER)
                {
                    cond_enqueue_.wait(&queue_lock_);
                }
                else if (wait_model == MQW_NO_WAIT)
                {
                    return false;
                }
            }

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }

        //通知所有等待的人
        cond_dequeue_.broadcast();

        return true;
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    bool dequeue_interior(T& value_data,
                          MQW_WAIT_MODEL wait_model,
                          const zce::time_value& wait_time)
    {
        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            zce::Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
            bool bret = false;

            //cond的语意是非常含混的，讨厌的，这个地方必须用while，
            //详细见pthread_condi的说明，
            while (queue_cur_size_ == 0)
            {
                //判断是否要进行超时等待
                if (wait_model == MQW_WAIT_TIMEOUT)
                {
                    //timed_wait里面放入锁的目的是为了解开（退出的时候加上），不是加锁，
                    //所以含义很含混
                    bret = cond_dequeue_.wait_for(&queue_lock_, wait_time);

                    //如果超时了，返回false
                    if (!bret)
                    {
                        return false;
                    }
                }
                else if (wait_model == MQW_WAIT_FOREVER)
                {
                    cond_dequeue_.wait(&queue_lock_);
                }
                else if (wait_model == MQW_NO_WAIT)
                {
                    return false;
                }
            }

            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }

        //通知所有等待的人
        cond_enqueue_.broadcast();

        return true;
    }

protected:

    //QUEUE的最大尺寸
    std::size_t                  queue_max_size_;

    //由于LIST的size()函数比较耗时，所以这儿还是用了几个计数器
    std::size_t                  queue_cur_size_;

    //队列的LOCK,用于读写操作的同步控制
    zce::Thread_Light_Mutex      queue_lock_;

    //插入保护的条件变量
    zce::Thread_Condition        cond_enqueue_;

    //取出进行保护的条件变量
    zce::Thread_Condition        cond_dequeue_;

    //容器类型，可以是list,dequeue,
    C              message_queue_;
};

/*!
* @brief      内部用list,deque,lord_rings实现的消息队列，
*             list MsgList_Condi<T> 性能低,边界保护用的条件变量。但一开始占用内存不多
*             deque MsgList_Condi<T> 性能较好
*             lord_rings MsgList_Condi<T> 内部用lord_rings实现的消息队列，性能非常好
* @tparam     T 消息队列保存的数据类型
* note        主要就是为了给你一些语法糖
*/
template <typename T >
class MsgList_Condi : public MsgQueue_Condi<T, std::list<T> >
{
public:
    explicit MsgList_Condi(size_t queue_max_size) :
        MsgQueue_Condi<T, std::list<T> >(queue_max_size)
    {
    }

    ~MsgList_Condi() = default;
};

/*!
* @brief      内部用DQUEUE实现的消息队列，性能较好,边界保护用的条件变量。
*
* @tparam     T 消息队列保存的数据类型
* note
*/
template <class T >
class MsgDeque_Condi : public MsgQueue_Condi<T, std::deque<T> >
{
public:
    explicit MsgDeque_Condi(size_t queue_max_size) :
        MsgQueue_Condi<T, std::deque<T> >(queue_max_size)
    {
    }

    ~MsgDeque_Condi() = default;
};

template <class T >
class MsgRings_Condi : public MsgQueue_Condi<T, zce::lord_rings<T> >
{
public:
    explicit MsgRings_Condi(size_t queue_max_size) :
        MsgRings_Condi<T, zce::lord_rings<T> >(queue_max_size)
    {
        MsgRings_Condi<T, zce::lord_rings<T> >::message_queue_.resize(queue_max_size);
    }

    ~MsgRings_Condi() = default;
};
}