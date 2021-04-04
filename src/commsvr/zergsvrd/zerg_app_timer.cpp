#include "zerg_predefine.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_comm_manager.h"
#include "zerg_application.h"
#include "zerg_app_timer.h"


/****************************************************************************************************
class  Zerg_App_Timer_Handler
****************************************************************************************************/

///ZERG��������ʱ��ID,
const int Zerg_App_Timer_Handler::ZERG_TIMER_ID[] =
{
    0x101,
    0x102,
};

//
Zerg_App_Timer_Handler::Zerg_App_Timer_Handler():
    Server_Timer_Base()
{

    //reload�������ӵļ��ʱ��, 300s
    const  int     AUTOCONNECT_RELOAD_INTERVAL = 300000;


    srand(static_cast<unsigned int>(zce::pthread_self()));
    random_reload_point_ = rand() % AUTOCONNECT_RELOAD_INTERVAL;

    //�����������ӵļ��ʱ��
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
    //�ȵ���ǰ��ʱ��
    Server_Timer_Base::timer_timeout(time_now, act);

    //��������
    const int zerg_timeid = *(static_cast<const int *>(act));
    if (ZERG_TIMER_ID[0] == zerg_timeid)
    {
        TCP_Svc_Handler::reconnect_allserver();

    }
    return 0;
}


