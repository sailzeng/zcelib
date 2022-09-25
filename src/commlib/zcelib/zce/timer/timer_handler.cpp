#include "zce/predefine.h"
#include "zce/os_adapt/time.h"
#include "zce/timer/timer_handler.h"

namespace zce
{
//构造函数和析构函数
timer_handler::timer_handler(zce::timer_queue* timer_queue) :
    timer_queue_(timer_queue)
{
}

timer_handler::timer_handler()
{
}

timer_handler::~timer_handler()
{
}

//得到Timer Queue
timer_queue* timer_handler::timer_queue()
{
    return timer_queue_;
}

//得到Timer Queue
void timer_handler::timer_queue(zce::timer_queue* set_timer_queue)
{
    timer_queue_ = set_timer_queue;
}

//定时器关闭
int timer_handler::timer_close()
{
    return 0;
}
}