/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_thread_msgque_condi.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version    
* @date       2011年6月17日
* @brief      多线程使用的Message Queue，内部的同步对象使用了条件变量
*             这个类暂时不要使用，他和整个体系是脱节的，完全是为了测试条件变量写的类
*             同时支持list，deque,和自己的一个容器ZCE_LIB::lordrings 环形队列
* @details    
*             
*             
*             
* @note       
*             
*/


#ifndef ZCE_LIB_THREAD_MESSAGE_QUEUE_CONDITION_H_
#define ZCE_LIB_THREAD_MESSAGE_QUEUE_CONDITION_H_

#include "zce_boost_non_copyable.h"
#include "zce_lock_thread_mutex.h"
#include "zce_lock_thread_condi.h"

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011年6月17日
Template Param  :
  Param1: class _value_type 消息队列放入的数据类型
  Param2: class _container_type =std::deque<_value_type> 消息队列内部容器类型
Class           : ZCE_Message_Queue_Condi
Inherit         : public boost::noncopyable 不能复制拷贝
Description     : 用条件变量+容器实现的消息队列，对于我个人来说，条件变量有点怪，装B？请问condi传入Mutex的目的是？
Other           :
Modify Record   :
************************************************************************************************************/
template <typename _value_type, typename _container_type = std::deque<_value_type> >
class ZCE_Message_Queue_Condi : public ZCE_NON_Copyable
{
protected:

    //QUEUE的最大尺寸
    std::size_t                  queue_max_size_;

    //由于LIST的size()函数比较耗时，所以这儿还是用了几个计数器
    std::size_t                  queue_cur_size_;

    //队列的LOCK,用于读写操作的同步控制
    ZCE_Thread_Light_Mutex       queue_lock_;

    //插入保护的条件变量
    ZCE_Thread_Condition_Mutex   cond_enqueue_;

    //取出进行保护的条件变量
    ZCE_Thread_Condition_Mutex   cond_dequeue_;

    //容器类型，可以是list,dequeue,
    _container_type              message_queue_;

public:

    //构造函数和析构函数
    ZCE_Message_Queue_Condi(size_t queue_max_size):
        queue_max_size_(queue_max_size),
        queue_cur_size_(0)
    {
    }

    ~ZCE_Message_Queue_Condi()
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

    //放入
    int enqueue(const _value_type &value_data)
    {
        ZCE_Time_Value  nouse_timeout;
        return enqueue(value_data,
                       false,
                       nouse_timeout);
    }

    //有超时放入
    int enqueue(const _value_type &value_data,
                const ZCE_Time_Value  &wait_time)
    {
        return enqueue(value_data,
                       true,
                       wait_time);
    }

    //取出
    int dequeue(_value_type &value_data)
    {
        ZCE_Time_Value  nouse_timeout;
        return dequeue(value_data,
                       false,
                       nouse_timeout);
    }

    //有超时处理的取出
    int dequeue(_value_type &value_data,
                const ZCE_Time_Value  &wait_time)
    {
        return dequeue(value_data,
                       true,
                       wait_time);
    }

    void clear()
    {
        ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
        message_queue_.clear();
        queue_cur_size_ = 0;
    }

    size_t size()
    {
        ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
        return queue_cur_size_;
    }

protected:

    //放入一个数据，根据参数确定是否等待一个相对时间
    int enqueue(const _value_type &value_data,
                bool if_wait_timeout,
                const timeval &wait_time)
    {
        //注意这段代码必须用{}保护，因为你必须先保证数据放入，再触发条件，
        //而条件触发其实内部是解开了保护的
        {
            ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
            bool bret = false;

            //cond的语意是非常含混的，讨厌的，这个地方必须用while，必须重入检查
            //详细见pthread_condi的说明，
            while (queue_cur_size_ == queue_max_size_)
            {
                if (if_wait_timeout)
                {
                    //timed_wait里面放入锁的目的是为了解开（退出的时候加上），不是加锁，
                    //所以含义很含混,WINDOWS下的实现应该是用信号灯模拟的
                    bret = cond_enqueue_.duration_wait(&queue_lock_, wait_time);

                    //如果超时了，返回false
                    if (!bret)
                    {
                        return -1;
                    }
                }
                else
                {
                    cond_enqueue_.wait(&queue_lock_);
                }

            }

            message_queue_.push_back(value_data);
            ++queue_cur_size_;

        }

        //通知所有等待的人
        cond_dequeue_.broadcast();

        return 0;
    }

    //取出一个数据，根据参数确定是否等待一个相对时间
    int dequeue(_value_type &value_data,
                bool if_wait_timeout,
                const ZCE_Time_Value  &wait_time)
    {
        //注意这段代码必须用{}保护，因为你必须先保证数据取出
        {
            ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
            bool bret = false;

            //cond的语意是非常含混的，讨厌的，这个地方必须用while，
            //详细见pthread_condi的说明，
            while (queue_cur_size_ == 0)
            {
                //判断是否要进行超时等待
                if (if_wait_timeout)
                {
                    //timed_wait里面放入锁的目的是为了解开（退出的时候加上），不是加锁，
                    //所以含义很含混
                    bret = cond_dequeue_.duration_wait(&queue_lock_, wait_time);

                    //如果超时了，返回false
                    if (!bret)
                    {
                        return -1;
                    }
                }
                else
                {
                    cond_dequeue_.wait(&queue_lock_);
                }
            }

            //
            value_data = *message_queue_.begin();
            message_queue_.pop_front();
            --queue_cur_size_;
        }

        //通知所有等待的人
        cond_enqueue_.broadcast();

        return 0;
    }
};

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011年6月17日
Template Param  :
  Param1: class _value_type 消息队列保存的数据类型
Class           : ZCE_Msgqueue_List_Condi
Inherit         : public ZCE_Message_Queue_Condi<_value_type,std::list>
Description     : 内部用LIST实现的消息队列，性能低,边界保护用的条件变量。但一开始占用内存不多
Other           : 主要就是为了给你一些语法糖
Modify Record   :
************************************************************************************************************/
template <typename _value_type >
class ZCE_Msgqueue_List_Condi : public ZCE_Message_Queue_Condi<_value_type, std::list<_value_type> >
{
public:
    //
    explicit ZCE_Msgqueue_List_Condi(size_t queue_max_size):
        ZCE_Message_Queue_Condi<_value_type, std::list<_value_type> >(queue_max_size)
    {
    }

    ~ZCE_Msgqueue_List_Condi()
    {
    }
};

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011年6月17日
Template Param  :
  Param1: class _value_type 消息队列保存的数据类型
Class           : ZCE_Msgqueue_Deque_Condi
Inherit         : public ZCE_Message_Queue_Condi<_value_type,std::deque<_value_type> >
Description     : 内部用DQUEUE实现的消息队列，性能较好,边界保护用的条件变量。
Other           : 封装的主要就是为了给你一些语法糖
Modify Record   :
************************************************************************************************************/
template <class _value_type >
class ZCE_Msgqueue_Deque_Condi : public ZCE_Message_Queue_Condi<_value_type, std::deque<_value_type> >
{
public:
    //
    explicit ZCE_Msgqueue_Deque_Condi(size_t queue_max_size):
        ZCE_Message_Queue_Condi<_value_type, std::deque<_value_type> >(queue_max_size)
    {
    }

    ~ZCE_Msgqueue_Deque_Condi()
    {
    }
};

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011年6月17日
Template Param  :
  Param1: class _value_type 消息队列保存的数据类型
Class           : zce_condi_msgqueue_ring
Inherit         : public ZCE_Message_Queue_Condi<_value_type,boost::circular_buffer<_value_type> >
Description     : 内部用circular_buffer实现的消息队列，性能非常好,边界保护用的条件变量。
Other           : 封装的主要不光是了为了给你语法糖，而且是为了极限性能
Modify Record   :
************************************************************************************************************/
//template <class _value_type >
//class zce_condi_msgqueue_ring : public ZCE_Message_Queue_Condi<_value_type,boost::circular_buffer<_value_type> >
//{
//    //
//    zce_condi_msgqueue_ring(size_t queue_max_size):
//        queue_max_size_(queue_max_size),
//        queue_cur_size_(0)
//    {
//        message_queue_.resize(queue_max_size);
//    }
//
//    ~zce_condi_msgqueue_ring()
//    {
//    }
//};

#endif //#ifndef ZCE_LIB_THREAD_MESSAGE_QUEUE_CONDITION_H_

