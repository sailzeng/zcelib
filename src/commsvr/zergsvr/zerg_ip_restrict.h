
#ifndef ZERG_IP_RESTRICT_H_
#define ZERG_IP_RESTRICT_H_

/****************************************************************************************************
class  Zerg_IPRestrict_Mgr
****************************************************************************************************/
class Zerg_IPRestrict_Mgr
{

    typedef unordered_set<unsigned int> SetOfIPAddress;

protected:
    //
    Zerg_IPRestrict_Mgr();
    ~Zerg_IPRestrict_Mgr();

public:
    //从配置文件中得到相关的配置
    int get_iprestrict_conf(const conf_zerg::ZERG_CONFIG &cfg_file);
    //检查IP限制
    int check_iprestrict(const ZCE_Sockaddr_In &remoteaddress);

protected:

    //允许进行连接的IP地址
    SetOfIPAddress           allow_ip_set_;
    //拒绝进行连接的IP地址
    SetOfIPAddress           reject_ip_set_;

protected:
    //单子实例
    static Zerg_IPRestrict_Mgr        *instance_;

public:

    //单子实例函数
    static Zerg_IPRestrict_Mgr *instance();
    //清理单子实例
    static void clean_instance();

};

#endif //_ZERG_IP_RESTRICT_H_

