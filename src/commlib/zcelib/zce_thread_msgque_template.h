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
* @tparam     _zce_synch       ZCE_MT_SYNCH 或者 ZCE_NULL_SYNCH
* @tparam     _value_type      容器里面的数据类型
* @tparam     _container_type  QUEUE的类型，deque,list,ZCE_LIB::lordrings
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
    * @brief      QUEUE是否为NULL
    * @return     bool
    */
    inline bool empty();

    /*!
    * @brief      QUEUE是否为满
    * @return     bool
    */
    inline bool full();


    /*!
    * @brief      放入数据,如果不能放入一直等待
    * @return     int
    * @param      value_data
    * @note
    */
    int enqueue(const _value_type &value_data);

    //放入一个数据，进行超时等待
    int enqueue(const _value_type &value_data,
                const ZCE_Time_Value & );

    //尝试放入数据到队列，立即返回
    int try_enqueue(_value_type &value_data);

    //取出数码，如果无法取出一直等待
    int dequeue(_value_type &value_data);

    //取出一个数据，进行超时等待
    int dequeue(_value_type &value_data,
                const ZCE_Time_Value & );

    //尝试取出数据到队列，立即返回
    int try_dequeue(_value_type &value_data);

    //清理消息队列
    void clear();

    //返回消息对象的尺寸
    size_t size();

protected:

    //QUEUE的最大尺寸
    size_t                                queue_max_size_;

    //由于LIST的size()函数比较耗时，所以这儿还是用了个计数器，而不直接使用_container_type.size()
    size_t                                queue_cur_size_;

    //容器类型，可以是list,dequeue,
    _container_type                       message_queue_;

};


#endif //ZCE_LIB_THREAD_MESSAGE_QUEUE_TEMPLATE_H_

