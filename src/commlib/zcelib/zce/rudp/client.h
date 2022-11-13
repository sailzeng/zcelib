#pragma once

#include "zce/rudp/peer.h"

namespace zce::rudp
{
//=====================================================================================
class client :public zce::rudp::peer
{
public:

    client() = default;
    client(const client&) = default;
    client& operator = (const client & other) = default;
    ~client();

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
             std::function<ssize_t(client *)> *callbak_recv = nullptr,
             std::function<int(client *, bool)> *callbak_connect = nullptr);

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
    int batch_receive(size_t *recv_size);

    //! 客户端阻塞（等待）收取数据
    int receive_timeout(zce::time_value* timeout_tv,
                        size_t *recv_size);

    //! 异步（非阻塞）连接，返回0并不表示真正成功，还没有对方确认
    int connect(bool link_test_mtu = false);

    //! 同步连接，等待@timeout_tv的时间，
    int connect_timeout(zce::time_value* timeout_tv,
                        bool link_test_mtu = false);

    //!超时处理，每10ms调用一次
    void time_out();

protected:
    //!
    bool is_callbak_recv_ = false;
    //! 发现接收数据时，接收回调函数，在函数里面调用outer_recv提取数据
    //! 第一个参数是接收数据的PEER *
    std::function<ssize_t(client *)> callbak_recv_;

    //!
    bool is_callbak_connect_ = false;
    //! 连接成功和失败的回调函数
    std::function<int(client *, bool)> callbak_connect_;
};
}
