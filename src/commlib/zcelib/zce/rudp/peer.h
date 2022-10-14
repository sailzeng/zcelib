#pragma once

#include "zce/os_adapt/common.h"
#include "zce/logger/log_comm.h"
#include "zce/os_adapt/socket.h"
#include "zce/container/static_list.h"
#include "zce/container/lord_rings.h"
#include "zce/buffer/cycle_buffer.h"
#include "zce/rudp/base.h"

namespace zce::rudp
{
class peer :public zce::rudp::base
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
    enum class RECV_DATA_LOCATION
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
        //没有足够的空间接纳数据
        NOROOM = 6,
        //错误
        ERR = 7,
    };

    //!改变"拥塞"窗口的事件
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

    //!
    enum class RECV_NEXT_CALL
    {
        //无效值
        INVALID = 0,
        //可以回调connect 函数
        CONNECT = 1,
        //可以回调 recv 函数
        RECEIVE = 2,
        //需要RESET peer，
        RESET_PEER = 3,
        //被RESET了，
        BE_RESET = 4,
        //需要发送ACK
        SENDBACK_ACK = 5,
    };

    struct SEND_RECORD;
public:

    peer() = default;
    peer(const peer&) = default;
    peer& operator = (const peer & other) = default;
protected:
    virtual ~peer() = default;
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
    inline size_t recv_bytes()
    {
        uint32_t can_read = rcv_wnd_series_end_ - rcv_wnd_first_;
        return can_read;
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
     * @param [out] next_call     返回下一步的操作类型是什么
     * @return 返回0表示成功
    */
    int deliver_recv(const zce::sockaddr_any *remote_addr,
                     RUDP_FRAME *recv_frame,
                     size_t frame_len,
                     bool *remote_change,
                     zce::sockaddr_any *old_remote,
                     RECV_NEXT_CALL *next_call);

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
                      bool prev_rec_ack = false,
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
                          RECV_DATA_LOCATION *op);

    //!超时处理，大约10ms调用一次他。
    void time_out(uint64_t now_clock_ms,
                  bool *not_alive,
                  bool *connect_fail);

    //!调整拥塞窗口
    void adjust_cwnd(CWND_EVENT event);

    //!记录要发送的ACK，等待
    inline void record_ack()
    {
        ++need_sendback_ack_;
        record_prev_ack_ = rcv_wnd_series_end_;
    }

    //!发送ACK
    void send_ack();

    //!DUMP信息
    void dump_info(const char *some_thing, zce::LOG_PRIORITY log_priority);

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

    //! 接收记录
    struct RECV_RECORD
    {
        //! 接受数据的SN
        uint32_t seq_start_;
        //! 接受数据结束的序列号，也就是SN+len
        uint32_t seq_end_;
    };
    typedef zce::static_list<RECV_RECORD >  RECV_RECORD_LIST;

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
    zce::sockaddr_any remote_addr_{};

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

    //! 是否需要调用recv 回调函数，大于0表示需要
    size_t need_callback_recv_ = 0;
    // 是否需要调用recv 回调函数，大于0表示需要
    size_t need_sendback_ack_ = 0;
    //!记录的钱一个可用的ACK值，多次recvfrom之后，如果
    uint32_t record_prev_ack_ = 0;

    //!发送的字节数量
    uint64_t send_bytes_ = 0;
    //!重复发送的字节数量
    uint64_t resend_bytes_ = 0;
    //!接收到的数据数量
    uint64_t recv_bytes_ = 0;
    //!重复接收到的数据数量
    uint64_t rerecv_bytes_ = 0;

    //对端最后活动（收到数据）的时间
    uint64_t peer_live_clock_ = 0;
};
}