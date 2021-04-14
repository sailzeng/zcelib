#ifndef ZCE_LIB_SOCKET_STREAM_H_
#define ZCE_LIB_SOCKET_STREAM_H_

class ZCE_Time_Value;
class ZCE_Sockaddr;
class ZCE_Socket_Base;

class ZCE_Socket_Stream : public ZCE_Socket_Base
{

public:

    //构造函数
    ZCE_Socket_Stream();
    explicit ZCE_Socket_Stream(const ZCE_SOCKET &socket_hanle);
    ~ZCE_Socket_Stream();

public:

    //Open SOCK句柄，不BIND本地地址的方式
    int open(int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = true);

    //Open SOCK句柄，BIND本地地址的方式,一般情况下不用这样使用，除非……
    //protocol_family 参数可以是AF_INET,或者AF_INET6等
    int open(const ZCE_Sockaddr *local_addr,
             int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = false);

    //shutdown,特殊的关闭方式函数，how参数向LINUX下靠齐,SHUT_RD,SHUT_WR,SHUT_RDWR
    int shutdown (int how) const;

    //带超时处理的发送函数，接收N个字节后返回,内部用的是select记录时间，计时精确一点
    ssize_t recv_n (void *buf,
                    size_t len,
                    ZCE_Time_Value  *timeout_tv,
                    int flags = 0)  const;

    //带超时处理的发送函数，发送N个字节后返回,内部用的是select记录时间，计时精确一点
    ssize_t send_n (const void *buf,
                    size_t len,
                    ZCE_Time_Value  *timeout_tv,
                    int flags = 0)  const;

    //带超时处理的接受函数，发送N个字节后返回,内部用的是SO_RCVTIMEO，记时不是特别准确，
    ssize_t recvn_timeout (void *buf,
                           size_t len,
                           ZCE_Time_Value &timeout_tv,
                           int flags = 0)  const;

    //带超时处理的发送函数，发送N个字节后返回,内部用的是SO_SNDTIMEO，记时不是特别准确，
    ssize_t sendn_timeout (void *buf,
                           size_t len,
                           ZCE_Time_Value &timeout_tv,
                           int flags = 0);

};

#endif //#ifndef ZCE_LIB_SOCKET_STREAM_H_

