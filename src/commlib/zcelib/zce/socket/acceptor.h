#pragma once

#include "zce/util/non_copyable.h"
#include "zce/os_adapt/socket.h"
#include "zce/os_adapt/time.h"
#include "zce/socket/stream.h"

namespace zce
{
/// Acceptor的工厂，用于产生Socket_Stream
class Socket_Acceptor : public zce::Socket_Base
{
public:
    Socket_Acceptor();
    ~Socket_Acceptor();

public:

    //打开一个监听地址，目前只支持AF_INET,和AFINET6
    int open(const Sockaddr_Base* local_addr,
             bool reuse_addr = true,
             int protocol_family = AF_UNSPEC,
             int backlog = ZCE_DEFAULT_BACKLOG,
             int protocol = 0);

    //接受一个SOCKET
    int accept(Socket_Stream& new_stream,
               Sockaddr_Base* remote_addr) const;

    //带有超时的处理的accept
    int accept(Socket_Stream& new_stream,
               time_value& timeout,
               Sockaddr_Base* remote_addr) const;
};
}
