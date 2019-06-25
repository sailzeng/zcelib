#ifndef ZCE_LIB_SOCKET_DATAGRAM_H_
#define ZCE_LIB_SOCKET_DATAGRAM_H_

class ZCE_Time_Value;
class ZCE_Sockaddr;
class ZCE_Socket_Base;

//UDP socket
class ZCE_Socket_DataGram : public ZCE_Socket_Base
{

public:

    //构造函数和析构函数等
    ZCE_Socket_DataGram();
    explicit ZCE_Socket_DataGram(const ZCE_SOCKET &socket_hanle);
    explicit ZCE_Socket_DataGram(const ZCE_Sockaddr &sock_addr);
    ~ZCE_Socket_DataGram();

    //Open SOCK句柄，不BIND本地地址的方式
    int open(int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = true);

    ///Open SOCK句柄，BIND本地地址的方式,一般情况下不用这样使用，除非……
    ///protocol_family 参数可以是AF_INET,或者AF_INET6等
    int open(const ZCE_Sockaddr *local_addr,
             int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = false);

    //UDP接收
    inline ssize_t recvfrom (void *buf,
                             size_t len,
                             int flags,
                             ZCE_Sockaddr *addr) const;

    //UDP发送
    inline ssize_t sendto (const void *buf,
                           size_t len,
                           int flags,
                           const ZCE_Sockaddr *to_addr) const;

    //收UDP的数据,也带有超时处理，但是是收到多少数据就是多少了，超时用select实现
    inline ssize_t recvfrom (void *buf,
                             size_t len,
                             ZCE_Sockaddr *addr,
                             ZCE_Time_Value  *timeout_tv,
                             int flags = 0) const;

    //UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
    //发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理，
    inline ssize_t sendto (const void *buf,
                           size_t len,
                           const ZCE_Sockaddr *addr,
                           ZCE_Time_Value *   /*timeout_tv*/,
                           int flags = 0) const;

    //收UDP的数据,也带有超时处理，但是是收到多少数据就是多少了，超时用SO_RCVTIMEO实现
    ssize_t recvfrom_timeout (void *buf,
                              size_t len,
                              ZCE_Sockaddr *addr,
                              ZCE_Time_Value  &timeout_tv,
                              int flags = 0) const;

    //UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
    //发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理
    ssize_t sendto_timeout (const void *buf,
                            size_t len,
                            const ZCE_Sockaddr *addr,
                            ZCE_Time_Value &   /*timeout_tv*/,
                            int flags = 0) const;
};

//UDP接收
inline ssize_t ZCE_Socket_DataGram::recvfrom (void *buf,
                                              size_t len,
                                              int flags,
                                              ZCE_Sockaddr *addr)  const
{
    return zce::recvfrom (socket_handle_,
                          buf,
                          len,
                          flags,
                          addr->sockaddr_ptr_,
                          &addr->sockaddr_size_
                         );
}

//UDP发送
inline ssize_t ZCE_Socket_DataGram::sendto (const void *buf,
                                            size_t len,
                                            int flags,
                                            const ZCE_Sockaddr *to_addr)  const
{
    return zce::sendto (socket_handle_,
                        buf,
                        len,
                        flags,
                        to_addr->sockaddr_ptr_,
                        to_addr->sockaddr_size_);
}

//收UDP的数据,也带有超时处理，但是是收到多少数据就是多少了，超时用select实现
inline ssize_t ZCE_Socket_DataGram::recvfrom (void *buf,
                                              size_t len,
                                              ZCE_Sockaddr *from_addr,
                                              ZCE_Time_Value *timeout_tv,
                                              int flags)  const
{
    return zce::recvfrom (socket_handle_,
                          buf,
                          len,
                          from_addr->sockaddr_ptr_,
                          &from_addr->sockaddr_size_,
                          timeout_tv,
                          flags);
}

//UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
//发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理，
inline ssize_t ZCE_Socket_DataGram::sendto (const void *buf,
                                            size_t len,
                                            const ZCE_Sockaddr *to_addr,
                                            ZCE_Time_Value *timeout_tv,
                                            int flags)  const
{
    return zce::sendto (socket_handle_,
                        buf,
                        len,
                        to_addr->sockaddr_ptr_,
                        to_addr->sockaddr_size_,
                        timeout_tv,
                        flags);
}

#endif //#ifndef ZCE_LIB_SOCKET_DATAGRAM_H_

