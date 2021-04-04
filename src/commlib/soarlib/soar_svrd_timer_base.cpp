
#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_stat_define.h"
#include "soar_stat_monitor.h"
#include "soar_svrd_cfg_base.h"
#include "soar_svrd_app_base.h"
#include "soar_svrd_timer_base.h"

ZCE_Time_Value Server_Timer_Base::now_time_ = zce::gettimeofday();

//��ʱ��ID,����New����,����
const int Server_Timer_Base::SERVER_TIMER_ID[] =
{
    0x201208,                      //����ID
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


//��ʼ�������ϣ������APP�Ķ�ʱ�����ߵ����������ȣ����ڵ����������ǰ���
int Server_Timer_Base::initialize(ZCE_Timer_Queue_Base *queue)
{
    stat_monitor_ = Soar_Stat_Monitor::instance();

    timer_queue(queue);

    now_time_ = zce::gettimeofday();
    last_check_ = now_time_.sec();

    timer_queue()->schedule_timer(this,
                                  &(SERVER_TIMER_ID[0]),
                                  ZCE_Time_Value::ZERO_TIME_VALUE,
                                  heart_precision_);

    //�������ð���Щ��ʱ��Ҳ����
    for (size_t i = 0; i < zan_timer_num_; ++i)
    {
        timer_queue()->schedule_timer(this,
                                      zan_timer_act_[i],
                                      zan_timer_internal_[i],
                                      zan_timer_internal_[i]);
    }
    return 0;
}

//��ʱ����
int Server_Timer_Base::timer_timeout(const ZCE_Time_Value &now_time,
                                     const void *act )
{
    ZCE_UNUSED_ARG(act);

    //��¼��ǰʱ�䣬
    now_time_ = now_time;


    const int timeid = *(static_cast<const int *>(act));
    if (SERVER_TIMER_ID[0] == timeid)
    {
        ++heartbeat_counter_;
        heart_total_mesc_ = heartbeat_counter_ * heart_precision_.total_msec();

        // ������
        check_monitor(now_time);
    }

    return 0;
}


//��ʱ���ر�
int Server_Timer_Base::timer_close()
{
    //ȡ����Event Handler��صĶ�ʱ��
    timer_queue()->cancel_timer(this);

    return 0;
}


// ������Ƿ�ʱ
void Server_Timer_Base::check_monitor(const ZCE_Time_Value &now_time)
{
    time_t now_sec = now_time.sec();

    // ����Ƿ���ͬһ��������
    if (now_sec / zce::FIVE_MINUTE_SECONDS != last_check_ / zce::FIVE_MINUTE_SECONDS)
    {
        // ��ӽ��̴����
        report_status();
        stat_monitor_->check_overtime(now_sec);

        if (now_sec - last_check_ > zce::FIVE_MINUTE_SECONDS)
        {
            ZCE_LOG(RS_ERROR, "check monitor more than five minutes:real_second=%d %d",
                    now_sec - last_check_, now_sec % zce::FIVE_MINUTE_SECONDS);
        }

        // ������Ϊ�˱�֤ÿ�μ����5������
        last_check_ = now_sec - (now_sec % zce::FIVE_MINUTE_SECONDS);
    }
}


// ϵͳ������״̬����
void Server_Timer_Base::report_status()
{
    // �ϱ����̴��״̬
    stat_monitor_->increase_once(COMM_STAT_APP_ALIVE, 0, 0);
    Soar_Svrd_Appliction *svrd_app = Soar_Svrd_Appliction::instance();

    {
        // ��ȡ������״̬��Ϣ
        int ret = svrd_app->watch_dog_status(false);

        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "watch dog get sys/app status failed, ret = %d", ret);
        }
        else
        {
            // �ϱ��������
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


//����������ʱ���Ľ��ȣ�Ĭ����0.5sһ�Σ�������ò�������initializeǰ��������
void Server_Timer_Base::set_heart_precision(const ZCE_Time_Value &precision)
{
    heart_precision_ = precision;
}



//����һ��APP�Ķ�ʱ��
void Server_Timer_Base::add_app_timer(const ZCE_Time_Value &interval, const void *act)
{
    ZCE_ASSERT(zan_timer_num_ + 1 >= MAX_APP_TIMER_NUMBER);

    zan_timer_internal_[zan_timer_num_] = interval;
    zan_timer_act_[zan_timer_num_] = act;
    ++zan_timer_num_;


}

