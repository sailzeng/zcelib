/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/thread/msgque_sema.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年6月17日
* @brief      用信号灯+容器实现的消息队列，对于我个人来说，还是信号灯好理解一些
*
*
* @details
*
*
*
* @note
*
*/


#ifndef ZCE_LIB_THREAD_MESSAGE_QUEUE_SEMAPHORE_H_
#define ZCE_LIB_THREAD_MESSAGE_QUEUE_SEMAPHORE_H_

#include "zce/lock/synch_traits.h"
#include "zce/thread/msgque_template.h"

#include "zce/util/non_copyable.h"
#include "zce/lock/thread_mutex.h"
#include "zce/lock/synch_traits.h"
#include "zce/lock/thread_semaphore.h"


/*!
* @brief      用信号灯+容器实现的消息队列，对于我个人来说，还是信号灯好理解一些
*
* @tparam     _value_type      消息队列放入的数据类型
* @tparam     _container_type  消息队列内部容器类型
*/
template < typename _value_type,
           typename _container_type >
class ZCE_Message_Queue<ZCE_MT_SYNCH, _value_type, _container_type> : public zce::NON_Copyable
{

public:

    //
    explicit ZCE_Message_Queue(size_t queue_max_size):
        queue_max_size_(queue_max_size),
        queue_cur_size_(0),
        sem_full_(static_cast<unsigned int>(queue_max_size)),
        sem_empty_(0)
    {
    }

    ~ZCE_Message_Queue()
    {
    }

    //QUEUE是否为NULL
    inline bool empty()
    {
        ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);

        if (queue_cur_size_ == 0)
        {
            return true;
        }

        return false;
    }

    //QUEUE是否为满
    inline bool full()
    {
        ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);

        if (queue_cur_size_ == queue_max_size_)
        {
            return true;
        }

        return false;
    }

    //放入数据，一直等待
    int enqueue(const _value_type &value_data)
    {
        sem_full_.lock();

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            ZCE_Thread_Light_Mutex::LOCK_GUARD lock_guard(queue_lock_);

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }
        sem_empty_.unlock();

        return 0;
    }

    //放入一个数据，进行超时等待
    int enqueue(const _value_type &value_data,
                const zce::Time_Value  &wait_time )
    {
        bool bret = false;
        bret = sem_full_.duration_lock(wait_time);

        //如果超时了，返回false
        if (!bret)
        {
            return -1;
        }

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            ZCE_Thread_Light_Mutex::LOCK_GUARD lock_guard(queue_lock_);

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }
        sem_empty_.unlock();

        return 0;
    }

    //试着放入新的数据进入队列，如果没有成功，立即返回
    int try_enqueue(const _value_type &value_data)
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
            ZCE_Thread_Light_Mutex::LOCK_GUARD lock_guard(queue_lock_);

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }
        sem_empty_.unlock();

        return 0;
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    int dequeue(_value_type &value_data,
                const zce::Time_Value  &wait_time )
    {
        bool bret = false;
        bret = sem_empty_.duration_lock(wait_time);

        //如果超时了，返回false
        if (!bret)
        {
            return -1;
        }

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }
        sem_full_.unlock();

        return 0;
    }

    //取出一个数据，一直等待
    int dequeue(_value_type &value_data)
    {
        sem_empty_.lock();

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }
        sem_full_.unlock();

        return 0;
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    int try_dequeue(_value_type &value_data)
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
            ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
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
        ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
        message_queue_.clear();
        queue_cur_size_ = 0;
    }

    //返回消息对象的尺寸
    size_t size()
    {
        ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
        return queue_cur_size_;
    }

protected:

    //取出一个数据，根据参数确定是否等待一个相对时间
    int dequeue(_value_type &value_data,
                bool if_wait_timeout,
                const zce::Time_Value  &wait_time )
    {

        //进行超时等待
        if (if_wait_timeout)
        {
            bool bret = false;
            bret = sem_empty_.duration_lock(wait_time);

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
            ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
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
    size_t                                         queue_max_size_;

    //由于LIST的size()函数比较耗时，所以这儿还是用了个计数器，而不直接使用_container_type.size()
    size_t                                         queue_cur_size_;

    //队列的LOCK,用于读写操作的同步控制
    ZCE_MT_SYNCH::MUTEX                            queue_lock_;

    //信号灯，满的信号灯
    ZCE_MT_SYNCH::SEMAPHORE                        sem_full_;

    //信号灯，空的信号灯，当数据
    ZCE_MT_SYNCH::SEMAPHORE                        sem_empty_;

    //容器类型，可以是list,dequeue,
    _container_type                                message_queue_;

};


/*!
* @brief      内部用LIST实现的消息队列，性能低,边界保护用的条件变量。但一开始占用内存不多
*             ZCE_Message_Queue_List <ZCE_MT_SYNCH,_value_type> ZCE_MT_SYNCH 参数特化
* @tparam     _value_type 消息队列保存的数据类型
* note        主要就是为了给你一些语法糖
*/
template <typename _value_type >
class ZCE_Message_Queue_List <ZCE_MT_SYNCH, _value_type>  : public ZCE_Message_Queue<ZCE_MT_SYNCH, _value_type, std::list<_value_type> >
{
public:
    //
    explicit ZCE_Message_Queue_List(size_t queue_max_size):
        ZCE_Message_Queue<ZCE_MT_SYNCH, _value_type, std::list<_value_type> >(queue_max_size)
    {
    }

    ~ZCE_Message_Queue_List()
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
class ZCE_Message_Queue_Deque <ZCE_MT_SYNCH, _value_type>  : public ZCE_Message_Queue<ZCE_MT_SYNCH, _value_type, std::deque<_value_type> >
{
public:
    //
    explicit ZCE_Message_Queue_Deque(size_t queue_max_size):
        ZCE_Message_Queue<ZCE_MT_SYNCH, _value_type, std::deque<_value_type> >(queue_max_size)
    {
    }

    ~ZCE_Message_Queue_Deque()
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
class ZCE_Message_Queue_Rings<ZCE_MT_SYNCH, _value_type>  : public ZCE_Message_Queue<ZCE_MT_SYNCH, _value_type, zce::lordrings<_value_type> >
{
public:
    //
    explicit ZCE_Message_Queue_Rings(size_t queue_max_size):
        ZCE_Message_Queue<ZCE_MT_SYNCH, _value_type, zce::lordrings<_value_type> >(queue_max_size)
    {
        ZCE_Message_Queue<ZCE_MT_SYNCH, _value_type, zce::lordrings<_value_type> >::message_queue_.resize(queue_max_size);
    }

    ~ZCE_Message_Queue_Rings()
    {
    }

};

#endif //#ifndef ZCE_LIB_THREAD_MESSAGE_QUEUE_SEMAPHORE_H_

