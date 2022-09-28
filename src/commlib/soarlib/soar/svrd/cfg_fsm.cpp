#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/zerg/services_info.h"
#include "soar_server_ver_define.h"
#include "soar/svrd/app_bus.h"
#include "soar/svrd/cfg_fsm.h"

svrd_cfg_fsm::svrd_cfg_fsm()
{
}

svrd_cfg_fsm::~svrd_cfg_fsm()
{
}

int svrd_cfg_fsm::read_cfgfile()
{
    // 加载zerg 配置
    int ret = 0;

    ret = svrd_cfg_base::read_cfgfile();
    if (ret != 0)
    {
        return ret;
    }

    // 配置加载成功
    ZCE_LOG(RS_INFO, "Comm_Svrd_Config load framework config succ.");
    return 0;
}