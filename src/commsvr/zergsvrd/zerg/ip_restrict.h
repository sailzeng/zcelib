#pragma once

class Zerg_Config;

namespace zerg
{
/*!
* @brief
*
* @note
*/
class IPRestrict_Mgr
{
    typedef std::unordered_set<unsigned int> Set_Of_IPAddress;

protected:
    //
    IPRestrict_Mgr();
    ~IPRestrict_Mgr();

public:

    ///从配置文件中得到相关的配置
    int get_config(const Zerg_Config *config);
    ///检查IP限制
    int check_iprestrict(const zce::Sockaddr_In &remoteaddress);

public:

    ///单子实例函数
    static IPRestrict_Mgr *instance();
    ///清理单子实例
    static void clean_instance();

protected:
    ///单子实例
    static IPRestrict_Mgr *instance_;

protected:

    //允许进行连接的IP地址
    Set_Of_IPAddress           allow_ip_set_;
    //拒绝进行连接的IP地址
    Set_Of_IPAddress           reject_ip_set_;
};
}
