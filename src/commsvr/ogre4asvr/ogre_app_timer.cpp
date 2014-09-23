#include "ogre_predefine.h"
#include "ogre_app_handler.h"
#include "ogre_tcp_ctrl_handler.h"
#include "ogre_application.h"

/****************************************************************************************************
class  Ogre_App_Handler
****************************************************************************************************/
//定时器ID,避免New传递,回收
const int Ogre_App_Handler::OGRE_APP_TIME_ID[] =
{
    0x1,                    //错误发送ID
    0x2                     //心跳ID
};

Ogre_App_Handler *Ogre_App_Handler::instance_ = NULL;

//
Ogre_App_Handler::Ogre_App_Handler()
{
}
Ogre_App_Handler::~Ogre_App_Handler()
{
}

//
int Ogre_App_Handler::handle_timeout(const ZCE_Time_Value &/*time*/, const void *arg)
{
    const int timeid = *(static_cast<const int *>(arg));

    //处理一个错误发送数据
    if (OGRE_APP_TIME_ID[0] == timeid)
    {
        //Ogre_TCP_Svc_Handler::GetErrorPipeDataToWrite();
    }
    else
    {
    }

    return 0;
}

//设置错误重试的定时器
int Ogre_App_Handler::SetErrorRetryTimer()
{
    ZCE_Time_Value delay(RETRY_TIME_INTERVAL, 0);
    timer_queue()->schedule_timer (this,
                                   &(OGRE_APP_TIME_ID[0]),
                                   delay);

    return 0;
}

//单子实例获得函数
Ogre_App_Handler *Ogre_App_Handler::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Ogre_App_Handler();
    }

    return instance_;
}

//单子实例清理函数
void Ogre_App_Handler::clean_instance()
{
    if (instance_ != NULL)
    {
        delete instance_;
        instance_ = NULL;
    }
}

