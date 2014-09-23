
#ifndef OGRE4A_IP_RESTRICT_H_
#define OGRE4A_IP_RESTRICT_H_

/****************************************************************************************************
class  Ogre4aIPRestrictMgr
****************************************************************************************************/
class Ogre4aIPRestrictMgr
{

    typedef hash_set<unsigned int> SetOfIPAddress;

protected:
    Ogre4aIPRestrictMgr();
    ~Ogre4aIPRestrictMgr();

public:

    //从配置文件中得到相关的配置
    int get_ip_restrict_conf(Zen_INI_PropertyTree &cfg_file);
    //检查IP限制
    int check_ip_restrict(const ZEN_Sockaddr_In &remoteaddress);

protected:

    //允许进行连接的IP地址
    SetOfIPAddress                  allow_ip_set_;
    //拒绝进行连接的IP地址
    SetOfIPAddress                  reject_ip_set_;

protected:
    //单子实例
    static Ogre4aIPRestrictMgr     *instance_;

public:

    //单子实例函数
    static Ogre4aIPRestrictMgr *instance();
    //清理单子实例
    static void clean_instance();
};

#endif //_OGRE4A_IP_RESTRICT_H_

