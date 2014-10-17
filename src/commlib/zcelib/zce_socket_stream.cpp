#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_os_adapt_socket.h"
#include "zce_socket_base.h"
#include "zce_socket_stream.h"

//构造函数
ZCE_Socket_Stream::ZCE_Socket_Stream():
    ZCE_Socket_Base()
{
}

ZCE_Socket_Stream::ZCE_Socket_Stream(const ZCE_SOCKET &socket_handle):
    ZCE_Socket_Base(socket_handle)
{
}

ZCE_Socket_Stream::~ZCE_Socket_Stream()
{
}

//Open SOCK句柄，不BIND本地地址的方式
int ZCE_Socket_Stream::open(int protocol_family,
                            int protocol,
                            bool reuse_addr)
{
    int ret = 0;
    //打开
    ret = ZCE_Socket_Base::open(SOCK_STREAM, protocol_family, protocol, reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

///Open SOCK句柄，BIND本地地址的方式,一般情况下不用这样使用，除非……
//protocol_family 参数可以是AF_INET,或者AF_INET6等
int ZCE_Socket_Stream::open(const ZCE_Sockaddr *local_addr,
                            int protocol_family,
                            int protocol,
                            bool reuse_addr)
{
    int ret = 0;

    ret = ZCE_Socket_Base::open(SOCK_STREAM,
                                local_addr,
                                protocol_family,
                                protocol,
                                reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//shutdown,特殊的关闭方式函数，how参数向LINUX下靠齐,SHUT_RD,SHUT_WR,SHUT_RDWR
int ZCE_Socket_Stream::shutdown (int how) const
{
    return ZCE_LIB::shutdown(socket_handle_, how);
}

//带超时处理的发送函数，接收N个字节后返回,内部用的是select记录时间，记时不是特别准确，
ssize_t ZCE_Socket_Stream::recv_n (void *buf,
                                   size_t len,
                                   ZCE_Time_Value  *timeout_tv,
                                   int flags)  const
{
    return ZCE_LIB::recv_n(socket_handle_,
                           buf,
                           len,
                           timeout_tv,
                           flags);
}

//带超时处理的发送函数，发送N个字节后返回,内部用的是select记录时间，记时不是特别准确，
ssize_t ZCE_Socket_Stream::send_n (const void *buf,
                                   size_t len,
                                   ZCE_Time_Value  *timeout_tv,
                                   int flags)  const
{
    return ZCE_LIB::send_n(socket_handle_,
                           buf,
                           len,
                           timeout_tv,
                           flags);
}

//带超时处理的接受函数，发送N个字节后返回,内部用的是SO_RCVTIMEO，记时不是特别准确，
ssize_t ZCE_Socket_Stream::recvn_timeout (void *buf,
                                          size_t len,
                                          ZCE_Time_Value  &timeout_tv,
                                          int flags)  const
{
    return ZCE_LIB::recvn_timeout(socket_handle_,
                                  buf,
                                  len,
                                  timeout_tv,
                                  flags);
}

//带超时处理的发送函数，发送N个字节后返回,内部用的是SO_SNDTIMEO，记时不是特别准确，
ssize_t ZCE_Socket_Stream::sendn_timeout (void *buf,
                                          size_t len,
                                          ZCE_Time_Value  &timeout_tv,
                                          int flags)
{
    return ZCE_LIB::sendn_timeout(socket_handle_,
                                  buf,
                                  len,
                                  timeout_tv,
                                  flags);
}

