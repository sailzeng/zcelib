#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/socket/addr_base.h"
#include "zce/socket/socket_base.h"
#include "zce/os_adapt/socket.h"
#include "zce/socket/datagram.h"

namespace zce
{
Socket_DataGram::Socket_DataGram() :
    zce::Socket_Base()
{
}

Socket_DataGram::Socket_DataGram(const ZCE_SOCKET& socket_hanle) :
    zce::Socket_Base(socket_hanle)
{
}

Socket_DataGram::~Socket_DataGram()
{
}

//Open SOCK句柄，不BIND本地地址的方式
int Socket_DataGram::open(int family,
                          int protocol,
                          bool reuse_addr)
{
    int ret = 0;
    //打开
    ret = zce::Socket_Base::open(SOCK_DGRAM,
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
int Socket_DataGram::open(const Sockaddr_Base* local_addr,
                          int protocol,
                          bool reuse_addr)
{
    int ret = 0;

    ret = zce::Socket_Base::open(SOCK_DGRAM,
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
ssize_t Socket_DataGram::recvfrom_timeout(void* buf,
                                          size_t len,
                                          Sockaddr_Base* from_addr,
                                          zce::Time_Value& timeout_tv,
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
ssize_t Socket_DataGram::sendto_timeout(const void* buf,
                                        size_t len,
                                        const Sockaddr_Base* to_addr,
                                        zce::Time_Value& timeout_tv,
                                        int flags)  const
{
    return zce::sendto_timeout(socket_handle_,
                               buf,
                               len,
                               to_addr->sockaddr_ptr_,
                               to_addr->sockaddr_size_,
                               timeout_tv,
                               flags);
}
}