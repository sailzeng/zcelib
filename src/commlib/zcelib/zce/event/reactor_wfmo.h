/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce/event/reactor_wfmo.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       Sunday, August 17, 2014
* @brief      WFMO 是WaitForMultipleObjects的缩写。这个事Windows专有的东东。
*             为什么非要有这个东东，主要是因为WaitForMultipleObjects才是Windwos，
*             核心的事件触发器，
*
* @details    WFMO 的Reactor 实现不算高效，完美
*             第一，WaitForMultipleObjects的特点决定了，他只能等待64个句柄，
*             第二，我目前只设计了SOCKET和INOTIFY相关的代码。
*             第三，WFMO对于写事件的处理时边缘触发，不是水平触发，写代码的时候要注意，
*             第四，在一些句柄处理时，会有一个循环检查处理
*
*
* @note
*
*/

#ifndef ZCE_LIB_EVENT_REACTOR_WFMO_H_
#define ZCE_LIB_EVENT_REACTOR_WFMO_H_

#include "zce/event/reactor_base.h"


#if defined ZCE_OS_WINDOWS

/*!
* @brief      WaitforMu 的IO反应器，IO多路复用模型
*
*/
class ZCE_WFMO_Reactor : public ZCE_Reactor
{

public:

    /*!
    * @brief    构造函数
    */
    ZCE_WFMO_Reactor();


    /*!
    * @brief      和析构函数 virtual的喔。
    */
    virtual ~ZCE_WFMO_Reactor();

public:

    /*!
    * @brief      初始化
    * @return     int              返回0表示成功，其他表示失败
    */
    int initialize();

    /*!
    * @brief      注册一个ZCE_Event_Handler到反应器,EPOLL是明确的注册操作的，所以需要重载这个函数
    * @return     int             返回0表示成功，其他表示失败
    * @param[in]  event_handler   注册的句柄
    * @param[in]  event_mask      注册后同时设置的MASK标志，请参考@ref EVENT_MASK ,可以多个值|使用。
    */
    virtual int register_handler(ZCE_Event_Handler *event_handler, int event_mask) override;

    /*!
    * @brief      从反应器注销一个ZCE_Event_Handler，同时取消他所有的mask
    * @return     int               0表示成功，否则失败
    * @param[in]  event_handler     注销的句柄
    * @param[in]  call_handle_close 注销后，是否自动调用句柄的handle_close函数
    * */
    virtual int remove_handler(ZCE_Event_Handler *event_handler, bool call_handle_close) override;

    /*!
    * @brief      取消某些mask标志，
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
    virtual int handle_events(zce::Time_Value *time_out, size_t *size_event) override;



    /*!
    * @brief      Windows 下 对Socket 根据EVENT_MASK设置其对应的网络事件，并且绑定到事件上
    * @return     int           返回0表示成功，其他表示失败
    * @param      event_handler SOCKET句柄对应的Event Handler
    * @param      socket_event  SOCKET EVENT，用于给WaitForMultipleObjects使用
    * @param      event_mask    希望设置的EVENT_MASK
    */
    static int wfmo_socket_event(ZCE_Event_Handler *event_handler,
                                 WSAEVENT socket_event,
                                 int event_mask);

protected:

    ///WaitForMultipleObjects等待的目录句柄
    ///为什么要有这个重复的结构，主要是为了方便使用WaitForMultipleObjects的速度
    ZCE_HANDLE     watch_handle_ary_[MAXIMUM_WAIT_OBJECTS];

    ///保留的SOCKET句柄，因为WaitForMultipleObjects处理SOCKET时，我们使用的是
    ///WSAEventSelect 处理后的事件句柄，所以这儿在对应的数组下标位置保留SOCKET
    SOCKET          watch_socket_ary_[MAXIMUM_WAIT_OBJECTS];
};

#endif //#if defined ZCE_OS_WINDOWS

#endif //ZCE_LIB_EVENT_REACTOR_WFMO_H_

