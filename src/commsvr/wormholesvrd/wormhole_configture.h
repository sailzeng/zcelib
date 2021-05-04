#ifndef WORMHOLE_SERVER_CONFIGTURE_H_
#define WORMHOLE_SERVER_CONFIGTURE_H_

#include "wormhole_proxyprocess.h"

class Wormhole_Server_Config: public Server_Config_Base
{
public:
    //
    Wormhole_Server_Config();
    virtual ~Wormhole_Server_Config();

public:

    ///读取配置文件，得到文件配置参数
    virtual int read_cfgfile();

    ///从配置中读取Wormhole svrd的配置
    int get_wormhole_cfg(const ZCE_Conf_PropertyTree *conf_tree);

public:

    ///代理类型
    Interface_WH_Proxy::PROXY_TYPE  proxy_type_ = Interface_WH_Proxy::INVALID_PROXY_TYPE;

    ///代理的配置数据要
    ZCE_Conf_PropertyTree proxy_conf_tree_;
};

#endif //WORMHOLE_SERVER_CONFIGTURE_H_
