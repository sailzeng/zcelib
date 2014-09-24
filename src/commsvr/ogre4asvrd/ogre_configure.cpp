

#include "ogre_predefine.h"
#include "ogre_configure.h"
#include "ogre_tcp_ctrl_handler.h"
#include "ogre_comm_manager.h"

Ogre_Svr_Config *Ogre_Svr_Config::instance_ = NULL;

//
Ogre_Svr_Config::Ogre_Svr_Config():
    self_svr_info_(0, 0),
    if_restore_pipe_(true),
    zerg_insurance_(true)
{
    ogre_cfg_path_ = "./cfg/ogre4ad.conf";
    log_file_prefix_ = "./log/ogre4ad";
}

Ogre_Svr_Config::~Ogre_Svr_Config()
{
}

int Ogre_Svr_Config::print_startup_paraminfo(const char *programname)
{
    std::cout << "Usage: " << programname << std::endl;
    std::cout << "   -z [ogre_cfg_path_]" << std::endl;
    std::cout << "   -n         : if restroe." << std::endl;
    std::cout << "   -f         : if open ACE log stderr." << std::endl;
    std::cout << "   -h         : Help." << std::endl;
    return 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月17日
Function        : Ogre_Svr_Config::get_startup_param
Return          : int
Parameter List  :
  Param1: int argc
  Param2: char* argv[]
Description     : 得到启动参数,进行配置
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_Svr_Config::get_startup_param(int argc, char *argv[])
{
    std::string process_mod_name = argv[0];
    //在这里保证配置和进程名相同
    ogre_cfg_path_ = "./cfg/" + process_mod_name + ".conf";

    ZCE_Get_Option get_opt(argc, argv, "nfhs:", 0);
    int c;

    while ((c = get_opt()) != EOF)
    {
        switch (c)
        {
            case 'n':
                if_restore_pipe_ = true;
                break;

            case 's':
                ogre_cfg_path_ = get_opt.optarg;
                break;

                //返回错误是让程序不继续运行
            case 'h':
            default:
                print_startup_paraminfo(argv[0]);
                return SOAR_RET::ERROR_GET_STARTUP_CONFIG_FAIL;
        }
    }

    return 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2005年11月17日
Function        : Ogre_Svr_Config::get_file_configure
Return          : int
Parameter List  : NULL
Description     : 得到文件配置参数
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Ogre_Svr_Config::get_file_configure()
{
    //
    int ret = 0;
    unsigned int tmp_uint = 0;
    char tmpbuf[256];

    ZCE_INI_Implemention ini_read;

    ret = ini_read.read(ogre_cfg_path_.c_str(), cfg_ogre4a_);
    snprintf(tmpbuf, 256, "Can't Open Ogre4a Configure file [%s].", ogre_cfg_path_.c_str());
    TESTCONFIG(ret == 0, tmpbuf);

    //读取自己的配置
    unsigned short svrtype = 0;
    ret = cfg_ogre4a_.get_uint32_value("SELFCFG", "SELFSVRTYPE", tmp_uint );
    svrtype = static_cast<unsigned short>(tmp_uint) ;
    TESTCONFIG((ret == 0 && svrtype != 0), "SELFCFG|SELFSVRTYPE key error.");

    unsigned int svrid = 0;
    ret = cfg_ogre4a_.get_uint32_value("SELFCFG", "SELFSVRID", tmp_uint);
    svrid = tmp_uint;
    TESTCONFIG((ret == 0 && svrid != 0), "SELFCFG|SELFSVRTYPE key error.");

    self_svr_info_.set_serviceid(svrtype, svrid);

    //最大的帧长度,在APPFRAME的长度基础上可以在限制,主要是写入的数据大小
    ret = cfg_ogre4a_.get_uint32_value("COMMCFG", "MAXFRAMELEN", tmp_uint);
    max_data_len_ = tmp_uint;
    TESTCONFIG((ret == 0 && max_data_len_ >= 1024 && max_data_len_ <= 1024 * 1024), "COMMCFG|MAXFRAMELEN key error.");

    //检查保险是否打开
    ret = cfg_ogre4a_.get_bool_value("COMMCFG", "INSURANCE", zerg_insurance_);
    TESTCONFIG(ret == 0, "COMMCFG|INSURANCE key error.");

    //
    Ogre4a_AppFrame::SetMaxFrameDataLen(max_data_len_);

    std::string tmpstr;
    //日志前缀
    ret = cfg_ogre4a_.get_string_value("LOGCFG", "LOGPREFIX", tmpstr);
    TESTCONFIG((ret == 0) , "LOGCFG|LOGPREFIX key error.n");
    log_file_prefix_ = tmpstr.c_str();

    ret = cfg_ogre4a_.get_string_value("LOGCFG", "PRIORITY", tmpstr);
    TESTCONFIG(ret == 0, "LOGCFG|PRIORITY");
    log_priority_ = ZCE_LogTrace_Basic::log_priorities(tmpstr.c_str());

    //Ogre_Comm_Manger 读取配置文件
    ret = Ogre_Comm_Manger::instance()->get_configure(cfg_ogre4a_);

    if (0 != ret )
    {
        return ret;
    }

    ZLOG_INFO( "Get File Configure Success.\n");

    return 0;
}

//单子实例获得函数
Ogre_Svr_Config *Ogre_Svr_Config::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Ogre_Svr_Config();
    }

    return instance_;
}

//单子实例清理函数
void Ogre_Svr_Config::clean_instance()
{
    if (instance_ != NULL)
    {
        delete instance_;
        instance_ = NULL;
    }
}

