#include "wormhole_predefine.h"
#include "wormhole_application.h"
#include "wormhole_app_timer.h"
#include "wormhole_stat_define.h"

//
ArbiterAppliction *ArbiterAppliction::instance_ = NULL;

/****************************************************************************************************
class  Arbiter_Appliction
****************************************************************************************************/
ArbiterAppliction::ArbiterAppliction():
    interface_proxy_(NULL)
{
    conf_ = new conf_proxysvr::CONFIG();
}

ArbiterAppliction::~ArbiterAppliction()
{
    if (conf_)
    {
        delete conf_;
        conf_ = NULL;
    }

    if (interface_proxy_)
    {
        delete interface_proxy_;
        interface_proxy_ = NULL;
    }
}

int ArbiterAppliction::init()
{
    ZLOG_INFO("Arbiter_Appliction::init begin. ");

    int ret = 0;

    // 注册定时器
    ZEN_Timer_Queue *timer_queue = ZEN_Timer_Queue::instance();
    ZEN_Time_Value delay;
    ZEN_Time_Value interval;

    delay.set(0, TIMER_INTERVAL);
    interval.set(0, TIMER_INTERVAL);
    timer_queue->schedule_timer(new ArbiterTimerHandler(timer_queue),
                                NULL,
                                delay,
                                interval);

    // 初始化数据转发模式
    interface_proxy_ = InterfaceProxyProcess::CreatePorxyFactory(
        static_cast<InterfaceProxyProcess::PROXY_TYPE>(conf_->proxy_type_));
    ZEN_ASSERT(interface_proxy_);

    ret = interface_proxy_->get_proxy_config(conf_);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    ret = interface_proxy_->InitProxyInstance();

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    // 账单写到网路用的Transaction_Manager，需要先初始化下
    Transaction_Manager *p_trans_mgr_ = new Transaction_Manager();
    p_trans_mgr_->initialize(Comm_Svrd_Config::instance()->framework_config_.trans_info_.trans_cmd_num_,
                             Comm_Svrd_Config::instance()->framework_config_.trans_info_.trans_num_,
                             self_services_id_,
                             ZEN_Timer_Queue::instance(),
                             Zerg_MMAP_BusPipe::instance());
    Transaction_Manager::instance(p_trans_mgr_);

    // 初始化统计MMAP文件
    ret = Comm_Stat_Monitor::instance()->initialize(false,
                                                    Comm_Svrd_Config::instance()->self_svr_id_,
                                                    ARBITER_FRATURE_NUM,
                                                    STR_ARBITER_STAT,
                                                    false);

    if (ret != 0)
    {
        ZLOG_ERROR("Zen_Server_Status init fail. ret=%d", ret);
        return ret;
    }

    // 设置日志优先级
    set_log_priority(ZEN_LogTrace_Basic::log_priorities(conf_->log_level_));

    ZLOG_INFO("Arbiter_Appliction::init end. ");

    return TSS_RET::TSS_RET_SUCC;
}

//
void ArbiterAppliction::exit()
{
    ZLOG_INFO("Arbiter_Appliction::exit. ");
}

int ArbiterAppliction::reload()
{
    int ret = 0;

    ZLOG_INFO("Arbiter_Appliction::reload start");

    // 重新初始化数据转发模式
    delete interface_proxy_;
    interface_proxy_ = InterfaceProxyProcess::CreatePorxyFactory(
        static_cast<InterfaceProxyProcess::PROXY_TYPE>(conf_->proxy_type_));
    ZEN_ASSERT(interface_proxy_);

    ret = interface_proxy_->get_proxy_config(conf_);

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    ret = interface_proxy_->InitProxyInstance();

    if (ret != TSS_RET::TSS_RET_SUCC)
    {
        return ret;
    }

    // 设置日志优先级
    set_log_priority(ZEN_LogTrace_Basic::log_priorities(conf_->log_level_));

    ZLOG_INFO("Arbiter_Appliction::reload end. ");

    return TSS_RET::TSS_RET_SUCC;
}

int ArbiterAppliction::process_recv_appframe(Comm_App_Frame *recv_frame)
{
    return interface_proxy_->process_proxy(recv_frame);
}

// 单子实例函数
ArbiterAppliction *ArbiterAppliction::instance()
{
    if (NULL == instance_)
    {
        instance_ = new ArbiterAppliction();
    }

    return instance_;
}

// 清理单子实例
void ArbiterAppliction::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
}

int ArbiterAppliction::load_app_conf()
{
    CommXMLConfig xml_config;
    int ret = xml_config.load(Comm_Svrd_Config::instance()->app_cfg_file_.c_str(), conf_);

    if (ret != 0)
    {
        ZLOG_ERROR("[%s], Arbiter_Appliction app load config fail, ret=%d",
            __ZEN_FUNCTION__, ret);
        return ret;
    }

    return TSS_RET::TSS_RET_SUCC;
}


int ArbiterAppliction::merge_app_cfg_file()
{
    Comm_Svrd_Config *svd_config = Comm_Svrd_Config::instance();
    int ret = TSS_RET::TSS_RET_SUCC;

    ret = merge_xml_file<conf_proxysvr::CONFIG>(svd_config->app_cfg_file_.c_str());
    return ret;
}


