
#ifndef SOARING_LIB_TIMER_HANDLER_H_
#define SOARING_LIB_TIMER_HANDLER_H_


class Comm_Stat_Monitor;
class Server_Config_Base;


/*!
* @brief      服务器框架的定时器处理类
*             可以从其得到当前的时钟
* @note       
*/
class Server_Timer_Base : public ZCE_Timer_Handler
{
    
    friend class Comm_Svrd_Appliction;
public:

    ///构造函数,因为框架的设计构造的时候不初始化timer queue，
    Server_Timer_Base();
    ///析构函数
    ~Server_Timer_Base();

protected:
    
    // 检查监控数据
    void check_monitor(const ZCE_Time_Value &now_time);

    // 系统及进程状态采样
    void report_status();

    ///取得当前的时间，用于对时间精度要求不高的场合
    ZCE_Time_Value gettimeofday();


protected:

    ///初始化
    virtual int initialize(ZCE_Timer_Queue *queue,
        Server_Config_Base *config_base);


    /// 定时处理监控数据
    virtual int timer_timeout(const ZCE_Time_Value &now_time,
        const void *act = 0);

    ///定时器关闭
    virtual int timer_close();

protected:
       
    ///服务器定时器ID
    static const  int  SERVER_TIMER_ID[];

    ///服务器每100 mesc 心跳一次
    static const time_t HEARTBEAT_INTERVAL_MSEC = 100;

public:
    ///当前时间
    static ZCE_Time_Value now_time_;

protected:

    ///
    ZCE_Time_Value heart_precision_;

    ///心跳计数器
    uint64_t  heartbeat_counter_;

    ///从开始心跳到现在的毫秒数，这个数值是通过heartbeat_counter_和精度得到的，
    ///并不准确，用于各种初略计算
    uint64_t  heart_total_mesc_;

    // 监控
    Comm_Stat_Monitor *stat_monitor_;

    time_t last_check_;

    

};

#endif // SOARING_LIB_TIMER_HANDLER_H_
