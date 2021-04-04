/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_event_reactor_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��9��1��
* @brief      ��Ӧ���Ļ���
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_EVENT_REACTOR_BASE_H_
#define ZCE_LIB_EVENT_REACTOR_BASE_H_

#include "zce_boost_non_copyable.h"
#include "zce_log_logging.h"

class ZCE_Event_Handler;
class ZCE_Time_Value;

/*!
* @brief      ��Ӧ���Ļ���
*
*/
class ZCE_Reactor : public ZCE_NON_Copyable
{

protected:

    ///ʹ��hansh map��������ZCE_Event_Handler��MAP ��������ߵ�����
    typedef unordered_map<ZCE_HANDLE, ZCE_Event_Handler *>  MAP_OF_HANDLER_TO_EVENT;

protected:

    /*!
    * @brief      ���캯��
    */
    ZCE_Reactor();
    /*!
    * @brief      ���캯��
    * @param[in]  max_event_number ����������
    */
    ZCE_Reactor(size_t max_event_number);
    /*!
    * @brief      ����������virtual���
    */
    virtual ~ZCE_Reactor();

public:

    /*!
    * @brief      ��ǰ��Ӧ�������ľ������
    * @return     size_t ���صĵ�ǰ�ڷ�Ӧ���ľ������
    */
    size_t size();

    /*!
    * @brief      ��Ӧ�������ߴ磬������
    * @return     size_t ��������
    */
    size_t max_size();

    /*!
    * @brief      ��ʼ��
    * @return     int
    * @param      max_event_number
    * @note
    */
    int initialize(size_t max_event_number);

    /*!
    * @brief      �رշ�Ӧ����������ע���EVENT HANDLER ע����
    * @return     int
    */
    virtual int close();

    /*!
    * @brief      ע��һ��ZCE_Event_Handler����Ӧ����
    *             register_handler �ǽ�һ��handlerע�ᵽ��Ӧ�����������ϱ�־λ��
    *             schedule_wakeup��ֻ�ǶԱ�־λ���д���
    * @return     int 0��ʾ�ɹ�������ʧ��
    * @param[in]  event_handler ע��ľ��
    * @param[in]  event_mask    ���Ҫ�����MASK
    */
    virtual int register_handler(ZCE_Event_Handler *event_handler, int event_mask);

    /*!
    * @brief      �ӷ�Ӧ��ע��һ��ZCE_Event_Handler��ͬʱȡ�������е�mask
    *             cancel_wakeup���Ǵ�
    * @return     int               0��ʾ�ɹ�������ʧ��
    * @param[in]  event_handler     ע���ľ��
    * @param[in]  call_handle_close ע�����Ƿ��Զ����þ����handle_close����
    */
    virtual int remove_handler(ZCE_Event_Handler *event_handler, bool call_handle_close);

    /*!
    * @brief      ȡ��ĳЩmask��־��
    * @return     int           0��ʾ�ɹ�������ʧ��
    * @param[in]  event_handler ����ľ��
    * @param[in]  cancel_mask   ȡ�����¼�mask��־
    */
    virtual int cancel_wakeup(ZCE_Event_Handler *event_handler, int cancel_mask) = 0;

    /*!
    * @brief      ��ĳЩmask��־��
    * @return     virtual int
    * @param[in]  event_handler ����ľ��
    * @param[in]  event_mask    ���õ��¼�mask��־
    */
    virtual int schedule_wakeup(ZCE_Event_Handler *event_handler, int event_mask) = 0;

    /*!
    * @brief      �����¼�,���麯��
    * @return     int           0��ʾ�ɹ�������ʧ��
    * @param[in,out]  time_out  ��ʱʱ��
    * @param[out] size_event    ���ش������¼��������
    */
    virtual int handle_events(ZCE_Time_Value *time_out, size_t *size_event) = 0;

protected:

    /*!
    * @brief      ��ѯһ��event handler�Ƿ�ע���ˣ�������ڷ���0
    * @return     int           ����ֵ,0��ʶ��ѯ���ˣ�-1��ʶû�в�ѯ��
    * @param[in]  event_handler ȷ���Ƿ���ڵ�ZCE_Event_Handler ���
    */
    inline int exist_event_handler(ZCE_Event_Handler *event_handler);

    /*!
    * @brief      ͨ�������ѯevent handler��������ڷ���0
    * @return     int           ����ֵ
    * @param[in]  socket_handle ��ѯ��ZCE_HANDLE���
    * @param[out] event_handler ��ѯ�õ��ľ����Ӧ��ZCE_Event_Handlerָ��
    */
    inline int find_event_handler(ZCE_HANDLE handle, ZCE_Event_Handler *&event_handler);

public:


    /*!
    * @brief      ��ȡ���Ӻ���
    * @return     ZCE_Reactor* ��Ӧ����ָ��
    */
    static ZCE_Reactor *instance();
    ///�����Ӻ���
    static void clean_instance();
    ///���õ��ӵĺ���
    static void instance(ZCE_Reactor *pinstatnce);

protected:

    ///���ZCE_SOCKET��ӦZCE_Event_Handler *��MAP,�����¼�������ʱ�򣬵���ZCE_Event_Handler *�ĺ���
    MAP_OF_HANDLER_TO_EVENT    handler_map_;

    ///���Ĵ�������С������һЩ������resize
    size_t                     max_event_number_;

protected:

    ///����ʵ��ָ��
    static ZCE_Reactor         *instance_;
};

//��ѯһ��event handler�Ƿ�ע���ˣ�������ڷ���0
inline int ZCE_Reactor::exist_event_handler(ZCE_Event_Handler *event_handler)
{
    ZCE_HANDLE socket_hd = event_handler->get_handle();

    MAP_OF_HANDLER_TO_EVENT::iterator iter_temp =  handler_map_.find(socket_hd);

    //�Ѿ���һ��HANDLE��
    if (iter_temp == handler_map_.end())
    {
        return -1;
    }

    return 0;
}

//ͨ�������ѯevent handler��������ڷ���0
inline int ZCE_Reactor::find_event_handler(ZCE_HANDLE handle,
                                           ZCE_Event_Handler *&event_handler)
{
    MAP_OF_HANDLER_TO_EVENT::iterator iter_temp = handler_map_.find(handle);

    //�Ѿ���һ��HANDLE��
    if (iter_temp == handler_map_.end())
    {
        event_handler = NULL;
        return -1;
    }

    event_handler = iter_temp->second;

    return 0;
}

#endif //ZCE_LIB_EVENT_REACTOR_BASE_H_

