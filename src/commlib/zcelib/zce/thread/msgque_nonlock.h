/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_thread_msgque_nonlock.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年10月8日
* @brief
*
*
* @details
*
*
*
* @note
*
*/

#pragma once

#include "zce/lock/synch_traits.h"
#include "zce/thread/msgque_template.h"

//使用偏特化得到一个ZCE_Message_Queue
template < typename _value_type,
           typename _container_type >
class ZCE_Message_Queue<ZCE_NULL_SYNCH, _value_type, _container_type> : public zce::NON_Copyable
{
protected:

    //    ZCE_NULL_SYNCH其实没有使用，因为没有更快

    //QUEUE的最大尺寸
    std::size_t                                    queue_max_size_;

    //由于LIST的size()函数比较耗时，所以这儿还是用了个计数器，而不直接使用_container_type.size()
    std::size_t                                    queue_cur_size_;

    //容器类型，可以是list,dequeue,
    _container_type                       message_queue_;

public:

    //
    ZCE_Message_Queue(size_t queue_max_size):
        queue_max_size_(queue_max_size),
        queue_cur_size_(0)
    {
    }

    ~ZCE_Message_Queue()
    {
    }

    //QUEUE是否为NULL
    inline bool empty()
    {
        if (queue_cur_size_ == 0)
        {
            return true;
        }

        return false;
    }

    //QUEUE是否为满
    inline bool full()
    {
        if (queue_cur_size_ == queue_max_size_)
        {
            return true;
        }

        return false;
    }

    //放入数据
    int enqueue(const _value_type &value_data)
    {
        if (queue_cur_size_ >= queue_max_size_)
        {
            return -1;
        }

        message_queue_.push_back(value_data);
        ++queue_cur_size_;

        return 0;
    }

    //放入一个数据，进行超时等待
    int enqueue(const _value_type &value_data,
                ZCE_Time_Value & )
    {
        return enqueue(value_data);
    }

    //
    int dequeue(_value_type &value_data)
    {
        if (queue_cur_size_ == 0)
        {
            return -1;
        }

        //
        value_data = *message_queue_.begin();
        message_queue_.pop_front();
        --queue_cur_size_;
        return 0;
    }

    int dequeue(_value_type &value_data,
                ZCE_Time_Value & )
    {
        return dequeue(value_data);
    }

    //清理消息队列
    void clear()
    {
        message_queue_.clear();
        queue_cur_size_ = 0;
    }

    //返回消息对象的尺寸
    size_t size()
    {
        return queue_cur_size_;
    }
};


