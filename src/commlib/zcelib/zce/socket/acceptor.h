#pragma once

#include "zce/util/non_copyable.h"
#include "zce/os_adapt/socket.h"
#include "zce/os_adapt/time.h"
#include "zce/socket/stream.h"

namespace zce::skt
{
/// Acceptor的工厂，用于产生Socket_Stream
class acceptor : public zce::skt::socket_base
{
public:
    acceptor();
    ~acceptor();

public:

    //打开一个监听地址，目前只支持AF_INET,和AFINET6
    int open(const zce::skt::addr_base* local_addr,
             bool reuse_addr = true,
             int protocol_family = AF_UNSPEC,
             int backlog = ZCE_DEFAULT_BACKLOG,
             int protocol = 0);

    //接受一个SOCKET
    int accept(zce::skt::stream& new_stream,
               zce::skt::addr_base* remote_addr) const;

    //带有超时的处理的accept
    int accept(zce::skt::stream& new_stream,
               time_value& timeout,
               zce::skt::addr_base* remote_addr) const;
};
}
