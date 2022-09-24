#ifndef SOARING_LIB_SERVER_CONFIG_TRANS_H_
#define SOARING_LIB_SERVER_CONFIG_TRANS_H_

#include "soar/zerg/services_info.h"
#include "soar/svrd/cfg_base.h"

struct FRAMEWORK_CONFIG
{
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
class Server_Config_FSM : public Server_Config_Base
{
protected:
    //构造函数
    Server_Config_FSM();
    virtual ~Server_Config_FSM();

public:

    /// 加载zerg framwork app的配置
    virtual int read_cfgfile();

protected:

    /// 使用帮助
    virtual int usage(const char* program_name);

public:

    ///
    FRAMEWORK_CONFIG framework_config_;

    ///
    zce::propertytree framework_ptree_;
};

#endif //SOARING_LIB_SERVER_CONFIG_TRANS_H_
