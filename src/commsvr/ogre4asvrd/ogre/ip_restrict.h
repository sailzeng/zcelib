#pragma once

namespace ogre
{
class ip_restrict
{
    typedef std::unordered_set<unsigned int> SET_OF_IPADDRESS;

protected:

    ip_restrict();
    ~ip_restrict();

public:

    ///从配置文件中得到相关的配置
    int get_config(const configure* config);
    //检查IP限制
    int check_ip_restrict(const zce::skt::addr_in& remoteaddress);

public:

    //单子实例函数
    static ip_restrict* instance();
    //清理单子实例
    static void clear_inst();

protected:

    //允许进行连接的IP地址
    SET_OF_IPADDRESS                  allow_ip_set_;
    //拒绝进行连接的IP地址
    SET_OF_IPADDRESS                  reject_ip_set_;

protected:
    //单子实例
    static ip_restrict* instance_;
};
}
