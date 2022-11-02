#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/stat/define.h"
#include "soar/stat/monitor.h"
#include "soar/svrd/cfg_base.h"
#include "soar/svrd/app_bus.h"
#include "soar/svrd/timer_base.h"

namespace soar
{
zce::time_value server_timer::now_time_ = zce::gettimeofday();

//
server_timer::server_timer() :
    zce::timer_handler()
{
}

server_timer::~server_timer()
{
    stat_monitor_ = nullptr;
}

//初始化，如果希望增加APP的定时器或者调整心跳进度，请在调用这个函数前完成
int server_timer::initialize(zce::timer_queue* queue)
{
    stat_monitor_ = soar::stat_monitor::instance();

    timer_queue(queue);

    now_time_ = zce::gettimeofday();
    last_check_ = now_time_.sec();

    timer_queue()->schedule_timer(this,
                                  server_timer_id_,
                                  zce::time_value::ZERO_TIME_VALUE,
                                  heart_precision_);

    //根据设置把这些定时器也开启
    for (size_t i = 0; i < zan_timer_num_; ++i)
    {
        timer_queue()->schedule_timer(this,
                                      zan_timer_act_[i],
                                      zan_timer_internal_[i],
                                      zan_timer_internal_[i]);
    }
    return 0;
}

//超时处理
int server_timer::timer_timeout(const zce::time_value& now_time,
                                int timer_id)
{
    //记录当前时间，
    now_time_ = now_time;

    if (server_timer_id_ == timer_id)
    {
        ++heartbeat_counter_;
        heart_total_mesc_ = heartbeat_counter_ * heart_precision_.total_msec();

        // 处理监控
        check_monitor(now_time);
    }

    return 0;
}

//定时器关闭
int server_timer::timer_close()
{
    //取消此Event Handler相关的定时器
    timer_queue()->cancel_timer(this);

    return 0;
}

// 检查监控是否超时
void server_timer::check_monitor(const zce::time_value& now_time)
{
    time_t now_sec = now_time.sec();

    // 检查是否在同一个周期内
    if (now_sec / zce::FIVE_MINUTE_SECONDS != last_check_ / zce::FIVE_MINUTE_SECONDS)
    {
        // 添加进程存活监控
        report_status();
        stat_monitor_->check_overtime(now_sec);

        if (now_sec - last_check_ > zce::FIVE_MINUTE_SECONDS)
        {
            ZCE_LOG(RS_ERROR, "check monitor more than five minutes:real_second=%d %d",
                    now_sec - last_check_, now_sec % zce::FIVE_MINUTE_SECONDS);
        }

        // 这里是为了保证每次检查在5分钟整
        last_check_ = now_sec - (now_sec % zce::FIVE_MINUTE_SECONDS);
    }
}

// 系统及进程状态采样
void server_timer::report_status()
{
    // 上报进程存活状态
    stat_monitor_->add_one(COMM_STAT_APP_ALIVE, 0, 0);
    soar::app_buspipe* svrd_app = soar::app_buspipe::instance();

    {
        // 获取服务器状态信息
        int ret = svrd_app->watch_dog_status(false);

        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "watch dog get sys/app status failed, ret = %d", ret);
        }
        else
        {
            // 上报监控数据
            stat_monitor_->set_counter(COMM_STAT_SYS_CPU_RATIO,
                                       0,
                                       0,
                                       svrd_app->system_cpu_ratio_);

            stat_monitor_->set_counter(COMM_STAT_PROCESS_CPU_RATIO,
                                       0,
                                       0,
                                       svrd_app->process_cpu_ratio_);

            stat_monitor_->set_counter(COMM_STAT_SYS_MEM_USE_RATIO,
                                       0,
                                       0,
                                       svrd_app->mem_use_ratio_);

            stat_monitor_->set_counter(COMM_STAT_AVAILABLE_MEM_SIZE,
                                       0,
                                       0,
                                       svrd_app->can_use_size_);
        }
    }
}

//设置心跳定时器的进度，默认是0.5s一次，如果觉得不够，在initialize前重新设置
void server_timer::set_heart_precision(const zce::time_value& precision)
{
    heart_precision_ = precision;
}

//增加一个APP的定时器
void server_timer::add_app_timer(const zce::time_value& interval)
{
    ZCE_ASSERT(zan_timer_num_ + 1 <= MAX_APP_TIMER_NUMBER);

    zan_timer_internal_[zan_timer_num_] = interval;
    zan_timer_act_[zan_timer_num_] = 0;
    ++zan_timer_num_;
}
}