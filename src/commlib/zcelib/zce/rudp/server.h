#pragma once

#include "zce/container/lord_rings.h"
#include "zce/buffer/cycle_buffer.h"
#include "zce/pool/buffer_pool.h"
#include "zce/rudp/peer.h"

namespace zce::rudp
{
class server_core;
class server_peer :public zce::rudp::peer
{
    //有些函数只能core调用
    friend class server_core;
public:
    server_peer() = default;
    server_peer(const server_peer&) = default;
    server_peer& operator = (const server_peer & other) = default;

protected:

    /**
     * @brief 服务器端CORE打开一个PEER，模式：PEER_CORE_CREATE
     * @param server_core
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
    int open(server_core * server_core,
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
    server_core *core_ = nullptr;
};

//=====================================================================================
///RUDP core，服务器端用的类
class server_core :public zce::rudp::base
{
public:
    server_core() = default;
    server_core(const server_core&) = default;
    server_core& operator = (const server_core & other) = default;
    //析构函数
    ~server_core() = default;

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
             std::function<ssize_t(server_peer *)> *callbak_recv = nullptr,
             std::function<int(server_peer *)> *callbak_accept = nullptr);

    ///关闭
    void close();

    /**
     * @brief 接受数据的处理,不阻塞,可以在select 时间触发后调用这个函数
     * @return 返回收到数据的尺寸，==0成功，非0失败
    */
    int batch_receive(size_t *recv_peer_num,
                      size_t *accpet_peer_num,
                      size_t *recv_bytes);

    /**
     * @brief 带超时的接收处理
     * @param timeout_tv 超时时间
     * @param recv_peer_num   输出参数，发生接收处理的peer数量
     * @param accpet_peer_num 输出参数，发生accept处理的peer数量
     * @param recv_bytes      输出参数，接收的字节数量
     * @return
    */
    int receive_timeout(zce::time_value* timeout_tv,
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
    void close_peer(server_peer *del_peer);

protected:

    //accept创建一个PEER
    int accept(const zce::sockaddr_any *remote_ip,
               zce::rudp::server_peer *& new_peer);

protected:
    //!
    const size_t ONCE_PROCESS_RECEIVE = 256;
protected:

    //!Socket 句柄
    ZCE_SOCKET core_socket_ = ZCE_INVALID_SOCKET;
    //!本地地址，CORE地址，服务器地址
    zce::sockaddr_any core_addr_;

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
    std::unordered_map<uint32_t,
        zce::rudp::server_peer*>  peer_map_;

    //地址对应的session id的map
    std::unordered_map<zce::sockaddr_any, uint32_t, sockaddr_ip_hash> peer_addr_set_;

    //! receive_i内完成(多次)接收后，需要回调接受函数的PEER的seesion id队列
    //! 为什么保存seesion id，而不保存指针，你可以想想
    uint32_t *once_callback_rcv_ = nullptr;
    //! @once_callback_rcv_ 队列长度
    size_t once_callback_rcv_num_ = 0;

    //! receive_i内完成(多次)接收后，需要会送ACK的PEER的session id 队列
    uint32_t *once_sendback_ack_ = nullptr;
    //! @once_sendback_ack_队列长度
    size_t once_sendback_ack_num_ = 0;

    //!是否调用recv 的回调函数
    bool is_callbak_recv_ = false;
    //! 发现接收数据时，接收回调函数，在函数里面调用outer_recv提取数据
    //! 第一个参数是接收数据的ACCEPT *
    std::function<ssize_t(zce::rudp::server_peer *)> callbak_recv_;

    //!是否调用accept 的回调函数
    bool is_callbak_accept_ = false;
    //!发生accept的时候，进行回调的函数
    std::function<int(zce::rudp::server_peer *)> callbak_accept_;
};
}
