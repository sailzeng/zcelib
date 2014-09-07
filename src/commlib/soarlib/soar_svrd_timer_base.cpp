
#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_stat_define.h"
#include "soar_stat_monitor.h"
#include "soar_svrd_cfg_base.h"
#include "soar_svrd_app_base.h"
#include "soar_svrd_timer_base.h"

ZCE_Time_Value Server_Timer_Base::now_time_ = ZCE_LIB::gettimeofday();

//定时器ID,避免New传递,回收
const int Server_Timer_Base::SERVER_TIMER_ID[] =
{
    0x1,                      //心跳ID
    0x101,                    //ZAN1
    0x102,                    //ZAN2
    0x103,                    //ZAN3
};


//
Server_Timer_Base::Server_Timer_Base() : 
    ZCE_Timer_Handler()
{
}

Server_Timer_Base::~Server_Timer_Base()
{
    stat_monitor_ = NULL;
}


//初始化，如果希望增加APP的定时器或者调整心跳进度，请在调用这个函数前完成
int Server_Timer_Base::initialize(ZCE_Timer_Queue *queue)
{
    stat_monitor_ = Comm_Stat_Monitor::instance();

    timer_queue(queue);

    now_time_ = ZCE_LIB::gettimeofday();

    timer_queue()->schedule_timer(this,
        &(SERVER_TIMER_ID[0]),
        ZCE_Time_Value::ZERO_TIME_VALUE,
        heart_precision_);
    
    last_check_ = time(NULL);

    return SOAR_RET::SOAR_RET_SUCC;
}

//超时处理
int Server_Timer_Base::timer_timeout(const ZCE_Time_Value &now_time,
                                       const void *act /*= 0*/)
{
    ZCE_UNUSED_ARG(act);

    //记录当前时间，
    now_time_ = now_time;


    const int timeid = *(static_cast<const int *>(act));
    if (SERVER_TIMER_ID[0] == timeid)
    {
        ++heartbeat_counter_;
        heart_total_mesc_ = heartbeat_counter_ * heart_precision_.total_msec();

        // 处理监控
        check_monitor(now_time);

    }
    

    return SOAR_RET::SOAR_RET_SUCC;
}


//定时器关闭
int Server_Timer_Base::timer_close()
{
    //取消此Event Handler相关的定时器
    timer_queue()->cancel_timer(this);

    return 0;
}


// 检查监控是否超时
void Server_Timer_Base::check_monitor(const ZCE_Time_Value &now_time)
{
    time_t now_sec = now_time.sec();

    // 检查是否在同一个周期内
    if (now_sec / FIVE_MINUTE_SECONDS != last_check_ / FIVE_MINUTE_SECONDS)
    {
        // 添加进程存活监控
        report_status();
        stat_monitor_->check_overtime(now_sec);

        if (now_sec - last_check_ > FIVE_MINUTE_SECONDS)
        {
            ZLOG_ERROR("check monitor more than five minutes:real_second=%d %d",
                       now_sec - last_check_, now_sec % FIVE_MINUTE_SECONDS);
        }

        // 这里是为了保证每次检查在5分钟整
        last_check_ = now_sec - (now_sec % FIVE_MINUTE_SECONDS);
    }
}





// 系统及进程状态采样
void Server_Timer_Base::report_status()
{
    // 上报进程存活状态
    stat_monitor_->increase_once(COMM_STAT_APP_ALIVE, 0,0);
    Comm_Svrd_Appliction *svrd_app = Comm_Svrd_Appliction::instance();

    {
        // 获取服务器状态信息
        int ret = svrd_app->watch_dog_status(false);

        if (ret != 0)
        {
            ZLOG_ERROR("watch dog get sys/app status failed, ret = %d", ret);
        }
        else
        {
            // 上报监控数据
            stat_monitor_->set_by_statid(COMM_STAT_SYS_CPU_RATIO,
                0, 
                0,
                svrd_app->system_cpu_ratio_);

            stat_monitor_->set_by_statid(COMM_STAT_PROCESS_CPU_RATIO,
                0, 
                0, 
                svrd_app->process_cpu_ratio_);

            stat_monitor_->set_by_statid(COMM_STAT_SYS_MEM_USE_RATIO,
                0, 
                0,
                svrd_app->mem_use_ratio_);

            stat_monitor_->set_by_statid(COMM_STAT_AVAILABLE_MEM_SIZE,
                0, 
                0, 
                svrd_app->can_use_size_);

        }
    }
}







