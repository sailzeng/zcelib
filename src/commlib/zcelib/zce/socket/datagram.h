#pragma once

#include "zce/time/time_value.h"
#include "zce/socket/socket_base.h"
#include "zce/socket/addr_base.h"

namespace zce::skt
{
//UDP socket
class datagram : public zce::skt::socket_base
{
public:

    //构造函数和析构函数等
    datagram() = default;
    //注意：子类socket_base  析构会关闭handle，考虑右值引用使用
    ~datagram() = default;
    explicit datagram(const ZCE_SOCKET& socket_hanle);

    //为了安全，避免反复关闭，可以使用右值引用构造 && 和右值赋值 && =
    explicit datagram(datagram&& others) noexcept;
    datagram& operator=(datagram&& others) noexcept;

    //Open SOCK句柄，不BIND本地地址的方式
    ///family 参数可以是AF_INET,或者AF_INET6等
    int open(int family = AF_INET,
             int protocol = 0,
             bool reuse_addr = true);

    ///Open SOCK句柄，BIND本地地址的方式,一般情况下不用这样使用，除非……
    int open(const zce::skt::addr_base* local_addr,
             int protocol = 0,
             bool reuse_addr = false);

    //UDP接收
    inline ssize_t recvfrom(void* buf,
                            size_t len,
                            int flags,
                            zce::skt::addr_base* addr) const;

    //UDP发送
    inline ssize_t sendto(const void* buf,
                          size_t len,
                          int flags,
                          const zce::skt::addr_base* to_addr) const;

    //收UDP的数据,也带有超时处理，但是是收到多少数据就是多少了，超时用select实现
    inline ssize_t recvfrom(void* buf,
                            size_t len,
                            zce::skt::addr_base* addr,
                            zce::time_value* timeout_tv,
                            int flags = 0) const;

    //UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
    //发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理，
    inline ssize_t sendto(const void* buf,
                          size_t len,
                          const zce::skt::addr_base* addr,
                          int flags = 0) const;

    //收UDP的数据,也带有超时处理，但是是收到多少数据就是多少了，超时用SO_RCVTIMEO实现
    ssize_t recvfrom_timeout(void* buf,
                             size_t len,
                             zce::skt::addr_base* addr,
                             zce::time_value& timeout_tv,
                             int flags = 0) const;

    //UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
    //发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理
    ssize_t sendto_timeout(const void* buf,
                           size_t len,
                           const zce::skt::addr_base* addr,
                           int flags = 0) const;
};

//UDP接收
inline ssize_t datagram::recvfrom(void* buf,
                                  size_t len,
                                  int flags,
                                  zce::skt::addr_base* addr)  const
{
    return zce::recvfrom(socket_handle_,
                         buf,
                         len,
                         flags,
                         addr->sockaddr_ptr_,
                         &addr->sockaddr_size_
    );
}

//UDP发送
inline ssize_t datagram::sendto(const void* buf,
                                size_t len,
                                int flags,
                                const zce::skt::addr_base* to_addr)  const
{
    return zce::sendto(socket_handle_,
                       buf,
                       len,
                       flags,
                       to_addr->sockaddr_ptr_,
                       to_addr->sockaddr_size_);
}

//收UDP的数据,也带有超时处理，但是是收到多少数据就是多少了，超时用select实现
inline ssize_t datagram::recvfrom(void* buf,
                                  size_t len,
                                  zce::skt::addr_base* from_addr,
                                  zce::time_value* timeout_tv,
                                  int flags)  const
{
    return zce::recvfrom(socket_handle_,
                         buf,
                         len,
                         from_addr->sockaddr_ptr_,
                         &from_addr->sockaddr_size_,
                         timeout_tv,
                         flags);
}

//UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
//发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理，
inline ssize_t datagram::sendto(const void* buf,
                                size_t len,
                                const zce::skt::addr_base* to_addr,
                                int flags)  const
{
    return zce::sendto(socket_handle_,
                       buf,
                       len,
                       to_addr->sockaddr_ptr_,
                       to_addr->sockaddr_size_,
                       flags);
}
}
