/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/event/reactor_base.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年9月1日
* @brief      反应器的基类
*
* @details
*
* @note
*
*/
#pragma once

#include "zce/util/non_copyable.h"

namespace zce
{
class event_handler;

/*!
* @brief      反应器的基类
*
*/
class reactor :public zce::non_copyable
{
protected:

    ///使用hansh map保存句柄到zce::Event_Handler的MAP ，力求最高的性能
    typedef std::unordered_map<ZCE_HANDLE,
        zce::event_handler* >  MAP_OF_HANDLER_TO_EVENT;

protected:

    /*!
    * @brief      构造函数
    */
    reactor();
    /*!
    * @brief      构造函数
    * @param[in]  max_event_number 最大的容量，
    */
    reactor(size_t max_event_number);
    /*!
    * @brief      析构函数，virtual的喔
    */
    virtual ~reactor();

public:

    /*!
    * @brief      当前反应器容器的句柄数量
    * @return     size_t 返回的当前在反应器的句柄数量
    */
    size_t size();

    /*!
    * @brief      反应器的最大尺寸，容量，
    * @return     size_t 返回容量
    */
    size_t max_size();

    /*!
    * @brief      初始化
    * @return     int
    * @param      max_event_number
    * @note
    */
    int initialize(size_t max_event_number);

    /*!
    * @brief      关闭反应器，将所有注册的EVENT HANDLER 注销掉
    * @return     int
    */
    virtual int close();

    /*!
    * @brief      注册一个zce::Event_Handler到反应器，
    *             register_handler 是讲一个handler注册到反应器，而且置上标志位，
    *             schedule_wakeup，只是对标志位进行处理
    * @return     int 0表示成功，否则失败
    * @param[in]  event_handler 注册的句柄
    * @param[in]  event_mask    句柄要处理的MASK
    */
    virtual int register_handler(zce::event_handler* event_handler,
                                 int event_mask);

    /*!
    * @brief      从反应器注销一个zce::Event_Handler，同时取消他所有的mask
    *             cancel_wakeup，是从
    * @return     int               0表示成功，否则失败
    * @param[in]  event_handler     注销的句柄
    * @param[in]  call_event_close  注销后，是否自动调用句柄的event_close函数
    */
    virtual int remove_handler(zce::event_handler* event_handler,
                               bool call_event_close);

    /*!
    * @brief      取消某些mask标志，
    * @return     int           0表示成功，否则失败
    * @param[in]  event_handler 处理的句柄
    * @param[in]  cancel_mask   取消的事件mask标志
    */
    virtual int cancel_wakeup(zce::event_handler* event_handler,
                              int cancel_mask) = 0;

    /*!
    * @brief      打开某些mask标志，
    * @return     virtual int
    * @param[in]  event_handler 处理的句柄
    * @param[in]  event_mask    设置的事件mask标志
    */
    virtual int schedule_wakeup(zce::event_handler* event_handler,
                                int event_mask) = 0;

    /*!
    * @brief      触发事件,纯虚函数
    * @return     int           0表示成功，否则失败
    * @param[in,out]  time_out  超时时间
    * @param[out] size_event    返回触发的事件句柄数量
    */
    virtual int handle_events(zce::time_value* time_out,
                              size_t* size_event) = 0;

protected:

    /*!
    * @brief      查询一个event handler是否注册了，如果存在返回0
    * @return     int           返回值,0标识查询到了，-1标识没有查询到
    * @param[in]  event_handler 确认是否存在的zce::event_handler 句柄
    */
    inline int exist_event_handler(zce::event_handler* event_handler);

    /*!
    * @brief      通过句柄查询event handler，如果存在返回0
    * @return     int           返回值
    * @param[in]  socket_handle 查询的ZCE_HANDLE句柄
    * @param[out] event_handler 查询得到的句柄对应的zce::Event_Handler指针
    */
    inline int find_event_handler(ZCE_HANDLE handle,
                                  zce::event_handler*& event_handler);

public:

    /*!
    * @brief      获取单子函数
    * @return     reactor* 反应器的指针
    */
    static reactor* instance();
    ///清理单子函数
    static void clear_inst();
    ///设置单子的函数
    static void instance(reactor* pinstatnce);

protected:

    ///存放ZCE_SOCKET对应zce::event_handler *的MAP,方便事件触发的时候，调用zce::event_handler *的函数
    MAP_OF_HANDLER_TO_EVENT    handler_map_;

    ///最大的处理句柄大小，用于一些容器的resize
    size_t                     max_event_number_;

protected:

    ///单子实例指针
    static reactor* instance_;
};
}
