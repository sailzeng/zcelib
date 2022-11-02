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
* @note      原来代码这儿我用了偏特化进行了炫技，为了兼容一个无锁的模式，后面
*            整理代码发现我有点太作了。删除了。保留下来其实算是一个挺好的偏特
*            化的例子的，但还是算了。
*/

#pragma once

#include "zce/predefine.h"
#include "zce/lock/synch_traits.h"
#include "zce/util/non_copyable.h"
#include "zce/lock/thread_mutex.h"
#include "zce/lock/synch_traits.h"
#include "zce/lock/thread_semaphore.h"
#include "zce/thread/msgque_waitmodel.h"

namespace zce
{
/*!
* @brief      用C++20信号灯+容器实现的消息队列，对于我个人来说，还是信号灯好理解一些
*             有C++20的情况下，推荐这个类
* @tparam     T  消息队列放入的数据类型
* @tparam     C  消息队列内部容器类型
*/
template <typename T, typename C >
class msgqueue_sema
{
public:

    //
    explicit msgqueue_sema(size_t queue_max_size) :
        queue_max_size_(queue_max_size),
        queue_cur_size_(0),
        sem_full_(queue_max_size),
        sem_empty_(0)
    {
    }

    ~msgqueue_sema() = default;

    //QUEUE是否为nullptr
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

    //放入数据，一直等待
    bool enqueue(const T& value_data)
    {
        std::chrono::microseconds no_use;
        return enqueue_i(value_data,
                         MQW_WAIT_FOREVER,
                         no_use);
    }

    //放入一个数据，进行超时等待
    template<class Rep, class Period>
    bool enqueue_wait(const T& value_data,
                      const std::chrono::duration<Rep, Period>& wait_time)
    {
        return enqueue_i(value_data,
                         MQW_WAIT_TIMEOUT,
                         wait_time);
    }
    bool enqueue_wait(const T& value_data,
                      const zce::time_value& wait_time)
    {
        std::chrono::microseconds wait_mircosec;
        wait_time.to(wait_mircosec);
        return enqueue_i(value_data,
                         MQW_WAIT_TIMEOUT,
                         wait_mircosec);
    }
    //试着放入新的数据进入队列，如果没有成功，立即返回
    bool try_enqueue(const T& value_data)
    {
        std::chrono::microseconds no_use;
        return enqueue_i(value_data,
                         MQW_NO_WAIT,
                         no_use);
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
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
    //取出一个数据，一直等待
    bool dequeue(T& value_data)
    {
        std::chrono::microseconds no_use;
        return dequeue_i(value_data,
                         MQW_WAIT_FOREVER,
                         no_use);
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    bool try_dequeue(T& value_data)
    {
        std::chrono::microseconds no_use;
        return dequeue_i(value_data,
                         MQW_NO_WAIT,
                         no_use);
    }

    //清理消息队列
    void clear()
    {
        std::lock_guard<std::mutex> guard(queue_lock_);
        message_queue_.clear();
        queue_cur_size_ = 0;
    }

    //返回消息对象的尺寸
    size_t size()
    {
        std::lock_guard<std::mutex> guard(queue_lock_);
        return queue_cur_size_;
    }

protected:

    //取出一个数据，根据参数确定是否等待一个相对时间
    template<class Rep, class Period>
    bool dequeue_i(T& value_data,
                   MQW_WAIT_MODEL model,
                   const std::chrono::duration<Rep, Period>& wait_time)
    {
        //进行超时等待
        bool bret = false;
        if (model == MQW_WAIT_MODEL::MQW_WAIT_TIMEOUT)
        {
            bret = sem_empty_.try_acquire_for(wait_time);
            //如果超时了，返回false
            if (!bret)
            {
                return false;
            }
        }
        else if (model == MQW_WAIT_MODEL::MQW_WAIT_FOREVER)
        {
            sem_empty_.acquire();
        }
        else if (model == MQW_WAIT_MODEL::MQW_NO_WAIT)
        {
            bret = sem_empty_.try_acquire();
            if (!bret)
            {
                return false;
            }
        }
        else
        {
        }

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            std::lock_guard<std::mutex> guard(queue_lock_);
            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }
        sem_full_.release();

        return true;
    }

    template<class Rep, class Period>
    bool enqueue_i(const T& value_data,
                   MQW_WAIT_MODEL model,
                   const std::chrono::duration<Rep, Period>& wait_time)
    {
        bool bret = false;
        if (model == MQW_WAIT_MODEL::MQW_WAIT_TIMEOUT)
        {
            bret = sem_full_.try_acquire_for(wait_time);
            //如果超时了，返回false
            if (!bret)
            {
                return false;
            }
        }
        else if (model == MQW_WAIT_MODEL::MQW_WAIT_FOREVER)
        {
            sem_full_.acquire();
        }
        else if (model == MQW_WAIT_MODEL::MQW_NO_WAIT)
        {
            bret = sem_full_.try_acquire();
            //如果超时了，返回false
            if (!bret)
            {
                errno = EWOULDBLOCK;
                return false;
            }
        }
        else
        {
        }

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            std::lock_guard<std::mutex> lock_guard(queue_lock_);

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }
        sem_empty_.release();

        return true;
    }

protected:
    //!
    size_t                  queue_max_size_;
    //!由于LIST的size()函数比较耗时，所以这儿还是用了个计数器，而不直接使用_container_type.size()
    size_t                  queue_cur_size_;

    //队列的LOCK,用于读写操作的同步控制
    std::mutex              queue_lock_;

    //信号灯，满的信号灯
    std::counting_semaphore<1024>   sem_full_;

    //信号灯，空的信号灯，当数据
    std::counting_semaphore<1024>   sem_empty_;

    //容器类型，可以是list,dequeue,
    C                       message_queue_;
};

/*!
* @brief      内部用LIST实现的消息队列，性能低,边界保护用的条件变量。但一开始占用内存不多
*             zce::MsgQueue_List <MT_SYNCH,_value_type> ZCE_MT_SYNCH 参数特化
* @tparam     T 消息队列保存的数据类型
* note        主要就是为了给你一些语法糖
*/
template <typename T >
class msglist_sema : public msgqueue_sema<T, std::list<T> >
{
public:
    msglist_sema(size_t queue_max_size) :
        msgqueue_sema<T, std::list<T> >(queue_max_size)
    {
    }
    ~msglist_sema() = default;
};

template <typename T >
class msgdeque_sema : public msgqueue_sema<T, std::deque<T> >
{
public:
    msgdeque_sema(size_t queue_max_size) :
        msgqueue_sema<T, std::deque<T> >(queue_max_size)
    {
    }
    ~msgdeque_sema() = default;
};

template <typename T >
class msgrings_sema : public msgqueue_sema<T, zce::lord_rings<T> >
{
public:
    msgrings_sema(size_t queue_max_size) :
        msgqueue_sema<T, zce::lord_rings<T> >(queue_max_size)
    {
        msgqueue_sema<T, zce::lord_rings<T> >::message_queue_.resize(queue_max_size);
    }
    ~msgrings_sema() = default;
};

//===============================================================================================

/*!
* @brief      用信号灯+容器实现的消息队列，对于我个人来说，还是信号灯好理解一些
*
* @tparam     T  消息队列放入的数据类型
* @tparam     C  消息队列内部容器类型
*/
template < typename T,
    typename C >
class MsgQueue_Sema : public zce::non_copyable
{
public:

    //
    explicit MsgQueue_Sema(size_t queue_max_size) :
        queue_max_size_(queue_max_size),
        queue_cur_size_(0),
        sem_full_(static_cast<unsigned int>(queue_max_size)),
        sem_empty_(0)
    {
    }

    ~MsgQueue_Sema() = default;

    //QUEUE是否为nullptr
    inline bool empty()
    {
        thread_light_mutex::LOCK_GUARD guard(queue_lock_);

        if (queue_cur_size_ == 0)
        {
            return true;
        }

        return false;
    }

    //QUEUE是否为满
    inline bool full()
    {
        thread_light_mutex::LOCK_GUARD guard(queue_lock_);
        if (queue_cur_size_ == queue_max_size_)
        {
            return true;
        }

        return false;
    }

    //放入数据，一直等待
    bool enqueue(const T& value_data)
    {
        zce::time_value no_use;
        return enqueue_i(value_data,
                         MQW_WAIT_MODEL::MQW_WAIT_FOREVER,
                         no_use);
    }

    //放入一个数据，进行超时等待
    bool enqueue(const T& value_data,
                 const zce::time_value& wait_time)
    {
        return enqueue_i(value_data,
                         MQW_WAIT_MODEL::MQW_WAIT_TIMEOUT,
                         wait_time);
    }

    //试着放入新的数据进入队列，如果没有成功，立即返回
    bool try_enqueue(const T& value_data)
    {
        zce::time_value no_use;
        return enqueue_i(value_data,
                         MQW_WAIT_MODEL::MQW_NO_WAIT,
                         no_use);
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    bool dequeue_wait(T& value_data,
                      const zce::time_value& wait_time)
    {
        return dequeue_i(value_data,
                         MQW_WAIT_MODEL::MQW_WAIT_TIMEOUT,
                         wait_time);
    }

    //取出一个数据，一直等待
    bool dequeue(T& value_data)
    {
        zce::time_value no_use;
        return dequeue_i(value_data,
                         MQW_WAIT_MODEL::MQW_WAIT_FOREVER,
                         no_use);
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    bool try_dequeue(T& value_data)
    {
        zce::time_value no_use;
        return dequeue_i(value_data,
                         MQW_WAIT_MODEL::MQW_NO_WAIT,
                         no_use);
    }

    //清理消息队列
    void clear()
    {
        thread_light_mutex::LOCK_GUARD guard(queue_lock_);
        message_queue_.clear();
        queue_cur_size_ = 0;
    }

    //返回消息对象的尺寸
    size_t size()
    {
        thread_light_mutex::LOCK_GUARD guard(queue_lock_);
        return queue_cur_size_;
    }

protected:

    bool enqueue_i(const T& value_data,
                   MQW_WAIT_MODEL model,
                   const zce::time_value& wait_time)
    {
        bool bret = false;
        if (model == MQW_WAIT_MODEL::MQW_WAIT_TIMEOUT)
        {
            bret = sem_full_.try_acquire_for(wait_time);
            //如果超时了，返回false
            if (!bret)
            {
                return false;
            }
        }
        else if (model == MQW_WAIT_MODEL::MQW_NO_WAIT)
        {
            bret = sem_full_.try_acquire();
            //如果超时了，返回false
            if (!bret)
            {
                return false;
            }
        }
        else if (model == MQW_WAIT_MODEL::MQW_WAIT_FOREVER)
        {
            sem_full_.acquire();
        }
        else
        {
        }

        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            thread_light_mutex::LOCK_GUARD guard(queue_lock_);
            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }
        sem_empty_.release();

        return true;
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    bool dequeue_i(T& value_data,
                   MQW_WAIT_MODEL model,
                   const zce::time_value& wait_time)
    {
        //进行超时等待
        if (model == MQW_WAIT_MODEL::MQW_WAIT_TIMEOUT)
        {
            bool bret = false;
            bret = sem_empty_.try_acquire_for(wait_time);
            //如果超时了，返回false
            if (!bret)
            {
                return false;
            }
        }
        else if (model == MQW_WAIT_MODEL::MQW_NO_WAIT)
        {
            bool bret = sem_empty_.try_acquire();
            //如果超时了，返回false
            if (!bret)
            {
                return false;
            }
        }
        else  if (model == MQW_WAIT_MODEL::MQW_WAIT_FOREVER)
        {
            sem_empty_.acquire();
        }
        else
        {
        }
        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            thread_light_mutex::LOCK_GUARD guard(queue_lock_);
            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }
        sem_full_.release();

        return true;
    }

protected:

    //QUEUE的最大尺寸
    size_t                           queue_max_size_;

    //由于LIST的size()函数比较耗时，所以这儿还是用了个计数器，而不直接使用_container_type.size()
    size_t                           queue_cur_size_;

    //队列的LOCK,用于读写操作的同步控制
    zce::thread_light_mutex          queue_lock_;

    //信号灯，满的信号灯
    zce::thread_semaphore            sem_full_;

    //信号灯，空的信号灯，当数据
    zce::thread_semaphore            sem_empty_;

    //容器类型，可以是list,dequeue,
    C                                message_queue_;
};

/*!
* @brief      内部用LIST实现的消息队列，性能低,边界保护用的条件变量。但一开始占用内存不多
*
* @tparam     T 消息队列保存的数据类型
* note        主要就是为了给你一些语法糖
*/
template <typename T >
class MsgList_Sema : public MsgQueue_Sema<T, std::list<T> >
{
public:
    explicit MsgList_Sema(size_t queue_max_size) :
        MsgQueue_Sema<T, std::list<T> >(queue_max_size)
    {
    }
    ~MsgList_Sema() = default;
};

template <typename T >
class MsgDeque_Sema : public MsgQueue_Sema<T, std::deque<T> >
{
public:
    explicit MsgDeque_Sema(size_t queue_max_size) :
        MsgQueue_Sema<T, std::deque<T> >(queue_max_size)
    {
    }
    ~MsgDeque_Sema() = default;
};

template <typename T >
class MsgRings_Sema : public MsgQueue_Sema<T, zce::lord_rings<T> >
{
public:
    explicit MsgRings_Sema(size_t queue_max_size) :
        MsgQueue_Sema<T, zce::lord_rings<T> >(queue_max_size)
    {
        MsgQueue_Sema<T, zce::lord_rings<T> >::message_queue_.resize(queue_max_size);
    }
    ~MsgRings_Sema() = default;
};
}
