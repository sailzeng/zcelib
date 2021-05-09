#pragma once

#include "zce/os_adapt/socket.h"

//在改写发生3个月左右，我都一致使用sockaddr sockaddr_in作为地址参数，为什么要变化呢，
//因为在有一天改写原来代码时，突然觉得为什么为什么彻底OO一点点？
//整体参考ACE_INET_Addr ACE INET Addr实现的，当然也有一些变化，ACE没有直接使用sockaddr，不知道为啥

namespace zce
{
//Socket地址的基类。
class Sockaddr_Base
{
public:

    //构造函数，
    Sockaddr_Base(sockaddr* sockaddr_ptr = NULL, int sa_size = -1);
    //析构函数,内部有virtual函数
    virtual ~Sockaddr_Base(void);

    //设置sockaddr地址信息,设置成纯虚函数的原因不想让你使用Sockaddr_Base
    virtual void set_sockaddr(sockaddr* sockaddr_ptr, socklen_t sockaddr_size) = 0;

    //Get/set the size of the address.
    inline socklen_t get_size(void) const;
    //
    inline void  set_size(int sa_size);

    //设置地址信息
    inline void set_addr(sockaddr* sockaddr_ptr);
    //取得地址信息
    inline sockaddr* get_addr(void) const;

    // 检查地址是否相等
    bool operator == (const Sockaddr_Base& others_sockaddr) const;
    // 检查地址是否不相等
    bool operator != (const Sockaddr_Base& others_sockaddr) const;

    //转换成字符串,同时输出字符串的长度
    inline const char* to_string(char* buffer,
                                 size_t buf_len,
                                 size_t& use_buf,
                                 bool out_port_info = true) const
    {
        return zce::socketaddr_ntop_ex(sockaddr_ptr_, buffer, buf_len, use_buf, out_port_info);
    }

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

//Get/set the size of the address.
inline socklen_t Sockaddr_Base::get_size(void) const
{
    return sockaddr_size_;
}
//
inline void Sockaddr_Base::set_size(int sa_size)
{
    sockaddr_size_ = sa_size;
}

//设置地址信息
inline void Sockaddr_Base::set_addr(sockaddr* sockaddr_ptr)
{
    sockaddr_ptr_ = sockaddr_ptr;
}
//取得地址信息
inline sockaddr* Sockaddr_Base::get_addr(void) const
{
    return sockaddr_ptr_;
}
}
