#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/os_adapt/common.h"
#include "zce/socket/addr_base.h"
#include "zce/socket/socket_base.h"
#include "zce/socket/stream.h"
#include "zce/socket/acceptor.h"

/************************************************************************************************************
Class           : acceptor
************************************************************************************************************/
namespace zce::skt
{
//构造函数
acceptor::acceptor() :
    zce::skt::socket_base()
{
}

acceptor::~acceptor()
{
    //为什么在这人不关闭socket_handle_,是考虑到万一要进行复制呢
    close();
}

//跟进地址参数等，打开一个Accepet的端口 (Bind,并且监听),
//打开一个监听地址，目前只支持AF_INET,和AFINET6
int acceptor::open(const zce::skt::addr_base* local_addr,
                   bool reuse_addr,
                   int protocol_family,
                   int backlog,
                   int protocol)
{
    int ret = 0;

    //如果没有标注协议类型，用地址
    if (protocol_family == AF_UNSPEC)
    {
        protocol_family = local_addr->sockaddr_ptr_->sa_family;
    }

    //
    ret = zce::skt::socket_base::open(SOCK_STREAM,
                                      protocol_family,
                                      protocol,
                                      reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    ret = zce::skt::socket_base::bind(local_addr);

    if (ret != 0)
    {
        zce::close_socket(socket_handle_);
        return ret;
    }

    //建立监听
    ret = zce::listen(socket_handle_,
                      backlog);

    if (ret != 0)
    {
        zce::close_socket(socket_handle_);
        return ret;
    }

    return 0;
}

//非超时处理的accept,NONBLOCK模式下会迅速退出，阻塞模式下会一致等待
int acceptor::accept(zce::skt::stream& new_stream,
                     zce::skt::addr_base* remote_addr) const
{
    ZCE_SOCKET sock_handle = zce::accept(socket_handle_,
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
int acceptor::accept(zce::skt::stream& new_stream,
                     time_value& timeout,
                     zce::skt::addr_base* remote_addr) const
{
    ZCE_SOCKET  sock_handle = ZCE_INVALID_SOCKET;
    int ret = zce::accept_timeout(socket_handle_,
                                  &sock_handle,
                                  remote_addr->sockaddr_ptr_,
                                  &remote_addr->sockaddr_size_,
                                  timeout);
    if (ret != 0)
    {
        return -1;
    }
    new_stream.set_handle(sock_handle);
    return 0;
}
}