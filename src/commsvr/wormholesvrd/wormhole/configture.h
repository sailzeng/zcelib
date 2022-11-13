#pragma once

#include "proxy_process.h"

namespace wormhole
{
class configure : public soar::svrd_cfg_base
{
public:
    //
    configure();
    virtual ~configure();

public:

    ///读取配置文件，得到文件配置参数
    virtual int read_cfgfile();

    ///从配置中读取Wormhole svrd的配置
    int get_wormhole_cfg(const zce::propertytree* conf_tree);

public:

    ///代理类型
    proxy_interface::PROXY_TYPE  proxy_type_ = proxy_interface::INVALID_PROXY_TYPE;

    ///代理的配置数据要
    zce::propertytree proxy_conf_tree_;
};
}
