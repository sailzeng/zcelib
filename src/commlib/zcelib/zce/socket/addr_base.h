#pragma once

#include "zce/os_adapt/socket.h"

//在改写发生3个月左右，我都一致使用sockaddr sockaddr_in作为地址参数，为什么要变化呢，
//因为在有一天改写原来代码时，突然觉得为什么为什么彻底OO一点点？
//整体参考ACE_INET_Addr ACE INET Addr实现的，当然也有非常大的变化，ACE没有直接使用sockaddr，不知道为啥

namespace zce::skt
{
//Socket地址的基类。
class addr_base
{
protected:

    addr_base() = delete;
    addr_base(const addr_base&) = delete;
    //构造函数，
    addr_base(sockaddr* sockaddr_ptr = nullptr, int sa_size = -1);
    //析构函数,内部有virtual函数
    virtual ~addr_base(void);

public:
    //设置sockaddr地址信息,设置成纯虚函数的原因不想让你使用addr_base
    virtual void set_sockaddr(sockaddr* sockaddr_ptr, socklen_t sockaddr_size) = 0;

    //通过域名得到IP地址
    virtual int getaddrinfo(const char* notename,
                            uint16_t port_number = 0) = 0;

    //通过IP取得域名
    virtual int getnameinfo(char* host_name, size_t name_len) const = 0;

    ///设置端口号，
    virtual void set_port(uint16_t) = 0;
    ///取得端口号
    virtual uint16_t get_port(void) const = 0;

    //!取得地址的长度
    inline socklen_t get_size(void) const;

    //!取得地址信息
    inline sockaddr* get_addr(void) const;

    //!取得地址的family
    inline int get_family(void) const;

    // 检查地址是否相等
    bool operator == (const addr_base& others) const;
    // 检查地址是否不相等
    bool operator != (const addr_base& others) const;

    //转换成字符串,同时输出字符串的长度
    inline const char* to_str(char* buffer,
                              size_t buf_len,
                              size_t& use_buf,
                              bool out_port_info = true) const;

    ///检查地址是否是一个内网地址
    bool is_internal();

    ///检查地址是否是一个外网地址，其实我就简单认为不是外网地址就是内网地址
    bool is_internet();

public:

    // 地址类型的指针,
    sockaddr* sockaddr_ptr_;

    // 地址结构的长度 Number of bytes in the address.
    socklen_t sockaddr_size_;
};

//!取得地址的长度
inline socklen_t addr_base::get_size(void) const
{
    return sockaddr_size_;
}

//取得地址信息
inline sockaddr* addr_base::get_addr(void) const
{
    return sockaddr_ptr_;
}

//!取得地址的family
inline int addr_base::get_family(void) const
{
    return sockaddr_ptr_->sa_family;
}

inline const char* addr_base::to_str(char* buffer,
                                     size_t buf_len,
                                     size_t& use_buf,
                                     bool out_port_info) const
{
    return zce::sockaddr_ntop_ex(sockaddr_ptr_,
                                 buffer,
                                 buf_len,
                                 use_buf,
                                 out_port_info);
}
}
