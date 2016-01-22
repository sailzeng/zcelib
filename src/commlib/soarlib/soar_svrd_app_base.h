
#ifndef SOARING_LIB_SERVER_APPLICATION_H_
#define SOARING_LIB_SERVER_APPLICATION_H_

#include "soar_services_info.h"
#include "soar_error_code.h"

class Server_Config_Base;
class Soar_MMAP_BusPipe;
class Server_Timer_Base;



/*!
* @brief      单线程自动机模型的程序框架基类
*
* @note
*/
class Soar_Svrd_Application : public ZCE_Server_Base
{
protected:

    ///构造函数和析构函数
    Soar_Svrd_Application();
    ///析构函数
    virtual ~Soar_Svrd_Application();

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
    virtual int app_start(int argc, const char *argv[]);

    /// app 运行
    virtual int app_run() = 0;

    /// app的退出
    virtual int app_exit();

protected:

    ///重新加载配置
    virtual int reload_config();

    ///初始化日志模块
    int init_log();

public:

    ///注册实例指针
    static void set_instance(Soar_Svrd_Application *inst);

    ///得到实例指针
    static Soar_Svrd_Application *instance();

    ///清理实例实例指针
    static void clean_instance();

protected:

    //实例指针
    static Soar_Svrd_Application *instance_;

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
    Soar_MMAP_BusPipe   *zerg_mmap_pipe_;


    ///框架定时器处理类
    Server_Timer_Base   *timer_base_;

    ///配置的处理的基类
    Server_Config_Base  *config_base_;

};

#endif //SOARING_LIB_SERVER_APPLICATION_H_
