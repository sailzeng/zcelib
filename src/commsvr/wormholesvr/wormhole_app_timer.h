/// @file       arbiter_timer_handler.h
/// @date       2011/12/22

/// @author     stefzhou

/// @brief      定时上报统计信息
/// @details

#ifndef ARBITER_TIMER_HANDLER_H_
#define ARBITER_TIMER_HANDLER_H_

class ArbiterTimerHandler : public ZEN_Timer_Handler
{
public:
    ArbiterTimerHandler(ZEN_Timer_Queue *timer_queue);
    ~ArbiterTimerHandler(void);

    virtual int handle_timeout(const ZEN_Time_Value &now,
                               const void *act = 0);

private:
    ZEN_Time_Value  last_report_time_;

};

#endif // ARBITER_TIMER_HANDLER_H_

