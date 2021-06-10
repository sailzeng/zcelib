#pragma once

#include "zce/util/lord_rings.h"
#include "zce/util/buffer.h"
#include "zce/pool/buffer_pool.h"
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
    //RESET
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

    //第一个uint32_t的内部数据表示，分成了若干段落
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
protected:
    //构造函数，复制函数，禁止大家都可以用的.
    RUDP_FRAME() = delete;
    RUDP_FRAME& operator = (const RUDP_FRAME& other) = delete;
    //析构函数
    ~RUDP_FRAME() = delete;

public:
    //填充Data数据到RUDP_FRAME
    int fill_data(const size_t szdata, const char* vardata);

public:
    char data_[1];
};

#pragma pack ()

//=====================================================================================
constexpr size_t BUF_BUCKET_NUM = 8;
constexpr size_t BUF_BUCKET_SIZE_ARY[] = { 64,128,256,512,768,1024,1280,1536 };
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

//=====================================================================================
class PEER
{
protected:

    enum class MODEL
    {
        PEER_INVALID = 0,
        //
        PEER_CLIENT = 1,
        //
        PEER_CORE_CREATE = 1,
    };

    enum class STATE
    {
        CLOSE = 0,
        SYN_SND = 1,
        SYN_RCV = 2,
        ESTABLISHED = 3,
    };

    typedef zce::queue_buffer RECV_BUFFER;
    struct SEND_BUFFER
    {
        zce::queue_buffer *buf_queue_;
        uint64_t send_clock_;
        uint64_t ack_clock_;
        size_t fail_num_;
    };

    typedef zce::lord_rings <RECV_BUFFER *> RECV_BUFFER_LIST;
    typedef zce::lord_rings <SEND_BUFFER *> SEND_BUFFER_LIST;

public:

    PEER() = default;
    PEER(const PEER&) = default;
    PEER& operator = (const PEER & other) = default;

    //服务器端CORE打开一个PEER
    int open(uint32_t session_id,
             uint32_t serial_id,
             ZCE_SOCKET peer_socket,
             const sockaddr *remote_addr,
             size_t send_list_num,
             size_t recv_list_num,
             zce::buffer_pool *buf_pool);

    //以客户端方式打开一个PEER
    int open(const sockaddr *remote_addr,
             size_t send_list_num,
             size_t recv_list_num);

    void close();

    //远端地址是否发生了变化
    bool remote_change(const zce::sockaddr_ip &new_remote,
                       zce::sockaddr_ip &old_remote);
    //
    int receive(const sockaddr *remote_addr,
                RUDP_FRAME *rudp_frame,
                size_t frame_len);

    //
    inline uint32_t seesion_id()
    {
        return session_id_;
    }

    int reset();

protected:
    //模式，不同的open函数决定不同的模式
    MODEL model_ = MODEL::PEER_INVALID;
    //
    uint32_t session_id_ = 0;
    //
    uint32_t serial_id_ = 0;

    //Socket 句柄
    ZCE_SOCKET peer_socket_ = ZCE_INVALID_SOCKET;
    //远端地址
    zce::sockaddr_ip remote_addr_;

    //buffer的缓冲池子
    zce::buffer_pool *buf_pool_;
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
    ~CORE();

    int initialize(const sockaddr *core_addr,
                   size_t max_num_of_peer,
                   size_t peer_send_list_num,
                   size_t peer_recv_list_num);

    void terminate();

    int receive(PEER *& recv_rudp,
                bool *new_rudp);

protected:

    int create_peer(const zce::sockaddr_ip *remote_ip,
                    PEER *& new_peer);

    //发送应答
    void send_head();

    //int register(uint32_t session_id,
    //             PEER *rudp);

    //int cancel(uint32_t session_id,
    //           HANDLE &handle);

protected:

    //Socket 句柄
    ZCE_SOCKET core_socket_ = ZCE_INVALID_SOCKET;
    //本地地址，CORE地址，服务器地址
    zce::sockaddr_ip core_addr_;

    //最大支持的RUDP PEER数量。
    size_t max_num_of_peer_ = 102400;

    //
    char *recv_buffer_ = nullptr;
    //
    char *send_buffer_ = nullptr;

    //
    zce::buffer_pool buf_pool_;

    //CORE创建的PEER的接收队列数量
    size_t peer_recv_list_num_;
    //CORE创建的PEER的发送队列数量
    size_t peer_send_list_num_;

    //随机数发生器，用于生产session id，和序号ID serial_id
    std::mt19937  random_gen_;

    //session id对应的PEER map
    std::unordered_map<uint32_t, PEER*>  peer_map_;
    //地址对应的session id的map
    std::unordered_map<zce::sockaddr_ip, uint32_t, sockaddr_ip_hash> peer_addr_set_;
};
}