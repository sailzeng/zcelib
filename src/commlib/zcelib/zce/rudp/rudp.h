#pragma once

#include "zce/util/lord_rings.h"
#include "zce/util/buffer.h"
#include "zce/os_adapt/common.h"
#include "zce/os_adapt/socket.h"

namespace zce::rudp
{
enum class STATE
{
    CLOSE = 0,
    SYN_SND = 1,
    SYN_RCV = 2,
    ESTABLISHED = 3,
};

class HANDLE
{
    typedef zce::lordrings<zce::buffer_queue>  buffer_list;

public:
    //
    ZCE_SOCKET udp_socket_;
    //
    sockaddr* sockaddr_local_ = nullptr;
    //
    sockaddr* sockaddr_peer_ = nullptr;

protected:
    //
    buffer_list send_list_;

    buffer_list recv_list_;

    STATE state_;

    time_t rto_;
};

#pragma pack (1)
class RUDP_HEAD
{
    uint16_t len_;
    uint8_t flag_;
    uint8_t windows_num_;

    uint32_t serial_number;

    uint32_t ack_;
};
#pragma pack ()

int open(zce::rudp::HANDLE *handle,
         int family);

int open(zce::rudp::HANDLE *handle,
         const sockaddr* local_addr,
         socklen_t addrlen);
}