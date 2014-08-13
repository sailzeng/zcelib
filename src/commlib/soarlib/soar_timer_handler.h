/// @file       soar_timer_handler.h
/// @date       2012/08/13 10:21
///
/// @author     yunfeiyang
///
/// @brief      框架定时器处理类
///
#ifndef SOARING_LIB_TIMER_HANDLER_H_
#define SOARING_LIB_TIMER_HANDLER_H_

class Comm_Time_Provider;
class Comm_Stat_Monitor;
class Comm_Svrd_Appliction;
class Comm_Timer_Handler : public ZCE_Timer_Handler
{
public:
    Comm_Timer_Handler(ZCE_Timer_Queue *queue);
    ~Comm_Timer_Handler();

protected:
    // 定时处理监控数据
    virtual int handle_timeout(const ZCE_Time_Value &now_time,
                               const void *act = 0);

public:
    // 处理配置重新加载事件
    void notify_reload();

    // update时间组件的时间
    void update_time(const ZCE_Time_Value &now_time);

    // 检查监控数据
    void check_monitor(const ZCE_Time_Value &now_time);

    // 系统及进程状态采样
    void report_status();

    
    ///
    /// @fn     init
    /// @brief  初始化
    ///
    /// @param  game_id 如果进程按游戏部署，则传入游戏id, 否则传入0
    /// @param  is_app 是app进程还是zerg进程
    /// @param  toolkit server监控实例
    ///
    /// @return int
    ///
    int init(unsigned int game_id, bool is_app, ZCE_Server_Base *toolkit);

private:
    // 时间提供实例
    Comm_Time_Provider *time_provider_;
    // 监控
    Comm_Stat_Monitor *stat_monitor_;
    // App对象
    ZCE_Server_Base *svrd_app_;

    time_t last_check_;

    // 游戏ID
    unsigned int game_id_;

    // 服务器classify_id
    uint32_t classify_id_;

};

#endif // SOARING_LIB_TIMER_HANDLER_H_
