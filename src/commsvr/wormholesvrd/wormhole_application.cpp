#include "wormhole_predefine.h"

#include "wormhole_app_timer.h"
#include "wormhole_configture.h"
#include "wormhole_stat_define.h"
#include "wormhole_application.h"

/****************************************************************************************************
class  Arbiter_Appliction
****************************************************************************************************/
Wormhole_Proxy_App::Wormhole_Proxy_App():
    interface_proxy_(NULL)
{
}

Wormhole_Proxy_App::~Wormhole_Proxy_App()
{

    if (interface_proxy_)
    {
        delete interface_proxy_;
        interface_proxy_ = NULL;
    }
}

int Wormhole_Proxy_App::on_start(int argc, const char *argv[])
{
    int ret = 0;
    ret = Soar_Svrd_Appliction::on_start(argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    ZLOG_INFO("ArbiterAppliction::initon_start begin. ");


    Wormhole_Server_Config *wh_cfg = dynamic_cast <Wormhole_Server_Config *>(config_base_);

    // 初始化数据转发模式
    interface_proxy_ = Interface_WH_Proxy::create_proxy_factory(
        static_cast<Interface_WH_Proxy::PROXY_TYPE>(wh_cfg->proxy_type_));
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

    ZLOG_INFO("ArbiterAppliction::on_start end. ");

    return 0;
}

//
int Wormhole_Proxy_App::on_exit()
{
    ZLOG_INFO("Wormhole_Proxy_App::exit. ");
    int ret = 0;

    //最后调用通用的退出模块
    ret = Soar_Svrd_Appliction::on_exit();
    
    return 0;
}




int Wormhole_Proxy_App::reload_config()
{
    int ret = 0;

    ZCE_TRACE_FUNC_RETURN(RS_INFO, &ret);

    // 重新初始化数据转发模式
    delete interface_proxy_;

    Wormhole_Server_Config *wh_cfg = dynamic_cast <Wormhole_Server_Config *>(config_base_);
    // 初始化数据转发模式
    interface_proxy_ = Interface_WH_Proxy::create_proxy_factory(
        static_cast<Interface_WH_Proxy::PROXY_TYPE>(wh_cfg->proxy_type_));
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

int Wormhole_Proxy_App::process_recv_frame(Zerg_App_Frame *recv_frame)
{
    return interface_proxy_->process_proxy(recv_frame);
}


