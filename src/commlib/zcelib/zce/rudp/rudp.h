/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   rudp.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2021年6月20日
* @brief      RUDP，
*
*
* @details    我用到的类似TCP的技术
*             接受窗口和发送窗口，都是用了滑动窗口进行处理，
*             Zero Window Probe  ZWP 当对端窗口尺寸是0是询问对端，并且如果如果窗口变化
*             而且窗口小于阈值，会主动通知对端。
*             Fast Recovery 快速重传机制 收到2个相同的ACK，主动进行重传。
*             TCP默认是3个相同的ACK，这儿有简化
*             SACK ，类似，发现如果有收到的数据不连续有，跳跃，会在ACK数据里面主动通知对端
*
* @note       最后至少写了2周，若干次反复。
*             拥塞控制（流控）还是其中很讨厌的地方。
*/

#pragma once

#include "zce/util/lord_rings.h"
#include "zce/util/static_list.h"
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
    SYN = (0x1 << 0),
    //ACK应答数据，表示ack_
    ACK = (0x1 << 1),
    //带有数据
    PSH = (0x1 << 2),
    //RESET，要求对方重置，感觉对方已经疯癫了。
    RST = (0x1 << 3),
    //link MTU TEST，链路的MTU测试帧
    LMT = (0x1 << 4),
    //are you ok？询问对方情况，也用于Keep live。
    AYO = (0x1 << 5),
    //应答AYO，Iam ok，会带上窗口信息。
    IAO = (0x1 << 6),
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
        uint32_t flag_ : 10;
        //保留
        uint32_t reserve_ : 4;
        //selective的数量
        uint32_t req_resend_num_ : 4;
        //采用什么MTU类型，参考MTU_TYPE
        uint32_t mtu_type_ : 2;

        //outer_recv list 接收List的数量，用windows_num_ 这个变量名称是为了方便大家理解
#else
        //大端的字段，顺序和小端相反
        uint32_t mtu_type_ : 2;
        uint32_t req_resend_num_ : 4;
        uint32_t reserve_ : 4;
        uint32_t flag_ : 10;
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
    //会话ID
    uint32_t session_id_ = 0;
    //序列号，数据对应的序列号
    uint32_t sequence_num_ = 0;
    //应答ID，确认收到的对端的序列号
    uint32_t ack_id_ = 0;
    //本地接收窗口大小
    uint32_t windows_size_ = 0;

    //请求重传的序列号，这个设计类似SACK，
    uint32_t req_resend_seq_[3] = { 0 };
};

/**
 * @brief RUDP_FRAME,数据头部
*/
class RUDP_FRAME : public RUDP_HEAD
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
class BASE
{
protected:
    //随机数发生器，用于生产session id，和序号ID serial_id
    static std::mt19937  random_gen_;

public:
    //取得随机数
    static uint32_t random();

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
};

//=====================================================================================

class PEER :public BASE
{
protected:

    enum class MODEL
    {
        //无效模式
        PEER_INVALID = 0,
        //客户端模式
        PEER_CLIENT = 1,
        //CORE创造的模式（类似服务器SOCKET）
        PEER_CORE_CREATE = 2,
    };

    //接受过程中发生的操作
    enum class RECV_OPERATION
    {
        //无效值
        INVALID = 0,
        //填充空档位
        FILL = 1,
        //重复收到数据
        REPEAT = 2,
        //提前到了
        ADVANCE = 3,
        //放到了尾部，（但不是顺序）
        TAIL = 4,
        //顺序收到 ：）
        SERIES = 5,
        //错误
        ERR = 6,
    };

    //改变拥塞窗口的事件
    enum class CWND_EVENT
    {
        INVALID = 0,
        //
        ACK,
        //
        FAST_RECOVERY,
        //
        RTO_RECOVERY,
        //
        SWND_CHANGE,
    };

    struct SEND_RECORD;
public:

    PEER() = default;
    PEER(const PEER&) = default;
    PEER& operator = (const PEER & other) = default;
protected:
    ~PEER() = default;
public:

    //!关闭，具体行为由继承类实现
    virtual void close() = 0;
    //!重置，具体行为由继承类实现
    virtual void reset() = 0;

    /**
     * @brief 给外部调用的接收接口,从接收窗口取数据，
     * @param[in]     buf
     * @param[in,out] recv_len
     * @return
    */
    int recv(char* buf,
             size_t *recv_len);

    /**
     * @brief 给外部调用的发送接口，把数据让如发送窗口，（是否实际发送看情况）
     *        会对数据会进行分包处理
     * @param[in]     buf       接收数据的buffer，
     * @param[in,out] send_len  发送数据长度
     * @return 返回0表示成功，如果返回-1，错误是EWOULDBLOCK表示请稍等一下发送
    */
    int send(const char* buf,
             size_t *send_len);

    //!可以接收的数据(窗口)尺寸
    inline size_t recv_wnd_size()
    {
        return recv_windows_.size();
    }

    //!得到PEER对应的session id
    inline uint32_t session_id()
    {
        return session_id_;
    }

    //连接是否建立
    inline bool established()
    {
        return established_;
    }

protected:

    /**
     * @brief 传递接收的数据给peer
     * @param [in]  remote_addr 远端地址,因为是UDP，可能会变化
     * @param [in]  recv_frame  接收的FRAME
     * @param [in]  frame_len  FRAME的长度
     * @param [out] remote_change 返回参数，远端地址是否改变
     * @param [out] old_remote    返回原来的远端地址
     * @param [out] call_recv     返回是否收到的连续的数据，可以调用callback recv
     * @param [out] call_connect  返回是否调用连接成功函数，可以调用callback connect
     * @param [out] reset         返回是否需要reset处理
     * @return 返回0表示成功
    */
    int deliver_recv(const zce::sockaddr_ip *remote_addr,
                     RUDP_FRAME *recv_frame,
                     size_t frame_len,
                     bool *remote_change,
                     zce::sockaddr_ip *old_remote,
                     bool *call_recv,
                     bool *call_connect,
                     bool *reset);

    /**
     * @brief 发送FRAME数据去远端，
     * @param[in] flag       发送的FLAG
     * @param[in] first_send 是否是第一次发送，
     * @param[in] data       发送的数据
     * @param[in] sz_data    发送的数据长度
     * @param[in] snd_rec    发送的记录，如果不是第一次发送，会使用发送记录进行重发
     * @return 返回0表示成功，
    */
    int send_frame_to(int flag,
                      bool first_send = true,
                      const char *data = nullptr,
                      size_t sz_data = 0,
                      SEND_RECORD *snd_rec = nullptr);

    //跟进收到ACK ID确认那些发送成功了
    int acknowledge_send(const RUDP_FRAME *recv_frame,
                         uint64_t now_clock);

    //!在收到ACK返回之后，计算RTO
    void calculate_rto(uint64_t send_clock,
                       uint64_t now_clock);

    ///处理接收的数据
    int process_recv_data(const RUDP_FRAME *recv_frame,
                          RECV_OPERATION *op);

    //!超时处理，大约10ms调用一次他。
    void time_out(uint64_t now_clock_ms,
                  bool *not_alive);

    //!
    void adjust_cwnd(CWND_EVENT event);

protected:

    //!发送（需要确认的发送）的记录，
    struct SEND_RECORD
    {
        //!发送的帧的FLAG，目前好像没用用到
        uint32_t flag_ = 0;
        //!这个发送报的序列号
        uint32_t sequence_num_ = 0;
        //!发送数据的长度
        size_t len_ = 0;
        //!记录这个数据在发送窗口的位置，
        char *buf_pos_ = nullptr;

        //!发送的时间，需要记录，在就算RTO时使用
        uint64_t send_clock_ = 0;
        //!超时的时间，大于这个时间，就可以进行重新发送
        uint64_t timeout_clock_ = 0;
        //!发送的次数
        size_t send_num_ = 0;
    };

    typedef zce::lord_rings<SEND_RECORD >  SEND_RECORD_LIST;

    //!接收记录
    struct RECV_RECORD
    {
        //
        uint32_t start_;
        //
        uint32_t end_;
    };
    typedef zce::static_list<RECV_RECORD >  RECV_RECORD_LIST;

    //!最小的RTO值
    static time_t min_rto_;
    //!超时阻塞的情况下，rto增加的比率
    static double blocking_rto_ratio_;
    //!如果在一段时间没有消息维持活跃，则关闭
    static time_t noalive_time_to_close_;

protected:
    //!模式，不同的open函数决定不同的模式
    MODEL model_ = MODEL::PEER_INVALID;

    //!会话ID，表示这个会话状态，由CORE 生产，客户端负责保存
    uint32_t session_id_ = 0;

    //!连接是否建立
    bool established_ = false;

    //自己的（发送）序列号
    uint32_t my_seq_num_counter_ = 0;
    //自己的已经确认的（发送）序列号ID,已经收到了ACK
    uint32_t my_seq_num_ack_ = 0;

    //!已经连续起来的SEQ，也就是期待收到对方的下一个数据的SEQ，也就是回复的ACK id
    uint32_t rcv_wnd_series_end_ = 0;
    //!接受窗口内最开始的序列号
    uint32_t rcv_wnd_first_ = 0;
    //!接受窗口收到的最后一个序列值（注意是尾部）
    uint32_t rcv_wnd_last_ = 0;

    //!Socket 句柄
    ZCE_SOCKET peer_socket_ = ZCE_INVALID_SOCKET;
    //!远端地址，注意UDP远端地址是可能变化的，
    zce::sockaddr_ip remote_addr_{};

    //!发送记录列表,最大记录数和窗口大小有关系
    SEND_RECORD_LIST send_rec_list_;
    //!发送数据的滑动窗口 swnd
    zce::cycle_buffer send_windows_;

    //!cmd 拥塞窗口，进行流量控制，这儿我不针对窗口大小处理，而是对一次可以发送的记录数量控制
    //!拥塞控制的在我们这种引用层面控制很难，
    size_t congestion_window_ = INIT_CWND_SIZE;

    //!接收记录列表,最大记录数和接收窗口大小有关系
    RECV_RECORD_LIST recv_rec_list_;
    //!接收的滑动窗口 rwnd
    zce::cycle_buffer recv_windows_;

    //!接收的BUFFER,根据model不同，生成（处理）方式不同
    char *recv_buffer_ = nullptr;
    //!发送的BUFFER,根据model不同，生成（处理）方式不同
    char *send_buffer_ = nullptr;

    //MTU的类型,从道理来说。两端的MTU可以不一样，因为走得线路都可能不一样，
    //但考虑到简单，我们先把两端的MTU约束成一样,
    MTU_TYPE mtu_type_ = MTU_TYPE::ETHERNET;

    //对端窗口的大小
    size_t peer_windows_size_ = 64 * 1024;
    //RTO，
    time_t rto_ = 80;

    //!发送的字节数量
    uint64_t send_bytes_ = 0;
    //!接收到的数据数量
    uint64_t recv_bytes_ = 0;

    //对端最后活动（收到数据）的时间
    uint64_t peer_live_clock_ = 0;
};

//=====================================================================================
class CLIENT :public PEER
{
public:

    CLIENT() = default;
    CLIENT(const CLIENT&) = default;
    CLIENT& operator = (const CLIENT & other) = default;

public:

    /**
     * @brief 以客户端方式打开一个PEER，模式：PEER_CLIENT
     * @param remote_addr     远端地址
     * @param send_wnd_size   发送窗口尺寸
     * @param recv_wnd_size   接收窗口尺寸
     * @param callbak_recv    可选参数，如果有接收数据，进行回调函数，
     * @param callbak_connect 可选参数，如果连接成功，进行回调函数，
     * @return
    */
    int open(const sockaddr *remote_addr,
             size_t send_wnd_size,
             size_t recv_wnd_size,
             std::function<ssize_t(CLIENT *)> *callbak_recv = nullptr,
             std::function<int(CLIENT *, bool)> *callbak_connect = nullptr);

    //!关闭，
    virtual void close() override;
    //!重置，
    virtual void reset() override;

    //!得到SOCKET句柄
    inline ZCE_SOCKET get_handle()
    {
        return peer_socket_;
    }

    //! 客户端无阻塞（无等待）收取数据,收取数据到内部接收窗口
    //! 如果发生了读取事件后，可以调用这个函数，你可以在select 等函数后调用这个函数
    int receive_i(size_t *recv_size);

    //! 客户端阻塞（等待）收取数据
    int receive_timeout_i(zce::Time_Value* timeout_tv,
                          size_t *recv_size);

    //! 异步（非阻塞）连接，返回0并不表示真正成功，还没有对方确认
    int connect(bool link_test_mtu = false);

    //! 同步连接，等待@timeout_tv的时间，
    int connect_timeout(zce::Time_Value* timeout_tv,
                        bool link_test_mtu = false);

    //!超时处理，每10ms调用一次
    void time_out();

protected:
    //!
    bool is_callbak_recv_ = false;
    //! 发现接收数据时，接收回调函数，在函数里面调用outer_recv提取数据
    //! 第一个参数是接收数据的PEER *
    std::function<ssize_t(CLIENT *)> callbak_recv_;

    //!
    bool is_callbak_connect_ = false;
    //! 连接成功和失败的回调函数
    std::function<int(CLIENT *, bool)> callbak_connect_;
};

//=====================================================================================
class CORE;
class ACCEPT :public PEER
{
    //有些函数只能core调用
    friend class CORE;
public:
    ACCEPT() = default;
    ACCEPT(const ACCEPT&) = default;
    ACCEPT& operator = (const ACCEPT & other) = default;

protected:

    /**
     * @brief 服务器端CORE打开一个PEER，模式：PEER_CORE_CREATE
     * @param core
     * @param session_id    会话ID，对应这个PEER的标识
     * @param serial_id     序列号ID，SEQUNCE ID
     * @param peer_socket   相应的SOCKET，注意RUDP是共用CORE的SOCKET
     * @param remote_addr   远端地址
     * @param send_buffer   发送的BUFFER
     * @param send_wnd_size 发送窗口的大小，发送记录数由窗口决定
     * @param recv_wnd_size 接收窗口的大小，接收记录数由窗口决定
     * @param callbak_recv  回调的接收函数，如果收到了数据，回调这个函数，
     * @return
    */
    int open(CORE * core,
             uint32_t session_id,
             uint32_t serial_id,
             ZCE_SOCKET peer_socket,
             const sockaddr * remote_addr,
             char *send_buffer,
             size_t send_wnd_size,
             size_t recv_wnd_size);

    virtual void close() override;

    virtual void reset() override;

protected:
    //!core的指针，如果peer是core创造的（），会保持core的指针
    CORE *core_ = nullptr;
};

//=====================================================================================
///RUDP core，服务器端用的类
class CORE :public BASE
{
public:
    CORE() = default;
    CORE(const CORE&) = default;
    CORE& operator = (const CORE & other) = default;
    //析构函数
    ~CORE() = default;

    /**
     * @brief 初始化CORE
     * @param core_addr 绑定的地址
     * @param max_num_of_peer 允许CORE同时管理的PEER数量
     * @param peer_send_wnd_size CORE创建的每个PEER的发送窗口尺寸，发送窗口保存没有确认的发送数据
     * @param peer_recv_wnd_size CORE创建的每个PEER的接受窗口尺寸，接收窗口保存上层没有提取的数据
     * @param callbak_recv   ACCEPT的PEER收到数据后的回调函数
     * @param callbak_accept ACCEPT的PEER后的回调函数
     * @return
    */
    int open(const sockaddr *core_addr,
             size_t max_num_of_peer,
             size_t peer_send_wnd_size,
             size_t peer_recv_wnd_size,
             std::function<ssize_t(ACCEPT *)> *callbak_recv = nullptr,
             std::function<int(ACCEPT *)> *callbak_accept = nullptr);

    void close();

    /**
     * @brief 接受数据的处理,不阻塞,可以在select 时间触发后调用这个函数
     * @return 返回收到数据的尺寸，==0成功，非0失败
    */
    int receive_i(size_t *recv_size,
                  ACCEPT *&recv_rudp,
                  bool *accpect);

    /**
     * @brief 带超时的接收处理
     * @param timeout_tv 超时时间
     * @param recv_peer_num   输出参数，发生接收处理的peer数量
     * @param accpet_peer_num 输出参数，发生accept处理的peer数量
     * @param recv_bytes      输出参数，接收的字节数量
     * @return
    */
    int receive_timeout_i(zce::Time_Value* timeout_tv,
                          size_t *recv_peer_num,
                          size_t *accpet_peer_num,
                          size_t *recv_bytes);

    //!超时处理，没10ms调用一次
    void time_out();

    //!得到SOCKET句柄
    inline ZCE_SOCKET get_handle()
    {
        return core_socket_;
    }

    //删除对应的PEER
    void close_peer(ACCEPT *del_peer);

protected:

    //创建一个PEER
    int accept_peer(const zce::sockaddr_ip *remote_ip,
                    ACCEPT *& new_peer);

protected:

    //!Socket 句柄
    ZCE_SOCKET core_socket_ = ZCE_INVALID_SOCKET;
    //!本地地址，CORE地址，服务器地址
    zce::sockaddr_ip core_addr_;

    //!最大支持的RUDP PEER数量。
    size_t max_num_of_peer_ = 102400;

    //!接收的BUFFER
    char *recv_buffer_ = nullptr;
    //!发送的BUFFER,
    char *send_buffer_ = nullptr;

    //!CORE创建的PEER的接收队列数量
    size_t peer_recv_wnd_size_ = 0;
    //!CORE创建的PEER的发送队列数量
    size_t peer_send_wnd_size_ = 0;

    //session id对应的PEER map
    ///note:unordered_map 有一个不太理想的地方，就是遍历慢，特别是负载低时遍历慢。
    std::unordered_map<uint32_t, ACCEPT*>  peer_map_;

    //地址对应的session id的map
    std::unordered_map<zce::sockaddr_ip, uint32_t, sockaddr_ip_hash> peer_addr_set_;

    //!是否调用recv 的回调函数
    bool is_callbak_recv_ = false;
    //! 发现接收数据时，接收回调函数，在函数里面调用outer_recv提取数据
    //! 第一个参数是接收数据的ACCEPT *
    std::function<ssize_t(ACCEPT *)> callbak_recv_;

    //!是否调用accept 的回调函数
    bool is_callbak_accept_ = false;
    //!发生accept的时候，进行回调的函数
    std::function<int(ACCEPT *)> callbak_accept_;
};
}