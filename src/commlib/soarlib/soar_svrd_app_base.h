
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
    Comm_Svrd_Appliction();
    virtual ~Comm_Svrd_Appliction();

public:

    ///设置日志的优先级
    void set_log_priority(ZCE_LOG_PRIORITY new_log_prio);

    ///获得日志的优先级
    ZCE_LOG_PRIORITY get_log_priority();

    ///得到APP的版本信息
    const char *get_app_version();

    ///重新加载配置
    int reload_config();


    /*!
    * @brief      初始化，放入一些基类的指针，
    * @return     int
    * @param      config_base
    * @param      timer_base
    */
    int initialize(Server_Config_Base *config_base, 
        Server_Timer_Base *timer_base);

protected:

    // app的开始运行
    virtual int on_start(int argc, const char *argv[]);

    /// app
    virtual int on_run() = 0;

    // app的退出
    virtual int on_exit();

    // 重新加载配置
    // 框架reload调用app接口的顺序: load_app_conf reload
    virtual int reload() = 0;


private:

    //设置定时器, 定时检查更新
    int register_soar_timer();

    // 初始化日志帐单
    int init_log();

public:

    ///注册实例指针
    static void instance(Comm_Svrd_Appliction *inst );

    ///得到实例指针
    static Comm_Svrd_Appliction *instance();


protected:


    //自己的服务器ID
    SERVICES_ID          self_services_id_;

    //以windows的服务方式运行，
    bool                 run_as_win_serivces_;

    //最大消息个数
    size_t               max_msg_num_;

    ///与zerg的管道
    Zerg_MMAP_BusPipe   *zerg_mmap_pipe_;


    ///框架定时器处理类
    Server_Timer_Base   *timer_base_;

    ///配置的处理的基类
    Server_Config_Base  *config_base_;

};

#endif //SOARING_LIB_SERVER_APPLICATION_H_
