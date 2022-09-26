#pragma  once

#include "zce/socket/addr_base.h"

namespace zce::skt
{
//IPV6是一个

//IPv6的物理结构
class addr_in6 : public zce::skt::addr_base
{
public:

    //默认构造函数
    addr_in6(void);

    //根据sockaddr_in构造，
    addr_in6(const ::sockaddr_in6* addr);

    //根据端口号，和IP地址字符串构造,ipv6_addr_str应该有<40个字节的长度
    addr_in6(const char* ipv6_addr_str,
             uint16_t port_number);

    //根据端口号，和IP地址信息构造
    addr_in6(uint16_t port_number,
             const char ipv6_addr_val[16]);

    //拷贝构造，一定要写，这个类的基类指针是指向自己的一个地址的，
    addr_in6(const addr_in6& others);

    virtual ~addr_in6();

public:
    //设置地址信息
    virtual void set_sockaddr(sockaddr* addr, socklen_t len) override;

    //取得域名相关的IP地址信息，调用的是getaddrinfo，notename可以是数值地址，或者域名
    virtual int getaddrinfo(const char* notename,
                            uint16_t port_number = 0)override;

    //DNS相关函数，
    //取得IP地址相关的域名信息,调用的是getnameinfo
    virtual int getnameinfo(char* host_name, size_t name_len) const override;

    ///设置端口好，
    virtual inline void set_port(uint16_t)override;
    ///取得端口号
    virtual inline uint16_t get_port(void) const override;

    /*!
    * @brief      根据字符串取得IP地址信息，以及端口号信息,
    * @return     int == 0表示设置成功
    * @param      ip_addr_str
    * @note       如果字符串里面有#,会被认为有端口号，如果没有，端口号为0
    */
    int set(const char* ip_addr_str);

    //根据地址名字，端口号设置
    int set(const char ip_addr_str[],
            uint16_t port_number);

    //根据地址IP，端口号设置
    int set(uint16_t port_number,
            const char ipv6_addr_val[16]);

    ///检查端口号是否是一个安全端口
    bool check_safeport();

    //不可冲入的非安全函数
    inline const char* get_host_name(void) const;

    //端口的打印输出使用to_string函数。取得以:冒号的IP地址信息STRING

    //取得IP地址,你要保证ipv6_addr_val有16个字节
    const char* get_ip_address(char* ipv6_addr_val) const;

    //比较两个地址是否相等
    bool operator == (const addr_in6& others) const;
    //比较两个地址是否不想等
    bool operator != (const addr_in6& others) const;

    //检查IP地址是否相等,忽视端口
    bool is_ip_equal(const addr_in6& others) const;

    //这个IPV6的地址是否是IPV4的地址映射的
    bool is_v4mapped() const;
    //从一个IPV4的地址得到对应映射的IPV6的地址，
    int map_from_inaddr(const zce::skt::addr_in& from);
    //如果这个IPV6的地址是IPV4映射过来的，将其还原为IPV4的地址
    int mapped_to_inaddr(zce::skt::addr_in& to) const;

    //各种操作符号转换函数，方便各种使用，让addr_in6的行为和addr_in6基本一致
    //返回addr_in6
    operator sockaddr_in6 () const;
    //返回内部const addr_in6的指针，（不可以被修改）
    operator const sockaddr_in6* () const;
    //返回内部 addr_in6的指针，（可以被修改）
    operator sockaddr_in6* ();

protected:

    //IPV6的地址
    ::sockaddr_in6         in6_addr_;
};

//取得IP地址，你要保证ipv6_addr_val的长度有16个字节
inline const char* addr_in6::get_ip_address(char* ipv6_addr_val) const
{
    memcpy(ipv6_addr_val, &(in6_addr_.sin6_addr), sizeof(in6_addr));
    return ipv6_addr_val;
}

//设置端口好，
inline void addr_in6::set_port(uint16_t port_number)
{
    in6_addr_.sin6_port = ntohs(port_number);
}
//取得端口号
inline uint16_t addr_in6::get_port(void) const
{
    return ntohs(in6_addr_.sin6_port);
}
}
