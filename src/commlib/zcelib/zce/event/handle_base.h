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

#include "zce/os_adapt/common.h"

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
    virtual ZCE_HANDLE get_handle() const = 0;

    /*!
    * @brief  读取事件触发调用函数，用于读取数据。
    */
    virtual void read_event();

    /*!
    * @brief      写入事件触发调用函数，用于写入事件
    */
    virtual void write_event();

    /*!
    * @brief      发生异常事件后调用，返回值参考其他函数
    */
    virtual void exception_event();

    /*!
      * @brief  发生了链接的事件
      * @param success 连接是否成功
     */
    virtual void connect_event(bool success);

    /*!
     * @brief  发生了accept的事件是调用
    */
    virtual void accept_event();

    /*!
     * @brief  发生了inotify的事件是调用
    */
    virtual void inotify_event();

    /*!
    * @brief      句柄关闭处理函数，基类函数调用了remove
    */
    virtual void close_handle();

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
