/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_thread_msgque_nonlock.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version    
* @date       Saturday, December 28, 2013
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

#ifndef ZCE_LIB_THREAD_MESSAGE_QUEUE_NONLOCK_H_
#define ZCE_LIB_THREAD_MESSAGE_QUEUE_NONLOCK_H_

#include "zce_lock_synch_traits.h"
#include "zce_thread_msgque_template.h"

//使用偏特化得到一个ZCE_Message_Queue
template < typename _value_type,
         typename _container_type >
class ZCELIB_EXPORT ZCE_Message_Queue<ZCE_NULL_SYNCH, _value_type, _container_type> : public ZCE_NON_Copyable
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

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011年10月8日
Template Param  :
  Param1: typename _value_type
Class           : ZCE_Message_Queue_List <ZCE_NULL_SYNCH,_value_type>
Inherit         : public ZCE_Message_Queue<ZCE_NULL_SYNCH,_value_type,std::list<_value_type> >
Description     : 用LIST作为容器的
Other           :
Modify Record   :
************************************************************************************************************/
template <typename _value_type >
class ZCE_Message_Queue_List <ZCE_NULL_SYNCH, _value_type>  : public ZCE_Message_Queue<ZCE_NULL_SYNCH, _value_type, std::list<_value_type> >
{
public:
    //
    ZCE_Message_Queue_List(size_t queue_max_size):
        ZCE_Message_Queue<ZCE_NULL_SYNCH, _value_type, std::list<_value_type> >(queue_max_size)
    {
    }

    ~ZCE_Message_Queue_List()
    {
    }
};

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011年10月8日
Template Param  :
  Param1: typename _value_type
Class           : ZCE_Message_Queue_Deque <ZCE_NULL_SYNCH,_value_type>
Inherit         : public ZCE_Message_Queue<ZCE_NULL_SYNCH,_value_type,std::deque<_value_type> >
Description     :
Other           :
Modify Record   :
************************************************************************************************************/
template <typename _value_type >
class ZCE_Message_Queue_Deque <ZCE_NULL_SYNCH, _value_type>: public ZCE_Message_Queue<ZCE_NULL_SYNCH, _value_type, std::deque<_value_type> >
{
public:
    //
    ZCE_Message_Queue_Deque(size_t queue_max_size):
        ZCE_Message_Queue<ZCE_NULL_SYNCH, _value_type, std::deque<_value_type> >(queue_max_size)
    {
    }

    ~ZCE_Message_Queue_Deque()
    {
    }
};

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011年6月17日
Template Param  :
  Param1: typename _value_type 消息队列保存的数据类型
Class           : zce_condi_msgqueue_ring
Inherit         : public ZCE_Message_Queue_Semaphore<_value_type,boost::circular_buffer<_value_type> >
Description     : 内部用circular_buffer实现的消息队列，性能非常好,边界保护用信号灯，的消息队列，但空间比较费
Other           : 这个封装的主要不光是了为了给你语法糖，而且是为了极限性能
Modify Record   :
************************************************************************************************************/
template <typename _value_type >
class ZCE_Message_Queue_Rings<ZCE_NULL_SYNCH, _value_type>  : public ZCE_Message_Queue<ZCE_NULL_SYNCH, _value_type, ZCE_LIB::lordrings<_value_type> >
{
public:
    //
    ZCE_Message_Queue_Rings(size_t queue_max_size):
        ZCE_Message_Queue<ZCE_NULL_SYNCH, _value_type, ZCE_LIB::lordrings<_value_type> >(queue_max_size)
    {
        ZCE_Message_Queue<ZCE_NULL_SYNCH, _value_type, ZCE_LIB::lordrings<_value_type> >::message_queue_.resize(queue_max_size);
    }

    ~ZCE_Message_Queue_Rings()
    {
    }
};

#endif // ZCE_LIB_THREAD_MESSAGE_QUEUE_NONLOCK_H_

