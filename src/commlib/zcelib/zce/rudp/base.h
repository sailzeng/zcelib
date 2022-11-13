#pragma once

namespace zce::rudp
{
//=====================================================================================

enum FLAG
{
    //客户端发往服务器的第一帧,标志，让服务器告知SESSION ID
    SYN = (0x1 << 0),
    //ACK应答数据，表示ack_
    ACK = (0x1 << 1),
    //带有数据
    PSH = (0x1 << 2),
    //
    UNA = (0x1 << 3),
    //RESET，要求对方重置，感觉对方已经疯癫了。
    RST = (0x1 << 4),
    //link MTU TEST，链路的MTU测试帧
    LMT = (0x1 << 5),
    //are you ok？询问对方情况，也用于Keep live。
    AYO = (0x1 << 6),
    //应答AYO，Iam ok，会带上窗口信息。
    IAO = (0x1 << 7),
};

//对应mtu_type_ 字段，可以有4种，目前用了2种
enum class MTU_TYPE
{
    ETHERNET = 0,
    WAN = 1,
};

#pragma pack (1)

struct RUDP_HEAD
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
        uint32_t flag_ : 10;
        //保留
        uint32_t reserve_ : 4;
        //UNA的数量
        uint32_t una_num_ : 4;
        //采用什么MTU类型，参考MTU_TYPE
        uint32_t mtu_type_ : 2;

        //outer_recv list 接收List的数量，用windows_num_ 这个变量名称是为了方便大家理解
#else
        //大端的字段，顺序和小端相反
        uint32_t mtu_type_ : 2;
        uint32_t una_num_ : 4;
        uint32_t reserve_ : 4;
        uint32_t flag_ : 10;
        uint32_t len_ : 12;
#endif
    };

public:
    //!最大的UNA数组数量
    static constexpr size_t MAX_UNA_NUMBER = 3;

public:
    union
    {
        ///frame 选项
        U32_ONE  u32_1_;
        /// 辅助编大小端转换
        uint32_t u32_1_copy_ = 0;
    };
    //会话ID
    uint32_t session_id_ = 0;
    //序列号，数据对应的序列号
    uint32_t sequence_num_ = 0;
    //应答ID，确认收到的对端的序列号
    uint32_t ack_id_ = 0;
    //本地接收窗口大小
    uint32_t windows_size_ = 0;

    //请求重传的序列号，这个设计类似SACK，目前最多3个
    uint32_t una_[MAX_UNA_NUMBER] = { 0 };
};

/**
 * @brief RUDP_FRAME,数据头部
*/
struct RUDP_FRAME : public RUDP_HEAD
{
protected:
    //构造函数，复制函数，禁止大家都可以用的.
    RUDP_FRAME() = delete;
    RUDP_FRAME& operator = (const RUDP_FRAME& other) = delete;
    //析构函数
    ~RUDP_FRAME() = delete;

public:
    /**
    * @brief new 一个frame
    * @param frame_len frame的长度，包括头部
    * @return 生产的frame指针
    */
    static RUDP_FRAME *new_frame(size_t frame_len);

    ///删除回收一个new的frame
    static void delete_frame(RUDP_FRAME *frame);

    //填充Data数据到RUDP_FRAME
    int fill_data(const size_t szdata, const char* vardata);

public:
    char data_[1];
};

#pragma pack ()

//=====================================================================================
class base
{
public:
    //取得随机数
    static uint32_t random();

    //!设置一些关键静态变量

    static void min_rto(time_t rto);

    static void blocking_rto_ratio(double rto_ratio);

    static void noalive_time_to_close(time_t to_close_time);

protected:

    //!WAN的MTU
    static constexpr size_t MTU_WAN = 576;
    //!UDP包在WAN网络的负载，去掉IP头部，UDP头部长度
    static constexpr size_t MSS_WAN_UDP = MTU_WAN - 20 - 8;
    //!RUDP在WAN的MTU
    static constexpr size_t MTU_WAN_RUDP = MSS_WAN_UDP;
    //!RUDP的MSS，要减去头部大小
    static constexpr size_t MSS_WAN_RUDP = MTU_WAN_RUDP - sizeof(RUDP_HEAD);

    //!以太网的MTU
    static constexpr size_t MTU_ETHERNET = 1480;
    //!UDP包在以太网ETHERNET网络的负载 1452
    static constexpr size_t MSS_ETHERNET = MTU_ETHERNET - 20 - 8;
    //!RUDP在以太网的MTU
    static constexpr size_t MTU_ETHERNET_RUDP = MSS_ETHERNET;
    //!RUDP在以太网的MSS,减去头部长度
    static constexpr size_t MSS_ETHERNET_RUDP = MTU_ETHERNET_RUDP - sizeof(RUDP_HEAD);

    //!最大数据处理的长度，这儿注意一下，其实最大长度只可能是MSS_ETHERNET，
    static constexpr size_t MAX_FRAME_LEN = MTU_ETHERNET_RUDP;
    //!最小的FRAME长度，头部大小
    static constexpr size_t MIN_FRAME_LEN = sizeof(RUDP_HEAD);

    //!BUFFER的大小 +4 的目的是为了方便判定错误
    static constexpr size_t MAX_BUFFER_LEN = MAX_FRAME_LEN + 4;

    static constexpr size_t INIT_CWND_SIZE = 4;

protected:
    //! 随机数发生器，用于生产session id，和序号ID serial_id
    static std::mt19937  random_gen_;
    //! 最小的RTO值
    static time_t min_rto_;
    //! 超时阻塞的情况下，rto增加的比率
    static double blocking_rto_ratio_;
    //! 如果在一段时间没有消息维持活跃，则关闭
    static time_t noalive_time_to_close_;
};
}