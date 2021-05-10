#include "zerg/predefine.h"
#include "zerg/tcp_ctrl_handler.h"
#include "zerg/comm_manager.h"
#include "zerg/application.h"
#include "zerg/app_timer.h"

namespace zerg
{
/****************************************************************************************************
class  App_Timer
****************************************************************************************************/

///ZERG服务器定时器ID,
const int App_Timer::ZERG_TIMER_ID[] =
{
    0x101,
    0x102,
};

//
App_Timer::App_Timer() :
    Server_Timer()
{
    //主动重现链接的间隔时间
    const time_t AUTOCONNECT_RETRY_SEC = 5;
    zce::Time_Value connect_all_internal(AUTOCONNECT_RETRY_SEC, 0);

    add_app_timer(connect_all_internal, &ZERG_TIMER_ID[0]);

    //主动重现链接的间隔时间
    const time_t RECORD_MONITOR_SEC = 60;
    zce::Time_Value monitor_internal(RECORD_MONITOR_SEC, 0);

    add_app_timer(monitor_internal, &ZERG_TIMER_ID[1]);
}

App_Timer::~App_Timer()
{
}

//
int App_Timer::timer_timeout(const zce::Time_Value& time_now, 
    const void* act)
{
    //等到当前的时间
    Server_Timer::timer_timeout(time_now, act);

    //心跳数据
    const int zerg_timeid = *(static_cast<const int*>(act));
    if (ZERG_TIMER_ID[0] == zerg_timeid)
    {
        TCP_Svc_Handler::reconnect_allserver();
    }
    else if (ZERG_TIMER_ID[1] == zerg_timeid)
    {
        Buffer_Storage::instance()->monitor();
    }
    return 0;
}
}