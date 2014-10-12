#include "zerg_predefine.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_comm_manager.h"
#include "zerg_application.h"
#include "zerg_app_timer.h"


/****************************************************************************************************
class  Zerg_App_Timer_Handler
****************************************************************************************************/


//
Zerg_App_Timer_Handler::Zerg_App_Timer_Handler():
    Server_Timer_Base(),
    connect_retry_heart_(0),
    send_check_heart_(0)
{
    srand(static_cast<unsigned int>(ZCE_LIB::pthread_self()));
    random_reload_point_ = rand() % AUTOCONNECT_RELOAD_INTERVAL;
}

Zerg_App_Timer_Handler::~Zerg_App_Timer_Handler()
{
}


//
int Zerg_App_Timer_Handler::initialize(ZCE_Timer_Queue *queue)
{
    int ret = 0;
    ret = Server_Timer_Base::initialize(queue);
    if (0 != ret)
    {
        return ret;
    }

    //计算一些触发需要多少个心跳，
    connect_retry_heart_ = AUTOCONNECT_RETRY_MESC / heart_precision_.total_msec() + 1;
    send_check_heart_ = SEND_FRAME_CHECK_MSEC / heart_precision_.total_msec() + 1;

    return 0;
}


//
int Zerg_App_Timer_Handler::timer_timeout(const ZCE_Time_Value &time_now, const void *act)
{
    //等到当前的时间

    Server_Timer_Base::timer_timeout(time_now, act);

    //心跳数据

    if (0 == heartbeat_counter_ % connect_retry_heart_)
    {
        size_t valid = 0;
        size_t succ = 0;
        size_t fail = 0;
        TCP_Svc_Handler::reconnect_allserver(valid, succ, fail);
        if (succ || fail)
        {
        }
    }

    if (0 == heartbeat_counter_ % send_check_heart_)
    {
        Zerg_Comm_Manager::instance()->check_freamcount(static_cast<unsigned int>(now_time_.sec()));
    }

    if (random_reload_point_ == heartbeat_counter_ % AUTOCONNECT_RELOAD_INTERVAL)
    {

    }

    return 0;
}


