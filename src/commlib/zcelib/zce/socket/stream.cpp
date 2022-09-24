#include "zce/predefine.h"
#include "zce/os_adapt/socket.h"
#include "zce/socket/stream.h"

namespace zce
{
//构造函数
Socket_Stream::Socket_Stream() :
    zce::Socket_Base()
{
}

Socket_Stream::Socket_Stream(const ZCE_SOCKET& socket_handle) :
    zce::Socket_Base(socket_handle)
{
}

Socket_Stream::~Socket_Stream()
{
}

//Open SOCK句柄，不BIND本地地址的方式
int Socket_Stream::open(int family,
                        int protocol,
                        bool reuse_addr)
{
    int ret = 0;
    //打开
    ret = zce::Socket_Base::open(SOCK_STREAM, family, protocol, reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

///Open SOCK句柄，BIND本地地址的方式,一般情况下不用这样使用，除非……
//protocol_family 参数可以是AF_INET,或者AF_INET6等
int Socket_Stream::open(const Sockaddr_Base* local_addr,
                        int protocol,
                        bool reuse_addr)
{
    int ret = 0;

    ret = zce::Socket_Base::open(SOCK_STREAM,
                                 local_addr,
                                 protocol,
                                 reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//shutdown,特殊的关闭方式函数，how参数向LINUX下靠齐,SHUT_RD,SHUT_WR,SHUT_RDWR
int Socket_Stream::shutdown(int how) const
{
    return zce::shutdown(socket_handle_, how);
}

//带超时处理的发送函数，接收N个字节后返回,内部用的是select记录时间，记时不是特别准确，
ssize_t Socket_Stream::recv_n(void* buf,
                              size_t len,
                              zce::time_value* timeout_tv,
                              int flags)  const
{
    return zce::recv_n(socket_handle_,
                       buf,
                       len,
                       timeout_tv,
                       flags);
}

//带超时处理的发送函数，发送N个字节后返回,内部用的是select记录时间，记时不是特别准确，
ssize_t Socket_Stream::send_n(const void* buf,
                              size_t len,
                              zce::time_value* timeout_tv,
                              int flags)  const
{
    return zce::send_n(socket_handle_,
                       buf,
                       len,
                       timeout_tv,
                       flags);
}

//带超时处理的接受函数，发送N个字节后返回,内部用的是SO_RCVTIMEO，记时不是特别准确，
ssize_t Socket_Stream::recvn_timeout(void* buf,
                                     size_t len,
                                     zce::time_value& timeout_tv,
                                     int flags)  const
{
    return zce::recvn_timeout(socket_handle_,
                              buf,
                              len,
                              timeout_tv,
                              flags);
}

//带超时处理的发送函数，发送N个字节后返回,内部用的是SO_SNDTIMEO，记时不是特别准确，
ssize_t Socket_Stream::sendn_timeout(void* buf,
                                     size_t len,
                                     zce::time_value& timeout_tv,
                                     int flags)
{
    return zce::sendn_timeout(socket_handle_,
                              buf,
                              len,
                              timeout_tv,
                              flags);
}
}