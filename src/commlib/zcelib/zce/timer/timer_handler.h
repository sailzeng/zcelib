/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/timer/timer_handler.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2008年10月6日
* @brief      Timer处理的基类
*
*
* @details
*
*
*
* @note       为什么要单独讲NODE放入一个文件中间，因为我可能写多个反应器
*
*/

#pragma once

#include "zce/util/non_copyable.h"
#include "zce/time/time_value.h"

namespace zce
{
class timer_queue;

/******************************************************************************************
class zce::Timer_Handler
******************************************************************************************/
class timer_handler
{
protected:

    //构造函数和析构函数
    timer_handler(zce::timer_queue* timer_queue);
    timer_handler();
    virtual ~timer_handler();

public:

    //超时触发
    virtual int timer_timeout(const zce::time_value& now_timenow_time,
                              int timer_id) = 0;
    //定时器关闭
    virtual int timer_close();

    //
    zce::timer_queue* timer_queue();

    //
    void timer_queue(zce::timer_queue* set_timer_queue);

    //考虑的半天，我觉得对于Timer的处理还是不用考虑实现一个event_close了。
protected:
    //
    zce::timer_queue* timer_queue_ = nullptr;
};
}
