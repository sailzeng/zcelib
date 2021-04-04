/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_thread_msgque_condi.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��6��17��
* @brief      ���߳�ʹ�õ�Message Queue���ڲ���ͬ������ʹ������������
*             �������ʱ��Ҫʹ�ã�����������ϵ���ѽڵģ���ȫ��Ϊ�˲�����������д����
*             ͬʱ֧��list��deque,���Լ���һ������zce::lordrings ���ζ���
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
#include "zce_boost_lord_rings.h"
#include "zce_lock_thread_condi.h"



/*!
* @brief      ����������+����ʵ�ֵ���Ϣ���У������Ҹ�����˵�����������е�֣�װB������condi����Mutex��Ŀ���ǣ�
*
* @tparam     _value_type ��Ϣ���з������������
* @tparam     _container_type ��Ϣ�����ڲ���������
* note
*/
template <typename _value_type, typename _container_type = std::deque<_value_type> >
class ZCE_Message_Queue_Condi : public ZCE_NON_Copyable
{

protected:

    enum MQW_WAIT_MODEL
    {
        MQW_NO_WAIT,
        MQW_WAIT_FOREVER,
        MQW_WAIT_TIMEOUT,
    };

public:

    //���캯������������
    ZCE_Message_Queue_Condi(size_t queue_max_size):
        queue_max_size_(queue_max_size),
        queue_cur_size_(0)
    {
    }

    ~ZCE_Message_Queue_Condi()
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



    //���룬һֱ�ȴ�
    int enqueue(const _value_type &value_data)
    {
        ZCE_Time_Value  nouse_timeout;
        return enqueue_interior(value_data,
                                MQW_WAIT_FOREVER,
                                nouse_timeout);
    }

    //�г�ʱ����
    int enqueue(const _value_type &value_data,
                const ZCE_Time_Value  &wait_time)
    {
        return enqueue_interior(value_data,
                                MQW_WAIT_FOREVER,
                                wait_time);
    }

    //���Է��룬��������
    int try_enqueue(const _value_type &value_data)
    {
        ZCE_Time_Value  nouse_timeout;
        return enqueue_interior(value_data,
                                MQW_WAIT_FOREVER,
                                nouse_timeout);
    }

    //ȡ��
    int dequeue(_value_type &value_data)
    {
        ZCE_Time_Value  nouse_timeout;
        return dequeue_interior(value_data,
                                MQW_WAIT_TIMEOUT,
                                nouse_timeout);
    }

    //�г�ʱ�����ȡ��
    int dequeue(_value_type &value_data,
                const ZCE_Time_Value  &wait_time)
    {
        return dequeue_interior(value_data,
                                MQW_WAIT_TIMEOUT,
                                wait_time);
    }

    //����ȡ������������
    int try_dequeue(_value_type &value_data)
    {
        ZCE_Time_Value  nouse_timeout;
        return dequeue_interior(value_data,
                                MQW_WAIT_TIMEOUT,
                                nouse_timeout);
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

    //����һ�����ݣ����ݲ���ȷ���Ƿ�ȴ�һ�����ʱ��
    int enqueue_interior(const _value_type &value_data,
                         MQW_WAIT_MODEL wait_model,
                         const timeval &wait_time)
    {
        //ע����δ��������{}��������Ϊ������ȱ�֤���ݷ��룬�ٴ���������
        //������������ʵ�ڲ��ǽ⿪�˱�����
        {
            ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
            bool bret = false;

            //cond�������Ƿǳ�����ģ�����ģ�����ط�������while������������
            //��ϸ��pthread_condi��˵����
            while (queue_cur_size_ == queue_max_size_)
            {
                if (wait_model == MQW_WAIT_TIMEOUT)
                {
                    //timed_wait�����������Ŀ����Ϊ�˽⿪���˳���ʱ����ϣ������Ǽ�����
                    //���Ժ���ܺ���,WINDOWS�µ�ʵ��Ӧ�������źŵ�ģ���
                    bret = cond_enqueue_.duration_wait(&queue_lock_, wait_time);

                    //�����ʱ�ˣ�����false
                    if (!bret)
                    {
                        zce::last_error(ETIMEDOUT);
                        return -1;
                    }
                }
                else if (wait_model == MQW_WAIT_FOREVER)
                {
                    cond_enqueue_.wait(&queue_lock_);
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

        //֪ͨ���еȴ�����
        cond_dequeue_.broadcast();

        return 0;
    }

    //ȡ��һ�����ݣ����ݲ���ȷ���Ƿ�ȴ�һ�����ʱ��
    int dequeue_interior(_value_type &value_data,
                         MQW_WAIT_MODEL wait_model,
                         const ZCE_Time_Value  &wait_time)
    {
        //ע����δ��������{}��������Ϊ������ȱ�֤����ȡ��
        {
            ZCE_Thread_Light_Mutex::LOCK_GUARD guard(queue_lock_);
            bool bret = false;

            //cond�������Ƿǳ�����ģ�����ģ�����ط�������while��
            //��ϸ��pthread_condi��˵����
            while (queue_cur_size_ == 0)
            {
                //�ж��Ƿ�Ҫ���г�ʱ�ȴ�
                if (wait_model == MQW_WAIT_TIMEOUT)
                {
                    //timed_wait�����������Ŀ����Ϊ�˽⿪���˳���ʱ����ϣ������Ǽ�����
                    //���Ժ���ܺ���
                    bret = cond_dequeue_.duration_wait(&queue_lock_, wait_time);

                    //�����ʱ�ˣ�����false
                    if (!bret)
                    {
                        zce::last_error(ETIMEDOUT);
                        return -1;
                    }
                }
                else if (wait_model == MQW_WAIT_FOREVER)
                {
                    cond_dequeue_.wait(&queue_lock_);
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

        //֪ͨ���еȴ�����
        cond_enqueue_.broadcast();

        return 0;
    }



protected:

    //QUEUE�����ߴ�
    std::size_t                  queue_max_size_;

    //����LIST��size()�����ȽϺ�ʱ����������������˼���������
    std::size_t                  queue_cur_size_;

    //���е�LOCK,���ڶ�д������ͬ������
    ZCE_Thread_Light_Mutex       queue_lock_;

    //���뱣������������
    ZCE_Thread_Condition_Mutex   cond_enqueue_;

    //ȡ�����б�������������
    ZCE_Thread_Condition_Mutex   cond_dequeue_;

    //�������ͣ�������list,dequeue,
    _container_type              message_queue_;
};


/*!
* @brief      �ڲ���LISTʵ�ֵ���Ϣ���У����ܵ�,�߽籣���õ�������������һ��ʼռ���ڴ治��
*
* @tparam     _value_type ��Ϣ���б������������
* note        ��Ҫ����Ϊ�˸���һЩ�﷨��
*/
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


/*!
* @brief      �ڲ���DQUEUEʵ�ֵ���Ϣ���У����ܽϺ�,�߽籣���õ�����������
*
* @tparam     _value_type ��Ϣ���б������������
* note
*/
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


/*!
* @brief      �ڲ���circular_bufferʵ�ֵ���Ϣ���У����ܷǳ���,�߽籣���õ�����������
*
* @tparam     _value_type ��Ϣ���б������������
* note       ��װ����Ҫ��������Ϊ�˸����﷨�ǣ�������Ϊ�˼�������
*/
template <class _value_type >
class ZCE_Msgqueue_Rings_Condi : public ZCE_Message_Queue_Condi<_value_type, zce::lordrings<_value_type> >
{
public:
    //
    explicit ZCE_Msgqueue_Rings_Condi(size_t queue_max_size) :
        ZCE_Message_Queue_Condi<_value_type, zce::lordrings<_value_type> >(queue_max_size)
    {
        ZCE_Message_Queue_Condi<_value_type, zce::lordrings<_value_type> >::message_queue_.resize(queue_max_size);
    }

    ~ZCE_Msgqueue_Rings_Condi()
    {
    }
};

#endif //#ifndef ZCE_LIB_THREAD_MESSAGE_QUEUE_CONDITION_H_

