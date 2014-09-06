/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_svrd_application.cpp
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2008年1月22日
* @brief      终于讲sonicmao的这个类发扬广大了，当然到今天，里面的代码已经基本100%重写了。
*             作为一个进程处理的过程，
*
* @details
*
*
*
* @note
*
*/

#include "soar_predefine.h"
#include "soar_svrd_app_base.h"
#include "soar_zerg_mmappipe.h"
#include "soar_zerg_svc_info.h"
#include "soar_svrd_cfg_fsm.h"
#include "soar_svrd_timer_base.h"
#include "soar_stat_define.h"
#include "soar_stat_monitor.h"


Comm_Svrd_Appliction *Comm_Svrd_Appliction::instance_ = NULL;

Comm_Svrd_Appliction::Comm_Svrd_Appliction() :
    business_id_(INVALID_BUSINESS_ID),
    self_svc_id_(),
    run_as_win_serivces_(false),
    max_msg_num_(1024),
    zerg_mmap_pipe_(NULL),
    timer_base_(NULL),
    config_base_(NULL)
{
    //作者名称
    app_author_ = "TSS Platform Server Dev Team.";
}

Comm_Svrd_Appliction::~Comm_Svrd_Appliction()
{
    delete timer_base_;
    timer_base_ = NULL;

    delete config_base_;
    config_base_ = NULL;
}

//初始化，放入一些基类的指针，
int Comm_Svrd_Appliction::initialize(Server_Config_Base *config_base,
                                     Server_Timer_Base *timer_base)
{
    config_base_ = config_base;
    timer_base_ = timer_base;
    return 0;
}

//获取配置的指针
Server_Config_Base *Comm_Svrd_Appliction::config_instance()
{
    return config_base_;
}


//启动过程的处理
int Comm_Svrd_Appliction::on_start(int argc, const char *argv[])
{

    //Comm_Svrd_Appliction 只可能启动一个实例，所以在这个地方初始化了static指针
    base_instance_ = this;
    int ret = SOAR_RET::SOAR_RET_SUCC;

    //得到APP的名字，去掉路径，后缀的名字
    ret = create_app_name(argv[0]);
    if (0 != ret)
    {
        ZLOG_ERROR("svr create_app_base_name init fail. ret=%d", ret);
        return ret;
    }
    //初始化SOCKET等
    ret = ZCE_Server_Base::socket_init();
    if (ret != 0)
    {
        return ret;
    }

    //忽视信号
    process_signal();

    std::string log_file_prefix = config_base_->app_run_dir_ + "/log/";
    log_file_prefix += app_base_name_;
    log_file_prefix += "_init";

    //先打开日志，记录一段数据，直到日志的启动参数获得
    // 初始化日志用滚动的方式可以保留的天数多点
    ZCE_Trace_LogMsg::instance()->init_size_log(
        log_file_prefix.c_str(),
        false,
        true,
        10 * 1024 * 1024,
        2);


    // 处理启动参数
    ret = config_base_->start_arg(argc, argv);
    if (ret != 0)
    {
        ZLOG_ERROR("svr config start_arg init fail. ret=%d", ret);
        return ret;
    }


#ifdef ZCE_OS_WINDOWS

    if (config_base_->win_install_service_)
    {
        // 安装服务
        ret = win_services_install();
        // 直接退出？
        ::exit(ret);
    }
    if (config_base_->win_uninstall_service_)
    {
        // 卸载服务
        ret = win_services_uninstall();
        // 直接退出？
        ::exit(ret);
    }

#endif

    //我是华丽的分割线
    ZLOG_INFO("======================================================================================================");
    ZLOG_INFO("======================================================================================================");
    ZLOG_INFO("======================================================================================================");
    ZLOG_INFO("[framework] %s start init", app_base_name_.c_str());

    // 切换运行目录
    ret = ZCE_LIB::chdir(config_base_->app_run_dir_.c_str());
    if (ret != 0)
    {
        ZLOG_ERROR("[framework] change run directory to %s fail. err=%d",
                   config_base_->app_run_dir_.c_str(), errno);
        return ret;
    }

    ZLOG_INFO("[framework] change work dir to %s", config_base_->app_run_dir_.c_str());

    // 运行目录写PID File.
    std::string app_path = config_base_->app_run_dir_
                           + "/"
                           + get_app_basename();
    ret = out_pid_file(app_path.c_str(), true);

    if (ret != 0)
    {
        //如果有错误显示错误，如果错误==16，表示可能是PID文件被锁定,
        ZLOG_ERROR("[framework] Create Pid file :%s.pid fail .last error =[%u|%s].",
                   app_path.c_str(), ZCE_LIB::last_error(),
                   strerror(ZCE_LIB::last_error()));

        ZLOG_ERROR("[framework] If last error == 16, could has a same process already run in this directory."
                   "Please check PID file or system processes.");
        return SOAR_RET::ERROR_WRITE_ERROR_PIDFILE;
    }

    // cfgsdk拉取配置
    ///*CfgSvrSdk *cfgsvr_sdk = CfgSvrSdk::instance();
    //ret = cfgsvr_sdk->init();*/

    //if (ret != SOAR_RET::SOAR_RET_SUCC)
    //{
    //    ZLOG_ERROR("[framework] cfgsvrsdk init fail. ret=%d", ret);
    //    return ret;
    //}

    ////启动配置更新线程
    //CfgSvrSdk::instance()->start_task();
    //ZLOG_INFO("[framework] cfgsdk init succ. start task succ");

    // 加载框架配置,由于是虚函数，也会调用到非框架的配置读取
    ret = config_base_->initialize(argc, argv);
    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("[framework] framwork config init fail. ret=%d", ret);
        return ret;
    }

    self_svc_id_ = config_base_->self_svc_id_;
    //取得配置信息后, 需要将启动参数全部配置OK. 以下的assert做强制检查
    ZCE_ASSERT((self_svc_id_.services_type_ != SERVICES_ID::INVALID_SERVICES_TYPE) &&
               (self_svc_id_.services_id_ != SERVICES_ID::INVALID_SERVICES_ID));

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("[framework] %s load app config fail. ret=%d",
                   __ZCE_FUNCTION__, ret);
        return ret;
    }

    //使用WHEEL型的定时器队列
    ZCE_Timer_Queue::instance(new ZCE_Timer_Wheel(
                                  config_base_->timer_nuamber_));

    //注册定时器
    timer_base_->initialize(ZCE_Timer_Queue::instance(), config_base_);


    //监控对象添加框架的监控对象
    Comm_Stat_Monitor::instance()->add_status_item(COMM_STAT_FRATURE_NUM,
                                                   COMM_STAT_ITEM_WITH_NAME);

    //业务服务器不可能需要很多的IO处理，默认一个值
#ifdef ZCE_OS_WINDOWS
    ZCE_Reactor::instance(new ZCE_Select_Reactor(1024));
    ZLOG_DEBUG("[framework] ZCE_Reactor and ZCE_Select_Reactor u.");
#else
    ZCE_Reactor::instance(new ZCE_Epoll_Reactor(1024));
    ZLOG_DEBUG("[framework] ZCE_Reactor and ZCE_Epoll_Reactor initialized.");
#endif

    //初始化内存管道
    //ret = Zerg_MMAP_BusPipe::instance()->getpara_from_zergcfg(
    //          svd_config->zerg_config_);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_INFO("[framework] Zerg_MMAP_BusPipe::instance()->getpara_from_zergcfg fail,ret = %d.", ret);
        return ret;
    }

    ret = Zerg_MMAP_BusPipe::instance()->
          init_after_getcfg(Zerg_App_Frame::MAX_LEN_OF_APPFRAME,
                            config_base_->if_restore_pipe_);
    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_INFO("[framework] Zerg_MMAP_BusPipe::instance()->init_by_cfg fail,ret = %d.", ret);
        return ret;
    }

    zerg_mmap_pipe_ = Zerg_MMAP_BusPipe::instance();

    ZLOG_INFO("[framework] MMAP Pipe init success,gogogo."
              "The more you have,the more you want. ");


    // 初始化日志
    ret = init_log();

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("[framework] init log fail. ret=%d", ret);
        return ret;
    }

    ZLOG_INFO("[framework] Comm_Svrd_Appliction::init_instance Success.");
    return SOAR_RET::SOAR_RET_SUCC;
}

//退出的工作
int Comm_Svrd_Appliction::on_exit()
{
    //可能要增加多线程的等待
    ZCE_Thread_Wait_Manager::instance()->wait_all();
    ZCE_Thread_Wait_Manager::clean_instance();

    //
    ZCE_Reactor::instance()->close();


    Zerg_MMAP_BusPipe::clean_instance();

    //
    ZLOG_INFO("[framework] %s exit_instance Succ.Have Fun.!!!",
              app_run_name_.c_str());
    ZLOG_INFO("======================================================================================================");
    ZLOG_INFO("======================================================================================================");
    ZLOG_INFO("======================================================================================================");

    return SOAR_RET::SOAR_RET_SUCC;
}

//设置日志的优先级
void Comm_Svrd_Appliction::set_log_priority(ZCE_LOG_PRIORITY log_prio)
{
    ZCE_Trace_LogMsg::instance()->set_log_priority(log_prio);
}

//获得日志的优先级
ZCE_LOG_PRIORITY Comm_Svrd_Appliction::get_log_priority()
{
    return ZCE_Trace_LogMsg::instance()->get_log_priority();
}



//注册定时器, 定时检查配置是否更新
int Comm_Svrd_Appliction::register_soar_timer()
{
    // 注册框架定时器


    //下次触发时间，以及间隔触发时间精度


    return SOAR_RET::SOAR_RET_SUCC;
}

//重新加载配置
int Comm_Svrd_Appliction::reload_config()
{
    // 重置重新加载的标志
    app_reload_ = false;

    // 先框架reload
    int ret = reload();

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("framework reload config error:ret=%d", ret);
        return ret;
    }

    return SOAR_RET::SOAR_RET_SUCC;
}

//int Comm_Svrd_Appliction::do_run()
//{
//    // 框架要先初始化
//    int ret = init_instance();
//
//    if (ret != SOAR_RET::SOAR_RET_SUCC)
//    {
//        ZLOG_ERROR("application: init_instance fail. ret=%d", ret);
//        return ret;
//    }
//
//    // 调用app的init函数
//    ret = init();
//
//    if (ret != SOAR_RET::SOAR_RET_SUCC)
//    {
//        ZLOG_ERROR("application: init_app fail. ret=%d", ret);
//        return ret;
//    }
//
//    ZLOG_INFO("[framework]application: init succ. start run");
//    ret = run_instance();
//
//    if (ret != SOAR_RET::SOAR_RET_SUCC)
//    {
//        ZLOG_ERROR("application: run_instance fail. ret=%d", ret);
//    }
//
//    // 不管run_instance返回什么值，退出时的清理都是需要的
//    // app的退出调用
//    exit();
//    // 最后才是框架的退出
//    exit_instance();
//
//    ZLOG_INFO("[framework] application exit.");
//
//    return ret;
//}



int Comm_Svrd_Appliction::init_log()
{
    int ret = 0;
    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("[framework] init bill fail. ret=%d", ret);
        return ret;
    }

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("[framework] init stat fail. ret=%d", ret);
        return ret;
    }

    ZLOG_DEBUG("log instance finalize .");
    //关闭原来的日志输出方法
    ZCE_Trace_LogMsg::instance()->finalize();

    // 初始化日志
    ZCE_Trace_LogMsg::instance()->initialize((ZCE_LOGFILE_DEVIDE)config_base_->log_info_.log_div_type_,
                                             config_base_->log_file_prefix_.c_str(),
                                             false,
                                             true,
                                             config_base_->log_info_.max_log_file_size_,
                                             config_base_->log_info_.reserve_file_num_,
                                             config_base_->log_info_.log_output_,
                                             LOG_HEAD_RECORD_CURRENTTIME | LOG_HEAD_RECORD_LOGLEVEL);

    ZLOG_DEBUG("log instance reinit .");

    return SOAR_RET::SOAR_RET_SUCC;
}


//重新加载配置
int Comm_Svrd_Appliction::reload()
{
    int ret = config_base_->reload_cfgfile();
    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("load frame config fail. ret=%d", ret);
        return ret;
    }
    return SOAR_RET::SOAR_RET_SUCC;
}



//注册实例指针
void Comm_Svrd_Appliction::set_instance(Comm_Svrd_Appliction *inst)
{
    instance_ = inst;
}

//得到实例指针
Comm_Svrd_Appliction *Comm_Svrd_Appliction::instance()
{
    return instance_;
}
