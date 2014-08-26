
#ifndef SOARING_LIB_SERVER_CONFIG_BASE_H_
#define SOARING_LIB_SERVER_CONFIG_BASE_H_

#include "soar_zerg_svc_info.h"





/*!
* @brief 配置
*
* @note
*/
class Server_Config_Base
{

protected:
    //构造函数
    Server_Config_Base();
    virtual ~Server_Config_Base();

public:

    /*!
    * @brief      取配置信息,取得配置信息后, 需要将各启动参数设置OK
    * @return     virtual int
    * @param      argc
    * @param      argv
    */
    virtual int initialize(int argc, const char *argv[]);

    /*!
    * @brief      处理命令行参数
    * @return     virtual int
    * @param      argc
    * @param      argv
    */
    virtual int proc_start_arg(int argc, const char *argv[]);

    /// 加载zerg framwork app的配置
    virtual int load_cfgfile();

    /// 重新加载配置
    virtual int reload_cfgfile();

protected:

    
    /// 使用帮助
    virtual int usage(const char *program_name);

public:
    //单子实例函数
    static Server_Config_Base *instance();
    //清理单子实例
    static void clean_instance();

public:
    // 服务器实例id
    unsigned int instance_id_;
    // 进行运行目录
    std::string   app_run_dir_;


    // 自己的服务器ID
    SERVICES_ID self_svr_id_;


    //是否恢复管道
    bool if_restore_pipe_;


    /// 是否后台运行, windows下以如果设置了此值，则以服务的方式运行
    bool app_run_daemon_;
    /// Windows下是否安装服务
    bool app_install_service_;
    /// Windows下是否卸载服务
    bool app_uninstall_service_;

    // 日志路径
    std::string log_file_prefix_;

    // ZERG的配置文件
    std::string zerg_cfg_file_;
    // 自己的配置文件
    std::string app_cfg_file_;
    // framework的配置文件
    std::string framework_cfg_file_;
    // svcid的配置文件
    std::string svcid_cfg_file_;

    // 是否使用配置服务器，如果false，则使用本地配置
    bool is_use_cfgsvr_;
    // 配置服务器信息
    std::string master_cfgsvr_ip_;
    
};

#endif //SOARING_LIB_SERVER_CONFIG_BASE_H_

