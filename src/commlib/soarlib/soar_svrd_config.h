
#ifndef SOARING_LIB_SERVER_CONFIG_H_
#define SOARING_LIB_SERVER_CONFIG_H_

#include "soar_zerg_svc_info.h"


struct ZERG_CONFIG
{

};



struct FRAMEWORK_CONFIG
{
    struct LogInfo
    {
    public: /* members */
        char log_level_[32]; // app日志级别: debug info error final
        uint32_t log_output_; // 日志输出方式: 0001=file 0010=stdout 0100=stderr 1000=windbg,多种输出方式可以组合
        uint32_t log_div_type_; // 日志分割方式:101按大小 201按小时 205按天
        uint32_t max_log_file_num_; // 日志文件保留个数，多出的日志文件将会被删除
        uint32_t max_log_file_size_; // 日志文件最大大小
    };

    struct TaskInfo
    {
    public: /* members */
        uint32_t task_thread_num_; // task线程数量
        uint32_t task_thread_stack_size_; // task线程堆栈大小
        uint32_t enqueue_timeout_sec_; // push数据到task队列时的超时时间秒
        uint32_t enqueue_timeout_usec_; // push数据到task队列时的超时时间微秒
    };

    struct TransInfo
    {
    public: /* members */
        uint32_t trans_num_; // 事务的数量
        uint32_t trans_cmd_num_; // 事务命令字的个数
        uint32_t func_cmd_num_; // 函数命令字的个数
    };

    struct MonitorInfo
    {
    public: /* members */
        uint32_t filter_statics_id_cnt_; // 要排除的统计ID个数
        uint32_t filter_statics_id_list_[100]; // 要排除的统计ID列表
    };

    TransInfo trans_info_; // 事务的配置
    TaskInfo task_info_; // Task的配置，使用notifytrans时有效
    LogInfo log_info_; // 日志帐单的配置信息
    MonitorInfo monitor_info_; // 监控配置信息
};

struct SVCID_CONFIG
{

};


/*!
* @brief 配置
*
* @note
*/
class Comm_Svrd_Config
{

protected:
    //构造函数
    Comm_Svrd_Config();
    virtual ~Comm_Svrd_Config();

public:

    /*!
    * @brief      取配置信息,取得配置信息后, 需要将各启动参数设置OK
    * @return     virtual int
    * @param      argc
    * @param      argv
    */
    virtual int init(int argc, const char *argv[]);

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
    static Comm_Svrd_Config *instance();
    //清理单子实例
    static void clean_instance();

public:
    // 服务器实例id
    unsigned int instance_id_;
    // 进行运行目录
    std::string   app_run_dir_;


    // 自己的服务器ID
    SERVICES_ID self_svr_id_;
    // framework的配置
    FRAMEWORK_CONFIG framework_config_;


    //是否恢复管道
    bool if_restore_pipe_;


    /// 是否后台运行, windows下以如果设置了此值，则以服务的方式运行
    bool app_run_daemon_;
    /// Windows下是否安装服务
    bool app_install_service_;
    /// Windows下是否卸载服务
    bool app_uninstall_service_;

    //
    ZCE_Conf_PropertyTree zerg_ptree_;
    //
    ZCE_Conf_PropertyTree framework_ptree_;


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


protected:
    // 单子实例
    static Comm_Svrd_Config *instance_;


};

#endif //SOARING_LIB_SERVER_CONFIG_H_

