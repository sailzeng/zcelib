#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/rudp/client.h"

namespace zce::rudp
{
//=================================================================================================
client::~client()
{
}

//客户端调用的接收数据到内部的函数
//打开一个客户端
int client::open(const sockaddr *remote_addr,
                 size_t send_wnd_size,
                 size_t recv_wnd_size,
                 std::function<ssize_t(client *)> *callbak_recv,
                 std::function<int(client *, bool)> *callbak_connect)
{
    int ret = 0;
    model_ = MODEL::PEER_CLIENT;
    my_seq_num_counter_ = base::random();

    remote_addr_ = remote_addr;
    ret = zce::open_socket(&peer_socket_,
                           SOCK_DGRAM,
                           remote_addr->sa_family);
    if (ret != 0)
    {
        return ret;
    }
    ret = zce::sock_enable(peer_socket_, O_NONBLOCK);
    if (ret != 0)
    {
        return ret;
    }

    bool bret = false;
    bret = send_rec_list_.initialize(send_wnd_size / 256 + 8);
    if (!bret)
    {
        return -2;
    }
    bret = recv_rec_list_.initialize(recv_wnd_size / 256 + 8);
    if (!bret)
    {
        return -3;
    }
    bret = send_windows_.initialize(send_wnd_size);
    if (!bret)
    {
        return -4;
    }
    bret = recv_windows_.initialize(recv_wnd_size);
    if (!bret)
    {
        return -5;
    }
    recv_buffer_ = new char[MAX_BUFFER_LEN];
    send_buffer_ = new char[MAX_BUFFER_LEN];
#if defined DEBUG || defined _DEBUG
    memset(send_buffer_, 0x0, MAX_BUFFER_LEN);
    memset(recv_buffer_, 0x0, MAX_BUFFER_LEN);
#endif
    if (callbak_recv)
    {
        is_callbak_recv_ = true;
        callbak_recv_ = *callbak_recv;
    }
    if (callbak_connect)
    {
        is_callbak_connect_ = true;
        callbak_connect_ = *callbak_connect;
    }

    peer_live_clock_ = zce::clock_ms();
    return 0;
}

void client::close()
{
    peer::close();
    zce::close_socket(peer_socket_);
    if (recv_buffer_)
    {
        delete[] recv_buffer_;
        recv_buffer_ = nullptr;
    }
    if (send_buffer_)
    {
        delete[] send_buffer_;
        send_buffer_ = nullptr;
    }
    return;
}

void client::reset()
{
    peer::close();
    send_rec_list_.clear();
    send_windows_.clear();
    recv_rec_list_.clear();
    recv_windows_.clear();
    session_id_ = 0;
    my_seq_num_counter_ = base::random();
    my_seq_num_ack_ = 0;
    rcv_wnd_first_ = 0;
    rcv_wnd_last_ = 0;
    rcv_wnd_series_end_ = 0;
    return;
}

//客户端无阻塞（无等待）收取数据,收取数据到内部接收窗口
int client::batch_receive(size_t *recv_size)
{
    *recv_size = 0;
    const size_t ONCE_PROCESS_TIMES = 256;
    const size_t buf_size = 64;
    char out_buf[buf_size];
    zce::sockaddr_any remote_ip;
    socklen_t sz_addr = sizeof(zce::sockaddr_any);
    for (size_t k = 0; k < ONCE_PROCESS_TIMES; ++k)
    {
        ssize_t ssz_recv = zce::recvfrom(peer_socket_,
                                         (void *)recv_buffer_,
                                         MAX_BUFFER_LEN,
                                         0,
                                         (sockaddr*)&remote_ip,
                                         &sz_addr);
        if (ssz_recv <= 0)
        {
            if (zce::last_error() == EWOULDBLOCK)
            {
                continue;
            }
            else
            {
                ZCE_LOG(RS_ERROR,
                        "[RUDP][CLIENT]zce::recvfrom return error ret = [%d] session[%u] remote[%s] errno=[%d]",
                        ssz_recv,
                        session_id_,
                        zce::get_host_addr_port((sockaddr *)&remote_addr_, out_buf, buf_size),
                        zce::last_error());
                break;
            }
        }
        else
        {
            //收到的数据长度不可能大于以太网的MSS
            if ((size_t)ssz_recv > MAX_FRAME_LEN || (size_t)ssz_recv < MIN_FRAME_LEN)
            {
                ZCE_LOG(RS_ERROR,
                        "[RUDP][CLIENT]zce::recvfrom ssz_recv [%u] session id [%u] remote[%s]",
                        ssz_recv,
                        session_id_,
                        zce::get_host_addr_port((sockaddr *)&remote_addr_, out_buf, buf_size));
                break;
            }
        }
        zce::sockaddr_any old_remote;
        bool change = false;
        RUDP_FRAME *recv_frame = (RUDP_FRAME *)recv_buffer_;
        recv_frame->ntoh();
        //
        RECV_NEXT_CALL next_call = RECV_NEXT_CALL::INVALID;
        deliver_recv(&remote_ip,
                     recv_frame,
                     (size_t)ssz_recv,
                     &change,
                     &old_remote,
                     &next_call);

        if (next_call == RECV_NEXT_CALL::CONNECT && is_callbak_connect_)
        {
            callbak_connect_(this, true);
        }
        else if (next_call == peer::RECV_NEXT_CALL::RESET_PEER)
        {
            send_frame_to(FLAG::RST);
            reset();
            close();
        }
        else if (next_call == peer::RECV_NEXT_CALL::BE_RESET)
        {
            reset();
            close();
        }
        else if (next_call == peer::RECV_NEXT_CALL::RECEIVE && is_callbak_recv_)
        {
            ++need_callback_recv_;
            record_ack();
        }
        //不直接处理RESET可能很好理解，但为什么不立即返回ACK呢。
        //如果一次接受的大量数据都是一个PEER的，那么会回复很多ACK和UNO，反而导致重发的风暴
        else if (next_call == peer::RECV_NEXT_CALL::SENDBACK_ACK)
        {
            record_ack();
        }
        *recv_size += (size_t)ssz_recv;
    }
    if (need_callback_recv_)
    {
        callbak_recv_(this);
        need_callback_recv_ = 0;
    }
    if (need_sendback_ack_)
    {
        send_ack();
    }
    return 0;
}

int client::receive_timeout(zce::time_value* timeout_tv,
                            size_t *recv_size)
{
    *recv_size = 0;
    int ret = 0;
    //进行超时处理
    ret = zce::handle_ready(peer_socket_,
                            timeout_tv,
                            zce::READ_MASK);

    const int HANDLE_READY_ONE = 1;
    if (ret != HANDLE_READY_ONE)
    {
        return -1;
    }
    batch_receive(recv_size);

    return 0;
}

//! 异步（非阻塞）连接，返回0并不表示真正成功，还没有对方确认
int client::connect(bool link_test_mtu)
{
    int ret = 0;
    if (link_test_mtu)
    {
        ret = send_frame_to(FLAG::SYN | FLAG::LMT);
        if (ret != 0)
        {
            return ret;
        }
    }
    else
    {
        ret = send_frame_to(FLAG::SYN);
        if (ret != 0)
        {
            return ret;
        }
    }
    return 0;
}

//! 同步连接，等待@timeout_tv的时间，
int client::connect_timeout(zce::time_value* timeout_tv,
                            bool link_test_mtu)
{
    int ret = 0;
    ret = connect(link_test_mtu);
    if (ret != 0)
    {
        return ret;
    }
    size_t recv_size;
    ret = receive_timeout(timeout_tv, &recv_size);
    if (ret != 0)
    {
        return ret;
    }
    if (!established_)
    {
        return -1;
    }
    return 0;
}

//!超时处理，每10ms调用一次
void client::time_out()
{
    uint64_t now_clock_ms = zce::clock_ms();

    bool no_alive = false;
    bool connect_fail = false;
    peer::time_out(now_clock_ms, &no_alive, &connect_fail);
    if (no_alive)
    {
        close();
    }
    if (connect_fail && callbak_connect_)
    {
        callbak_connect_(this, false);
    }
}
}