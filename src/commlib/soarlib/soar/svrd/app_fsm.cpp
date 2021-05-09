#include "soar/predefine.h"
#include "soar/fsm/fsm_mgr.h"
#include "soar/svrd/app_fsm.h"
#include "soar/svrd/app_buspipe.h"
#include "soar/svrd/cfg_fsm.h"

//
Comm_SvrdApp_FSM::Comm_SvrdApp_FSM() :
    soar::Svrd_Appliction()
{
}

//
Comm_SvrdApp_FSM::~Comm_SvrdApp_FSM()
{
}

//增加调用register_func_cmd
int Comm_SvrdApp_FSM::app_start(int argc, const char* argv[])
{
    int ret = 0;
    ret = soar::Svrd_Appliction::app_start(argc, argv);

    if (0 != ret)
    {
        return ret;
    }

    Server_Config_FSM* svd_config = dynamic_cast<Server_Config_FSM*>(config_base_);

    //事务管理器的初始化, 自动机不使用notify
    soar::FSM_Manager* p_trans_mgr_ = new soar::FSM_Manager();
    p_trans_mgr_->initialize(zce::Timer_Queue::instance(),
                             svd_config->framework_config_.trans_info_.trans_cmd_num_,
                             svd_config->framework_config_.trans_info_.trans_num_,
                             self_svc_info_,

                             soar::App_BusPipe::instance());
    soar::FSM_Manager::instance(p_trans_mgr_);

    ret = register_trans_cmd();

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] register trans cmd fail. ret=%d", ret);
        return ret;
    }

    return 0;
}

//运行处理,
int Comm_SvrdApp_FSM::app_run()
{
    ZCE_LOG(RS_INFO, "======================================================================================================");
    ZCE_LOG(RS_INFO, "[framework] app %s class [%s] run_instance start.",
            get_app_basename(),
            typeid(*this).name());

    //空闲N次后,调整SELECT的等待时间间隔
    const unsigned int LIGHT_IDLE_SELECT_INTERVAL = 128;
    //空闲N次后,SLEEP的时间间隔
    const unsigned int HEAVY_IDLE_SLEEP_INTERVAL = 10240;

    //microsecond
    // 64位tlinux下idle的时间如果太短会导致cpu过高
    const int LIGHT_IDLE_INTERVAL_MICROSECOND = 10000;
    const int HEAVY_IDLE_INTERVAL_MICROSECOND = 100000;

    soar::FSM_Manager* trans_mgr = soar::FSM_Manager::instance();

    size_t size_io_event = 0, num_timer_expire = 0;

    size_t proc_frame = 0, gen_trans = 0, proc_data_num = 0;
    unsigned int idle = 0;

    zce::Time_Value select_interval(0, 0);

    zce::Timer_Queue* time_queue = zce::Timer_Queue::instance();
    zce::ZCE_Reactor* reactor = zce::ZCE_Reactor::instance();

    for (; app_run_;)
    {
        //处理收到的命令
        trans_mgr->process_pipe_frame(proc_frame, gen_trans);

        //超时
        num_timer_expire = time_queue->expire();

        // IO事件
        size_io_event = 0;
        reactor->handle_events(&select_interval, &size_io_event);

        //如果没有处理任何帧
        if ((proc_frame + num_timer_expire + proc_data_num + size_io_event) <= 0)
        {
            ++idle;
        }
        else
        {
            idle = 0;
        }

        //如果忙，继续干活
        if (idle < LIGHT_IDLE_SELECT_INTERVAL)
        {
            select_interval.usec(0);
            continue;
        }
        //如果空闲很多,休息一下,如果你比较空闲，在这儿SELECT相当于Sleep，
        else if (idle >= HEAVY_IDLE_SLEEP_INTERVAL)
        {
            select_interval.usec(HEAVY_IDLE_INTERVAL_MICROSECOND);
        }
        //else 相当于 else if (idle >= LIGHT_IDLE_SELECT_INTERVAL)
        else
        {
            select_interval.usec(LIGHT_IDLE_INTERVAL_MICROSECOND);
        }
    }

    ZCE_LOG(RS_INFO, "[framework] app %s class [%s] run_instance end.",
            get_app_basename(),
            typeid(*this).name());
    ZCE_LOG(RS_INFO, "======================================================================================================");
    return 0;
}

//退出处理
int Comm_SvrdApp_FSM::app_exit()
{
    int ret = 0;
    soar::FSM_Manager::clean_instance();

    ret = soar::Svrd_Appliction::app_exit();

    if (0 != ret)
    {
        return ret;
    }

    return 0;
}