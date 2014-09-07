
#ifndef SOARING_LIB_SERVER_CONFIG_BASE_H_
#define SOARING_LIB_SERVER_CONFIG_BASE_H_

#include "soar_zerg_svc_info.h"



//============================================================================================
/*!
* @brief      
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
    ZCE_LOGFILE_DEVIDE log_div_type_ = LOGDEVIDE_BY_DAY;

    // 日志文件保留个数，多出的日志文件将会被删除
    uint32_t reserve_file_num_ = DEF_RESERVE_FILE_NUM;

    // 日志文件最大大小,当log_div_type_ 是 LOGDEVIDE_BY_SIZE 时有效。
    uint32_t max_log_file_size_ = 32 * 1024 * 1024;
};


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
    virtual int start_arg(int argc, const char *argv[]);

    /// 加载zerg framwork app的配置
    virtual int load_cfgfile();

    /// 重新加载配置
    virtual int reload_cfgfile();


    /*!
    * @brief      读取，在日志里面输出一些配置信息，以便跟踪回溯
    * @param      out_lvl 输出级别，
    */
    virtual void dump_cfg_info(ZCE_LOG_PRIORITY out_lvl);

protected:

    /// 使用帮助
    virtual int usage(const char *program_name);

    //由于

    ///从配置中读取self_svc_id_的
    int get_selfsvcid_cfg(const char *cfg_file_name, 
        const ZCE_Conf_PropertyTree *conf_tree);

    ///从配置中读取日志的配置
    int get_log_cfg(const char *cfg_file_name, 
        const ZCE_Conf_PropertyTree *conf_tree);


public:
    //
    static const size_t MAX_ALL_TIMER_NUMBER = 1024;

public:

    // 自己的服务器ID
    SERVICES_ID self_svc_id_;

    // 服务器实例id
    unsigned int instance_id_ = 0;

    //是否恢复管道
    bool if_restore_pipe_ = true;


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

    size_t timer_nuamber_ = 1024;

    ///进行运行目录
    std::string app_run_dir_;

    ///日志路径
    std::string log_file_prefix_;

    ///ZERG的配置文件
    std::string zerg_cfg_file_;
    ///自己的配置文件
    std::string app_cfg_file_;
    ///framework的配置文件
    std::string framework_cfg_file_;
    ///svcid的配置文件
    std::string svcid_table_file_;

    ///日志帐单的配置数据   
    SOAR_LOG_CFG_DATA log_config_;
    
};

#endif //SOARING_LIB_SERVER_CONFIG_BASE_H_

