
#ifndef SOARING_LIB_SERVER_CONFIG_BASE_H_
#define SOARING_LIB_SERVER_CONFIG_BASE_H_

#include "soar_services_info.h"

//============================================================================================

/*!
* @brief      PIPE的配置数据
*
* @note
*/
struct SOAR_PIPE_CFG_DATA
{

public:
    ///是否恢复管道
    bool if_restore_pipe_ = true;

    /// 接收管道长度, 默认50M
    uint32_t recv_pipe_len_ = 50 * 1024 * 1024;
    /// 发送管道长度, 默认50M
    uint32_t send_pipe_len_ = 50 * 1024 * 1024;
};

//============================================================================================
/*!
* @brief      日志的配置数据
*
* @note
*/
struct SOAR_LOG_CFG_DATA
{
public:

    ///保留日志文件数量，
    static const size_t DEF_RESERVE_FILE_NUM = 8;

public:

    ///app日志级别: @ref ZCE_LOG_PRIORITY
    ZCE_LOG_PRIORITY log_level_ = RS_DEBUG;

    ///日志输出方式: @ref LOG_OUTPUT_WAY ,多种输出方式可以组合
    uint32_t log_output_ = LOG_OUTPUT_FILE | LOG_OUTPUT_ERROUT;

    // 日志分割方式:101按大小 201按小时 205按天
    ZCE_LOGFILE_NAME_DEVIDE log_div_type_ = NAME_TIME_DAY_DEVIDE_TIME;

    // 日志文件保留个数，多出的日志文件将会被删除
    uint32_t reserve_file_num_ = DEF_RESERVE_FILE_NUM;

    // 日志文件最大大小,当log_div_type_ 是 LOGDEVIDE_BY_SIZE 时有效。
    uint32_t max_log_file_size_ = 32 * 1024 * 1024;
};


///用于读取配置失败记录发送失败的地点的地方，
#ifndef SOAR_CFG_READ_FAIL
#define SOAR_CFG_READ_FAIL(x)        ZCE_LOG(x,"[CFG]Config file read fail. code line [%s|%d],function:%s.",\
                                             __FILE__,__LINE__,__ZCE_FUNC__)
#endif

//============================================================================================
/*!
* @brief 配置
*
* @note
*/
class Server_Config_Base
{

public:
    //构造函数
    Server_Config_Base();
    virtual ~Server_Config_Base();

public:

    /*!
    * @brief      处理命令行参数
    * @return     virtual int
    * @param      argc
    * @param      argv
    */
    virtual int read_start_arg(int argc, const char *argv[]);

    /*!
    * @brief      加载配置文件的配置
    * @return     int
    */
    virtual int read_cfgfile();

    /*!
    * @brief      读取，在日志里面输出一些配置信息，以便跟踪回溯
    * @param      out_lvl 输出级别，
    */
    virtual void dump_cfg_info(ZCE_LOG_PRIORITY out_lvl);

protected:

    /// 使用帮助
    virtual int usage(const char *program_name);

    //由于

    ///从配置中读取框架基础的配置,包括self_svc_id_等
    int get_common_cfg(const ZCE_Conf_PropertyTree *conf_tree);

    ///从配置中读取日志的配置
    int get_log_cfg(const ZCE_Conf_PropertyTree *conf_tree);


public:
    //
    static const size_t MAX_ALL_TIMER_NUMBER = 1024;

public:

    /// 自己的服务器ID
    SERVICES_ID self_svc_id_;

    /// 服务器实例id
    unsigned int instance_id_ = 1;

    ///管道的配置
    SOAR_PIPE_CFG_DATA pipe_cfg_;

    ///日志帐单的配置数据
    SOAR_LOG_CFG_DATA log_config_;

    ///是否后台运行, windows下以如果设置了此值，则以服务的方式运行
    bool app_run_daemon_ = false;

    /// Windows下是否安装服务
    bool win_install_service_ = false;
    /// Windows下是否卸载服务
    bool win_uninstall_service_ = false;


    ///是否使用配置服务器，如果false，则使用本地配置
    bool is_use_cfgsvr_ = false;
    ///配置服务器信息
    ZCE_Sockaddr_In master_cfgsvr_ip_;

    ///最大的定时器个数
    size_t max_timer_nuamber_ = 1024;
    ///最大的反应器的句柄数量
    size_t max_reactor_hdl_num_ = 1024;

    ///进行运行目录
    std::string app_run_dir_;

    ///日志路径
    std::string log_file_prefix_;

    ///业务和通信进程公用的的配置文件
    std::string common_cfg_file_;

    ///自己的配置文件
    std::string app_cfg_file_;

    ///svcid的配置文件
    std::string svc_table_file_;



};

#endif //SOARING_LIB_SERVER_CONFIG_BASE_H_

