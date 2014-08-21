#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_os_adapt_predefine.h"
#include "zce_socket_addr_base.h"
#include "zce_socket_base.h"
#include "zce_socket_stream.h"
#include "zce_socket_acceptor.h"

/************************************************************************************************************
Class           : ZCE_Socket_Acceptor
************************************************************************************************************/

//构造函数
ZCE_Socket_Acceptor::ZCE_Socket_Acceptor():
    ZCE_Socket_Base()
{
}

ZCE_Socket_Acceptor::~ZCE_Socket_Acceptor()
{
    //为什么在这人不关闭socket_handle_,是考虑到万一要进行复制呢
    close();
}

//跟进地址参数等，打开一个Accepet的端口 (Bind,并且监听),
//打开一个监听地址，目前只支持AF_INET,和AFINET6
int ZCE_Socket_Acceptor::open(const ZCE_Sockaddr *local_addr,
                              bool reuse_addr,
                              int protocol_family ,
                              int backlog ,
                              int protocol)
{

    int ret = 0;

    //如果没有标注协议类型，用地址
    if (protocol_family == AF_UNSPEC)
    {
        protocol_family = local_addr->sockaddr_ptr_->sa_family;
    }

    //
    ret = ZCE_Socket_Base::open(SOCK_STREAM,
                                protocol_family,
                                protocol,
                                reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    ret = ZCE_Socket_Base::bind(local_addr);

    if (ret != 0)
    {
        ZCE_OS::closesocket(socket_handle_);
        return ret;
    }

    //建立监听
    ret = ZCE_OS::listen (socket_handle_,
                          backlog);

    if (ret != 0)
    {
        ZCE_OS::closesocket(socket_handle_);
        return ret;
    }

    return 0;
}

//非超时处理的accept,NONBLOCK模式下会迅速退出，阻塞模式下会一致等待
int ZCE_Socket_Acceptor::accept (ZCE_Socket_Stream &new_stream,
                                 ZCE_Sockaddr *remote_addr) const
{
    ZCE_SOCKET sock_handle = ZCE_OS::accept(socket_handle_,
                                            remote_addr->sockaddr_ptr_,
                                            &remote_addr->sockaddr_size_);

    if (sock_handle == ZCE_INVALID_SOCKET)
    {
        return -1;
    }

    new_stream.set_handle(sock_handle);
    return 0;
}

//
int ZCE_Socket_Acceptor::accept (ZCE_Socket_Stream &new_stream,
                                 ZCE_Time_Value &timeout,
                                 ZCE_Sockaddr *remote_addr) const
{
    int ret = 0;
    ret = ZCE_OS::handle_ready(socket_handle_,
                               &timeout,
                               ZCE_OS::HANDLE_READY_ACCEPT);

    const int HANDLE_READY_ONE = 1;

    if (ret != HANDLE_READY_ONE)
    {
        return -1;
    }

    //
    ZCE_SOCKET sock_handle = ZCE_OS::accept(socket_handle_,
                                            remote_addr->sockaddr_ptr_,
                                            &remote_addr->sockaddr_size_);

    if (sock_handle == ZCE_INVALID_SOCKET)
    {
        return -1;
    }

    new_stream.set_handle(sock_handle);
    return 0;
}

