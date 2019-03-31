#include "zce_predefine.h"
#include "zce_os_adapt_time.h"
#include "zce_timer_handler_base.h"

//构造函数和析构函数
ZCE_Timer_Handler::ZCE_Timer_Handler(ZCE_Timer_Queue_Base *timer_queue):
    timer_queue_(timer_queue)
{
}

ZCE_Timer_Handler::ZCE_Timer_Handler()
{
}

ZCE_Timer_Handler::~ZCE_Timer_Handler()
{
}

//得到Timer Queue
ZCE_Timer_Queue_Base *ZCE_Timer_Handler::timer_queue()
{
    return timer_queue_;
}

//得到Timer Queue
void ZCE_Timer_Handler::timer_queue(ZCE_Timer_Queue_Base *set_timer_queue)
{
    timer_queue_ = set_timer_queue;
}

//定时器关闭
int ZCE_Timer_Handler::timer_close()
{
    return 0;
}

