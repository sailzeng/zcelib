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
    app_author_ = "FXL Platform Server Dev Team.";
}

Comm_Svrd_Appliction::~Comm_Svrd_Appliction()
{
    if (timer_base_)
    {
        delete timer_base_;
        timer_base_ = NULL;
    }

    if (config_base_)
    {
        delete config_base_;
        config_base_ = NULL;
    }
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
    int ret = 0;

    //得到APP的名字，去掉路径，后缀的名字
    ret = create_app_name(argv[0]);
    if (0 != ret)
    {
        printf("svr create_app_base_name init fail. ret=%d", ret);
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

    // 处理启动参数
    ret = config_base_->read_start_arg(argc, argv);
    if (ret != 0)
    {
        printf("svr config start_arg init fail. ret=%d", ret);
        return ret;
    }

    // 切换运行目录
    ret = ZCE_LIB::chdir(config_base_->app_run_dir_.c_str());
    if (ret != 0)
    {
        printf("[framework] change run directory to %s fail. error=%d",
               config_base_->app_run_dir_.c_str(), errno);
        return ret;
    }


    //先打开日志，记录一段数据，直到日志的启动参数获得
    // 初始化日志用滚动的方式可以保留的天数多点
    std::string init_log_name(config_base_->log_file_prefix_.c_str());
    init_log_name += "_init";
    ZCE_Trace_LogMsg::instance()->init_size_log(
        init_log_name.c_str(),
        false,
        true,
        10 * 1024 * 1024,
        3);

    ZCE_LOGMSG(RS_INFO, "[framework] change run directory to %s .",
               config_base_->app_run_dir_.c_str());

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
    ret = out_pid_file(app_path.c_str());

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

    //if (ret != 0)
    //{
    //    ZLOG_ERROR("[framework] cfgsvrsdk init fail. ret=%d", ret);
    //    return ret;
    //}

    ////启动配置更新线程
    //CfgSvrSdk::instance()->start_task();
    //ZLOG_INFO("[framework] cfgsdk init succ. start task succ");

    // 加载框架配置,由于是虚函数，也会调用到非框架的配置读取
    ret = config_base_->read_cfgfile();
    if (ret != 0)
    {
        ZLOG_ERROR("[framework] framwork config load_cfgfile fail. ret=%d", ret);
        return ret;
    }


    // 初始化日志
    ret = init_log();
    if (ret != 0)
    {
        ZLOG_ERROR("[framework] init log fail. ret=%d", ret);
        return ret;
    }

    ZLOG_INFO("======================================================================================================");
    ZLOG_INFO("======================================================================================================");
    ZLOG_INFO("[framework] %s load_cfgfile success and init_log success.", app_base_name_.c_str());

    self_svc_id_ = config_base_->self_svc_id_;
    //取得配置信息后, 需要将启动参数全部配置OK. 以下的assert做强制检查
    ZCE_ASSERT((self_svc_id_.services_type_ != SERVICES_ID::INVALID_SERVICES_TYPE) &&
               (self_svc_id_.services_id_ != SERVICES_ID::INVALID_SERVICES_ID));


    //加载配置
    ret = load_config();
    if (0 != ret)
    {
        ZLOG_INFO("[framework] load_config fail,ret = %d.", ret);
        return ret;
    }


    //使用WHEEL型的定时器队列
    ZCE_Timer_Queue::instance(new ZCE_Timer_Wheel(
                                  config_base_->max_timer_nuamber_));

    //注册定时器
    timer_base_->initialize(ZCE_Timer_Queue::instance());


    //监控对象添加框架的监控对象
    Comm_Stat_Monitor::instance()->add_status_item(COMM_STAT_FRATURE_NUM,
                                                   COMM_STAT_ITEM_WITH_NAME);

    //Reactor的修改一定要放在前面(读取配置后面)，至少吃了4次亏
    //居然在同一条河里淹死了好几次。最新的一次是20070929，
    //根据所需的IO句柄数量初始化
    size_t max_reactor_hdl = config_base_->max_reactor_hdl_num_ + 16;
#ifdef ZCE_OS_WINDOWS
    ZCE_Reactor::instance(new ZCE_Select_Reactor(max_reactor_hdl));
    ZLOG_DEBUG("[framework] ZCE_Reactor and ZCE_Select_Reactor u.");
#else
    ZCE_Reactor::instance(new ZCE_Epoll_Reactor(max_reactor_hdl));
    ZLOG_DEBUG("[framework] ZCE_Reactor and ZCE_Epoll_Reactor initialized.");
#endif

    //初始化内存管道
    ret = Zerg_MMAP_BusPipe::instance()->
          initialize(self_svc_id_,
                     config_base_->pipe_cfg_.recv_pipe_len_,
                     config_base_->pipe_cfg_.send_pipe_len_,
                     Zerg_App_Frame::MAX_LEN_OF_APPFRAME,
                     config_base_->pipe_cfg_.if_restore_pipe_);

    if (0 != ret)
    {
        ZLOG_INFO("[framework] Zerg_MMAP_BusPipe::instance()->init_by_cfg fail,ret = %d.", ret);
        return ret;
    }

    zerg_mmap_pipe_ = Zerg_MMAP_BusPipe::instance();

    ZLOG_INFO("[framework] MMAP Pipe init success,gogogo."
              "The more you have,the more you want. ");

    ZLOG_INFO("[framework] Comm_Svrd_Appliction::init_instance Success.");
    return 0;
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
    return 0;
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

//日志初始化
int Comm_Svrd_Appliction::init_log()
{
    int ret = 0;

    ZLOG_DEBUG("log instance finalize .");
    //关闭原来的日志输出方法
    ZCE_Trace_LogMsg::instance()->finalize();

    // 初始化日志
    ret = ZCE_Trace_LogMsg::instance()->initialize(config_base_->log_config_.log_div_type_,
                                                   config_base_->log_file_prefix_.c_str(),
                                                   false,
                                                   true,
                                                   config_base_->log_config_.max_log_file_size_,
                                                   config_base_->log_config_.reserve_file_num_,
                                                   config_base_->log_config_.log_output_,
                                                   LOG_HEAD_RECORD_CURRENTTIME | LOG_HEAD_RECORD_LOGLEVEL);
    if (0 != ret)
    {
        ZCE_LOGMSG(RS_ERROR, "ZCE_Trace_LogMsg::instance()->initialize ret fail.");
        return ret;
    }

    ZLOG_DEBUG("log instance reinit .");

    return 0;
}


//加载配置,不在读取配置的时候加载配置，
int Comm_Svrd_Appliction::load_config()
{
    return 0;
}

//重新加载配置
int Comm_Svrd_Appliction::reload()
{
    return 0;
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

//清理实例指针
void Comm_Svrd_Appliction::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }

}

