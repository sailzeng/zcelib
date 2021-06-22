/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   rudp.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2021年6月20日
* @brief
*
*
* @details
*
*
*
* @note
*
*/

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
    SYN = (0x1 << 1),
    //ACK应答数据，表示ack_
    ACK = (0x1 << 2),
    //带有数据
    PSH = (0x1 << 3),
    //RESET
    RST = (0x1 << 4),
    //link MTU TEST，链路的MTU测试帧
    LMT = (0x1 << 5),
    //Keep live
    KPL = (0x1 << 6),
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
        //selective的数量
        uint32_t selective_num_ : 2;
        //采用什么MTU类型，参考MTU_TYPE
        uint32_t mtu_type_ : 2;

        //outer_recv list 接收List的数量，用windows_num_ 这个变量名称是为了方便大家理解
#else
        //大端的字段，顺序和小端相反
        uint32_t mtu_type_ : 2;
        uint32_t selective_num_ : 2;
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
    //会话ID
    uint32_t session_id_ = 0;
    //序列号，数据对应的序列号
    uint32_t sequence_num_ = 0;
    //应答ID，确认收到的对端的序列号
    uint32_t ack_id_ = 0;
    //本地接收窗口大小
    uint32_t windows_size_ = 0;
    //SACK
    uint32_t sack_[3] = { 0 };
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

    //WAN的MTU
    static constexpr size_t MTU_WAN = 576;
    //UDP包在WAN网络的负载
    static constexpr size_t MSS_WAN_UDP = MTU_WAN - 20 - 8;
    //RUDP的MTU
    static constexpr size_t MTU_WAN_RUDP = MSS_WAN_UDP;
    //RUDP的MSS，要减去头部大小
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
    static constexpr size_t MAX_FRAME_LEN = MTU_ETHERNET_RUDP;
    //
    static constexpr size_t MIN_FRAME_LEN = sizeof(RUDP_HEAD);
    //+4 的目的是为了方便判定错误
    static constexpr size_t MAX_BUFFER_LEN = MAX_FRAME_LEN + 4;
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

    //! 给外部调用的接收接口,从接收窗口取数据，
    int recv(char* buf,
             size_t &len);

    //! 给外部调用的发送接口，把数据让如发送窗口，（是否实际发送看情况）
    //! 内部对数据会进行分包处理
    int send(const char* buf,
             size_t &len);

    //!可以接收的数据(窗口)尺寸
    inline size_t recv_wnd_size()
    {
        return recv_windows_.size();
    }

    inline uint32_t session_id()
    {
        return session_id_;
    }

protected:

    //core 传递 接受数据给peer
    int deliver_recv(const zce::sockaddr_ip *remote_addr,
                     RUDP_FRAME *recv_frame,
                     size_t frame_len,
                     bool *remote_change,
                     zce::sockaddr_ip *old_remote);
    //发送frame
    int send_frame_to(int flag,
                      bool first_send = true,
                      const char *data = nullptr,
                      size_t sz_data = 0,
                      SEND_RECORD *snd_rec = nullptr);

    //跟进收到ACK ID确认那些发送成功了
    int acknowledge_send(RUDP_FRAME *recv_frame,
                         uint64_t now_clock);

    ///在收到ACK返回之后，计算RTO
    void calculate_rto(uint64_t send_clock,
                       uint64_t now_clock);

    ///记录收到的跳跃(selective)数据
    void record_selective(RUDP_FRAME *selective_frame);

    ///处理跳跃的数据，
    void proces_selective();

    ///处理接收的数据
    bool process_recv_data(RUDP_FRAME *recv_frame,
                           bool *already_processed,
                           bool *advance_arrive);

    //!超时处理，大约10ms调用一次他。如果是CORE模式，
    void time_out(uint64_t now_clock_ms);

protected:

    //发送（需要确认的发送）的记录，
    struct SEND_RECORD
    {
        //发送的帧的FLAG，目前好像没用用到
        uint32_t flag_ = 0;
        //这个发送报的序列号
        uint32_t sequence_num_ = 0;
        //发送数据的长度
        size_t len_ = 0;
        //记录这个数据在发送窗口的位置，
        char *buf_pos_ = nullptr;

        //发送的时间，需要记录，在就算RTO时使用
        uint64_t send_clock_ = 0;
        //超时的时间，大于这个时间，就可以进行重新发送
        uint64_t timeout_clock_ = 0;
        //发送的次数
        size_t send_num_ = 0;
    };

    typedef zce::lord_rings<SEND_RECORD >  SEND_RECORD_LIST;

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

    //对方期待收到的下一个数据的seq num，也就是回复的ACK id
    uint32_t peer_expect_seq_num_ = 0;

    //!Socket 句柄
    ZCE_SOCKET peer_socket_ = ZCE_INVALID_SOCKET;
    //!远端地址，注意UDP远端地址是可能变化的，
    zce::sockaddr_ip remote_addr_{};

    //!发送记录列表
    SEND_RECORD_LIST send_rec_list_;
    //!接收的滑动窗口
    zce::cycle_buffer recv_windows_;
    //!发送数据的滑动窗口
    zce::cycle_buffer send_windows_;

    //!接收的BUFFER,根据model不同，生成（处理）方式不同
    char *recv_buffer_ = nullptr;
    //!发送的BUFFER,根据model不同，生成（处理）方式不同
    char *send_buffer_ = nullptr;

    //! 发现接收数据时，接收回调函数，在函数里面调用outer_recv提取数据
    //! 第一个参数是接收数据的PEER *
    std::function<ssize_t(PEER *)> callbak_recv_;

    ///收到的跳跃包队列数量，最大是3
    size_t selective_ack_num_ = 0;
    //跳跃包的队列
    RUDP_FRAME *selective_ack_ary_[3] = { nullptr };

    //MTU的类型,从道理来说。两端的MTU可以不一样，因为走得线路都可能不一样，
    //但考虑到简单，我们先把两端的MTU约束成一样,
    MTU_TYPE mtu_type_ = MTU_TYPE::ETHERNET;

    //对端窗口的大小
    size_t peer_windows_size_ = 64 * 1024;
    //RTO，
    time_t rto_ = 80;

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
    //以客户端方式打开一个PEER，模式：PEER_CLIENT
    int open(const sockaddr *remote_addr,
             size_t send_rec_list_size,
             size_t send_wnd_size,
             size_t recv_wnd_size,
             std::function<ssize_t(PEER *)> &callbak_recv);

    //!关闭，
    virtual void close() override;
    //!重置，
    virtual void reset() override;

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
    //服务器端CORE打开一个PEER，模式：PEER_CORE_CREATE
    int open(CORE * core,
             uint32_t session_id,
             uint32_t serial_id,
             ZCE_SOCKET peer_socket,
             const sockaddr * remote_addr,
             char *send_buffer,
             size_t send_rec_list_size,
             size_t send_wnd_size,
             size_t recv_wnd_size,
             std::function<ssize_t(PEER *)> &callbak_recv);

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
    ~CORE();

    /**
     * @brief 初始化CORE
     * @param core_addr 绑定的地址
     * @param max_num_of_peer 允许CORE同时管理的PEER数量
     * @param peer_send_rec_list_size CORE创建的每个PEER的发送记录数量，发送记录要保存没有确认的帧
     * @param peer_send_wnd_size CORE创建的每个PEER的发送窗口尺寸，发送窗口保存没有确认的发送数据
     * @param peer_recv_wnd_size CORE创建的每个PEER的接受窗口尺寸，接收窗口保存上层没有提取的数据
     * @param callbak_recv CORE创建的每个PEER的接受窗口尺寸，接收窗口保存上层没有提取的数据
     * @return
    */
    int open(const sockaddr *core_addr,
             size_t max_num_of_peer,
             size_t peer_send_rec_list_size,
             size_t peer_send_wnd_size,
             size_t peer_recv_wnd_size,
             std::function<ssize_t(PEER *)> &peer_callbak_recv);

    void close();

    /**
     * @brief 接受数据的处理,不阻塞,可以在select 时间触发后调用这个函数
     * @return 返回收到数据的尺寸，==0成功，非0失败
    */
    int receive_i(size_t *recv_size);
    /**
     * @brief 带超时处理的接收，
     * @param timeout_tv 超时时间，
    */
    int receive_timeout_i(zce::Time_Value* timeout_tv,
                          size_t *recv_size);

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

    //Socket 句柄
    ZCE_SOCKET core_socket_ = ZCE_INVALID_SOCKET;
    //本地地址，CORE地址，服务器地址
    zce::sockaddr_ip core_addr_;

    //最大支持的RUDP PEER数量。
    size_t max_num_of_peer_ = 102400;

    //!接收的BUFFER
    char *recv_buffer_ = nullptr;
    //!发送的BUFFER,
    char *send_buffer_ = nullptr;

    //发送记录列表的数量
    size_t peer_send_rec_list_size_ = 0;
    //CORE创建的PEER的接收队列数量
    size_t peer_recv_wnd_size_ = 0;
    //CORE创建的PEER的发送队列数量
    size_t peer_send_wnd_size_ = 0;

    //!PEER收到数据的回调函数
    //! 第一个参数是session id，第二个参数是接收数据的PEER *
    std::function<ssize_t(PEER *)> peer_callbak_recv_;

    //session id对应的PEER map
    ///note:unordered_map 有一个不太理想的地方，就是遍历慢，特别是负载低时遍历慢。
    std::unordered_map<uint32_t, ACCEPT*>  peer_map_;
    //地址对应的session id的map
    std::unordered_map<zce::sockaddr_ip, uint32_t, sockaddr_ip_hash> peer_addr_set_;
};
}