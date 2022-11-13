#include "ogre/predefine.h"
#include "ogre/app_timer.h"
#include "ogre/svc_tcp.h"
#include "ogre/application.h"

namespace ogre
{
//
app_timer::app_timer()
{
    //主动重现链接的间隔时间
    const time_t AUTOCONNECT_RETRY_SEC = 5;
    zce::time_value connect_all_internal(AUTOCONNECT_RETRY_SEC, 0);

    add_app_timer(connect_all_internal);
}

app_timer::~app_timer()
{
}

//
int app_timer::timer_timeout(const zce::time_value& time_now, int time_id)
{
    //等到当前的时间
    server_timer::timer_timeout(time_now, time_id);

    //处理一个错误发送数据
    if (zan_timer_act_[0] == time_id)
    {
        svc_tcp::connect_all_server();
    }
    else
    {
    }

    return 0;
}
}