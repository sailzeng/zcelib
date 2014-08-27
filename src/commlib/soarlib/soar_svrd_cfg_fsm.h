
#ifndef SOARING_LIB_SERVER_CONFIG_TRANS_H_
#define SOARING_LIB_SERVER_CONFIG_TRANS_H_

#include "soar_zerg_svc_info.h"
#include "soar_svrd_cfg_base.h"


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
* @brief 框架业务服务器的配置
*
* @note
*/
class Server_Config_FSM :public Server_Config_Base
{

protected:
    //构造函数
    Server_Config_FSM();
    virtual ~Server_Config_FSM();

public:

    /// 加载zerg framwork app的配置
    virtual int load_cfgfile();

protected:

    
    /// 使用帮助
    virtual int usage(const char *program_name);

public:
    //单子实例函数
    static Server_Config_FSM *instance();
    //清理单子实例
    static void clean_instance();

public:
    
    //
    FRAMEWORK_CONFIG framework_config_;

    ZCE_Conf_PropertyTree zerg_ptree_;
    ZCE_Conf_PropertyTree framework_ptree_;

protected:
    // 单子实例
    static Server_Config_FSM *instance_;


};

#endif //SOARING_LIB_SERVER_CONFIG_TRANS_H_

