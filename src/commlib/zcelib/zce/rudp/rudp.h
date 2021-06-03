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

//
#pragma pack (1)
class RUDP_HEAD
{
    uint16_t len_;
    uint8_t flag_;
    uint8_t windows_num_;
    uint32_t serial_number_;
    uint32_t ack_;
    uint32_t uno1_;
    uint32_t uno2_;
    char data_[1];
};
#pragma pack ()

//
static constexpr size_t MTU_TCP_DEFAULT = 576;
//
static constexpr size_t MTU_ETHERNET_PPOE = 1480;

//
static constexpr size_t MAX_NUM_SEND_LIST = 512;
//
static constexpr size_t MAX_NUM_RECV_LIST = 512;

struct SEND_BUFFER
{
    zce::buffer_queue *buf_queue_;
    uint64_t send_clock_;
    uint64_t ack_clock_;
    size_t fail_num_;
};

class HANDLE
{
protected:

    typedef zce::lordrings<zce::buffer_queue*>  RECV_BUFFER_LIST;

public:
    //
    ZCE_SOCKET udp_socket_;
    //
    sockaddr* sockaddr_local_ = nullptr;
    //
    sockaddr* sockaddr_peer_ = nullptr;

protected:
    //
    RECV_BUFFER_LIST send_list_;
    //
    RECV_BUFFER_LIST recv_list_;
    //
    STATE state_;
    //
    size_t mtu_ = MTU_ETHERNET_PPOE;
    //
    time_t rto_;
};

struct RUDP_CORE
{
};

int init();

int open(zce::rudp::HANDLE *handle,
         int family);

int open(zce::rudp::HANDLE *handle,
         const sockaddr* local_addr,
         socklen_t addrlen);
}