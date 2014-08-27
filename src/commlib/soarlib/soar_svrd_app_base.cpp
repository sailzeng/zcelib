/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar_svrd_application.cpp
* @author     Sailzeng <sailerzeng@gmail.com>
* @version    
* @date       2008年1月22日
* @brief      
*             终于讲sonicmao的这个类发扬广大了，
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




Comm_Svrd_Appliction::Comm_Svrd_Appliction():
    self_services_id_(),
    run_as_win_serivces_(false),
    max_msg_num_(1024),
    zerg_mmap_pipe_(NULL),
    timer_handler_(NULL)
{
    //作者名称
    app_author_ = "TSS Platform Server Dev Team.";
    argc_ = 0;
    argv_ = NULL;
}

Comm_Svrd_Appliction::~Comm_Svrd_Appliction()
{
    delete timer_handler_;
    timer_handler_ = NULL;
}

int Comm_Svrd_Appliction::run(int argc, const char *argv[])
{
    // 先处理命令行数据, 加载配置
    int ret = proc_start_args(argc, argv);

    if (ret != 0)
    {
        return ret;
    }

    Server_Config_FSM *config = Server_Config_FSM::instance();

    if (config->app_run_daemon_)
    {
#ifdef ZCE_OS_WINDOWS
        // 如果在windows下后台运行，则以服务的方式运行
        return win_services_run();
#elif defined(ZCE_OS_LINUX)
        daemon_init();
#endif
    }

    //
    return do_run();
}

/******************************************************************************************
Author          : Sonicmao(MaoJian)  Date Of Creation: 2008年1月22日
Function        : Comm_Svrd_Appliction::Init
Return          : int
Parameter List  :
  Param1: int argc     启动参数个数
  Param2: char* argv[] 启动参数信息
  Param2: Comm_Svrd_Config *svd_config 配置类,
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Comm_Svrd_Appliction::init_instance()
{
    //忽视信号
    process_signal();

    std::string log_file_prefix = Server_Config_FSM::instance()->app_run_dir_ + "/log/";
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

    //我是华丽的分割线
    ZLOG_INFO("------------------------------------------------------------------------------------------------------");
    ZLOG_INFO("------------------------------------------------------------------------------------------------------");
    ZLOG_INFO("------------------------------------------------------------------------------------------------------");
    ZLOG_INFO("[framework] %s start init", app_base_name_.c_str());

    int ret = SOAR_RET::SOAR_RET_SUCC;
    //初始化SOCKET等
    ret = ZCE_Server_Base::socket_init();

    if (ret != 0)
    {
        return ret;
    }

    // 切换运行目录
    ret = ZCE_OS::chdir(Server_Config_FSM::instance()->app_run_dir_.c_str());

    if (ret != 0)
    {
        ZLOG_ERROR("[framework] change run directory to %s fail. err=%d",
                   Server_Config_FSM::instance()->app_run_dir_.c_str(), errno);
        return ret;
    }

    ZLOG_INFO("[framework] change work dir to %s", Server_Config_FSM::instance()->app_run_dir_.c_str());

    // 运行目录写PID File.
    std::string app_path = Server_Config_FSM::instance()->app_run_dir_
                           + "/"
                           + get_app_basename();
    ret = out_pid_file(app_path.c_str(), true);

    if (ret != 0)
    {
        //如果有错误显示错误，如果错误==16，表示可能是PID文件被锁定,
        ZLOG_ERROR("[framework] Create Pid file :%s.pid fail .last error =[%u|%s].",
                   app_path.c_str(), ZCE_OS::last_error(),
                   strerror(ZCE_OS::last_error()));

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

    // 加载框架配置
    Server_Config_FSM *svd_config = Server_Config_FSM::instance();
    ret = svd_config->initialize(argc_, argv_);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("[framework] framwork config init fail. ret=%d", ret);
        return ret;
    }

    self_services_id_ = svd_config->self_svr_id_;
    //取得配置信息后, 需要将启动参数全部配置OK. 以下的assert做强制检查
    assert((self_services_id_.services_type_ != SERVICES_ID::INVALID_SERVICES_TYPE) &&
           (self_services_id_.services_id_ != SERVICES_ID::INVALID_SERVICES_ID));

    // 加载App配置
    ret = load_app_conf();

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("[framework] %s load app config fail. ret=%d",
                   __ZCE_FUNCTION__, ret);
        return ret;
    }

    //使用WHEEL型的定时器队列
    ZCE_Timer_Queue::instance(new ZCE_Timer_Wheel(
                                  svd_config->framework_config_.trans_info_.trans_num_ + 1024));

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

    ret = Zerg_MMAP_BusPipe::instance()->init_after_getcfg(Zerg_App_Frame::MAX_LEN_OF_APPFRAME,
                                                           svd_config->if_restore_pipe_);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_INFO("[framework] Zerg_MMAP_BusPipe::instance()->init_by_cfg fail,ret = %d.", ret);
        return ret;
    }

    zerg_mmap_pipe_ = Zerg_MMAP_BusPipe::instance();

    ZLOG_INFO("[framework] MMAP Pipe init success,gogogo."
              "The more you have,the more you want. ");

    //注册配置更新检查定时器
    register_soar_timer();

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

// 加载app配置, app需自己实现
int Comm_Svrd_Appliction::load_app_conf()
{
    return SOAR_RET::SOAR_RET_SUCC;
}

// app的退出
void Comm_Svrd_Appliction::exit()
{
}

//得到app_base_name_，app_run_name_
int Comm_Svrd_Appliction::create_app_name(const char *argv_0)
{
    app_run_name_ = argv_0;
    // 取得base name
    char str_base_name[PATH_MAX + 1];
    str_base_name[PATH_MAX] = '\0';
    ZCE_OS::basename(argv_0, str_base_name, PATH_MAX);

#if defined ZCE_OS_WINDOWS

    //Windows下要去掉,EXE后缀
    const size_t WIN_EXE_SUFFIX_LEN = 4;
    size_t name_len = strlen(str_base_name);

    if (name_len <= WIN_EXE_SUFFIX_LEN)
    {
        ZLOG_ERROR("[framework] Exe file name is not expect?Path name[%s].", argv_0);
        return -1;
    }

    //如果有后缀才取消，没有就放鸭子
    if (strcasecmp(str_base_name + name_len - WIN_EXE_SUFFIX_LEN, ".EXE") == 0)
    {
        str_base_name[name_len - WIN_EXE_SUFFIX_LEN] = '\0';
    }

#endif

    //如果是调试版本，去掉后缀符号_d
#if defined (DEBUG) || defined (_DEBUG)

    //如果是调试版本，去掉后缀符号_d
    const size_t DEBUG_SUFFIX_LEN = 2;
    size_t debug_name_len = strlen(str_base_name);

    if (debug_name_len <= DEBUG_SUFFIX_LEN)
    {
        ZLOG_ERROR("[framework] Exe file name is not debug _d suffix?str_base_name[%s].", str_base_name);
        return -1;
    }

    if (0 == strcasecmp(str_base_name + debug_name_len - DEBUG_SUFFIX_LEN, "_D") )
    {
        str_base_name[debug_name_len - DEBUG_SUFFIX_LEN] = '\0';
    }

#endif

    app_base_name_ = str_base_name;

    return SOAR_RET::SOAR_RET_SUCC;
}

//重新加载配置
int Comm_Svrd_Appliction::reload_instance()
{
    int ret = Server_Config_FSM::instance()->reload_cfgfile();
    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("load frame config fail. ret=%d", ret);
        return ret;
    }
    return SOAR_RET::SOAR_RET_SUCC;
}

//退出的工作
int Comm_Svrd_Appliction::exit_instance()
{
    //可能要增加多线程的等待
    ZCE_Thread_Wait_Manager::instance()->wait_all();
    ZCE_Thread_Wait_Manager::clean_instance();

    //
    ZCE_Reactor::instance()->close();
    //清理Instacne
    //Transaction_Manager::clean_instance();

    Zerg_MMAP_BusPipe::clean_instance();

    Server_Config_FSM::clean_instance();
    //
    ZLOG_INFO("[framework] %s exit_instance Succ.Have Fun.!!!",
              app_run_name_.c_str());
    ZLOG_INFO("------------------------------------------------------------------------------------------------------");
    ZLOG_INFO("------------------------------------------------------------------------------------------------------");
    ZLOG_INFO("------------------------------------------------------------------------------------------------------");

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
    ZCE_ASSERT(timer_handler_ == NULL);
    ZCE_Timer_Queue *timer_queue = ZCE_Timer_Queue::instance();
    timer_handler_ = new Server_Timer_Base(timer_queue);

    // 通过进程名判断是否app进程还是zerg进程
    bool is_app = true;
    if (strncasecmp(app_base_name_.c_str(), "zergsvr", strlen("zergsvr") == 0))
    {
        // zergsvr
        is_app = false;
    }
    
    //timer_handler_->init(CfgSvrSdk::instance()->get_game_id(), is_app, this);

    ZCE_Time_Value delay;
    ZCE_Time_Value interval;

    //下次触发时间
    delay.set(0, 0);

    //每次触发间隔时间，每100毫秒触发一次
    interval.set(0, FRAMEWORK_TIMER_INTERVAL);

    //注册定时器
    timer_queue->schedule_timer(timer_handler_,
                                NULL,
                                delay,
                                interval);

    return SOAR_RET::SOAR_RET_SUCC;
}

//重新加载配置
int Comm_Svrd_Appliction::reload_config()
{
    // 重置重新加载的标志
    app_reload_ = false;

    // 先框架reload
    int ret = reload_instance();

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("framework reload config error:ret=%d", ret);
        return ret;
    }

    // app load config
    ret = load_app_conf();

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("load app config error, ret=%d", ret);
        return ret;
    }

    // 然后app reload
    ret = reload();

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("app reload config error, ret=%d", ret);
        return ret;
    }

    return SOAR_RET::SOAR_RET_SUCC;
}



int Comm_Svrd_Appliction::proc_start_args(int argc, const char *argv[])
{
    int ret = 0;
    argc_ = argc;
    argv_ = argv;

    //Comm_Svrd_Appliction 只可能启动一个实例，所以在这个地方初始化了static指针
    base_instance_ = this;

    //得到APP的名字，去掉路径，后缀的名字
    ret = create_app_name(argv[0]);

    if (0 != ret )
    {
        ZLOG_ERROR("svr create_app_base_name init fail. ret=%d", ret);
        return ret;
    }

    Server_Config_FSM *svd_config = Server_Config_FSM::instance();

    // 处理启动参数
    ret = svd_config->proc_start_arg(argc, argv);

    if (ret != 0)
    {
        ZLOG_ERROR("svr config init fail. ret=%d", ret);
        return ret;
    }

#ifdef ZCE_OS_WINDOWS

    if (svd_config->app_install_service_)
    {
        // 安装服务
        ret = win_services_install();
        // 直接退出？
        ::exit(ret);
    }

    if (svd_config->app_uninstall_service_)
    {
        // 卸载服务
        ret = win_services_uninstall();
        // 直接退出？
        ::exit(ret);
    }

#endif

    return SOAR_RET::SOAR_RET_SUCC;
}

int Comm_Svrd_Appliction::do_run()
{
    // 框架要先初始化
    int ret = init_instance();

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("application: init_instance fail. ret=%d", ret);
        return ret;
    }

    // 调用app的init函数
    ret = init();

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("application: init_app fail. ret=%d", ret);
        return ret;
    }

    ZLOG_INFO("[framework]application: init succ. start run");
    ret = run_instance();

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        ZLOG_ERROR("application: run_instance fail. ret=%d", ret);
    }

    // 不管run_instance返回什么值，退出时的清理都是需要的
    // app的退出调用
    exit();
    // 最后才是框架的退出
    exit_instance();

    ZLOG_INFO("[framework] application exit.");

    return ret;
}

void Comm_Svrd_Appliction::set_service_info(const char *svc_name, const char *svc_desc)
{
    if (svc_name != NULL)
    {
        service_name_ = svc_name;
    }

    if (svc_desc != NULL)
    {
        service_desc_ = svc_desc;
    }
}

int Comm_Svrd_Appliction::init_log()
{
    Server_Config_FSM *config = Server_Config_FSM::instance();
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
    ZCE_Trace_LogMsg::instance()->initialize((ZCE_LOGFILE_DEVIDE)config->framework_config_.log_info_.log_div_type_,
                                             config->log_file_prefix_.c_str(),
                                             false,
                                             true,
                                             config->framework_config_.log_info_.max_log_file_size_,
                                             config->framework_config_.log_info_.max_log_file_num_,
                                             config->framework_config_.log_info_.log_output_,
                                             LOG_HEAD_RECORD_CURRENTTIME | LOG_HEAD_RECORD_LOGLEVEL);

    ZLOG_DEBUG("log instance reinit .");

    return SOAR_RET::SOAR_RET_SUCC;
}

