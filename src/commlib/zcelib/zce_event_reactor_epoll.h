/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_event_reactor_epoll.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��8��28��
* @brief      EPOLL ��IO��Ӧ����IO��·����ģ��
*             ��ֻ����LINUX��ʹ�ã���Ȼ���Ա��룩��
*
* @details
*
*/

#ifndef ZCE_LIB_EVENT_REACTOR_EPOLL_H_
#define ZCE_LIB_EVENT_REACTOR_EPOLL_H_

#include "zce_event_reactor_base.h"

/*!
* @brief      EPOLL ��IO��Ӧ����IO��·����ģ��
*
*/
class  ZCE_Epoll_Reactor : public ZCE_Reactor
{

protected:

    //Ĭ�ϵ�һ��������ʱ���������������ò����󣬿��Ե���һ�£�
    static const int DEFAULT_ONCE_TRIGGER_MAX_EVENT = 2048;

public:

    /*!
    * @brief      ���캯��
    */
    ZCE_Epoll_Reactor();

    /*!
    * @brief      ���캯��,�൱�ڰѳ�ʼ��Ҳ���ˣ�
    * @param[in]  max_event_number ���������
    * @param[in]  edge_triggered   �Ƿ���б�Ե������ʽ
    * @param[in]  once_max_event   һ������������¼�����
    */
    ZCE_Epoll_Reactor(size_t max_event_number,
                      bool edge_triggered = false,
                      int once_max_event = DEFAULT_ONCE_TRIGGER_MAX_EVENT);
    /*!
    * @brief      ��������
    */
    virtual ~ZCE_Epoll_Reactor();

public:

    /*!
    * @brief      ��ʼ��
    * @return     int              ����0��ʾ�ɹ���������ʾʧ��
    * @param[in]  max_event_number ���������
    * @param[in]  edge_triggered   �Ƿ���б�Ե������ʽ
    * @param[in]  once_max_event   һ������������¼�����
    * @note
    */
    int initialize(size_t max_event_number,
                   bool edge_triggered = false,
                   int once_max_event = DEFAULT_ONCE_TRIGGER_MAX_EVENT);

    /*!
    * @brief      ע��һ��ZCE_Event_Handler����Ӧ��,EPOLL����ȷ��ע������ģ�������Ҫ�����������
    * @return     int             ����0��ʾ�ɹ���������ʾʧ��
    * @param[in]  event_handler   ע��ľ��
    * @param[in]  event_mask      ע���ͬʱ���õ�MASK��־����ο�@ref EVENT_MASK ,���Զ��ֵ|ʹ�á�
    */
    virtual int register_handler(ZCE_Event_Handler *event_handler, int event_mask) override;

    /*!
    * @brief      �ӷ�Ӧ��ע��һ��ZCE_Event_Handler��ͬ��ȡ�������е�mask
    * @return     int               0��ʾ�ɹ�������ʧ��
    * @param[in]  event_handler     ע���ľ��
    * @param[in]  call_handle_close ע�����Ƿ��Զ����þ����handle_close����
    * */
    virtual int remove_handler(ZCE_Event_Handler *event_handler, bool call_handle_close) override;

    /*!
    * @brief      ȡ��ĳЩmask��־����
    * @return     int           ����0��ʾ�ɹ���������ʾʧ��
    * @param[in]  event_handler �����ľ��
    * @param[in]  cancel_mask   Ҫȡ����MASK��־����ο�@ref EVENT_MASK ,���Զ��ֵ|ʹ�á�
    * */
    virtual int cancel_wakeup(ZCE_Event_Handler *event_handler, int cancel_mask) override;

    /*!
    * @brief      ��ĳЩmask��־��
    * @return     int             ����0��ʾ�ɹ���������ʾʧ��
    * @param[in]  event_handler   �����ľ��
    * @param[in]  event_mask      Ҫ�򿪵ı�־����ο�@ref EVENT_MASK ,���Զ��ֵ|ʹ�á�
    * */
    virtual int schedule_wakeup(ZCE_Event_Handler *event_handler, int event_mask) override;

    /*!
    * @brief      ����IO��������
    * @return        int           ����0��ʾ�ɹ���������ʾʧ��
    * @param[in,out] time_out      ��ʱʱ�䣬��Ϻ󷵻�ʣ��ʱ��
    * @param[out]    size_event    �����ľ������
    */
    virtual int handle_events(ZCE_Time_Value *time_out, size_t *size_event) override;

protected:

    /*!
    * @brief      ���inline���������Ͽ��Է���CPP,��maskת��Ϊepoll_event�ṹ,
    * @param[out] ep_event      EPOLL ���������Ľṹ
    * @param[in]  event_handler Ҫ�����EVENT���
    */
    inline void make_epoll_event(struct epoll_event *ep_event, ZCE_Event_Handler *event_handler) const;

    /*!
    * @brief      �����Ѿ������ľ����������Ӧ���麯�������д��������㴦��
    * @param[in]  ep_event  epoll ���صľ������
    */
    void process_ready_event(struct epoll_event *ep_event);

protected:

    ///EPOLL�Լ����ļ���������Ҫ�ر�֮
    int          epoll_fd_;

    ///�Ƿ�ʹ�ñ߽紥�����߽紥���ڴ����д����Ҫ���Ӱ�С��һЩ
    bool         edge_triggered_;

    ///һ�δ��������ľ������
    int          once_max_events_;

    ///һ�δ���������epoll_event����
    epoll_event *once_events_ary_;
};

//��maskת��Ϊepoll_event�ṹ
inline void ZCE_Epoll_Reactor::make_epoll_event(struct epoll_event *ep_event, ZCE_Event_Handler *event_handler) const
{
    ep_event->events = 0;

#if defined (ZCE_OS_LINUX)
    ep_event->data.fd = event_handler->get_handle();
#endif

    int event_mask = event_handler->get_mask();

    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::READ_MASK))
    {
        ep_event->events |= EPOLLIN;
    }

    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::WRITE_MASK))
    {
        ep_event->events |= EPOLLOUT;
    }

    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::EXCEPT_MASK))
    {
        ep_event->events |= EPOLLERR;
    }
    //Connect�гɹ���ʧ���������
    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::CONNECT_MASK))
    {
        ep_event->events |= EPOLLOUT;
        ep_event->events |= EPOLLIN;
    }

    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::ACCEPT_MASK))
    {
        ep_event->events |= EPOLLIN;
    }

    if (ZCE_BIT_IS_SET(event_mask, ZCE_Event_Handler::INOTIFY_MASK))
    {
        ep_event->events |= EPOLLIN;
    }
    //�����Ҫ��Ե����
    if (edge_triggered_)
    {
        ep_event->events |= EPOLLET;
    }
}

#endif //ZCE_LIB_EVENT_REACTOR_EPOLL_H_

