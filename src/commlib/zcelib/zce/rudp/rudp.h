#pragma once

#include "zce/util/lord_rings.h"
#include "zce/util/buffer.h"
#include "zce/os_adapt/common.h"
#include "zce/os_adapt/socket.h"

namespace zce::rudp
{
//=====================================================================================

enum FLAG
{
    //客户端发往服务器的第一帧,标志，让服务器告知SESSION ID
    SYN = (0x1 > 1),
    //ACK应答数据，表示ack_
    ACK = (0x1 > 2),
    //带有数据
    PSH = (0x1 > 3),
    //
    RST = (0x1 > 4),
    //link MTU TEST，链路的MTU测试帧
    LMT = (0x1 > 5),
};

#pragma pack (1)

class RUDP_HEAD
{
public:

    //构造函数，复制函数，禁止大家都可以用的.
    RUDP_HEAD() = default;
    RUDP_HEAD& operator = (const RUDP_HEAD& other) = default;
    //析构函数
    ~RUDP_HEAD() = default;

    ///将头部数据转换网络序
    void hton();
    ///将头部数据转换为本地序
    void ntoh();

protected:

    struct U32_ONE
    {
    public:
#if ZCE_BYTES_ORDER == ZCE_LITTLE_ENDIAN
        uint32_t len_ : 12;
        uint32_t flag_ : 6;
        uint32_t mtu_type_ : 2;
        uint32_t windows_num_ : 12;
#else
        uint32_t windows_num_ : 12;
        uint32_t mtu_type_ : 2;
        uint32_t flag_ : 6;
        uint32_t len_ : 12;
#endif
    };

public:

    union
    {
        ///frame 选项
        U32_ONE  u32_1_;
        /// 辅助编大小端转换
        uint32_t u32_1_copy_ = 0;
    };
    uint32_t session_id_ = 0;
    uint32_t serial_id_ = 0;
    uint32_t ack_ = 0;
    uint32_t uno1_ = 0;
    uint32_t uno2_ = 0;
};

class RUDP_FRAME : public RUDP_HEAD
{
public:
    char data_[1];
};

#pragma pack ()

//=====================================================================================

//
static constexpr size_t MTU_WAN = 576;
//
static constexpr size_t MSS_WAN = 576 - 20 - 8;
//
static constexpr size_t MTU_ETHERNET = 1480;
//
static constexpr size_t MSS_ETHERNET = 1480 - 20 - 8;

//最大数据处理的长度，这儿注意一下，其实最大长度只可能是MSS_ETHERNET，+4 的目的是为了方便判定错误
static constexpr size_t MAX_FRAME_LEN = MSS_ETHERNET + 4;
//
static constexpr size_t MIN_FRAME_LEN = sizeof(RUDP_HEAD);

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
class PEER
{
protected:

    enum class STATE
    {
        CLOSE = 0,
        SYN_SND = 1,
        SYN_RCV = 2,
        ESTABLISHED = 3,
    };

    typedef zce::lord_rings <RECV_BUFFER *> RECV_BUFFER_LIST;
    typedef zce::lord_rings <SEND_BUFFER *> SEND_BUFFER_LIST;

protected:
    //
    uint32_t session_id_ = 0;

    //
    ZCE_SOCKET udp_socket_ = ZCE_INVALID_SOCKET;

    zce::sockaddr_ip remote_;

    //
    RECV_BUFFER_LIST send_list_;
    //
    RECV_BUFFER_LIST recv_list_;

    //
    STATE state_ = STATE::CLOSE;
    //
    size_t mtu_ = MSS_ETHERNET;
    //
    time_t rto_;
};

//=====================================================================================
///RUDP core，服务器端用的类
class CORE
{
public:
    CORE() = default;
    CORE(const CORE&) = default;
    CORE& operator = (const CORE & other) = default;
    //析构函数
    ~CORE() = default;

    int initialize(int family,
                   size_t send_pool_num,
                   size_t recv_pool_num);

    void terminate();

    int receive(PEER *& recv_rudp,
                bool *new_rudp);

    //发送应答
    void send_head();

    //int register(uint32_t session_id,
    //             PEER *rudp);

    //int cancel(uint32_t session_id,
    //           HANDLE &handle);

protected:
    //
    int family_ = AF_INET;
    //
    ZCE_SOCKET udp_socket_ = ZCE_INVALID_SOCKET;

    //
    char *recv_buffer_ = nullptr;
    //
    char *send_buffer_ = nullptr;

    //随机数发生器，用于生产session id，和序号ID serial_id
    std::mt19937  random_gen_;

    //
    std::unordered_map<uint32_t, PEER*>  peer_map_;
};

int open(zce::rudp::PEER *handle,
         ZCE_SOCKET udp_socket_,
         CORE *rudp_core);

int open(zce::rudp::PEER *handle,
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