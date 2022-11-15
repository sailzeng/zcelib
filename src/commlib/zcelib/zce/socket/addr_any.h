#pragma once

#include "zce/socket/addr_base.h"

namespace zce::skt
{
//Socket地址的通用类，用于有时候，你要用第一个地址类型同时适配in，in6的时候。
class addr_any : public zce::skt::addr_base
{
public:

    //构造函数，默认初始化为in的地址，使用是要记得改变
    addr_any();
    //构造函数，根据family确定初始化的类型
    explicit addr_any(int family);
    //析构函数,内部有virtual函数
    virtual ~addr_any(void);

    //设置sockaddr地址信息,设置成纯虚函数的原因不想让你使用addr_any
    virtual void set_sockaddr(sockaddr* sockaddr_ptr,
                              socklen_t sockaddr_size) override;

    //通过IP取得域名
    virtual int getnameinfo(char* host_name,
                            size_t name_len) const override;

    //通过域名得到IP地址
    virtual int getaddrinfo(const char* nodename,
                            uint16_t port_number = 0) override;

    //根据family决定地址类型，然后再通过域名获取IP
    int getaddrinfo(int family,
                    const char* node_name,
                    uint16_t port_number = 0);

    ///设置端口号，
    virtual void set_port(uint16_t) override;
    ///取得端口号
    virtual uint16_t get_port(void) const override;

    //! 设置地址的family
    void set_family(int family);

    //! 检查地址是否相等
    bool operator == (const addr_any& others) const;
    //! 检查地址是否不相等
    bool operator != (const addr_any& others) const;

public:

    union
    {
        //sockaddr_in 16个字节
        ::sockaddr_in  in4_addr_;
        //sockaddr_in6 28个字节
        ::sockaddr_in6 in6_addr_;
    };
};
}
