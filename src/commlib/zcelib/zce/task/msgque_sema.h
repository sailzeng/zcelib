

#pragma once

#include "zce/lock/synch_traits.h"
#include "zce/util/non_copyable.h"


namespace zce
{
/*!
* @brief      用信号灯+容器实现的消息队列，对于我个人来说，还是信号灯好理解一些
*
* @tparam     T      消息队列放入的数据类型
* @tparam     C  消息队列内部容器类型
*/
template < typename T,
    typename C >
    class msgqueue_sema : public zce::NON_Copyable
{
public:

    //
    explicit msgqueue_sema(size_t queue_max_size) :
        queue_max_size_(queue_max_size),
        queue_cur_size_(0),
        sem_full_(queue_max_size)),
        sem_empty_(0)
    {
    }

    ~msgqueue_sema()
    {
    }

    //QUEUE是否为NULL
    inline bool empty()
    {
        Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);

        if (queue_cur_size_ == 0)
        {
            return true;
        }

        return false;
    }

    //QUEUE是否为满
    inline bool full()
    {
        std::unique_lock<std::mutex> guard(queue_lock_);

        if (queue_cur_size_ == queue_max_size_)
        {
            return true;
        }

        return false;
    }

    //放入数据，一直等待
    int enqueue(const T& value_data)
    {
        sem_full_.lock();

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            std::unique_lock<std::mutex> lock_guard(queue_lock_);

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }
        sem_empty_.unlock();

        return 0;
    }

    //放入一个数据，进行超时等待
    int enqueue(const T& value_data,
                const zce::Time_Value& wait_time)
    {
        bool bret = false;
        bret = sem_full_.lock_for(wait_time);

        //如果超时了，返回false
        if (!bret)
        {
            return -1;
        }

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            std::unique_lock<std::mutex> lock_guard(queue_lock_);

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }
        sem_empty_.unlock();

        return 0;
    }

    //试着放入新的数据进入队列，如果没有成功，立即返回
    int try_enqueue(const T& value_data)
    {
        bool bret = false;
        bret = sem_full_.try_lock();

        //如果超时了，返回false
        if (!bret)
        {
            errno = EWOULDBLOCK;
            return -1;
        }

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            std::unique_lock<std::mutex> lock_guard(queue_lock_);

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }
        sem_empty_.unlock();

        return 0;
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    int dequeue(T& value_data,
                const zce::Time_Value& wait_time)
    {
        bool bret = false;
        bret = sem_empty_.lock_for(wait_time);

        //如果超时了，返回false
        if (!bret)
        {
            return -1;
        }

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            std::unique_lock<std::mutex> guard(queue_lock_);
            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }
        sem_full_.unlock();

        return 0;
    }

    //取出一个数据，一直等待
    int dequeue(T& value_data)
    {
        sem_empty_.lock();

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            std::unique_lock<std::mutex> guard(queue_lock_);
            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }
        sem_full_.unlock();

        return 0;
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    int try_dequeue(T& value_data)
    {
        bool bret = false;
        bret = sem_empty_.try_lock();

        //如果超时了，返回false
        if (!bret)
        {
            errno = EWOULDBLOCK;
            return -1;
        }

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            std::unique_lock<std::mutex> guard(queue_lock_);
            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }
        sem_full_.unlock();

        return 0;
    }

    //清理消息队列
    void clear()
    {
        Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
        message_queue_.clear();
        queue_cur_size_ = 0;
    }

    //返回消息对象的尺寸
    size_t size()
    {
        Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
        return queue_cur_size_;
    }

protected:

    //取出一个数据，根据参数确定是否等待一个相对时间
    int dequeue(T& value_data,
                bool if_wait_timeout,
                const std::chrono::microseconds& wait_time& wait_time)
    {
        //进行超时等待
        if (if_wait_timeout)
        {
            bool bret = false;
            bret = sem_empty_.lock_for(wait_time);

            //如果超时了，返回false
            if (!bret)
            {
                return -1;
            }
        }
        else
        {
            sem_empty_.lock();
        }

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }
        sem_full_.unlock();

        return 0;
    }

protected:

    //QUEUE的最大尺寸
    size_t                               queue_max_size_;

    //由于LIST的size()函数比较耗时，所以这儿还是用了个计数器，而不直接使用_container_type.size()
    size_t                               queue_cur_size_;

    //队列的LOCK,用于读写操作的同步控制
    std::mutex                           queue_lock_;

    //信号灯，满的信号灯
    std::binary_semaphore                sem_full_;

    //信号灯，空的信号灯，当数据
    std::binary_semaphore                sem_empty_;

    //容器类型，可以是list,dequeue,
    C                                    message_queue_;
};

/*!
* @brief      内部用LIST实现的消息队列，性能低,边界保护用的条件变量。但一开始占用内存不多
*             zce::MsgQueue_List <ZCE_MT_SYNCH,_value_type> ZCE_MT_SYNCH 参数特化
* @tparam     _value_type 消息队列保存的数据类型
* note        主要就是为了给你一些语法糖
*/
template <typename _value_type >
class MsgQueue_List : public Message_Queue<_value_type, std::list<_value_type> >
{
public:
    //
    explicit MsgQueue_List(size_t queue_max_size) :
        Message_Queue<_value_type, std::list<_value_type> >(queue_max_size)
    {
    }

    ~MsgQueue_List()
    {
    }
};

/*!
* @brief      内部用DQUEUE实现的消息队列，性能较好,边界保护用的条件变量。
*             边界保护用信号灯
* @tparam     _value_type 消息队列保存的数据类型
* note       封装的主要就是为了给你一些语法糖
*/
template <typename _value_type >
class MsgQueue_Deque : public Message_Queue<_value_type, std::deque<_value_type> >
{
public:
    //
    explicit MsgQueue_Deque(size_t queue_max_size) :
        Message_Queue<_value_type, std::deque<_value_type> >(queue_max_size)
    {
    }

    ~MsgQueue_Deque()
    {
    }
};

/*!
* @brief      内部用circular_buffer实现的消息队列，性能非常好,边界保护用信号灯，的消息队列，
*             但空间比较费
* @tparam     _value_type 消息队列保存的数据类型
* note        这个封装的主要不光是了为了给你语法糖，而且是为了极限性能
*/
template <typename _value_type >
class MsgQueue_Rings : public Message_Queue<_value_type, zce::lord_rings<_value_type> >
{
public:
    //
    explicit MsgQueue_Rings(size_t queue_max_size) :
        Message_Queue<_value_type, zce::lord_rings<_value_type> >(queue_max_size)
    {
        Message_Queue<_value_type, zce::lord_rings<_value_type> >::message_queue_.resize(queue_max_size);
    }

    ~MsgQueue_Rings()
    {
    }
};
}
