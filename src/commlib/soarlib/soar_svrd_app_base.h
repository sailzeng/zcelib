
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

    ///构造函数和析构函数
    Comm_Svrd_Appliction();
    ///析构函数
    virtual ~Comm_Svrd_Appliction();

public:

    ///设置日志的优先级
    void set_log_priority(ZCE_LOG_PRIORITY new_log_prio);

    ///获得日志的优先级
    ZCE_LOG_PRIORITY get_log_priority();

    ///得到APP的版本信息
    const char *get_app_version();


    /*!
    * @brief      初始化，放入一些基类的指针，
    * @return     int
    * @param      config_base 配置类指针，注意new了放进来
    * @param      timer_base  定时器触发句柄的指针，注意new了放进来
    */
    int initialize(Server_Config_Base *config_base,
                   Server_Timer_Base *timer_base);


    /*!
    * @brief      返回APP的配置类指针
    * @return     Server_Config_Base*配置类的指针
    */
    Server_Config_Base  *config_instance();

public:

    /// app的开始运行
    virtual int on_start(int argc, const char *argv[]);

    /// app
    virtual int on_run() = 0;

    /// app的退出
    virtual int on_exit();

protected:

    /// 配置类读取配置完成后，APP（其他）加载配置
    virtual int load_config();

    /// 重新读取并且加载配置,如果服务器需要加载配置时，重载这个函数
    virtual int re_read_load_cfg();

    /// 初始化日志帐单
    int init_log();

public:

    ///注册实例指针
    static void set_instance(Comm_Svrd_Appliction *inst);

    ///得到实例指针
    static Comm_Svrd_Appliction *instance();

    ///清理实例实例指针
    static void clean_instance();

protected:

    //实例指针
    static Comm_Svrd_Appliction *instance_;

protected:

    //业务ID
    unsigned int         business_id_;
    //自己的服务器ID
    SERVICES_ID          self_svc_id_;

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
