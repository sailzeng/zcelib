/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_event_reactor_epoll.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年8月28日
* @brief      EPOLL 的IO反应器，IO多路复用模型
*             他只能在LINUX下使用（虽然可以编译），
*
* @details
*
*/

#ifndef ZCE_LIB_EVENT_REACTOR_EPOLL_H_
#define ZCE_LIB_EVENT_REACTOR_EPOLL_H_

#include "zce_event_reactor_base.h"

/*!
* @brief      EPOLL 的IO反应器，IO多路复用模型
*
*/
class  ZCE_Epoll_Reactor : public ZCE_Reactor
{

protected:

    //默认的一次最大处理的时间数量，如果你觉得不够大，可以调整一下，
    static const int DEFAULT_ONCE_TRIGGER_MAX_EVENT = 2048;

public:

    /*!
    * @brief      构造函数
    */
    ZCE_Epoll_Reactor();

    /*!
    * @brief      构造函数,相当于把初始化也干了，
    * @param[in]  max_event_number 最大句柄数量
    * @param[in]  edge_triggered   是否进行边缘触发方式
    * @param[in]  once_max_event   一次最大处理的最大事件数量
    */
    ZCE_Epoll_Reactor(size_t max_event_number,
                      bool edge_triggered = false,
                      int once_max_event = DEFAULT_ONCE_TRIGGER_MAX_EVENT);
    /*!
    * @brief      析构函数
    */
    virtual ~ZCE_Epoll_Reactor();

public:

    /*!
    * @brief      初始化
    * @return     int              返回0表示成功，其他表示失败
    * @param[in]  max_event_number 最大句柄数量
    * @param[in]  edge_triggered   是否进行边缘触发方式
    * @param[in]  once_max_event   一次最大处理的最大事件数量
    * @note
    */
    int initialize(size_t max_event_number,
                   bool edge_triggered = false,
                   int once_max_event = DEFAULT_ONCE_TRIGGER_MAX_EVENT);

    /*!
    * @brief      注册一个ZCE_Event_Handler到反应器,EPOLL是明确的注册操作的，所以需要重载这个函数
    * @return     int             返回0表示成功，其他表示失败
    * @param[in]  event_handler   注册的句柄
    * @param[in]  event_mask      注册后同时设置的MASK标志，请参考@ref EVENT_MASK ,可以多个值|使用。
    */
    virtual int register_handler(ZCE_Event_Handler *event_handler, int event_mask) override;

    /*!
    * @brief      从反应器注销一个ZCE_Event_Handler，同事取消他所有的mask
    * @return     int               0表示成功，否则失败
    * @param[in]  event_handler     注销的句柄
    * @param[in]  call_handle_close 注销后，是否自动调用句柄的handle_close函数
    * */
    virtual int remove_handler(ZCE_Event_Handler *event_handler, bool call_handle_close) override;

    /*!
    * @brief      取消某些mask标志，，
    * @return     int           返回0表示成功，其他表示失败
    * @param[in]  event_handler 操作的句柄
    * @param[in]  cancel_mask   要取消的MASK标志，请参考@ref EVENT_MASK ,可以多个值|使用。
    * */
    virtual int cancel_wakeup(ZCE_Event_Handler *event_handler, int cancel_mask) override;

    /*!
    * @brief      打开某些mask标志，
    * @return     int             返回0表示成功，其他表示失败
    * @param[in]  event_handler   操作的句柄
    * @param[in]  event_mask      要打开的标志，请参考@ref EVENT_MASK ,可以多个值|使用。
    * */
    virtual int schedule_wakeup(ZCE_Event_Handler *event_handler, int event_mask) override;

    /*!
    * @brief      进行IO触发操作
    * @return        int           返回0表示成功，其他表示失败
    * @param[in,out] time_out      超时时间，完毕后返回剩余时间
    * @param[out]    size_event    触发的句柄数量
    */
    virtual int handle_events(ZCE_Time_Value *time_out, size_t *size_event) override;

protected:

    /*!
    * @brief      这个inline函数理论上可以放入CPP,将mask转换为epoll_event结构,
    * @param[out] ep_event      EPOLL 函数操作的结构
    * @param[in]  event_handler 要处理的EVENT句柄
    */
    inline void make_epoll_event(struct epoll_event *ep_event, ZCE_Event_Handler *event_handler) const;

    /*!
    * @brief      处理已经触发的句柄，调用相应的虚函数，进行触发，让你处理
    * @param[in]  ep_event  epoll 返回的句柄集合
    */
    void process_ready_event(struct epoll_event *ep_event);

protected:

    ///EPOLL自己的文件句柄，最后要关闭之
    int          epoll_fd_;

    ///是否使用边界触发，边界触发在代码编写中需要更加啊小心一些
    bool         edge_triggered_;

    ///一次触发最大处理的句柄数量
    int          once_max_events_;

    ///一次触发最大处理的epoll_event数组
    epoll_event *once_events_ary_;
};

//将mask转换为epoll_event结构
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
    //Connect有成功和失败两种情况
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
    //如果你要边缘触发
    if (edge_triggered_)
    {
        ep_event->events |= EPOLLET;
    }
}

#endif //ZCE_LIB_EVENT_REACTOR_EPOLL_H_

