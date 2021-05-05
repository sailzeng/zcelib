#include "zce/predefine.h"
#include "zce/os_adapt/time.h"
#include "zce/timer/handler_base.h"

namespace zce
{
//构造函数和析构函数
Timer_Handler::Timer_Handler(zce::Timer_Queue *timer_queue):
    timer_queue_(timer_queue)
{
}

Timer_Handler::Timer_Handler()
{
}

Timer_Handler::~Timer_Handler()
{
}

//得到Timer Queue
Timer_Queue *Timer_Handler::timer_queue()
{
    return timer_queue_;
}

//得到Timer Queue
void Timer_Handler::timer_queue(Timer_Queue *set_timer_queue)
{
    timer_queue_ = set_timer_queue;
}

//定时器关闭
int Timer_Handler::timer_close()
{
    return 0;
}
}