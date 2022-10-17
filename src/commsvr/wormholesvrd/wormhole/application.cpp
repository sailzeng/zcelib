#include "predefine.h"
#include "configture.h"
#include "stat_define.h"
#include "application.h"

/****************************************************************************************************
class  Arbiter_Appliction
****************************************************************************************************/
namespace wormhole
{
application::application() :
    interface_proxy_(NULL)
{
}

application::~application()
{
    if (interface_proxy_)
    {
        delete interface_proxy_;
        interface_proxy_ = NULL;
    }
}

int application::app_start(int argc, const char* argv[])
{
    int ret = 0;
    ret = soar::app_buspipe::app_start(argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    ZCE_LOG(RS_INFO, "ArbiterAppliction::initon_start begin. ");

    wormhole::configure* wh_cfg = dynamic_cast <wormhole::configure*>(config_base_);

    // 初始化数据转发模式
    interface_proxy_ = proxy_i::create_proxy_factory(
        static_cast<proxy_i::PROXY_TYPE>(wh_cfg->proxy_type_));
    ZCE_ASSERT(interface_proxy_);

    ret = interface_proxy_->get_proxy_config(&(wh_cfg->proxy_conf_tree_));

    if (ret != 0)
    {
        return ret;
    }

    ret = interface_proxy_->init_proxy_instance();

    if (ret != 0)
    {
        return ret;
    }

    ZCE_LOG(RS_INFO, "ArbiterAppliction::app_start end. ");

    return 0;
}

//
int application::app_exit()
{
    ZCE_LOG(RS_INFO, "application::exit. ");
    int ret = 0;

    //最后调用通用的退出模块
    ret = soar::app_buspipe::app_exit();
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int application::reload_config()
{
    int ret = 0;

    ZCE_TRACE_FUNC_RETURN(RS_INFO, &ret);

    // 重新初始化数据转发模式
    delete interface_proxy_;

    wormhole::configure* wh_cfg = dynamic_cast <wormhole::configure*>(config_base_);
    // 初始化数据转发模式
    interface_proxy_ = proxy_i::create_proxy_factory(
        static_cast<proxy_i::PROXY_TYPE>(wh_cfg->proxy_type_));
    ZCE_ASSERT(interface_proxy_);

    ret = interface_proxy_->get_proxy_config(&(wh_cfg->proxy_conf_tree_));
    if (ret != 0)
    {
        return ret;
    }

    ret = interface_proxy_->init_proxy_instance();
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int application::process_recv_frame(soar::zerg_frame* recv_frame)
{
    return interface_proxy_->process_proxy(recv_frame);
}
}