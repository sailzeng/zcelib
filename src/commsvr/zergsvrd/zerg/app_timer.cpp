#include "zerg/predefine.h"
#include "zerg/tcp_ctrl_handler.h"
#include "zerg/comm_manager.h"
#include "zerg/application.h"
#include "zerg/app_timer.h"

/****************************************************************************************************
class  Zerg_App_Timer
****************************************************************************************************/

///ZERG服务器定时器ID,
const int Zerg_App_Timer::ZERG_TIMER_ID[] =
{
    0x101,
    0x102,
};

//
Zerg_App_Timer::Zerg_App_Timer():
    Server_Timer_Base()
{
    //reload主动连接的间隔时间, 300s
    const  int     AUTOCONNECT_RELOAD_INTERVAL = 300000;

    srand(static_cast<unsigned int>(zce::pthread_self()));
    random_reload_point_ = rand() % AUTOCONNECT_RELOAD_INTERVAL;

    //主动重现链接的间隔时间
    const time_t AUTOCONNECT_RETRY_SEC = 5;
    zce::Time_Value connect_all_internal(AUTOCONNECT_RETRY_SEC,0);

    add_app_timer(connect_all_internal,&ZERG_TIMER_ID[0]);
}

Zerg_App_Timer::~Zerg_App_Timer()
{
}

//
int Zerg_App_Timer::timer_timeout(const zce::Time_Value &time_now,const void *act)
{
    //等到当前的时间
    Server_Timer_Base::timer_timeout(time_now,act);

    //心跳数据
    const int zerg_timeid = *(static_cast<const int *>(act));
    if (ZERG_TIMER_ID[0] == zerg_timeid)
    {
        TCP_Svc_Handler::reconnect_allserver();
    }
    return 0;
}