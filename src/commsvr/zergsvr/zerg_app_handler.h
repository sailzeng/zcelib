
#ifndef ZERG_APPLICATION_HANDLER_H_
#define ZERG_APPLICATION_HANDLER_H_

/****************************************************************************************************
class  Zerg_App_Timer_Handler
****************************************************************************************************/
class  Zerg_App_Timer_Handler : public  Server_Timer_Base
{

public:
    //定时器ID,避免New传递,回收
    static const  int      ZERGAPP_TIME_ID[];

    //心跳的时间间隔
    static const  int      HEARTBEAT_TIME_INTERVAL = 1;
    //主动重现链接的间隔时间，5s,
    static const  int      AUTOCONNECT_RETRY_INTERVAL = 5;
    //reload主动连接的间隔时间, 300s
    static const  int      AUTOCONNECT_RELOAD_INTERVAL = 300;
    //自动连接有连接异常时会触发一次reload操作重新拉取ip, 一定时间内只触发一次, 120s
    static const unsigned int  RETRY_TRIGGER_RELOAD_INTERVAL = 120;

    //发包频率检查
    static const unsigned int SEND_FRAME_CHECK_INTERVAL = 10;

    //当前的时间
    static ZCE_Time_Value  now_time_;

protected:
    //心跳计数器
    unsigned int           heartbeat_counter_;

    //定时器触发的随机偏移, 避免所有服务器同时触发
    unsigned int           random_reload_point_;

    //自动连接有连接异常时会触发一次reload操作重新拉取ip, 一定时间内只触发一次 
    unsigned int           last_trigger_reload_time_;

public:
    //
    Zerg_App_Timer_Handler(ZCE_Timer_Queue *time_queue);
protected:
    //自己管理自己的清理
    ~Zerg_App_Timer_Handler();

    //
    virtual int handle_timeout(const ZCE_Time_Value &time, const void *arg);
    //
    virtual int timer_close();

};

#endif //_ZERG_APPLICATION_HANDLER_H_

