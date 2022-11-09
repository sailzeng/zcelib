#pragma once

#include "zce/time/time_value.h"
#include "zce/socket/addr_base.h"
#include "zce/socket/socket_base.h"

namespace zce::skt
{
class stream : public zce::skt::socket_base
{
public:

    //构造函数
    stream() = default;
    //注意：子类socket_base  析构会关闭handle，考虑右值引用使用
    ~stream() = default;
    explicit stream(const ZCE_SOCKET& socket_hanle);

    //为了安全，避免反复关闭，可以使用右值引用 && 构造和 && =
    explicit stream(stream&& others) noexcept;
    stream& operator=(stream&& others) noexcept;

public:

    //Open SOCK句柄，不BIND本地地址的方式
    int open(int family = AF_INET,
             int protocol = 0,
             bool reuse_addr = true);

    //Open SOCK句柄，BIND本地地址的方式,一般情况下不用这样使用，除非……
    //protocol_family 参数可以是AF_INET,或者AF_INET6等
    int open(const zce::skt::addr_base* local_addr,
             int protocol = 0,
             bool reuse_addr = false);

    //shutdown,特殊的关闭方式函数，how参数向LINUX下靠齐,SHUT_RD,SHUT_WR,SHUT_RDWR
    int shutdown(int how) const;

    //带超时处理的发送函数，接收N个字节后返回,内部用的是select记录时间，计时精确一点
    ssize_t recv_n(void* buf,
                   size_t len,
                   time_value* timeout_tv,
                   int flags = 0)  const;

    //带超时处理的发送函数，发送N个字节后返回,内部用的是select记录时间，计时精确一点
    ssize_t send_n(const void* buf,
                   size_t len,
                   time_value* timeout_tv,
                   int flags = 0)  const;

    //带超时处理的接受函数，发送N个字节后返回,内部用的是SO_RCVTIMEO，记时不是特别准确，
    ssize_t recvn_timeout(void* buf,
                          size_t len,
                          time_value& timeout_tv,
                          int flags = 0)  const;

    //带超时处理的发送函数，发送N个字节后返回,内部用的是SO_SNDTIMEO，记时不是特别准确，
    ssize_t sendn_timeout(void* buf,
                          size_t len,
                          time_value& timeout_tv,
                          int flags = 0);
};
}
