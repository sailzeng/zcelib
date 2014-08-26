#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_zerg_svc_info.h"
#include "soar_server_ver_define.h"
#include "soar_svrd_application.h"
#include "soar_svrd_cfg_trans.h"


Comm_Svrd_Config *Comm_Svrd_Config::instance_ = NULL;

Comm_Svrd_Config::Comm_Svrd_Config()
{
}

Comm_Svrd_Config::~Comm_Svrd_Config()
{
}



int Comm_Svrd_Config::load_cfgfile()
{
    // 加载zerg 配置
    int ret = 0;

    ret = ZCE_INI_Implement::read(zerg_cfg_file_.c_str(), &zerg_ptree_);
    if (ret != 0)
    {
        return SOAR_RET::ERROR_FRAMEWORK_READ_ZERG_CFG_FAIL;
    }

    ret = ZCE_INI_Implement::read(framework_cfg_file_.c_str(), &framework_ptree_);
    if (ret != 0)
    {
        return SOAR_RET::ERROR_FRAMEWORK_READ_ZERG_CFG_FAIL;
    }

    // 配置加载成功
    ZLOG_INFO("Comm_Svrd_Config: load framework config succ.");
    return SOAR_RET::SOAR_RET_SUCC;
}

int Comm_Svrd_Config::reload_cfgfile()
{
    ZLOG_INFO("app start reload");
    return load_cfgfile();
}


