
#include "zerg_predefine.h"
#include "zerg_ip_restrict.h"


//实例
Zerg_IPRestrict_Mgr *Zerg_IPRestrict_Mgr::instance_ = NULL;

/****************************************************************************************************
class  Zerg_IPRestrict_Mgr 处理通讯中间的IP限制问题
****************************************************************************************************/
Zerg_IPRestrict_Mgr::Zerg_IPRestrict_Mgr()
{
}
//自己清理的类型，统一关闭在handle_close,这个地方不用关闭
Zerg_IPRestrict_Mgr::~Zerg_IPRestrict_Mgr()
{
}

//从配置文件中得到相关的配置
int Zerg_IPRestrict_Mgr::get_iprestrict_conf(const conf_zerg::ZERG_CONFIG &config)
{
    int ret = 0;
    //    unsigned int tmp_uint= 0;

    allow_ip_set_.clear();
    reject_ip_set_.clear();

    const size_t TMP_BUFFER_LEN = 256;
    //char tmp_key[TMP_BUFFER_LEN+1] = {0};
    //char tmp_value[TMP_BUFFER_LEN+1] = {0};
    char err_outbuf[TMP_BUFFER_LEN + 1] = {0};

    //ret  = cfg_file.get_uint32_value("RESTRICT","NUMALLOW",tmp_uint);
    std::vector<std::string> v;
    split(config.restrict_cfg.allow_ips, " ", v);
    allow_ip_set_.rehash(v.size());

    //读取运行连接的服务器IP地址
    for (unsigned int i = 0; i < v.size(); ++i )
    {

        //tmp_key[0] = '\0';
        //snprintf(tmp_key,sizeof(tmp_key)-1,"ALLOWIP%d",i);
        //snprintf(err_outbuf,TMP_BUFFER_LEN,"RESTRICT|%s key error.",tmp_key);
        //
        //cfg_file.get_string_value("RESTRICT",tmp_key,tmp_value,TMP_BUFFER_LEN);
        ////TESTCONFIG((ret == 0),err_outbuf);

        ZCE_Sockaddr_In     inetadd;
        ret = inetadd.set(v[i].c_str(), 10);
        TESTCONFIG((ret == 0), err_outbuf);

        allow_ip_set_.insert(inetadd.get_ip_address());
    }

    //读取拒绝连接的服务器IP地址
    //ret  = cfg_file.get_uint32_value("RESTRICT","NUMREJECT",tmp_uint);
    v.clear();
    split(config.restrict_cfg.reject_ips, " ", v);
    reject_ip_set_.rehash(v.size());

    for (unsigned int i = 0; i < v.size(); ++i)
    {
        //tmp_key[0] = '\0';
        //snprintf(tmp_key,sizeof(tmp_key)-1,"REJECTIP%d",i+1);
        //snprintf(err_outbuf,TMP_BUFFER_LEN,"RESTRICT|%s key error.",tmp_key);

        //ret = cfg_file.get_string_value("RESTRICT",tmp_key,tmp_value,TMP_BUFFER_LEN);
        //TESTCONFIG((ret == 0),err_outbuf);

        ZCE_Sockaddr_In     inetadd;
        ret = inetadd.set(v[i].c_str(), 10);
        TESTCONFIG((ret == 0), err_outbuf);

        reject_ip_set_.insert(inetadd.get_ip_address());
    }

    return 0;
}

int Zerg_IPRestrict_Mgr::check_iprestrict(const ZCE_Sockaddr_In &remoteaddress)
{

    //如果允许的连接的服务器地址中间没有... kill
    if (allow_ip_set_.empty() == false )
    {
        Set_Of_IPAddress::iterator iter = allow_ip_set_.find(remoteaddress.get_ip_address());

        if ( iter == allow_ip_set_.end() )
        {
            ZLOG_INFO("[zergsvr] A NO Allowed IP|Port : [%s|%u] Connect me.",
                      remoteaddress.get_host_addr(),
                      remoteaddress.get_port_number());
            return SOAR_RET::ERR_ZERG_IP_RESTRICT_CHECK_FAIL;
        }
    }

    //如果是要被拒绝连接的IP地址...kill
    if (reject_ip_set_.empty() == false )
    {
        Set_Of_IPAddress::iterator iter = reject_ip_set_.find(remoteaddress.get_ip_address());

        if ( iter != reject_ip_set_.end() )
        {
            ZLOG_INFO("[zergsvr] Reject IP|Port : %s|%u connect me.",
                      remoteaddress.get_host_addr(),
                      remoteaddress.get_port_number());
            return SOAR_RET::ERR_ZERG_IP_RESTRICT_CHECK_FAIL;
        }
    }

    return SOAR_RET::SOAR_RET_SUCC;
}

//单子实例函数
Zerg_IPRestrict_Mgr *Zerg_IPRestrict_Mgr::instance()
{
    //如果没有初始化
    if (instance_ == NULL)
    {
        instance_ = new Zerg_IPRestrict_Mgr();
    }

    return instance_;
}

//清理实例
void Zerg_IPRestrict_Mgr::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }
}

