
#ifndef SOARING_LIB_SERVER_CONFIG_H_
#define SOARING_LIB_SERVER_CONFIG_H_

#include "soar_zerg_svc_info.h"

/****************************************************************************************************
class  Comm_Svrd_Appliction 程服务器配置,鸡肋==基类
****************************************************************************************************/
class Comm_Svrd_Config
{

private:
    Comm_Svrd_Config();
    virtual ~Comm_Svrd_Config();

public:
    //取配置信息,取得配置信息后, 需要将各启动参数设置OK
    int init(int argc, const char *argv[]);

    //重新加载配置
    int reload();

    // 处理命令行参数
    int proc_start_arg(int argc, const char *argv[]);

    // 加载zerg framwork app的配置
    int load_config();

    // 判定uin是否为监控uin
    bool is_monitor_uin(unsigned int uin);

private:
    // 使用帮助
    int usage(const char *program_name);

public:
    //单子实例函数
    static Comm_Svrd_Config *instance();
    //清理单子实例
    static void clean_instance();

public:
    // 自己的服务器ID
    SERVICES_ID self_svr_id_;

    // 进行运行目录
    std::string app_run_dir_;

    //是否恢复管道
    bool if_restore_pipe_;

    // 是否后台运行, windows下以如果设置了此值，则以服务的方式运行
    bool app_run_daemon_;

    // 是否安装服务
    bool app_install_service_;

    // 是否卸载服务
    bool app_uninstall_service_;

    //// zerg的配置
    //conf_zerg::ZERG_CONFIG zerg_config_;

    //// framework的配置
    //conf_framework::FRAMEWORK_CONFIG framework_config_;

    //// svcid的配置
    //conf_svcid::SVCID_CONFIG svcid_config_;

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

    // 配置服务器信息
    std::string master_cfgsvr_ip_;

    // 服务器实例id
    unsigned short instance_id_;

    // 是否使用配置服务器，如果false，则使用本地配置
    bool is_use_cfgsvr_;

    // 监控uin
    std::set<unsigned int> monitor_uin_set_;
protected:
    // 单子实例
    static Comm_Svrd_Config *instance_;


};

#endif //SOARING_LIB_SERVER_CONFIG_H_

