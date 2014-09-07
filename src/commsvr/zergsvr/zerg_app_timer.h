
#ifndef ZERG_APPLICATION_TIMER_H_
#define ZERG_APPLICATION_TIMER_H_



/*!
* @brief      ZERG服务的定时器，定时器用于各种处理
*             
* @note       
*/
class  Zerg_App_Timer_Handler : public  Server_Timer_Base
{
    
public:
    //
    Zerg_App_Timer_Handler();
protected:
    //自己管理自己的清理
    ~Zerg_App_Timer_Handler();

    ///初始化，
    int initialize(ZCE_Timer_Queue *queue);

    ///定时器触发
    virtual int timer_timeout(const ZCE_Time_Value &time, const void *arg);

    

public:

    //主动重现链接的间隔时间，单位毫秒
    static const uint32_t AUTOCONNECT_RETRY_MESC = 8000;

    //发包频率检查的间隔时间，单位毫秒
    static const uint32_t SEND_FRAME_CHECK_MSEC = 10000;

    //reload主动连接的间隔时间, 300s
    static const  int      AUTOCONNECT_RELOAD_INTERVAL = 300000;
    //自动连接有连接异常时会触发一次reload操作重新拉取ip, 一定时间内只触发一次, 120s
    static const unsigned int  RETRY_TRIGGER_RELOAD_INTERVAL = 120000;

protected:
    
    ///发包频率检查间隔的心跳次数，根据心跳精度计算
    uint64_t connect_retry_heart_;

    ///发包频率检查间隔的心跳次数
    uint64_t send_check_heart_;


    //定时器触发的随机偏移, 避免所有服务器同时触发
    unsigned int random_reload_point_;


};

#endif //ZERG_APPLICATION_TIMER_H_

