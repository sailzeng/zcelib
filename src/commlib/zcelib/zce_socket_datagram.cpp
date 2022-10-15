#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_socket_addr_base.h"
#include "zce_socket_base.h"
#include "zce_os_adapt_socket.h"
#include "zce_socket_datagram.h"

ZCE_Socket_DataGram::ZCE_Socket_DataGram():
    ZCE_Socket_Base()
{
}

ZCE_Socket_DataGram::ZCE_Socket_DataGram(const ZCE_SOCKET &socket_hanle):
    ZCE_Socket_Base(socket_hanle)
{

}

ZCE_Socket_DataGram::~ZCE_Socket_DataGram()
{
}

//Open SOCK句柄，不BIND本地地址的方式
int ZCE_Socket_DataGram::open(int protocol_family,
                              int protocol,
                              bool reuse_addr)
{
    int ret = 0;
    //打开
    ret = ZCE_Socket_Base::open(SOCK_DGRAM,
                                protocol_family,
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
int ZCE_Socket_DataGram::open(const ZCE_Sockaddr *local_addr,
                              int protocol_family,
                              int protocol,
                              bool reuse_addr)
{
    int ret = 0;

    ret = ZCE_Socket_Base::open(SOCK_DGRAM,
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

//收UDP的数据,也带有超时处理，但是是收到多少数据就是多少了，超时用SO_RCVTIMEO实现
ssize_t ZCE_Socket_DataGram::recvfrom_timeout (void *buf,
                                               size_t len,
                                               ZCE_Sockaddr *from_addr,
                                               ZCE_Time_Value &timeout_tv,
                                               int flags)  const
{
    return zce::recvfrom_timeout (socket_handle_,
                                  buf,
                                  len,
                                  from_addr->sockaddr_ptr_,
                                  &from_addr->sockaddr_size_,
                                  timeout_tv,
                                  flags);
}

//UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
//发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理
ssize_t ZCE_Socket_DataGram::sendto_timeout (const void *buf,
                                             size_t len,
                                             const ZCE_Sockaddr *to_addr,
                                             ZCE_Time_Value &timeout_tv,
                                             int flags)  const
{
    return zce::sendto_timeout (socket_handle_,
                                buf,
                                len,
                                to_addr->sockaddr_ptr_,
                                to_addr->sockaddr_size_,
                                timeout_tv,
                                flags);
}

