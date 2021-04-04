/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_thread_msgque_template.h
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
#ifndef ZCE_LIB_THREAD_MESSAGE_QUEUE_TEMPLATE_H_
#define ZCE_LIB_THREAD_MESSAGE_QUEUE_TEMPLATE_H_

#include "zce_boost_lord_rings.h"



/*!
* @brief
*
* @tparam     _zce_synch       ZCE_MT_SYNCH ���� ZCE_NULL_SYNCH
* @tparam     _value_type      �����������������
* @tparam     _container_type  QUEUE�����ͣ�deque,list,zce::lordrings
* @note
*/
template < typename _zce_synch,
           typename _value_type,
           typename _container_type = std::deque<_value_type> >
class ZCE_Message_Queue : public ZCE_NON_Copyable
{


public:

    //
    ZCE_Message_Queue(size_t queue_max_size);
    ~ZCE_Message_Queue();

    /*!
    * @brief      QUEUE�Ƿ�ΪNULL
    * @return     bool
    */
    inline bool empty();

    /*!
    * @brief      QUEUE�Ƿ�Ϊ��
    * @return     bool
    */
    inline bool full();


    /*!
    * @brief      ��������,������ܷ���һֱ�ȴ�
    * @return     int
    * @param      value_data
    * @note
    */
    int enqueue(const _value_type &value_data);

    //����һ�����ݣ����г�ʱ�ȴ�
    int enqueue(const _value_type &value_data,
                const ZCE_Time_Value & );

    //���Է������ݵ����У���������
    int try_enqueue(_value_type &value_data);

    //ȡ�����룬����޷�ȡ��һֱ�ȴ�
    int dequeue(_value_type &value_data);

    //ȡ��һ�����ݣ����г�ʱ�ȴ�
    int dequeue(_value_type &value_data,
                const ZCE_Time_Value & );

    //����ȡ�����ݵ����У���������
    int try_dequeue(_value_type &value_data);

    //������Ϣ����
    void clear();

    //������Ϣ����ĳߴ�
    size_t size();

protected:

    //QUEUE�����ߴ�
    size_t                                queue_max_size_;

    //����LIST��size()�����ȽϺ�ʱ����������������˸�������������ֱ��ʹ��_container_type.size()
    size_t                                queue_cur_size_;

    //�������ͣ�������list,dequeue,
    _container_type                       message_queue_;

};


template < typename _zce_synch,
           typename _value_type >
class ZCE_Message_Queue_List : public ZCE_Message_Queue<_zce_synch, _value_type, std::list<_value_type> >
{
public:
    //
    ZCE_Message_Queue_List(size_t queue_max_size) :
        ZCE_Message_Queue<_zce_synch, _value_type, std::list<_value_type> >(queue_max_size)
    {
    }

    ~ZCE_Message_Queue_List()
    {
    }
};

template < typename _zce_synch,
           typename _value_type >
class ZCE_Message_Queue_Deque : public ZCE_Message_Queue<_zce_synch, _value_type, std::deque<_value_type> >
{
public:
    //
    ZCE_Message_Queue_Deque(size_t queue_max_size) :
        ZCE_Message_Queue<_zce_synch, _value_type, std::deque<_value_type> >(queue_max_size)
    {
    }

    ZCE_Message_Queue_Deque() :
        ZCE_Message_Queue<_zce_synch, _value_type, std::deque<_value_type> >(1024)
    {
    }

    ~ZCE_Message_Queue_Deque()
    {
    }
};

template < typename _zce_synch,
           typename _value_type >
class ZCE_Message_Queue_Rings : public ZCE_Message_Queue<_zce_synch, _value_type, zce::lordrings<_value_type> >
{
public:
    //
    ZCE_Message_Queue_Rings(size_t queue_max_size) :
        ZCE_Message_Queue<_zce_synch, _value_type, zce::lordrings<_value_type> >(queue_max_size)
    {
    }
    ~ZCE_Message_Queue_Rings()
    {
    }
};


#endif //ZCE_LIB_THREAD_MESSAGE_QUEUE_TEMPLATE_H_

