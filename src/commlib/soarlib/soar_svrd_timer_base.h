
#ifndef SOARING_LIB_TIMER_HANDLER_H_
#define SOARING_LIB_TIMER_HANDLER_H_


class Comm_Stat_Monitor;
class Comm_Svrd_Appliction;


/*!
* @brief      服务器框架的定时器处理类
*             可以从其得到当前的时钟
* @note       
*/
class Server_Timer_Base : public ZCE_Timer_Handler
{
    

public:
    //构造函数
    Server_Timer_Base(ZCE_Timer_Queue *queue);
    ~Server_Timer_Base();



protected:

    ///初始化
    int initialize();


    // 检查监控数据
    void check_monitor(const ZCE_Time_Value &now_time);

    // 系统及进程状态采样
    void report_status();

    ///取得当前的时间，用于对时间精度要求不高的场合
    ZCE_Time_Value gettimeofday();


protected:

    // 定时处理监控数据
    virtual int handle_timeout(const ZCE_Time_Value &now_time,
        const void *act = 0);


protected:
       
    ///服务器定时器ID
    static const  int      SERVER_TIMER_ID[];

    ///服务器每100 mesc 心跳一次
    static const time_t HEARTBEAT_INTERVAL_MSEC = 100;

    ///当前时间
    static ZCE_Time_Value cur_time_;

protected:

    // 监控
    Comm_Stat_Monitor *stat_monitor_;

    time_t last_check_;

    

};

#endif // SOARING_LIB_TIMER_HANDLER_H_
