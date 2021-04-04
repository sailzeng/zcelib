#ifndef WORMHOLE_SERVER_CONFIGTURE_H_
#define WORMHOLE_SERVER_CONFIGTURE_H_

#include "wormhole_proxyprocess.h"




class Wormhole_Server_Config : public Server_Config_Base
{
public:
    //
    Wormhole_Server_Config();
    virtual ~Wormhole_Server_Config();


public:

    ///��ȡ�����ļ����õ��ļ����ò���
    virtual int read_cfgfile();


    ///�������ж�ȡWormhole svrd������
    int get_wormhole_cfg(const ZCE_Conf_PropertyTree *conf_tree);

public:

    ///��������
    Interface_WH_Proxy::PROXY_TYPE  proxy_type_ = Interface_WH_Proxy::INVALID_PROXY_TYPE;

    ///�������������Ҫ
    ZCE_Conf_PropertyTree proxy_conf_tree_;

};


#endif //WORMHOLE_SERVER_CONFIGTURE_H_

