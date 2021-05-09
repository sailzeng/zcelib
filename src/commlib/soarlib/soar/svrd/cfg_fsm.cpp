#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/zerg/services_info.h"
#include "soar_server_ver_define.h"
#include "soar/svrd/app_base.h"
#include "soar/svrd/cfg_fsm.h"

Server_Config_FSM::Server_Config_FSM()
{
}

Server_Config_FSM::~Server_Config_FSM()
{
}

int Server_Config_FSM::read_cfgfile()
{
    // 加载zerg 配置
    int ret = 0;

    ret = Server_Config_Base::read_cfgfile();
    if (ret != 0)
    {
        return ret;
    }

    // 配置加载成功
    ZCE_LOG(RS_INFO, "Comm_Svrd_Config load framework config succ.");
    return 0;
}