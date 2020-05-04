
#ifndef OGRE4A_IP_RESTRICT_H_
#define OGRE4A_IP_RESTRICT_H_

/****************************************************************************************************
class  Ogre4aIPRestrictMgr
****************************************************************************************************/
class Ogre_IPRestrict_Mgr
{

    typedef std::unordered_set<unsigned int> SET_OF_IPADDRESS;

protected:

    Ogre_IPRestrict_Mgr();
    ~Ogre_IPRestrict_Mgr();

public:

    ///从配置文件中得到相关的配置
    int get_config(const Ogre_Server_Config *config);
    //检查IP限制
    int check_ip_restrict(const ZCE_Sockaddr_In &remoteaddress);

public:

    //单子实例函数
    static Ogre_IPRestrict_Mgr *instance();
    //清理单子实例
    static void clean_instance();

protected:

    //允许进行连接的IP地址
    SET_OF_IPADDRESS                  allow_ip_set_;
    //拒绝进行连接的IP地址
    SET_OF_IPADDRESS                  reject_ip_set_;

protected:
    //单子实例
    static Ogre_IPRestrict_Mgr     *instance_;
};

#endif //OGRE4A_IP_RESTRICT_H_

