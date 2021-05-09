/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/event/handle_base.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年8月29日
* @brief      IO反应器所使用的事件句柄封装
*
* @details
*
* @note
*
*/

#pragma once

namespace zce
{
class ZCE_Reactor;

/*!
* @brief      IO反应器所使用的事件句柄，当IO事件触发后，相应的函数
*             被调用
*             最后考虑再三，没有在反应器里面集成定时器的处理
*/
class Event_Handler
{
public:

    ///IO触发事件, 这儿不要改造成enum class
    enum EVENT_MASK
    {
        NULL_MASK = 0,

        ///SOCKET读取事件，但句柄变为可读是，调用handle_input
        READ_MASK = (1 << 1),
        ///SOCKET写事件，当句柄变为可写是，handle_output
        WRITE_MASK = (1 << 2),
        ///SOCKET异常事件，触发后调用handle_exception
        EXCEPT_MASK = (1 << 3),
        ///SOCKET异步CONNECT成功后，调用handle_output，异步CONNECT失败后，
        ///会调用handle_input，(多系统差异会统一)
        CONNECT_MASK = (1 << 4),
        ///SOCKET ACCEPT事件，当一个ACCEPT端口可以链接后，调用handle_input，
        ACCEPT_MASK = (1 << 5),

        ///iNotify通知事件，文件系统的改变通知,调用handle_input，
        INOTIFY_MASK = (1 << 9),
    };

    /*!
    * @brief      构造函数
    */
    Event_Handler();
    /*!
    * @brief      构造函数，同时设置香港的反应器指针
    * @param      reactor 句柄相关的反应器指针
    */
    Event_Handler(zce::ZCE_Reactor* reactor);
    /*!
    * @brief      析构函数
    */
    virtual ~Event_Handler();

public:

    /*!
    * @brief      取回对应的ZCE_HANDLE 句柄
    * @return     ZCE_HANDLE Event_Handler 对应的ZCE_HANDLE 句柄
    */
    virtual ZCE_HANDLE get_handle(void) const = 0;

    /*!
    * @brief      读取事件触发调用函数，用于读取数据，accept成功，（connect失败）处理，
    * @return     int 返回0表示句柄处理正常，return -1后，反应器会主动handle_close，帮助结束句柄
    * @note       return -1 后反应器帮助主动调用handle_close这个特性，其实我持保留意见，因为其实
    *             可以直接调用handle_close，而这两种方式并不兼容，而且直接调用可能更加清晰一下，我个
    *             不建议使用这个特性，保留其主要是为了和ACE兼容
    */
    virtual int handle_input();

    /*!
    * @brief      写入事件触发调用函数，用于写数据，connect成功，
    * @return     int int 返回0表示句柄处理正常，return -1后，反应器会主动handle_close，帮助结束句柄
    */
    virtual int handle_output();

    /*!
    * @brief      调用异常，return -1表示调用handle_close
    * @return     int
    */
    virtual int handle_exception();

    /*!
    * @brief      句柄关闭处理函数，基类函数调用了remove
    * @return     int
    */
    virtual int handle_close();

    /*!
    * @brief      取得当前的标志位
    * @return     int 返回的当前的MASK值
    */
    inline int get_mask();

    /*!
    * @brief      设置当前标志位
    * @param      mask 设置的MASK值
    */
    inline void set_mask(int mask);

    /*!
    * @brief      enable mask所带的标志位
    * @param      en_mask 打开的MASK的值
    */
    inline void enable_mask(int en_mask);

    /*!
    * @brief      disable mask所带的标志位
    * @param      dis_mask 关闭的MASK值
    */
    inline void disable_mask(int dis_mask);

    /*!
    * @brief      设置反应器
    * @param      reactor
    */
    virtual void reactor(zce::ZCE_Reactor* reactor);

    /*!
    * @brief      取得自己所属的反应器
    * @return     zce::ZCE_Reactor*
    */
    virtual zce::ZCE_Reactor* reactor(void) const;

    //超时处理，最后考虑再三，没有在反应器里面集成定时器的处理
    //virtual int timer_timeout (const zce::Time_Value &tv, const void *arg = 0);

protected:

    ///反应器
    zce::ZCE_Reactor* zce_reactor_;

    ///这个句柄对应要处理的事件MASK
    int                event_mask_;
};

//取得当前的标志位
inline int Event_Handler::get_mask()
{
    return event_mask_;
}

//设置当前标志位
inline void Event_Handler::set_mask(int mask)
{
    event_mask_ = mask;
}

//enable mask所带的标志位
inline void Event_Handler::enable_mask(int en_mask)
{
    event_mask_ |= en_mask;
}
//disable mask所带的标志位
inline void Event_Handler::disable_mask(int dis_mask)
{
    event_mask_ &= (~dis_mask);
}
}
