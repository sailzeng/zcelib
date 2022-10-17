#include "predefine.h"
#include "application.h"
#include "configture.h"

namespace wormhole
{
configure::configure()
{
}

configure::~configure()
{
}

int configure::read_cfgfile()
{
    //
    int ret = 0;

    ret = svrd_cfg_base::read_cfgfile();
    if (ret != 0)
    {
        return ret;
    }

    proxy_conf_tree_.clear();
    ret = zce::cfg::read_ini(app_cfg_file_.c_str(), &proxy_conf_tree_);
    ZCE_LOG(RS_INFO, "[%s] read config file [%s] ret [%d].",
            application::instance()->get_app_basename(),
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

int configure::get_wormhole_cfg(const zce::propertytree* conf_tree)
{
    int ret = 0;
    std::string temp_value;

    //最大Accept 数量
    ret = conf_tree->path_get_leaf("PROXY_CFG", "PROXY_TYPE",
                                   temp_value);
    if (0 != ret)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    proxy_type_ = proxy_i::str_to_proxytype(temp_value.c_str());
    if (proxy_type_ == proxy_i::INVALID_PROXY_TYPE)
    {
        SOAR_CFG_READ_FAIL(RS_ERROR);
        return SOAR_RET::ERROR_GET_CFGFILE_CONFIG_FAIL;
    }

    return 0;
}
}