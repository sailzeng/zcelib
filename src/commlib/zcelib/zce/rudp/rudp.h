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

enum FLAG
{
    //客户端发往服务器的第一帧,标志，让服务器告知SESSION ID
    SYN = (0x1 > 1),
    //
    ACK = (0x1 > 2),
    //带有数据
    PSH = (0x1 > 3),
    //
    RST = (0x1 > 4),
    //
    MTT = (0x1 > 5),
};

//
#pragma pack (1)

class RUDP_HEAD
{
    uint32_t len_ : 12;
    uint32_t flag_ : 8;
    uint32_t windows_num_ : 12;
    uint32_t session_id_;
    uint32_t serial_number_;
    uint32_t ack_;
    uint32_t uno1_;
    uint32_t uno2_;
    char data_[1];
};
#pragma pack ()

//
static constexpr size_t MTU_WAN = 576;
//
static constexpr size_t MSS_WAN = 576 - 20 - 8;
//
static constexpr size_t MTU_ETHERNET = 1480;
//
static constexpr size_t MSS_ETHERNET = 1480 - 20 - 8;

//最大数据处理的长度，这儿注意一下，其实最大长度只可能是MSS_ETHERNET，+4 的目的是为了方便判定错误
static constexpr size_t MAX_PROCESS_LEN = MSS_ETHERNET + 4;

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

//=====================================================================================

//=====================================================================================

class CORE;

class PEER
{
protected:

    typedef zce::lord_rings <RECV_BUFFER *> RECV_BUFFER_LIST;
    typedef zce::lord_rings <SEND_BUFFER *> SEND_BUFFER_LIST;

protected:
    //
    uint32_t session_id_;

    //
    ZCE_SOCKET udp_socket_ = ZCE_INVALID_SOCKET;

    zce::sockaddr_ip remote_;

    //
    RECV_BUFFER_LIST send_list_;
    //
    RECV_BUFFER_LIST recv_list_;

    //
    STATE state_;
    //
    size_t mtu_ = MSS_ETHERNET;
    //
    time_t rto_;
};

//=====================================================================================

class CORE
{
    int initialize(CORE *core,
                   size_t send_pool_num,
                   size_t recv_pool_num,
                   int family);

    int terminate(CORE *core);

    int receive(PEER *& recv_rudp,
                bool *new_rudp);

    int core_register(CORE *core,
                      HANDLE &handle,
                      PEER *rudp);

    int core_cancel(CORE *core,
                    HANDLE &handle);

public:
    //
    ZCE_SOCKET udp_socket_;

    int family_;

    char *receive_buffer_;

    size_t receive_len_;
    //
    uint32_t session_gen_;
    //
    std::unordered_map<uint32_t, PEER*>  rudp_map_;
};

int open(zce::rudp::PEER *handle,
         ZCE_SOCKET udp_socket_,
         CORE *rudp_core);

int open(zce::rudp::PEER *handle,
         CORE *rudp_core,
         const sockaddr* remote_addr,
         socklen_t addrlen);

ssize_t rudp_sendto(zce::rudp::PEER *handle,
                    const void* buf,
                    size_t len,
                    zce::Time_Value* timeout_tv = NULL);

ssize_t rudp_recvfrom(PEER *rudp,
                      void* buf,
                      size_t len,
                      int flags);
}