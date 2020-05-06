
#ifndef ZERG_IP_RESTRICT_H_
#define ZERG_IP_RESTRICT_H_

class Zerg_Server_Config;



/*!
* @brief
*
* @note
*/
class Zerg_IPRestrict_Mgr
{

    typedef std::unordered_set<unsigned int> Set_Of_IPAddress;

protected:
    //
    Zerg_IPRestrict_Mgr();
    ~Zerg_IPRestrict_Mgr();

public:

    ///从配置文件中得到相关的配置
    int get_config(const Zerg_Server_Config *config);
    ///检查IP限制
    int check_iprestrict(const ZCE_Sockaddr_In &remoteaddress);

public:

    ///单子实例函数
    static Zerg_IPRestrict_Mgr *instance();
    ///清理单子实例
    static void clean_instance();

protected:
    ///单子实例
    static Zerg_IPRestrict_Mgr        *instance_;

protected:

    //允许进行连接的IP地址
    Set_Of_IPAddress           allow_ip_set_;
    //拒绝进行连接的IP地址
    Set_Of_IPAddress           reject_ip_set_;

};

#endif //_ZERG_IP_RESTRICT_H_

