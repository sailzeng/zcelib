#include "wormhole_predefine.h"
#include "wormhole_application.h"
#include "wormhole_configture.h"


Wormhole_Server_Config::Wormhole_Server_Config()
{

}

Wormhole_Server_Config::~Wormhole_Server_Config()
{

}



int Wormhole_Server_Config::read_cfgfile()
{
    //
    int ret = 0;

    ret = Server_Config_Base::read_cfgfile();
    if (ret != 0)
    {
        return ret;
    }

    proxy_conf_tree_.clear();
    ret = ZCE_INI_Implement::read(app_cfg_file_.c_str(), &proxy_conf_tree_);
    ZCE_LOGMSG(RS_INFO, "[%s] read config file [%s] ret [%d].",
        Wormhole_Proxy_App::instance()->get_app_basename(),
        app_cfg_file_.c_str(), ret);
    if (ret != 0)
    {
        return ret;
    }

    ret = get_log_cfg(&proxy_conf_tree_);
    if (ret != 0)
    {
        return ret;
    }

    ret = get_wormhole_cfg(&proxy_conf_tree_);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}


int Wormhole_Server_Config::get_wormhole_cfg(const ZCE_Conf_PropertyTree *conf_tree)
{
    int ret = 0;
    std::string temp_value;

    //最大Accept 数量
    ret = conf_tree->path_get_leaf("PROXY_CFG", "PROXY_TYPE",
        temp_value);
    if ( 0 != ret )
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }
    
    proxy_type_ = Interface_WH_Proxy::str_to_proxytype(temp_value.c_str());
    if (proxy_type_ == Interface_WH_Proxy::INVALID_PROXY_TYPE)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    return 0;
}


