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

//
app_timer::app_timer() :
    server_timer()
{
    //主动重现链接的间隔时间
    const time_t AUTOCONNECT_RETRY_SEC = 5;
    zce::time_value connect_all_internal(AUTOCONNECT_RETRY_SEC, 0);

    add_app_timer(connect_all_internal);

    //主动重现链接的间隔时间
    const time_t RECORD_MONITOR_SEC = 60;
    zce::time_value monitor_internal(RECORD_MONITOR_SEC, 0);

    add_app_timer(monitor_internal);
}

app_timer::~app_timer()
{
}

//
int app_timer::timer_timeout(const zce::time_value& time_now,
                             int timer_id)
{
    //等到当前的时间
    server_timer::timer_timeout(time_now, timer_id);

    //心跳数据
    if (zan_timer_act_[0] == timer_id)
    {
        svc_tcp::reconnect_allserver();
    }
    else if (zan_timer_act_[1] == timer_id)
    {
        //Buffer_Storage::instance()->monitor();
    }
    return 0;
}
}