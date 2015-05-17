/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_timer_handler_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
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

#ifndef ZCE_LIB_TIMER_HANDLER_BASE_H_
#define ZCE_LIB_TIMER_HANDLER_BASE_H_

#include "zce_boost_non_copyable.h"
#include "zce_time_value.h"

class ZCE_Timer_Queue;

/******************************************************************************************
class ZCE_Timer_Handler
******************************************************************************************/
class ZCE_Timer_Handler
{

protected:

    //构造函数和析构函数
    ZCE_Timer_Handler(ZCE_Timer_Queue *timer_queue);
    ZCE_Timer_Handler();
    virtual ~ZCE_Timer_Handler();

protected:
    //
    ZCE_Timer_Queue    *timer_queue_;
    //

public:

    //超时触发
    virtual int timer_timeout(const ZCE_Time_Value &now_timenow_time,
                              const void *act = 0)  = 0;
    //定时器关闭
    virtual int timer_close();

    //
    ZCE_Timer_Queue *timer_queue();

    //
    void timer_queue(ZCE_Timer_Queue *set_timer_queue);

    //考虑的半天，我觉得对于Timer的处理还是不用考虑实现一个handle_close了。
    //handle_close
};

#endif //# ZCE_LIB_TIMER_HANDLER_BASE_H_
