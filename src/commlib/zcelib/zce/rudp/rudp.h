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
    //Keep live
    KPL = (0x1 > 6),
};

//对应mtu_type_ 字段，可以有4种，目前用了2种
enum class MTU_TYPE
{
    ETHERNET = 0,
    WAN = 1,
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

    //清理
    void clear();

protected:

    //第一个uint32_t的内部数据表示，分成了若干段落
    struct U32_ONE
    {
    public:
#if ZCE_BYTES_ORDER == ZCE_LITTLE_ENDIAN
        //帧的长度，包括头部，最长4096一个包，但其实最大长度会按MTU TYPE进行控制
        uint32_t len_ : 12;
        //标志为，参考FLAG
        uint32_t flag_ : 8;
        //保留
        uint32_t reserve_ : 8;
        //采用什么MTU类型，参考MTU_TYPE
        uint32_t mtu_type_ : 4;

        //recv list 接收List的数量，用windows_num_ 这个变量名称是为了方便大家理解
#else
        //大端的字段，顺序和小端相反
        uint32_t mtu_type_ : 4;
        uint32_t reserve_ : 8;
        uint32_t flag_ : 8;
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
    uint32_t sequence_num_ = 0;
    uint32_t ack_id_ = 0;
    uint32_t windows_size_ = 0;
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
static constexpr size_t MSS_WAN = MTU_WAN - 20 - 8;
//
static constexpr size_t MTU_WAN_RUDP = MSS_WAN;
//
static constexpr size_t MSS_WAN_RUDP = MTU_WAN_RUDP - sizeof(RUDP_HEAD);

//
static constexpr size_t MTU_ETHERNET = 1480;
//
static constexpr size_t MSS_ETHERNET = MTU_ETHERNET - 20 - 8;
//
static constexpr size_t MTU_ETHERNET_RUDP = MSS_ETHERNET;
//
static constexpr size_t MSS_ETHERNET_RUDP = MTU_ETHERNET_RUDP - sizeof(RUDP_HEAD);

//最大数据处理的长度，这儿注意一下，其实最大长度只可能是MSS_ETHERNET，
static constexpr size_t MAX_FRAME_LEN = MSS_ETHERNET;
//
static constexpr size_t MIN_FRAME_LEN = sizeof(RUDP_HEAD);
//+4 的目的是为了方便判定错误
static constexpr size_t MAX_BUFFER_LEN = MAX_FRAME_LEN + 4;

//
static constexpr size_t MAX_NUM_SEND_LIST = 512;
//
static constexpr size_t MAX_NUM_RECV_LIST = 512;

//=====================================================================================
class CORE;
class PEER
{
    //有些函数只能core调用
    friend class CORE;
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
        SYS_SEND = 1,
        SYN_RCVD = 2,
        ESTABLISHED = 3,
    };

public:

    PEER() = default;
    PEER(const PEER&) = default;
    PEER& operator = (const PEER & other) = default;

    //服务器端CORE打开一个PEER
    int open(uint32_t session_id,
             uint32_t serial_id,
             ZCE_SOCKET peer_socket,
             const sockaddr *remote_addr,
             char *send_buffer,
             size_t send_rec_list_size,
             size_t send_windows_capacity,
             size_t recv_windows_capacity);

    //以客户端方式打开一个PEER
    int open(const sockaddr *remote_addr,
             size_t send_rec_list_size,
             size_t send_list_capacity,
             size_t recv_list_capacity);

    void close();

    ///给外部调用的接收接口
    int recv(char* buf,
             size_t &len);

    ///给外部调用的发送接口
    int send(const char* buf,
             size_t &len);

    //
    inline uint32_t seesion_id()
    {
        return session_id_;
    }

    int reset();

protected:

    //
    int deliver_recv(const zce::sockaddr_ip *remote_addr,
                     RUDP_FRAME *recv_frame,
                     size_t frame_len,
                     bool *remote_change,
                     zce::sockaddr_ip *old_remote);
    //
    int send_frame_to(int flag,
                      uint32_t sequence_num = 0,
                      bool first_send = true,
                      const char *data = nullptr,
                      size_t sz_data = 0);

    int state_changes(RUDP_FRAME *recv_frame);

    //
    int recvfrom();

    //跟进收到ACK ID确认那些发送成功了
    int acknowledge_send(uint32_t recv_ack_id);

    /// 计算RTO
    void calculate_rto(uint64_t send_clock);

protected:

    typedef zce::cycle_buffer RECV_RECORD;
    struct SEND_RECORD
    {
        //这个发送报的序列号
        uint32_t sequence_num_ = 0;
        //
        size_t len_ = 0;
        //
        char *buf_pos_ = nullptr;

        //发送的时间，
        uint64_t send_clock_ = 0;
        //超时的时间
        uint64_t timeout_clock_ = 0;
        //发送的次数
        size_t send_num_ = 0;
    };

    typedef zce::lord_rings<SEND_RECORD >  SEND_RECORD_LIST;

protected:
    //模式，不同的open函数决定不同的模式
    MODEL model_ = MODEL::PEER_INVALID;

    //状态
    STATE rudp_state_ = STATE::CLOSE;

    //会话ID，表示这个会话状态，由CORE 生产，客户端负责每次携带
    uint32_t session_id_ = 0;
    //序列号
    uint32_t snd_seq_num_counter_ = 0;
    //已经确认的序列号ID,已经收到了ACK
    uint32_t snd_seq_num_ack_ = 0;

    //期待收到的下一个seq num，也就是回复的ack id
    uint32_t rcv_expect_seq_num_ = 0;

    //Socket 句柄
    ZCE_SOCKET peer_socket_ = ZCE_INVALID_SOCKET;
    //远端地址，注意UDP远端地址是可能变化的，
    zce::sockaddr_ip remote_addr_;

    //发送记录列表
    SEND_RECORD_LIST send_rec_list_;
    //接收的滑动窗口
    zce::cycle_buffer recv_windows_;
    //发送数据的滑动窗口
    zce::cycle_buffer send_windows_;

    //
    char *recv_buffer_ = nullptr;
    //
    char *send_buffer_ = nullptr;

    //MTU的类型,从道理来说。两端的MTU可以不一样，因为走得线路都可能不一样，
    //但考虑到简单，我们先把两端的MTU约束成一样,
    MTU_TYPE mtu_type_ = MTU_TYPE::ETHERNET;

    //
    size_t peer_windows_size_ = 64 * 1024;
    //
    time_t rto_ = 80;
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
                   size_t peer_send_rec_list_size,
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

    //发送记录列表的数量
    size_t peer_send_rec_list_size_ = 0;
    //CORE创建的PEER的接收队列数量
    size_t peer_recv_list_num_ = 0;
    //CORE创建的PEER的发送队列数量
    size_t peer_send_list_num_ = 0;

    //随机数发生器，用于生产session id，和序号ID serial_id
    std::mt19937  random_gen_;

    //session id对应的PEER map
    std::unordered_map<uint32_t, PEER*>  peer_map_;
    //地址对应的session id的map
    std::unordered_map<zce::sockaddr_ip, uint32_t, sockaddr_ip_hash> peer_addr_set_;
};
}