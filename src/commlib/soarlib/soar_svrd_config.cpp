#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_zerg_svc_info.h"
#include "soar_server_ver_define.h"
#include "soar_svrd_application.h"
#include "soar_svrd_config.h"


Comm_Svrd_Config *Comm_Svrd_Config::instance_ = NULL;

Comm_Svrd_Config::Comm_Svrd_Config():
    self_svr_id_(0, 0),
    if_restore_pipe_(true),
    app_run_daemon_(false),
    app_install_service_(false),
    app_uninstall_service_(false),
    instance_id_(1),
    is_use_cfgsvr_(false)
{
}

Comm_Svrd_Config::~Comm_Svrd_Config()
{
}

// 取配置信息,取得配置信息后, 需要将各启动参数设置OK
int Comm_Svrd_Config::init(int argc, const char *argv[])
{
    // 处理命令行参数
    int ret = SOAR_RET::SOAR_RET_SUCC;
    ret = proc_start_arg(argc, argv);

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        return ret;
    }

    // 加载配置
    ret = load_config();

    if (ret != SOAR_RET::SOAR_RET_SUCC)
    {
        return ret;
    }

    return ret;
}

int Comm_Svrd_Config::proc_start_arg(int argc, const char *argv[])
{
    // 取得运行目录
    char cur_dir[PATH_MAX + 1];
    cur_dir[PATH_MAX] = 0;
    ZCE_OS::getcwd(cur_dir, sizeof(cur_dir) - 1);

    app_run_dir_ = cur_dir;

    // 指明RETURN_IN_ORDER 不调整顺序
    ZCE_Get_Option get_opt(argc, (char **)argv,
                           "umvndhpi:t:r:a:", 1, 0, ZCE_Get_Option::RETURN_IN_ORDER);
    int c;

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
                if_restore_pipe_ = true;
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
                ZLOG_INFO("app run dir = %s", app_run_dir_.c_str());
                app_run_dir_ = get_opt.optarg;
                break;
            }

            case 'a':
            {
                // 主cfgsvr ip地址
                // 指定了配置地址，则从配置服务器拉配置
                is_use_cfgsvr_ = true;
                master_cfgsvr_ip_ = get_opt.optarg;
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
                self_svr_id_.services_type_ = static_cast<unsigned short>(atoi(get_opt.optarg));
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
                app_uninstall_service_ = true;
                break;
            }

            case 'm':
            {
                // windows安装服务
                app_install_service_ = true;
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

    log_file_prefix_ = app_run_dir_ + "/log/";
    log_file_prefix_ += Comm_Svrd_Appliction::instance()->get_app_basename();
    log_file_prefix_ += "_init";

    // 未指定app的配置文件，则使用默认的
    app_cfg_file_ = app_run_dir_ + "/cfg/";
    app_cfg_file_ += Comm_Svrd_Appliction::instance()->get_app_basename();
    app_cfg_file_ += "_config.xml";

    // 未指定通讯服务器配置
    zerg_cfg_file_ = app_run_dir_ + "/cfg/zergsvrd.xml";

    // 未指定svcid配置文件
    svcid_cfg_file_ = app_run_dir_ + "/cfg/svcid.xml";

    // 框架的配置是不会变的
    framework_cfg_file_ = app_run_dir_ + "/cfg/framework.xml";

    return SOAR_RET::SOAR_RET_SUCC;
}

//
int Comm_Svrd_Config::usage(const char *program_name)
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

    return SOAR_RET::SOAR_RET_SUCC;
}

//单子实例函数
Comm_Svrd_Config *Comm_Svrd_Config::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Comm_Svrd_Config();
    }

    return instance_;
}

//清理单子实例
void Comm_Svrd_Config::clean_instance()
{
    delete instance_;
    instance_ = NULL;
    return;
}

int
Comm_Svrd_Config::load_config()
{
    // 加载zerg 配置
    int ret = zerg_config_.fromXmlFile(zerg_cfg_file_.c_str());

    if (ret != 0)
    {
        ZLOG_ERROR("Comm_Svrd_Config: load zerg config fail. ret=%d, path=%s", ret, zerg_cfg_file_.c_str());
        return SOAR_RET::ERROR_FRAMEWORK_READ_ZERG_CFG_FAIL;
    }

    self_svr_id_.services_id_ = zerg_config_.self_cfg.self_svr_info.svr_id;
    self_svr_id_.services_type_ = zerg_config_.self_cfg.self_svr_info.svr_type;

    // 加载framework的配置
    ret = framework_config_.fromXmlFile(framework_cfg_file_.c_str());

    if (ret != 0)
    {
        // 加载framework配置失败
        ZLOG_ERROR("Comm_Svrd_Config: load framework config fail.ret=%d", ret);
        return SOAR_RET::ERROR_FRAMEWORK_READ_SELF_CFG_FAIL;
    }

    // 重置监控uin列表
    monitor_uin_set_.clear();

    for (unsigned int i = 0; i < framework_config_.log_info_.monitor_uin_count_; ++i)
    {
        monitor_uin_set_.insert(framework_config_.log_info_.monitor_uin_list_[i]);
    }

    // 总感觉这样处理好不好，框架需要感知zerg的配置来进行初始化 by stefzhou
    if ((strcasecmp(zerg_config_.soar_cfg.get_svr_info_type, "cfgfile")) == 0)
    {
        // 加载svcid配置
        ret = svcid_config_.fromXmlFile(svcid_cfg_file_.c_str());

        if (ret != 0)
        {
            // 加载svcid配置失败
            ZLOG_ERROR("Comm_Svrd_Config: load svcid config fail. ret=%d", ret);
            return SOAR_RET::ERROR_FRAMEWORK_READ_SVCID_CFG_FAIL;
        }
    }

    log_file_prefix_ = app_run_dir_ + "/log/";
    log_file_prefix_ += Comm_Svrd_Appliction::instance()->get_app_basename();

    // 配置加载成功
    ZLOG_INFO("Comm_Svrd_Config: load framework config succ.");
    return SOAR_RET::SOAR_RET_SUCC;
}

int
Comm_Svrd_Config::reload()
{
    ZLOG_INFO("app start reload");
    return load_config();
}

bool
Comm_Svrd_Config::is_monitor_uin(unsigned int uin)
{
    return (monitor_uin_set_.find(uin) != monitor_uin_set_.end());
}
