

#ifndef OGRE_APPLICATION_HANDLER_H_
#define OGRE_APPLICATION_HANDLER_H_

/****************************************************************************************************
class  Ogre_App_Handler APP的定时器Handler处理类
****************************************************************************************************/
class  Ogre_App_Handler: public  ZCE_Timer_Handler
{

public:
    //
    Ogre_App_Handler();
protected:
    ~Ogre_App_Handler();

    //
    virtual int handle_timeout(const ZCE_Time_Value &time, const void *arg);

public:
    //设置错误重试的定时器
    int SetErrorRetryTimer();

public:
    //
    static Ogre_App_Handler *instance();
    //
    static void clean_instance();

public:
    //定时器ID,避免New传递,回收
    static const  int      OGRE_APP_TIME_ID[];
    //
    static const  int      STAT_TIME_INTERVAL = 30;
    //N秒以后尝试一次重新发送
    static const  int      RETRY_TIME_INTERVAL = 90;

protected:

    //单子实例指针
    static Ogre_App_Handler   *instance_;

};

#endif //_OGRE_APPLICATION_HANDLER_H_

