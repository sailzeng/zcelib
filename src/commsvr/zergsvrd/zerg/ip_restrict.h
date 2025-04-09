#pragma once

class zerg_config;

namespace zerg
{
/*!
* @brief
*
* @note
*/
class ip_restrict
{
    using SET_OF_IPADDRESS = std::unordered_set<unsigned int>;

protected:
    //
    ip_restrict();
    ~ip_restrict();

public:

    ///从配置文件中得到相关的配置
    int get_config(const zerg_config* config);
    ///检查IP限制
    int check_iprestrict(const zce::skt::addr_in& remoteaddress);

public:

    ///单子实例函数
    static ip_restrict* instance();
    ///清理单子实例
    static void clear_inst();

protected:
    ///单子实例
    static ip_restrict* instance_;

protected:

    //允许进行连接的IP地址
    SET_OF_IPADDRESS           allow_ip_set_;
    //拒绝进行连接的IP地址
    SET_OF_IPADDRESS           reject_ip_set_;
};
}
