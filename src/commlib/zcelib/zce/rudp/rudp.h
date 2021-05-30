#pragma once

#include "zce/util/lord_rings.h"
#include "zce/util/buffer.h"
#include "zce/os_adapt/common.h"
#include "zce/os_adapt/socket.h"

namespace zce::rudp
{
class HANDLE
{
    typedef zce::lordrings<zce::buffer_queue>  buffer_list;

public:
    //
    ZCE_SOCKET udp_socket_;

    sockaddr* sockaddr_local_ = nullptr;
    //
    sockaddr* sockaddr_peer_ = nullptr;

protected:
    //
    buffer_list send_list_;

    buffer_list recv_list_;

    time_t rto_;
};

#pragma pack (1)
class RUDP_HEAD
{
};
#pragma pack ()

int open(zce::rudp::HANDLE *handle,
         int family);

int open(zce::rudp::HANDLE *handle,
         const sockaddr* local_addr,
         socklen_t addrlen);
}