/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_thread_msgque_sema.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��6��17��
* @brief      ���źŵ�+����ʵ�ֵ���Ϣ���У������Ҹ�����˵�������źŵƺ����һЩ
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

#include "zce_lock_synch_traits.h"
#include "zce_thread_msgque_template.h"

#include "zce_boost_non_copyable.h"
#include "zce_lock_thread_mutex.h"
#include "zce_lock_synch_traits.h"
#include "zce_lock_thread_semaphore.h"


/*!
* @brief      ���źŵ�+����ʵ�ֵ���Ϣ���У������Ҹ�����˵�������źŵƺ����һЩ
*
* @tparam     _value_type      ��Ϣ���з������������
* @tparam     _container_type  ��Ϣ�����ڲ���������
*/
template < typename _value_type,
           typename _container_type >
class ZCE_Message_Queue<ZCE_MT_SYNCH, _value_type, _container_type> : public ZCE_NON_Copyable
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

    //QUEUE�Ƿ�ΪNULL
    inline bool empty()
    {
        ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);

        if (queue_cur_size_ == 0)
        {
            return true;
        }

        return false;
    }

    //QUEUE�Ƿ�Ϊ��
    inline bool full()
    {
        ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);

        if (queue_cur_size_ == queue_max_size_)
        {
            return true;
        }

        return false;
    }

    //�������ݣ�һֱ�ȴ�
    int enqueue(const _value_type &value_data)
    {
        sem_full_.lock();

        //ע����δ��������{}��������Ϊ������ȱ�֤����ȡ��
        {
            ZCE_Thread_Light_Mutex::LOCK_GUARD lock_guard(queue_lock_);

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }
        sem_empty_.unlock();

        return 0;
    }

    //����һ�����ݣ����г�ʱ�ȴ�
    int enqueue(const _value_type &value_data,
                const ZCE_Time_Value  &wait_time )
    {
        bool bret = false;
        bret = sem_full_.duration_lock(wait_time);

        //�����ʱ�ˣ�����false
        if (!bret)
        {
            return -1;
        }

        //ע����δ��������{}��������Ϊ������ȱ�֤����ȡ��
        {
            ZCE_Thread_Light_Mutex::LOCK_GUARD lock_guard(queue_lock_);

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }
        sem_empty_.unlock();

        return 0;
    }

    //���ŷ����µ����ݽ�����У����û�гɹ�����������
    int try_enqueue(const _value_type &value_data)
    {
        bool bret = false;
        bret = sem_full_.try_lock();

        //�����ʱ�ˣ�����false
        if (!bret)
        {
            errno = EWOULDBLOCK;
            return -1;
        }

        //ע����δ��������{}��������Ϊ������ȱ�֤����ȡ��
        {
            ZCE_Thread_Light_Mutex::LOCK_GUARD lock_guard(queue_lock_);

            message_queue_.push_back(value_data);
            ++queue_cur_size_;
        }
        sem_empty_.unlock();

        return 0;
    }

    //ȡ��һ�����ݣ����ݲ���ȷ���Ƿ�ȴ�һ�����ʱ��
    int dequeue(_value_type &value_data,
                const ZCE_Time_Value  &wait_time )
    {
        bool bret = false;
        bret = sem_empty_.duration_lock(wait_time);

        //�����ʱ�ˣ�����false
        if (!bret)
        {
            return -1;
        }

        //ע����δ��������{}��������Ϊ������ȱ�֤����ȡ��
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

    //ȡ��һ�����ݣ�һֱ�ȴ�
    int dequeue(_value_type &value_data)
    {
        sem_empty_.lock();

        //ע����δ��������{}��������Ϊ������ȱ�֤����ȡ��
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

    //ȡ��һ�����ݣ����ݲ���ȷ���Ƿ�ȴ�һ�����ʱ��
    int try_dequeue(_value_type &value_data)
    {
        bool bret = false;
        bret = sem_empty_.try_lock();

        //�����ʱ�ˣ�����false
        if (!bret)
        {
            errno = EWOULDBLOCK;
            return -1;
        }

        //ע����δ��������{}��������Ϊ������ȱ�֤����ȡ��
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

    //������Ϣ����
    void clear()
    {
        ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
        message_queue_.clear();
        queue_cur_size_ = 0;
    }

    //������Ϣ����ĳߴ�
    size_t size()
    {
        ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
        return queue_cur_size_;
    }

protected:

    //ȡ��һ�����ݣ����ݲ���ȷ���Ƿ�ȴ�һ�����ʱ��
    int dequeue(_value_type &value_data,
                bool if_wait_timeout,
                const ZCE_Time_Value  &wait_time )
    {

        //���г�ʱ�ȴ�
        if (if_wait_timeout)
        {
            bool bret = false;
            bret = sem_empty_.duration_lock(wait_time);

            //�����ʱ�ˣ�����false
            if (!bret)
            {
                return -1;
            }
        }
        else
        {
            sem_empty_.lock();
        }

        //ע����δ��������{}��������Ϊ������ȱ�֤����ȡ��
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

    //QUEUE�����ߴ�
    size_t                                         queue_max_size_;

    //����LIST��size()�����ȽϺ�ʱ����������������˸�������������ֱ��ʹ��_container_type.size()
    size_t                                         queue_cur_size_;

    //���е�LOCK,���ڶ�д������ͬ������
    ZCE_MT_SYNCH::MUTEX                            queue_lock_;

    //�źŵƣ������źŵ�
    ZCE_MT_SYNCH::SEMAPHORE                        sem_full_;

    //�źŵƣ��յ��źŵƣ�������
    ZCE_MT_SYNCH::SEMAPHORE                        sem_empty_;

    //�������ͣ�������list,dequeue,
    _container_type                                message_queue_;

};


/*!
* @brief      �ڲ���LISTʵ�ֵ���Ϣ���У����ܵ�,�߽籣���õ�������������һ��ʼռ���ڴ治��
*             ZCE_Message_Queue_List <ZCE_MT_SYNCH,_value_type> ZCE_MT_SYNCH �����ػ�
* @tparam     _value_type ��Ϣ���б������������
* note        ��Ҫ����Ϊ�˸���һЩ�﷨��
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
* @brief      �ڲ���DQUEUEʵ�ֵ���Ϣ���У����ܽϺ�,�߽籣���õ�����������
*             �߽籣�����źŵ�
* @tparam     _value_type ��Ϣ���б������������
* note       ��װ����Ҫ����Ϊ�˸���һЩ�﷨��
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
* @brief      �ڲ���circular_bufferʵ�ֵ���Ϣ���У����ܷǳ���,�߽籣�����źŵƣ�����Ϣ���У�
*             ���ռ�ȽϷ�
* @tparam     _value_type ��Ϣ���б������������
* note        �����װ����Ҫ��������Ϊ�˸����﷨�ǣ�������Ϊ�˼�������
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

