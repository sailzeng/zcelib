#ifndef SOARING_LIB_SERVICE_CONFIG_H_
#define SOARING_LIB_SERVICE_CONFIG_H_

#include "soar_zerg_svc_info.h"

//用XML作为配置文件还是INI作为配置文件,经过投票调查,大部分人喜欢INI,
//看来喜欢简单的土人居多,


/****************************************************************************************************
class  Services_Table_Config 服务配置信息表,通过SERVICEINFO　找到IP地址，端口号的配置,
****************************************************************************************************/


//服务配置
class SOARING_EXPORT SERVICES_INFO_TABLE
{

    typedef unordered_set<SERVICES_INFO, HASH_OF_SVCINFO, EQUAL_OF_SVCINFO> Set_Of_SvcInfo;

public:
    //构造函数,
    SERVICES_INFO_TABLE(size_t sz_table = INIT_NUM_OF_SVR_CONFG);
    ~SERVICES_INFO_TABLE();


    /*!
    * @brief      根据SvrInfo信息查询IP配置信息
    * @return     int
    * @param      svc_id      传入参数,服务器信息SVCINFO
    * @param      ip_address  输出参数,服务器信息SVCINFO
    * @param      idc_no      输出参数,IDC信息
    * @param      business_id 输出参数,服务器编号
    * @note
    */
    int find_svcinfo(const SERVICES_ID &svr_id,
                     ZCE_Sockaddr_In &ip_address,
                     unsigned int &idc_no,
                     unsigned int &business_id) const;

    //根据SvrInfo信息查询IP配置信息
    int find_svcinfo(const SERVICES_ID &svc_id,
                     SERVICES_INFO &svc_info) const;

    //检查是否拥有相应的Services Info
    bool hash_svcinfo(const SERVICES_ID &svrinfo) const;

    //设置配置信息
    int add_svcinfo(const SERVICES_INFO &svc_info);

    //得到某个配置文件的配置信息,配置文件其中有[SERVICESINFO]字段
    int loadtable_from_cfg(const ZCE_Conf_PropertyTree *conf_tree);

    //清理SVR配置信息.
    void clear();


protected:
    //配置的个数
    static const int INIT_NUM_OF_SVR_CONFG = 2048;

protected:
    //
    Set_Of_SvcInfo    services_table_;

};

#endif //SOARING_LIB_SERVICE_CONFIG_H_

