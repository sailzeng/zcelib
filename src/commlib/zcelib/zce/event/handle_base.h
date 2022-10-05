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
class reactor;

/*!
* @brief      IO反应器所使用的事件句柄，当IO事件触发后，相应的函数
*             被调用
*             最后考虑再三，没有在反应器里面集成定时器的处理
*/
class event_handler
{
public:

    ///IO触发事件, 这儿不要改造成enum class
    enum EVENT_MASK
    {
        NULL_MASK = 0,

        ///SOCKET读取事件，但句柄变为可读是，调用read_event
        READ_MASK = (1 << 1),
        ///SOCKET写事件，当句柄变为可写是，write_event
        WRITE_MASK = (1 << 2),
        ///SOCKET异常事件，触发后调用exception_event
        EXCEPT_MASK = (1 << 3),
        ///SOCKET异步CONNECT成功后，调用write_event，异步CONNECT失败后，
        ///会调用read_event，(多系统差异会统一)
        CONNECT_MASK = (1 << 4),
        ///SOCKET ACCEPT事件，当一个ACCEPT端口可以链接后，调用handle_input，
        ACCEPT_MASK = (1 << 5),

        ///iNotify通知事件，文件系统的改变通知,调用read_event，
        INOTIFY_MASK = (1 << 9),
    };

    /*!
    * @brief      构造函数
    */
    event_handler();
    /*!
    * @brief  构造函数，同时设置香港的反应器指针
    * @param reactor 句柄相关的反应器指针
    */
    event_handler(zce::reactor* reactor);
    /*!
    * @brief  析构函数
    */
    virtual ~event_handler();

public:

    /*!
    * @brief  取回对应的ZCE_HANDLE 句柄
    * @return ZCE_HANDLE event_handler 对应的ZCE_HANDLE 句柄
    */
    virtual ZCE_HANDLE get_handle(void) const = 0;

    /*!
    * @brief  读取事件触发调用函数，用于读取数据。
    * @return int return -1会记录，error等信息，
    */
    virtual int read_event();

    /*!
    * @brief      写入事件触发调用函数，用于写入事件
    * @return int return -1会记录，error等信息，
    */
    virtual int write_event();

    /*!
    * @brief      发生异常事件后调用，返回值参考其他函数
    */
    virtual int exception_event();

    /*!
      * @brief  发生了链接的事件
      * @param success 连接是否成功
     */
    virtual int connect_event(bool success);

    /*!
     * @brief  发生了accept的事件是调用
    */
    virtual int accept_event();

    /*!
     * @brief  发生了inotify的事件是调用
    */
    virtual int inotify_event();

    /*!
    * @brief      句柄关闭处理函数，基类函数调用了remove
    */
    virtual int event_close();

    /*!
    * @brief      取得当前的标志位
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
    virtual void reactor(zce::reactor* reactor);

    /*!
    * @brief      取得自己所属的反应器
    * @return     zce::reactor*
    */
    virtual zce::reactor* reactor(void) const;

    //超时处理，最后考虑再三，没有在反应器里面集成定时器的处理
    //virtual int timer_timeout (const zce::time_value &tv, const void *arg = 0);

protected:

    ///反应器
    zce::reactor* zce_reactor_;

    ///这个句柄对应要处理的事件MASK
    int           event_mask_;
};

//取得当前的标志位
inline int event_handler::get_mask()
{
    return event_mask_;
}

//设置当前标志位
inline void event_handler::set_mask(int mask)
{
    event_mask_ = mask;
}

//enable mask所带的标志位
inline void event_handler::enable_mask(int en_mask)
{
    event_mask_ |= en_mask;
}
//disable mask所带的标志位
inline void event_handler::disable_mask(int dis_mask)
{
    event_mask_ &= (~dis_mask);
}
}
