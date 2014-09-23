

#ifndef WORMHOLE_TIMER_HANDLER_H_
#define WORMHOLE_TIMER_HANDLER_H_

class ArbiterTimerHandler : public Server_Timer_Base
{
public:
    ArbiterTimerHandler();
    ~ArbiterTimerHandler(void);

    virtual int handle_timeout(const ZCE_Time_Value &now,
                               const void *act = 0);

private:
    ZCE_Time_Value  last_report_time_;

};

#endif // WORMHOLE_TIMER_HANDLER_H_

