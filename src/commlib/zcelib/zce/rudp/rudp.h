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
static constexpr size_t MSS_ETHERNET_PPOE = 1480 - 20 - 8;

//
static constexpr size_t MAX_NUM_SEND_LIST = 512;
//
static constexpr size_t MAX_NUM_RECV_LIST = 512;

typedef zce::queue_buffer RECV_BUFFER;
struct SEND_BUFFER
{
    zce::queue_buffer *buf_queue_;
    uint64_t send_clock_;
    uint64_t ack_clock_;
    size_t fail_num_;
};

class HANDLE
{
public:

    //
    ZCE_SOCKET udp_socket_;

    //
    zce::sockaddr_ip local_;

    zce::sockaddr_ip remote_;

    HANDLE()
    {
    }
    ~HANDLE() = default;

    void clear()
    {
        udp_socket_ = ZCE_INVALID_SOCKET;
        memset(&local_, 0, sizeof(zce::sockaddr_ip));
        memset(&remote_, 0, sizeof(zce::sockaddr_ip));
    }
};

struct CORE
{
public:

    int family_;
};

class RUDP
{
protected:

    typedef zce::lord_rings <RECV_BUFFER *> RECV_BUFFER_LIST;
    typedef zce::lord_rings <SEND_BUFFER *> SEND_BUFFER_LIST;

public:

    HANDLE hanlde_;
protected:
    //
    RECV_BUFFER_LIST send_list_;
    //
    RECV_BUFFER_LIST recv_list_;
    //
    CORE *rudp_core_ = nullptr;
    //
    STATE state_;
    //
    size_t mtu_ = MTU_ETHERNET_PPOE;
    //
    time_t rto_;
};

int open(zce::rudp::RUDP *handle,
         ZCE_SOCKET udp_socket_,
         CORE *rudp_core);

int open(zce::rudp::RUDP *handle,
         CORE *rudp_core,
         const sockaddr* remote_addr,
         socklen_t addrlen);

ssize_t rudp_sendto(zce::rudp::RUDP *handle,
                    const void* buf,
                    size_t len,
                    zce::Time_Value* timeout_tv = NULL);

ssize_t rudp_recvfrom(RUDP *rudp,
                      void* buf,
                      size_t len,
                      int flags);

int core_init(CORE *core,
              size_t send_pool_num,
              size_t recv_pool_num,
              int family);

int core_dispach(CORE *core,
                 ZCE_SOCKET socket,
                 RUDP *&rudp,
                 RUDP *new_rudp);

int core_register();
}