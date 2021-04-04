#include "zerg_predefine.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_comm_manager.h"
#include "zerg_application.h"
#include "zerg_app_timer.h"


/****************************************************************************************************
class  Zerg_App_Timer_Handler
****************************************************************************************************/

///ZERG服务器定时器ID,
const int Zerg_App_Timer_Handler::ZERG_TIMER_ID[] =
{
    0x101,
    0x102,
};

//
Zerg_App_Timer_Handler::Zerg_App_Timer_Handler():
    Server_Timer_Base()
{

    //reload主动连接的间隔时间, 300s
    const  int     AUTOCONNECT_RELOAD_INTERVAL = 300000;


    srand(static_cast<unsigned int>(zce::pthread_self()));
    random_reload_point_ = rand() % AUTOCONNECT_RELOAD_INTERVAL;

    //主动重现链接的间隔时间
    const time_t AUTOCONNECT_RETRY_SEC = 5;
    ZCE_Time_Value connect_all_internal(AUTOCONNECT_RETRY_SEC, 0);

    add_app_timer(connect_all_internal, &ZERG_TIMER_ID[0]);
}

Zerg_App_Timer_Handler::~Zerg_App_Timer_Handler()
{
}


//
int Zerg_App_Timer_Handler::timer_timeout(const ZCE_Time_Value &time_now, const void *act)
{
    //等到当前的时间
    Server_Timer_Base::timer_timeout(time_now, act);

    //心跳数据
    const int zerg_timeid = *(static_cast<const int *>(act));
    if (ZERG_TIMER_ID[0] == zerg_timeid)
    {
        TCP_Svc_Handler::reconnect_allserver();

    }
    return 0;
}


