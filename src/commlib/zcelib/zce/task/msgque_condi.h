#pragma once

#include "zce/predefine.h"

namespace zce
{
/*!
* @brief      用条件变量+容器实现的消息队列，对于我个人来说，条件变量有点怪，装B？请问condi传入Mutex的目的是？
*
* @tparam     T 消息队列放入的数据类型
* @tparam     C 消息队列内部容器类型
* note
*/
template <typename T, typename C = std::deque<T> >
class msgqueue_condi
{
protected:

    enum MQW_WAIT_MODEL
    {
        //不用等待，尝试一下
        MQW_NO_WAIT,
        //一直等待
        MQW_WAIT_FOREVER,
        //在超时时间内等待
        MQW_WAIT_TIMEOUT,
    };

public:

    //构造函数和析构函数
    msgqueue_condi(size_t queue_max_size) :
        queue_max_size_(queue_max_size),
        queue_cur_size_(0)
    {
    }

    ~msgqueue_condi()
    {
    }

    //QUEUE是否为NULL
    inline bool empty()
    {
        std::unique_lock<std::mutex> guard(queue_lock_);
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
    int enqueue(const T& value_data)
    {
        std::chrono::microseconds nouse_timeout;
        return enqueue_interior(value_data,
                                MQW_WAIT_FOREVER,
                                nouse_timeout);
    }

    //有超时放入
    int enqueue(const T& value_data,
                std::chrono::microseconds& dura)
    {
        return enqueue_interior(value_data,
                                MQW_WAIT_TIMEOUT,
                                dura);
    }

    //尝试放入，立即返回
    int try_enqueue(const T& value_data)
    {
        std::chrono::microseconds nouse_timeout;
        return enqueue_interior(value_data,
                                MQW_NO_WAIT,
                                nouse_timeout);
    }

    //取出
    int dequeue(T& value_data)
    {
        std::chrono::microseconds nouse_timeout;
        return dequeue_interior(value_data,
                                MQW_WAIT_FOREVER,
                                nouse_timeout);
    }

    //有超时处理的取出
    int dequeue(T& value_data,
                const std::chrono::microseconds& dura)
    {
        return dequeue_interior(value_data,
                                MQW_WAIT_TIMEOUT,
                                dura);
    }

    //尝试取出，立即返回
    int try_dequeue(T& value_data)
    {
        std::chrono::microseconds nouse_timeout;
        return dequeue_interior(value_data,
                                MQW_NO_WAIT,
                                nouse_timeout);
    }

    void clear()
    {
        std::unique_lock<std::mutex> guard(queue_lock_);
        message_queue_.clear();
        queue_cur_size_ = 0;
    }

    size_t size()
    {
        std::unique_lock<std::mutex> guard(queue_lock_);
        return queue_cur_size_;
    }

protected:

    //放入一个数据，根据参数确定是否等待一个相对时间
    int enqueue_interior(const T& value_data,
                         MQW_WAIT_MODEL wait_model,
                         const std::chrono::microseconds& wait_time)
    {
        //注意这段代码必须用{}保护，因为你必须先保证数据放入，再触发条件，
        //而条件触发其实内部是解开了保护的
        {
            std::unique_lock<std::mutex> lock(queue_lock_);
            bool bret = false;

            //cond的语意是非常含混的，讨厌的，这个地方必须用while，必须重入检查
            //详细见pthread_condi的说明，
            while (queue_cur_size_ == queue_max_size_)
            {
                if (wait_model == MQW_WAIT_TIMEOUT)
                {
                    //timed_wait里面放入锁的目的是为了解开（退出的时候加上），不是加锁，
                    //所以含义很含混,WINDOWS下的实现应该是用信号灯模拟的
                    auto cvs = cv_en_.wait_for(lock, wait_time);

                    //如果超时了，返回false
                    if (cvs == std::cv_status::timeout)
                    {
                        zce::last_error(ETIMEDOUT);
                        return -1;
                    }
                }
                else if (wait_model == MQW_WAIT_FOREVER)
                {
                    cv_en_.wait(lock);
                }
                else if (wait_model == MQW_NO_WAIT)
                {
                    zce::last_error(EWOULDBLOCK);
                    return -1;
                }
            }

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }

        //通知一个等待的人
        cv_de_.notify_one();

        return 0;
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    int dequeue_interior(_value_type& value_data,
                         MQW_WAIT_MODEL wait_model,
                         const std::chrono::microseconds& wait_time)
    {
        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            std::unique_lock<std::mutex> lock(queue_lock_);

            //cond的语意是非常含混的，讨厌的，这个地方必须用while，
            //详细见pthread_condi的说明，
            while (queue_cur_size_ == 0)
            {
                //判断是否要进行超时等待
                if (wait_model == MQW_WAIT_TIMEOUT)
                {
                    //timed_wait里面放入锁的目的是为了解开（退出的时候加上），不是加锁，
                    //所以含义很含混
                    auto cvs = cv_de_.wait_for(lock, wait_time);

                    //如果超时了，返回false
                    if (cvs == std::cv_status::timeout)
                    {
                        zce::last_error(ETIMEDOUT);
                        return -1;
                    }
                }
                else if (wait_model == MQW_WAIT_FOREVER)
                {
                    cv_de_.wait(lock);
                }
                else if (wait_model == MQW_NO_WAIT)
                {
                    zce::last_error(EWOULDBLOCK);
                    return -1;
                }
            }

            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }

        //通知一个等待的人
        cv_en_.notify_one();

        return 0;
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
* @brief      内部用LIST实现的消息队列，性能低,边界保护用的条件变量。但一开始占用内存不多
*
* @tparam     _value_type 消息队列保存的数据类型
* note        主要就是为了给你一些语法糖
*/
template <typename T>
class msgqueue_condi_list : public msgqueue_condi<T, std::list<T> >
{
public:
    //
    explicit msgqueue_condi_list(size_t queue_max_size) :
        msgqueue_condi<T, std::list<T> >(queue_max_size)
    {
    }

    ~msgqueue_condi_list()
    {
    }
};

/*!
* @brief      内部用DQUEUE实现的消息队列，性能较好,边界保护用的条件变量。
*
* @tparam     _value_type 消息队列保存的数据类型
* note
*/
template <class T >
class msgqueue_condi_deque : public msgqueue_condi<T, std::deque<T> >
{
public:
    //
    explicit msgqueue_condi_deque(size_t queue_max_size) :
        msgqueue_condi<T, std::deque<T> >(queue_max_size)
    {
    }

    ~msgqueue_condi_deque()
    {
    }
};

/*!
* @brief      内部用circular_buffer实现的消息队列，性能非常好,边界保护用的条件变量。
*
* @tparam     _value_type 消息队列保存的数据类型
* note       封装的主要不光是了为了给你语法糖，而且是为了极限性能
*/
template <class T >
class msgqueue_condi_ring : public msgqueue_condi<T, zce::lord_rings<T> >
{
public:
    //
    explicit msgqueue_condi_ring(size_t queue_max_size) :
        msgqueue_condi<T, zce::lord_rings<T> >(queue_max_size)
    {
        msgqueue_condi<T, zce::lord_rings<T> >::message_queue_.resize(queue_max_size);
    }

    ~msgqueue_condi_ring()
    {
    }
};
}
