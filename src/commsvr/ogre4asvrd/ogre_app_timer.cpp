#include "ogre_predefine.h"
#include "ogre_app_timer.h"
#include "ogre_tcp_ctrl_handler.h"
#include "ogre_application.h"

/****************************************************************************************************
class  Ogre_App_Timer_Handler
****************************************************************************************************/
//定时器ID,避免New传递,回收
const int Ogre_App_Timer_Handler::OGRE_APP_TIME_ID[] =
{
    0x101,                    //重连服务器的ID
    0x102                     //
};


//
Ogre_App_Timer_Handler::Ogre_App_Timer_Handler()
{
    //主动重现链接的间隔时间
    const time_t AUTOCONNECT_RETRY_SEC = 5;
    ZCE_Time_Value connect_all_internal(AUTOCONNECT_RETRY_SEC, 0);

    add_app_timer(connect_all_internal, &OGRE_APP_TIME_ID[0]);
}

Ogre_App_Timer_Handler::~Ogre_App_Timer_Handler()
{
}

//
int Ogre_App_Timer_Handler::timer_timeout(const ZCE_Time_Value &time_now, const void *arg)
{
    //等到当前的时间
    Server_Timer_Base::timer_timeout(time_now, arg);

    const int timeid = *(static_cast<const int *>(arg));

    //处理一个错误发送数据
    if (OGRE_APP_TIME_ID[0] == timeid)
    {
        Ogre_TCP_Svc_Handler::connect_all_server();
    }
    else
    {
    }

    return 0;
}

