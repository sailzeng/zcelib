/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   soar/svrd/application.cpp
* @author     Sailzeng <sailzeng.cn@gmail.com>
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

#include "soar/predefine.h"
#include "soar/svrd/app_bus.h"
#include "soar/svrd/svrd_buspipe.h"
#include "soar/zerg/services_info.h"
#include "soar/svrd/cfg_fsm.h"
#include "soar/svrd/timer_base.h"
#include "soar/stat/define.h"
#include "soar/stat/monitor.h"

namespace soar
{
app_buspipe* app_buspipe::instance_ = nullptr;

app_buspipe::app_buspipe() :
    self_svc_info_(),
    max_msg_num_(1024),
    zerg_mmap_pipe_(nullptr),
    timer_base_(nullptr),
    config_base_(nullptr)
{
    //作者名称
    app_author_ = "FXL Platform Server Dev Team.";
}

app_buspipe::~app_buspipe()
{
    if (timer_base_)
    {
        delete timer_base_;
        timer_base_ = nullptr;
    }

    if (config_base_)
    {
        delete config_base_;
        config_base_ = nullptr;
    }
}

//初始化，放入一些基类的指针，
int app_buspipe::initialize(svrd_cfg_base* config_base,
                            soar::server_timer* timer_base)
{
    config_base_ = config_base;
    timer_base_ = timer_base;
    return 0;
}

//获取配置的指针
svrd_cfg_base* app_buspipe::config_instance()
{
    return config_base_;
}

//启动过程的处理
int app_buspipe::app_start(int argc, const char* argv[])
{
    int ret = 0;
    ::srand(static_cast<unsigned int>(time(nullptr)));

    //app_buspipe 只可能启动一个实例，所以在这个地方初始化了static指针
    base_instance_ = this;

    //得到APP的名字，去掉路径，后缀的名字
    ret = create_app_name(argv[0]);
    if (0 != ret)
    {
        printf("svr create_app_base_name init fail. ret=%d", ret);
        return ret;
    }
    //初始化SOCKET等
    ret = zce::server_base::socket_init();
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
    ret = zce::chdir(config_base_->app_run_dir_.c_str());
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
    zce::log_msg::instance()->open_size_log(
        init_log_name.c_str(),
        10 * 1024 * 1024,
        3,
        false,
        true);

    ZCE_LOG(RS_INFO, "[framework] change run directory to %s .",
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
    ZCE_LOG(RS_INFO, "======================================================================================================");
    ZCE_LOG(RS_INFO, "======================================================================================================");
    ZCE_LOG(RS_INFO, "[framework] %s start init", app_base_name_.c_str());

    // 切换运行目录
    ret = zce::chdir(config_base_->app_run_dir_.c_str());
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] change run directory to %s fail. err=%d",
                config_base_->app_run_dir_.c_str(), errno);
        return ret;
    }

    ZCE_LOG(RS_INFO, "[framework] change work dir to %s", config_base_->app_run_dir_.c_str());

    // 运行目录写PID File.
    std::string app_path = config_base_->app_run_dir_
        + "/"
        + get_app_basename();
    ret = out_pid_file(app_path.c_str());

    if (ret != 0)
    {
        //如果有错误显示错误，如果错误==16，表示可能是PID文件被锁定,
        ZCE_LOG(RS_ERROR, "[framework] Create Pid file :%s.pid fail .last error =[%u|%s].",
                app_path.c_str(), zce::last_error(),
                strerror(zce::last_error()));

        ZCE_LOG(RS_ERROR, "[framework] If last error == 16, could has a same process already run in this directory."
                "Please check PID file or system processes.");
        return SOAR_RET::ERROR_WRITE_ERROR_PIDFILE;
    }

    // 加载框架配置,由于是虚函数，也会调用到非框架的配置读取
    ret = config_base_->read_cfgfile();
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] framwork config read_cfgfile fail. ret=%d", ret);
        return ret;
    }

    // 初始化日志
    ret = open_log();
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] init log fail. ret=%d", ret);
        return ret;
    }

    ZCE_LOG(RS_INFO, "======================================================================================================");
    ZCE_LOG(RS_INFO, "======================================================================================================");
    ZCE_LOG(RS_INFO, "[framework] %s read_cfgfile success and open_log success.", app_base_name_.c_str());

    self_svc_info_ = config_base_->self_svc_info_;
    //取得配置信息后, 需要将启动参数全部配置OK. 以下的assert做强制检查
    ZCE_ASSERT((self_svc_info_.svc_id_.services_type_ != soar::SERVICES_ID::INVALID_SERVICES_TYPE) &&
               (self_svc_info_.svc_id_.services_id_ != soar::SERVICES_ID::INVALID_SERVICES_ID));

    //初始化统计模块
    //因为配置初始化时会从配置服务器拉取ip，触发统计，因此需要提前初始化
    ret = soar::stat_monitor::instance()->initialize(app_base_name_.c_str(),
                                                     self_svc_info_,
                                                     COMM_STAT_FRATURE_NUM,
                                                     COMM_STAT_ITEM_WITH_NAME,
                                                     false);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "zce_Server_Status init fail. ret=%d", ret);
        return ret;
    }

    //使用WHEEL型的定时器队列
    zce::timer_queue::instance(new zce::timer_wheel(
        config_base_->max_timer_nuamber_));

    //注册定时器
    timer_base_->initialize(zce::timer_queue::instance());

    //ZCE_Reactor的修改一定要放在前面(读取配置后面)，至少吃了4次亏
    //居然在同一条河里淹死了好几次。最新的一次是20070929，
    //根据所需的IO句柄数量初始化
    size_t max_reactor_hdl = config_base_->max_reactor_hdl_num_;
#ifdef ZCE_OS_WINDOWS
    zce::reactor::instance(new zce::select_reactor(max_reactor_hdl));
    ZCE_LOG(RS_DEBUG, "[framework] zce::reactor and zce::select_reactor u.");
#else
    zce::reactor::instance(new zce::epoll_reactor(max_reactor_hdl));
    ZCE_LOG(RS_DEBUG, "[framework] zce::reactor and zce::epoll_reactor initialized.");
#endif

    //初始化内存管道
    ret = soar::svrd_buspipe::instance()->
        initialize(self_svc_info_,
                   config_base_->pipe_cfg_.recv_pipe_len_,
                   config_base_->pipe_cfg_.send_pipe_len_,
                   soar::zerg_frame::MAX_LEN_OF_FRAME,
                   config_base_->pipe_cfg_.if_restore_pipe_);

    if (0 != ret)
    {
        ZCE_LOG(RS_INFO, "[framework] soar::svrd_buspipe::instance()->init_by_cfg fail,ret = %d.", ret);
        return ret;
    }

    zerg_mmap_pipe_ = soar::svrd_buspipe::instance();

    soar::stat_monitor::instance()->
        add_one(COMM_STAT_APP_RESTART_TIMES, 0, 0);

    ZCE_LOG(RS_INFO, "[framework] MMAP Pipe init success,gogogo."
            "The more you have,the more you want. ");

    ZCE_LOG(RS_INFO, "[framework] app_buspipe::init_instance Success.");

    return 0;
}

//退出的工作
int app_buspipe::app_exit()
{
    //可能要增加多线程的等待
    zce::thread_task_wait::instance()->wait_all();
    zce::thread_task_wait::clear_inst();

    soar::stat_monitor::clear_inst();

    soar::svrd_buspipe::clear_inst();

    //释放所有资源,会关闭所有的handle吗,zce::reactor 会，
    if (zce::reactor::instance())
    {
        zce::reactor::instance()->close();
    }
    zce::reactor::clear_inst();

    //
    if (zce::timer_queue::instance())
    {
        zce::timer_queue::instance()->close();
    }
    zce::timer_queue::clear_inst();

    //
    if (zce::timer_queue::instance())
    {
        zce::timer_queue::instance()->close();
    }
    soar::stat_monitor::instance()->
        add_one(COMM_STAT_APP_RESTART_TIMES, 0, 0);

    //单子实例清空
    zce::reactor::clear_inst();
    zce::timer_queue::clear_inst();
    soar::stat_monitor::clear_inst();

    ZCE_LOG(RS_INFO, "[framework] %s exit_instance Succ.Have Fun.!!!",
            app_run_name_.c_str());
    ZCE_LOG(RS_INFO, "======================================================================================================");
    ZCE_LOG(RS_INFO, "======================================================================================================");
    return 0;
}

//设置日志的优先级
void app_buspipe::set_log_priority(zce::LOG_PRIORITY log_prio)
{
    zce::log_msg::instance()->set_log_priority(log_prio);
}

//获得日志的优先级
zce::LOG_PRIORITY app_buspipe::get_log_priority()
{
    return zce::log_msg::instance()->get_log_priority();
}

//日志初始化
int app_buspipe::open_log()
{
    int ret = 0;

    ZCE_LOG(RS_DEBUG, "log instance initialize .");
    //关闭原来的日志输出方法
    zce::log_msg::instance()->terminate();

    // 初始化日志
    ret = zce::log_msg::instance()->
        open_log(config_base_->log_config_.log_output_,
                 config_base_->log_config_.log_div_type_,
                 config_base_->log_file_prefix_.c_str(),
                 true,
                 false,
                 true,
                 false,
                 config_base_->log_config_.max_log_file_size_,
                 config_base_->log_config_.reserve_file_num_,
                 (zce::LOG_HEAD::CURRENTTIME | zce::LOG_HEAD::LOGLEVEL));
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "zce::Log_Msg::instance()->initialize ret fail.");
        return ret;
    }

    ZCE_LOG(RS_DEBUG, "log instance reinit .");

    return 0;
}

//重新加载配置
int app_buspipe::reload_config()
{
    return 0;
}

//注册实例指针
void app_buspipe::set_instance(app_buspipe* inst)
{
    instance_ = inst;
}

//得到实例指针
app_buspipe* app_buspipe::instance()
{
    return instance_;
}

//清理实例指针
void app_buspipe::clear_inst()
{
    if (instance_)
    {
        delete instance_;
        instance_ = nullptr;
    }
}
} //namespace soar