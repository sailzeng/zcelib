
#ifndef SOARING_LIB_SERVER_APPLICATION_H_
#define SOARING_LIB_SERVER_APPLICATION_H_

#include "soar_zerg_svc_info.h"
#include "soar_error_code.h"

class Server_Config_Base;
class Zerg_MMAP_BusPipe;
class Server_Timer_Base;
/****************************************************************************************************
class  Comm_Svrd_Appliction
单线程自动机模型的程序框架基类.
****************************************************************************************************/
class Comm_Svrd_Appliction : public ZCE_Server_Base
{
protected:

    //构造函数和析构函数
    Comm_Svrd_Appliction(Server_Config_Base *config);
    virtual ~Comm_Svrd_Appliction();

public:

    //设置日志的优先级
    void set_log_priority(ZCE_LOG_PRIORITY new_log_prio);
    //获得日志的优先级
    ZCE_LOG_PRIORITY get_log_priority();



    int run(int argc, const char *argv[]);

    //int do_run();

    //得到APP的版本信息
    const char *get_app_version();



    //重新加载配置
    int reload_config();

protected:

    // app的初始化
    virtual int init(int argc, const char *argv[]) = 0;

    // 重新加载配置
    // 框架reload调用app接口的顺序: load_app_conf reload
    virtual int reload() = 0;

    // app的退出
    virtual int exit();


private:
    //设置定时器, 定时检查更新
    int register_soar_timer();

    // 初始化日志帐单
    int init_log();

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


    //自己的服务器ID
    SERVICES_ID          self_services_id_;

    //以windows的服务方式运行，
    bool                 run_as_win_serivces_;

    //最大消息个数
    size_t               max_msg_num_;

    // 与zerg的管道
    Zerg_MMAP_BusPipe   *zerg_mmap_pipe_;


    /// 框架定时器处理类
    Server_Timer_Base   *timer_handler_;

    ///
    Server_Config_Base  *config_;

};

#endif //SOARING_LIB_SERVER_APPLICATION_H_
