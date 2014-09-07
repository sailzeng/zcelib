
#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_enum_define.h"
#include "soar_zerg_svc_table.h"

/****************************************************************************************************
class  ServicesConfig  服务配置信息表,通过SERVICEINFO　找到IP地址，端口号的配置,
****************************************************************************************************/
SERVICES_INFO_TABLE::SERVICES_INFO_TABLE(size_t szconf)
{
    services_table_.rehash(szconf);
}

SERVICES_INFO_TABLE::~SERVICES_INFO_TABLE()
{
}


int SERVICES_INFO_TABLE::find_svcinfo(const SERVICES_ID &svc_id,
                                      ZCE_Sockaddr_In &ip_address,
                                      unsigned int &idc_no,
                                      unsigned int &business_id) const
{
    SERVICES_INFO svc_ip_info;
    svc_ip_info.svc_id_ = svc_id;
    Set_Of_SvcInfo::iterator iter = services_table_.find(svc_ip_info);

    if (iter == services_table_.end() )
    {
        ZCE_LOGMSG(RS_ERROR, "[soarlib]Can't find svc id [%u|%u] info.", 
            svc_id.services_type_,
            svc_id.services_id_);
        return SOAR_RET::ERROR_SERVICES_INFO_CONFIG;
    }

    ip_address = iter->ip_address_;
    idc_no = iter->idc_no_;
    business_id = iter->business_id_;
    
    return SOAR_RET::SOAR_RET_SUCC;
}


//根据SvrInfo信息查询IP配置信息
int SERVICES_INFO_TABLE::find_svcinfo(const SERVICES_ID &svc_id,
    SERVICES_INFO &svc_info) const
{
    svc_info.svc_id_ = svc_id;
    Set_Of_SvcInfo::iterator iter = services_table_.find(svc_info);

    if (iter == services_table_.end())
    {
        ZCE_LOGMSG(RS_ERROR, "[soarlib]Can't find svc id [%u|%u] info.",
            svc_id.services_type_,
            svc_id.services_id_);
        return SOAR_RET::ERROR_SERVICES_INFO_CONFIG;
    }
    svc_info = *iter;

    return SOAR_RET::SOAR_RET_SUCC;
}

//检查是否拥有相应的Services Info
bool SERVICES_INFO_TABLE::hash_svcinfo(const SERVICES_ID &svrinfo) const
{
    SERVICES_INFO svc_ip_info;
    svc_ip_info.svc_id_ = svrinfo;
    Set_Of_SvcInfo::iterator iter = services_table_.find(svc_ip_info);

    if (iter == services_table_.end() )
    {
        return false;
    }

    return true;
}

//设置配置信息
int SERVICES_INFO_TABLE::add_svcinfo(const SERVICES_INFO &svc_info)
{

    std::pair<Set_Of_SvcInfo::iterator, bool> insert_result = services_table_.insert(svc_info);

    if (insert_result.second == false)
    {
        return SOAR_RET::ERROR_SERVICES_INFO_CONFIG;
    }

    return SOAR_RET::SOAR_RET_SUCC;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2005年11月17日
Function        : ServicesConfig::LoadSvrConfig
Return          : int == 0表示成功
Parameter List  :
  Param1: const char* cfgfile
Description     : 得到某个服务配置文件的配置信息
Calls           :
Called By       :
Other           : 配置文件其中有[SERVICESINFO]字段.原有的数据会清空.
Modify Record   :
******************************************************************************************/
//int ServicesConfig::LoadSvrConfig(const char *cfgfile)
//{
//    int ret = 0;
//    const int BUFFER_LEN = 128;
//    char tmpkey[BUFFER_LEN],tmpbuf[BUFFER_LEN],outbuf[BUFFER_LEN];
//
//    ZCE_INI_Implemention   read_cfg;
//    zce_INI_PropertyTree   svrinfo_cfg;
//
//    ZLOG_INFO("[framework] Read Sevices Info Configure File. [%s] .",cfgfile);
//    ret = read_cfg.read(cfgfile,svrinfo_cfg);
//    snprintf(outbuf,BUFFER_LEN,"Can't Open Sevices Info Configure file %s.",cfgfile);
//    TESTCONFIG(ret == 0,outbuf);
//
//    uint32_t num_svr;
//    ret = svrinfo_cfg.get_uint32_value("SERVICESINFO","NUMSVRINFO",num_svr);
//    TESTCONFIG(( ret == 0 && num_svr != 0),"SERVICESINFO|NUMSVRINFO key error.");
//
//
//    uint32_t int_value;
//
//    SERVICES_IP_INFO svc_ip_info;
//
//    //清理掉原来的数据.
//    services_conf_map_.clear();
//
//    for (uint32_t i=1;i<=num_svr;++i)
//    {
//        //记录服务器编号
//        svc_ip_info.server_guid_no_ = i;
//
//        snprintf(tmpkey,BUFFER_LEN,"SERVICESTYPE%u",i);
//        ret = svrinfo_cfg.get_uint32_value("SERVICESINFO",tmpkey,int_value);
//        snprintf(outbuf,BUFFER_LEN,"SERVICESINFO|%s key error.",tmpkey);
//        TESTCONFIG((ret ==0 && int_value != 0),outbuf);
//        svc_ip_info.services_info_.services_type_ = static_cast<unsigned short>( int_value);
//
//        snprintf(tmpkey,BUFFER_LEN,"SERVICESID%u",i);
//        ret = svrinfo_cfg.get_uint32_value("SERVICESINFO",tmpkey,int_value);
//        snprintf(outbuf,128,"SERVICESINFO|%s key error.",tmpkey);
//        TESTCONFIG((ret ==0 && int_value != 0),outbuf);
//        svc_ip_info.services_info_.services_id_ = static_cast<unsigned int>(int_value);
//
//        snprintf(tmpkey,BUFFER_LEN,"SERVICEIP%u",i);
//        ret = svrinfo_cfg.get_string_value("SERVICESINFO",tmpkey,tmpbuf,BUFFER_LEN);
//        snprintf(outbuf,BUFFER_LEN,"SERVICESINFO|%s key error.",tmpkey);
//        TESTCONFIG((ret ==0 ),outbuf);
//
//        snprintf(tmpkey,BUFFER_LEN,"SERVICESPORT%u",i);
//        ret = svrinfo_cfg.get_uint32_value("SERVICESINFO",tmpkey,int_value);
//        snprintf(outbuf,BUFFER_LEN,"SERVICESINFO|%s key error.",tmpkey);
//        TESTCONFIG((ret ==0 && int_value != 0),outbuf);
//
//        //检查IP地址的写法,不是多此一举
//        ZCE_Sockaddr_In inetadd;
//        ret = inetadd.set(tmpbuf,
//            static_cast<unsigned short>(int_value));
//
//        snprintf(outbuf,BUFFER_LEN,"SERVICESINFO|SERVICEIP|%s key error.",tmpkey);
//        TESTCONFIG((ret == 0),outbuf);
//
//        svc_ip_info.ip_addr_ = inetadd.get_ip_address();
//        svc_ip_info.port_ = inetadd.get_port_number();
//
//        //得到IDC信息
//        snprintf(tmpkey,BUFFER_LEN,"SERVICESIDC%u",i);
//        ret = svrinfo_cfg.get_uint32_value("SERVICESINFO",tmpkey,int_value);
//        snprintf(outbuf,BUFFER_LEN,"SERVICESINFO|SERVICEIP|%s key error.",tmpkey);
//        TESTCONFIG((ret == 0),outbuf);
//        svc_ip_info.idc_no_ = int_value;
//
//
//        //配置信息不要超过32个字母LEN_OF_NAME_BUFFER= MAX_NAME_LEN_STRING+1
//        char cfg_info[MAX_NAME_LEN_STRING+1];
//        //得到IDC信息
//        snprintf(tmpkey,BUFFER_LEN,"SVCCFGINFO%u",i);
//        ret = svrinfo_cfg.get_string_value("SERVICESINFO",tmpkey,cfg_info,MAX_NAME_LEN_STRING);
//        snprintf(outbuf,BUFFER_LEN,"SERVICESINFO|SERVICEIP|%s key error.",tmpkey);
//        TESTCONFIG((ret == 0),outbuf);
//        strncpy(svc_ip_info.cfg_info_ ,cfg_info,MAX_NAME_LEN_STRING);
//        svc_ip_info.cfg_info_[MAX_NAME_LEN_STRING]='\0';
//
//        //如果有重复的我只能默哀了
//        std::pair<SetOfServicesIPInfo::iterator,bool> insert_result = services_conf_map_.insert(svc_ip_info);
//        if (insert_result.second == false)
//        {
//            ZLOG_ERROR("[framework] ServicesConfig::LoadSvrConfig svrinfo repeat type:%u id:%u",
//                svc_ip_info.services_info_.services_type_, svc_ip_info.services_info_.services_id_);
//            return SOAR_RET::ERROR_SERVICES_INFO_CONFIG;
//        }
//    }
//
//    return SOAR_RET::SOAR_RET_SUCC;
//}



void SERVICES_INFO_TABLE::clear()
{
    services_table_.clear();
}

///******************************************************************************************
//Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2006年5月7日
//Function        : FindSelfSERVICES_INFO
//Return          : static int
//Parameter List  :
//  Param1: const char* cfgfile
//  Param2: SERVICES_ID& svrinfo
//Description     : 查询自己的Services Info配置,配置文件其中有[SELFCFG]字段
//Calls           :
//Called By       :
//Other           :
//Modify Record   :
//******************************************************************************************/
//int Services_Table_Config::FindSelfSERVICES_INFO(const char *cfgfile, SERVICES_ID &svrinfo)
//{
//    //只使用一次,有点浪费,所以不要频繁使用这个函数
//    int ret = 0;
//    const int BUFFER_LEN = 128;
//    char outbuf[BUFFER_LEN];
//    uint32_t int_value;
//
//    ZCE_INI_Implement   read_cfg;
//    ZCE_INI_PropertyTree   self_svr_cfg;
//
//    ZLOG_INFO("[framework] Read Self Services Configure File. [%s] .", cfgfile);
//    ret = read_cfg.read(cfgfile, self_svr_cfg);
//    snprintf(outbuf, 256, "Can't Self Services Configure file [%s].", cfgfile);
//    TESTCONFIG(ret == 0, outbuf);
//
//    //读取自己的SVR INFO配置
//    ret = self_svr_cfg.get_uint32_value("SELFCFG", "SELFSVRTYPE", int_value);
//    TESTCONFIG((ret == 0 && int_value != 0), "SELFCFG|SELFSVRTYPE key error.");
//    svrinfo.services_type_ = static_cast<unsigned short>(int_value);
//
//    ret = self_svr_cfg.get_uint32_value("SELFCFG", "SELFSVRID", int_value);
//    TESTCONFIG((ret == 0 && int_value != 0), "SELFCFG|SELFSVRTYPE key error.");
//    svrinfo.services_id_ = static_cast<unsigned int>(int_value);
//
//    return SOAR_RET::SOAR_RET_SUCC;
//}
//
