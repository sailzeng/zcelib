#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_services_info.h"
#include "soar_server_ver_define.h"
#include "soar_svrd_app_base.h"
#include "soar_svrd_cfg_base.h"


Server_Config_Base::Server_Config_Base() :
    self_svc_id_(0, 0),
    instance_id_(1)
{

    //默认定时器的事数量
    //const size_t DEFAULT_TIMER_NUMBER = 1024;


    //默认框架定时器间隔时间 100毫秒
    //
    //heart_precision_.set(0, DEF_TIMER_INTERVAL_USEC);

    const size_t DEF_RESERVE_FILE_NUM = 10;
    const size_t DEF_MAX_LOG_FILE_SIZE = 16 * 1024 * 1024;

    log_config_.log_level_ = RS_DEBUG;
    log_config_.log_div_type_ = NAME_TIME_DAY_DEVIDE_TIME;
    log_config_.log_output_ = LOG_OUTPUT_FILE | LOG_OUTPUT_ERROUT;

    log_config_.max_log_file_size_ = DEF_MAX_LOG_FILE_SIZE;
    log_config_.reserve_file_num_ = DEF_RESERVE_FILE_NUM;

}

Server_Config_Base::~Server_Config_Base()
{
}


int Server_Config_Base::read_start_arg(int argc, const char *argv[])
{

    // 指明RETURN_IN_ORDER 不调整顺序
    ZCE_Get_Option get_opt(argc, (char **)argv,
                           "umvndhpi:t:r:a:", 1, 0, ZCE_Get_Option::RETURN_IN_ORDER);
    int c = 0;
    while ((c = get_opt()) != EOF)
    {
        switch (c)
        {
            case 'v':
            {
                // 打印版本信息
                printf("%s\n", TSS_SERVER_VER_DECLARE);
                exit(0);
                break;
            }

            case 'n':
            {
                // 从管道恢复数据
                pipe_cfg_.if_restore_pipe_ = true;
                break;
            }

            case 'd':
            {
                // 后台运行
                app_run_daemon_ = true;
                break;
            }

            case 'r':
            {
                // 指定运行目录, 以服务运行时，需要指定此参数
                printf("app run dir = %s\n", app_run_dir_.c_str());
                app_run_dir_ = get_opt.optarg;
                break;
            }

            case 'a':
            {
                // 主cfgsvr ip地址 端口号用#隔离
                // 指定了配置地址，则从配置服务器拉配置
                is_use_cfgsvr_ = true;
                master_cfgsvr_ip_.set(get_opt.optarg);
                break;
            }

            case 'i':
            {
                // 指定了服务器实体id
                instance_id_ = static_cast<unsigned short>(atoi(get_opt.optarg));
                break;
            }

            case 't':
            {
                // 指定了服务器type
                self_svc_id_.services_type_ = static_cast<unsigned short>(atoi(get_opt.optarg));
                break;
            }

            case 'p':
            {
                // 从服务器拉配置
                is_use_cfgsvr_ = true;
                break;
            }

            case 'u':
            {
                // windows卸载服务
                win_uninstall_service_ = true;
                break;
            }

            case 'm':
            {
                // windows安装服务
                win_install_service_ = true;
                break;
            }

            case 'h':
            {
                usage(argv[0]);
                exit(0);
            }

            default:
            {
                printf("unknow argu %c\n", c);
                usage(argv[0]);
                return SOAR_RET::ERR_ZERG_GET_STARTUP_CONFIG_FAIL;
            }
        }
    }

    //如果没有设置运行目录
    if (app_run_dir_.empty())
    {
        char cur_dir[PATH_MAX + 1];
        cur_dir[PATH_MAX] = 0;
        zce::getcwd(cur_dir, sizeof(cur_dir) - 1);

        app_run_dir_ = cur_dir;
    }

    log_file_prefix_ = app_run_dir_ + "/log/";
    log_file_prefix_ += Soar_Svrd_Appliction::instance()->get_app_basename();



    return 0;
}

//
int Server_Config_Base::usage(const char *program_name)
{
    std::cout << "usage: " << program_name << std::endl;
    std::cout << "   -z [zergling cfg path]" << std::endl;
    std::cout << "   -c [services cfg file]" << std::endl;
    std::cout << "   -d run as daemon" << std::endl;
    std::cout << "   -n reset channel mmp" << std::endl;
    std::cout << "   -v show version" << std::endl;
    std::cout << "   -t service type" << std::endl;
    std::cout << "   -i service index" << std::endl;
    std::cout << "   -p pull config from cfgsvr" << std::endl;
    std::cout << "   -m install app as windows servcie" << std::endl;
    std::cout << "   -u uninstall app as windows servcie" << std::endl;
    std::cout << "   -h show help info." << std::endl;
    std::cout << TSS_SERVER_VER_DECLARE << std::endl;

    return 0;
}

//读取配置文件，主要是框架的配置，包括日志，定时器等
int Server_Config_Base::read_cfgfile()
{
    int ret = 0;

    // 未指定app的配置文件，则使用默认的
    app_cfg_file_ = app_run_dir_ + "/cfg/";
    app_cfg_file_ += Soar_Svrd_Appliction::instance()->get_app_basename();
    app_cfg_file_ += ".cfg";

    // 未指定svcid配置文件
    svc_table_file_ = app_run_dir_ + "/cfg/svctable.cfg";

    // 框架的配置是不会变的
    common_cfg_file_ = app_run_dir_ + "/cfg/common.cfg";

    ZCE_Conf_PropertyTree pt_tree;
    ret = ZCE_INI_Implement::read(common_cfg_file_.c_str(), &pt_tree);
    ZCE_LOG(RS_INFO, "Application read config file [%s] ret [%d].",
            common_cfg_file_.c_str(), ret);
    if (ret != 0)
    {
        return ret;
    }

    ret = get_common_cfg(&pt_tree);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

void Server_Config_Base::dump_cfg_info(ZCE_LOG_PRIORITY out_lvl)
{
    ZCE_LOG(out_lvl, "Application base name %s svc id:%hu.%u",
            Soar_Svrd_Appliction::instance()->get_app_basename(),
            self_svc_id_.services_type_,
            self_svc_id_.services_id_);
    ZCE_LOG(out_lvl, "Application run dir :%s", app_run_dir_.c_str());
    ZCE_LOG(out_lvl, "Application log file prefix :%s", log_file_prefix_.c_str());
    ZCE_LOG(out_lvl, "Application self config file :%s", app_cfg_file_.c_str());
    ZCE_LOG(out_lvl, "Application frame work config file :%s", common_cfg_file_.c_str());
    ZCE_LOG(out_lvl, "Application svc id table config file :%s", svc_table_file_.c_str());
    ZCE_LOG(out_lvl, "Application get  :%s", svc_table_file_.c_str());
    ZCE_LOG(out_lvl, "[PIPE] if_restore_pipe_ :%s", pipe_cfg_.if_restore_pipe_ ? "TRUE" : "FALSE");
    ZCE_LOG(out_lvl, "[PIPE]recv_pipe_len_ :%lu ,send_pipe_len_ :%lu",
            pipe_cfg_.recv_pipe_len_, pipe_cfg_.send_pipe_len_);
}

//从配置中读取self_svc_id_的
int Server_Config_Base::get_common_cfg(const ZCE_Conf_PropertyTree *conf_tree)
{
    int ret = 0;
    std::string temp_value;

    ret = conf_tree->path_get_leaf("SELF_SVCID", "SERVICES_ID", temp_value);
    if (0 != ret )
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    ret = self_svc_id_.from_str(temp_value.c_str(), true);
    if (0 != ret )
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    ret = conf_tree->path_get_leaf("PIPE_CONFIG", "RECV_PIPE_LEN",
                                   pipe_cfg_.recv_pipe_len_);
    if (0 != ret
        || pipe_cfg_.recv_pipe_len_ < 1024 * 1024
        || pipe_cfg_.recv_pipe_len_ > 1024 * 1024 * 1024)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    ret = conf_tree->path_get_leaf("PIPE_CONFIG", "SEND_PIPE_LEN",
                                   pipe_cfg_.send_pipe_len_);
    if (0 != ret
        || pipe_cfg_.send_pipe_len_ < 1024 * 1024
        || pipe_cfg_.send_pipe_len_ > 1024 * 1024 * 1024)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }


    return 0;
}

//从配置中读取日志的配置
int Server_Config_Base::get_log_cfg(const ZCE_Conf_PropertyTree *conf_tree)
{
    int ret = 0;
    std::string temp_value;

    ret = conf_tree->path_get_leaf("LOG_CFG", "LOG_LEVEL", temp_value);
    log_config_.log_level_ = ZCE_LogTrace_Basic::log_priorities(temp_value.c_str());
    if (0 != ret )
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    ret = conf_tree->path_get_leaf("LOG_CFG", "FILE_DEVIDE", temp_value);
    log_config_.log_div_type_ = ZCE_LogTrace_Basic::log_file_devide(temp_value.c_str());
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    ret = conf_tree->path_get_leaf("LOG_CFG", "RESERVE_FILE_NUM", log_config_.reserve_file_num_);
    if (0 != ret )
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    //仅仅当分割方式是SIZE时有用
    ret = conf_tree->path_get_leaf("LOG_CFG", "MAX_FILE_SIZE", log_config_.max_log_file_size_);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    return 0;
}


