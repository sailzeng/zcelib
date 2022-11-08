#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/socket/addr_base.h"
#include "zce/socket/socket_base.h"
#include "zce/os_adapt/socket.h"
#include "zce/socket/datagram.h"

namespace zce::skt
{
datagram::datagram(const ZCE_SOCKET& socket_hanle) :
    zce::skt::socket_base(socket_hanle)
{
}

datagram::datagram(const datagram& others) :
    zce::skt::socket_base(others.socket_handle_)
{
}

datagram::datagram(datagram&& others) noexcept :
    zce::skt::socket_base(others.socket_handle_)
{
    others.socket_handle_ = ZCE_INVALID_SOCKET;
}

datagram& datagram::operator=(const datagram& others)
{
    if (this == &others)
    {
        return *this;
    }
    if (socket_handle_ != ZCE_INVALID_SOCKET)
    {
        zce::close_socket(socket_handle_);
    }
    socket_handle_ = others.socket_handle_;
    return *this;
}

datagram& datagram::operator=(datagram&& others) noexcept
{
    if (this == &others)
    {
        return *this;
    }
    if (socket_handle_ != ZCE_INVALID_SOCKET)
    {
        zce::close_socket(socket_handle_);
    }
    socket_handle_ = others.socket_handle_;
    others.socket_handle_ = ZCE_INVALID_SOCKET;
    return *this;
}

//Open SOCK句柄，不BIND本地地址的方式
int datagram::open(int family,
                   int protocol,
                   bool reuse_addr)
{
    int ret = 0;
    //打开
    ret = zce::skt::socket_base::open(SOCK_DGRAM,
                                      family,
                                      protocol,
                                      reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

///Open SOCK句柄，BIND本地地址的方式,一般情况下不用这样使用，除非……
//protocol_family 参数可以是AF_INET,或者AF_INET6等
int datagram::open(const zce::skt::addr_base* local_addr,
                   int protocol,
                   bool reuse_addr)
{
    int ret = 0;

    ret = zce::skt::socket_base::open(SOCK_DGRAM,
                                      local_addr,
                                      protocol,
                                      reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//收UDP的数据,也带有超时处理，但是是收到多少数据就是多少了，超时用SO_RCVTIMEO实现
ssize_t datagram::recvfrom_timeout(void* buf,
                                   size_t len,
                                   zce::skt::addr_base* from_addr,
                                   zce::time_value& timeout_tv,
                                   int flags)  const
{
    return zce::recvfrom_timeout(socket_handle_,
                                 buf,
                                 len,
                                 from_addr->sockaddr_ptr_,
                                 &from_addr->sockaddr_size_,
                                 timeout_tv,
                                 flags);
}

//UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
//发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理
ssize_t datagram::sendto_timeout(const void* buf,
                                 size_t len,
                                 const zce::skt::addr_base* to_addr,
                                 int flags)  const
{
    return zce::sendto_timeout(socket_handle_,
                               buf,
                               len,
                               to_addr->sockaddr_ptr_,
                               to_addr->sockaddr_size_,
                               flags);
}
}