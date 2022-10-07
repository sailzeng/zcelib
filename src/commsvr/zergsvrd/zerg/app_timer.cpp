#include "zerg/predefine.h"
#include "zerg/svc_tcp.h"
#include "zerg/comm_manager.h"
#include "zerg/application.h"
#include "zerg/app_timer.h"

namespace zerg
{
/****************************************************************************************************
class  app_timer
****************************************************************************************************/

///ZERG服务器定时器ID,
const int app_timer::ZERG_TIMER_ID[] =
{
    0x101,
    0x102,
};

//
app_timer::app_timer() :
    Server_Timer()
{
    //主动重现链接的间隔时间
    const time_t AUTOCONNECT_RETRY_SEC = 5;
    zce::time_value connect_all_internal(AUTOCONNECT_RETRY_SEC, 0);

    add_app_timer(connect_all_internal, &ZERG_TIMER_ID[0]);

    //主动重现链接的间隔时间
    const time_t RECORD_MONITOR_SEC = 60;
    zce::time_value monitor_internal(RECORD_MONITOR_SEC, 0);

    add_app_timer(monitor_internal, &ZERG_TIMER_ID[1]);
}

app_timer::~app_timer()
{
}

//
int app_timer::timer_timeout(const zce::time_value& time_now,
                             const void* act)
{
    //等到当前的时间
    Server_Timer::timer_timeout(time_now, act);

    //心跳数据
    const int zerg_timeid = *(static_cast<const int*>(act));
    if (ZERG_TIMER_ID[0] == zerg_timeid)
    {
        svc_tcp::reconnect_allserver();
    }
    else if (ZERG_TIMER_ID[1] == zerg_timeid)
    {
        //Buffer_Storage::instance()->monitor();
    }
    return 0;
}
}