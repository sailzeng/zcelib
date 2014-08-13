
#ifndef SOARING_LIB_SERVER_APPLICATION_H_
#define SOARING_LIB_SERVER_APPLICATION_H_

#include "soar_service_info.h"
#include "soar_error_code.h"

class Comm_Svrd_Config;
class Zerg_MMAP_BusPipe;
class Comm_Timer_Handler;
/****************************************************************************************************
class  Comm_Svrd_Appliction
单线程自动机模型的程序框架基类.
****************************************************************************************************/
class Comm_Svrd_Appliction : public ZCE_Server_Base
{
protected:

    //构造函数和析构函数
    Comm_Svrd_Appliction();
    virtual ~Comm_Svrd_Appliction();

public:

    //设置进程暂停运行的标志
    void set_pause_sign(bool app_pause);



    //设置日志的优先级
    void set_log_priority(ZCE_LOG_PRIORITY new_log_prio);
    //获得日志的优先级
    ZCE_LOG_PRIORITY get_log_priority();



    int run(int argc, const char *argv[]);

    int do_run();

    //得到APP的版本信息
    const char *get_app_version();

    // windows下设置服务信息
    void set_service_info(const char *svc_name, const char *svc_desc);

    //重新加载配置
    int reload_config();
protected:
    // 应用需要实现的函数

    // 加载app配置, app需自己实现
    // 框架初始化调用app接口的顺序： load_app_conf register_notify_task(notifytrans才有) init
    virtual int load_app_conf();

    // app的初始化
    virtual int init() = 0;

    // 重新加载配置
    // 框架reload调用app接口的顺序: load_app_conf reload
    virtual int reload() = 0;

    // app的退出
    virtual void exit();

protected:
    // 框架中不同的app类可以重载的函数
    // 框架初始化
    virtual int init_instance();

    int proc_start_args(int argc, const char *argv[]);

    // 框架退出
    virtual int exit_instance();

    // 框架运行处理,
    virtual int run_instance() = 0;

    // 框架重新加载配置
    virtual int reload_instance();

private:
    //设置定时器, 定时检查更新
    int register_soar_timer();

    //得到app_base_name_，app_run_name_
    int create_app_name(const char *argv_0);


    // 初始化日志帐单
    int init_log();

    //WIN 服务的代码，用于服务器的注册注销等
#ifdef ZCE_OS_WINDOWS

    //运行服务
    int win_services_run();

    //WIN 下面的退出处理
    static BOOL exit_signal(DWORD );

    //安装服务
    int win_services_install();
    //卸载服务
    int win_services_uninstall();
    //检查服务是否安装
    bool win_services_isinstalled();

    //服务运行函数
    static void WINAPI win_service_main();
    //服务控制台所需要的控制函数
    static void WINAPI win_services_ctrl(DWORD op_code);

#else

    //退出信号
    static void exit_signal(int );

    //重新加载配置
    static void reload_config_signal(int );
#endif

public:

    //得到实例指针
    static Comm_Svrd_Appliction *instance();

protected:

    // 一次最大处理的FRAME个数
    static const size_t MAX_ONCE_PROCESS_FRAME  = 2048;

    //空闲N次后,调整SELECT的等待时间间隔
    static const unsigned int LIGHT_IDLE_SELECT_INTERVAL = 512;

    //空闲N次后,SLEEP的时间间隔
    static const unsigned int HEAVY_IDLE_SLEEP_INTERVAL  = 10240;

    //microsecond
    // 64位tlinux下idle的时间如果太短会导致cpu过高
    static const int LIGHT_IDLE_INTERVAL_MICROSECOND  = 10000;
    static const int HEAVY_IDLE_INTERVAL_MICROSECOND  = 100000;

    //定时检查配置是否更新时间(秒)
    static const int CHECK_CONFIG_UPDATE_TIME = 10;
    // 框架定时器间隔时间 100毫秒
    static const unsigned int FRAMEWORK_TIMER_INTERVAL = 100000;

protected:

    //为了加载卸载so，加一个是否暂停的变量
    bool                         app_pause_;



    //自己的服务器ID
    SERVICES_ID                  self_services_id_;

    //以windows的服务方式运行，
    bool                         run_as_win_serivces_;

    //最大消息个数
    size_t                       max_msg_num_;



    // 与zerg的管道
    Zerg_MMAP_BusPipe           *zerg_mmap_pipe_;



    // 启动参数
    int                         argc_;
    const char                **argv_;


private:
    // 框架定时器处理类
    Comm_Timer_Handler          *timer_handler_;

};

#endif //SOARING_LIB_SERVER_APPLICATION_H_
