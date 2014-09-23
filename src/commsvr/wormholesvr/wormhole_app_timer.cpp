#include "wormhole_predefine.h"
#include "wormhole_application.h"
#include "wormhole_app_timer.h"
#include "wormhole_flow_stat.h"

ArbiterTimerHandler::ArbiterTimerHandler(ZEN_Timer_Queue *timer_queue): ZEN_Timer_Handler(timer_queue)
{
    last_report_time_.sec(time(NULL));
}

ArbiterTimerHandler::~ArbiterTimerHandler(void)
{
}

int ArbiterTimerHandler::handle_timeout(const ZEN_Time_Value &now,
                                        const void *act)
{
    ZEN_UNUSED_ARG(act);

    if ((unsigned int)(now.sec() - last_report_time_.sec())
        >= ArbiterAppliction::instance()->get_config()->report_time_)
    {
        // 定时写统计账单
        ArbiterFlowStat::instance()->write_flow_stat();

        // 输出同一svr_type、不同ID的数据量
        ArbiterSendStat::instance()->show_flow_info();

        // 更新上报时间
        last_report_time_ = now;
    }

    return TSS_RET::TSS_RET_SUCC;
}
