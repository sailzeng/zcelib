#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_zerg_svc_info.h"
#include "soar_server_ver_define.h"
#include "soar_svrd_app_base.h"
#include "soar_svrd_cfg_fsm.h"


Server_Config_FSM *Server_Config_FSM::instance_ = NULL;

Server_Config_FSM::Server_Config_FSM()
{
}

Server_Config_FSM::~Server_Config_FSM()
{
}



int Server_Config_FSM::load_cfgfile()
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



