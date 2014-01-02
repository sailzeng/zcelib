#ifndef SOARING_LIB_SERVICE_CONFIG_H_
#define SOARING_LIB_SERVICE_CONFIG_H_

#include "soar_service_info.h"

//用XML作为配置文件还是INI作为配置文件,经过投票调查,大部分人喜欢INI,
//看来喜欢简单的土人居多,
/****************************************************************************************************
class  ServicesConfig 服务配置信息表,通过SERVICEINFO　找到IP地址，端口号的配置,
****************************************************************************************************/

typedef unordered_set<SERVICES_IP_INFO, HashofSvrIPInfo, EqualSvrIPInfo> SetOfServicesIPInfo;
//服务配置
class SOARING_EXPORT ServicesConfig
{
protected:
    //配置的个数
    static const int INIT_NUM_OF_SVR_CONFG = 2048;

protected:

    //
    SetOfServicesIPInfo    services_conf_map_;

public:
    //构造函数,
    ServicesConfig(size_t szconf = INIT_NUM_OF_SVR_CONFG);
    ~ServicesConfig();

    //根据SvrInfo信息查询IP配置信息
    int FindServicesIPConf(const SERVICES_ID &svrinfo,
                           unsigned int &ipaddress,
                           unsigned short &port,
                           unsigned int &idc_no,
                           unsigned int &server_guid_no,
                           char *cfg_info = NULL);
    //根据SvrInfo信息查询IP配置信息
    int FindServicesIPConf(const SERVICES_ID &svrinfo,
                           ZCE_Sockaddr_In &inetadd,
                           unsigned int &idc_no,
                           unsigned int &server_guid_no,
                           char *cfg_info = NULL);

    //检查是否拥有相应的Services Info
    bool IfHashSERVICES_INFO(const SERVICES_ID &svrinfo);

    //设置配置信息
    int AddServicesConf(const SERVICES_ID &svrinfo, const ZCE_Sockaddr_In &inetadd, unsigned int idc_no);

    //得到某个配置文件的配置信息,配置文件其中有[SERVICESINFO]字段
    //int LoadSvrConfig(const char *cfgfile);
    int LoadSvrConfig(const conf_svcid::SVCID_CONFIG *config);
    //清理SVR配置信息.
    void ClearSvrConfig();

public:
    //
    static int FindSelfSERVICES_INFO(const char *cfgfile, SERVICES_ID &svrinfo);
};

#endif //SOARING_LIB_SERVICE_CONFIG_H_

