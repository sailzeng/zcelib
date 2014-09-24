#include "wormhole_predefine.h"
#include "wormhole_application.h"
#include "wormhole_app_timer.h"


ArbiterTimerHandler::ArbiterTimerHandler() :
    Server_Timer_Base()
{
    last_report_time_.sec(time(NULL));
}

ArbiterTimerHandler::~ArbiterTimerHandler(void)
{
}

int ArbiterTimerHandler::handle_timeout(const ZCE_Time_Value &now,
                                        const void *act)
{
    ZCE_UNUSED_ARG(act);
    ZCE_UNUSED_ARG(now);
    return 0;
}
