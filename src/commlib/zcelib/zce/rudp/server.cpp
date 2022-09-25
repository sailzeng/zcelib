#include "zce/predefine.h"
#include "zce/rudp/server.h"

//=================================================================================================
namespace zce::rudp
{
//服务器端CORE打开一个PEER
int server_peer::open(server_core *server_core,
                      uint32_t session_id,
                      uint32_t sequence_num,
                      ZCE_SOCKET peer_socket,
                      const sockaddr *remote_addr,
                      char *send_buffer,
                      size_t send_wnd_size,
                      size_t recv_wnd_size)
{
    bool bret = false;
    model_ = MODEL::PEER_CORE_CREATE;
    core_ = server_core;
    session_id_ = session_id;
    my_seq_num_counter_ = sequence_num;
    peer_socket_ = peer_socket;
    if (remote_addr->sa_family == AF_INET)
    {
        ::memcpy(&remote_addr_, remote_addr, sizeof(sockaddr_in));
    }
    else if (remote_addr->sa_family == AF_INET6)
    {
        ::memcpy(&remote_addr_, remote_addr, sizeof(sockaddr_in6));
    }
    else
    {
        assert(false);
        return -1;
    }
    //使用CORE的buffer
    send_buffer_ = send_buffer;
    bret = send_rec_list_.initialize(send_wnd_size / 256 + 8);
    if (!bret)
    {
        return -1;
    }
    bret = recv_rec_list_.initialize(recv_wnd_size / 256 + 8);
    if (!bret)
    {
        return -1;
    }
    bret = send_windows_.initialize(send_wnd_size);
    if (!bret)
    {
        return -1;
    }
    bret = recv_windows_.initialize(recv_wnd_size);
    if (!bret)
    {
        return -1;
    }
    //peer_live_clock_服务端打开的 PEER是收到数据创建的，所以后面会更新peer_live_clock_
    return 0;
}

void server_peer::close()
{
    peer::close();
    return;
}

void server_peer::reset()
{
    peer::close();
    return;
}

//=================================================================================================
//class server_core

int server_core::open(const sockaddr *core_addr,
                      size_t max_num_of_peer,
                      size_t peer_send_list_num,
                      size_t peer_recv_list_num,
                      std::function<ssize_t(server_peer *)> *callbak_recv,
                      std::function<int(server_peer *)> *callbak_accept)
{
    int ret = 0;
    socklen_t socket_len = sizeof(core_addr_);
    max_num_of_peer_ = max_num_of_peer;
    core_addr_ = core_addr;
    ret = zce::open_socket(&core_socket_,
                           SOCK_DGRAM,
                           core_addr,
                           socket_len);
    if (ret != 0)
    {
        return ret;
    }
    ret = zce::sock_enable(core_socket_, O_NONBLOCK);
    if (ret != 0)
    {
        return ret;
    }
    recv_buffer_ = new char[MAX_BUFFER_LEN];
    send_buffer_ = new char[MAX_BUFFER_LEN];
    memset(send_buffer_, 0x0, MAX_BUFFER_LEN);
    memset(recv_buffer_, 0x0, MAX_BUFFER_LEN);

    once_callback_rcv_ = new uint32_t[ONCE_PROCESS_RECEIVE];
    once_sendback_ack_ = new uint32_t[ONCE_PROCESS_RECEIVE];
#if defined DEBUG || defined _DEBUG
    memset(once_callback_rcv_, 0x0, ONCE_PROCESS_RECEIVE * sizeof(uint32_t));
    memset(once_sendback_ack_, 0x0, ONCE_PROCESS_RECEIVE * sizeof(uint32_t));
#endif
    peer_send_wnd_size_ = peer_send_list_num;
    peer_recv_wnd_size_ = peer_recv_list_num;
    if (callbak_recv)
    {
        is_callbak_recv_ = true;
        callbak_recv_ = *callbak_recv;
    }
    if (callbak_accept)
    {
        is_callbak_accept_ = true;
        callbak_accept_ = *callbak_accept;
    }

    return 0;
}

void server_core::close()
{
    if (recv_buffer_)
    {
        delete[]recv_buffer_;
        recv_buffer_ = nullptr;
    }
    if (send_buffer_)
    {
        delete[]send_buffer_;
        send_buffer_ = nullptr;
    }
    if (once_callback_rcv_)
    {
        delete[]once_callback_rcv_;
        once_callback_rcv_ = nullptr;
    }
    if (once_sendback_ack_)
    {
        delete[]once_sendback_ack_;
        once_sendback_ack_ = nullptr;
    }
    zce::close_socket(core_socket_);
}

int server_core::batch_receive(size_t *recv_peer_num,
                               size_t *accpet_peer_num,
                               size_t *recv_bytes)
{
    int ret = 0;

    *recv_bytes = 0;
    *recv_peer_num = 0;
    *accpet_peer_num = 0;
    once_callback_rcv_num_ = 0;
    once_sendback_ack_num_ = 0;
    for (size_t k = 0; k < ONCE_PROCESS_RECEIVE; ++k)
    {
        zce::rudp::server_peer *recv_peer = nullptr;
        zce::sockaddr_any remote_ip;
        socklen_t sz_addr = sizeof(zce::sockaddr_any);
        ssize_t ssz_recv = zce::recvfrom(core_socket_,
                                         (void *)recv_buffer_,
                                         MAX_BUFFER_LEN,
                                         0,
                                         (sockaddr*)&remote_ip,
                                         &sz_addr);
        if (ssz_recv <= 0)
        {
            if (zce::last_error() != EWOULDBLOCK)
            {
                ZCE_LOG(RS_ERROR,
                        "[RUDP][core]zce::recvfrom return error ret = [%d] errno=[%d]",
                        ssz_recv,
                        zce::last_error());
            }
            break;
        }
        else
        {
            //收到的数据长度不可能大于以太网的MSS
            if (ssz_recv > MAX_FRAME_LEN || ssz_recv < MIN_FRAME_LEN)
            {
                ZCE_LOG(RS_ERROR, "[RUDP][core] batch_receive ssz_recv [%u] error. "
                        "MIN_FRAME_LEN <= ssz_recv <=MAX_FRAME_LEN",
                        ssz_recv);
                continue;
            }
        }

        RUDP_FRAME * frame = (RUDP_FRAME *)recv_buffer_;
        frame->ntoh();
        if (frame->u32_1_.len_ != (uint32_t)ssz_recv)
        {
            //收到的FRAME数据肯定和标识长度应该一致.
            ZCE_LOG(RS_ERROR,
                    "[RUDP][core]batch_receive frame len error. ssz_recv [%d] frame len[%d]",
                    ssz_recv,
                    frame->u32_1_.len_);
            continue;
        }
        if (frame->u32_1_.flag_ & FLAG::SYN)
        {
            if (frame->session_id_ == 0)
            {
                if (peer_map_.size() >= max_num_of_peer_)
                {
                    ZCE_LOG(RS_ERROR, "[RUDP][core]batch_receive already process max. peer_map_ size[%u] > "
                            " max_num_of_peer_[%u]",
                            peer_map_.size(),
                            max_num_of_peer_);
                    break;
                }
                ret = accept(&remote_ip, recv_peer);
                if (ret != 0)
                {
                    return ret;
                }
                *accpet_peer_num = *accpet_peer_num + 1;
                if (is_callbak_accept_)
                {
                    callbak_accept_(recv_peer);
                }
            }
            else
            {
                ZCE_LOG_DEBUG(RS_ERROR, "[RUDP][core] SYN error, flag [%u] but "
                              "session id!=0,[%u]",
                              frame->u32_1_.flag_,
                              frame->session_id_);
                continue;
            }
        }
        else
        {
            auto iter = peer_map_.find(frame->session_id_);
            if (iter == peer_map_.end())
            {
                ZCE_LOG_DEBUG(RS_ERROR, "[RUDP][core] session[%u] not map to peer.",
                              frame->session_id_);
                continue;
            }
            recv_peer = iter->second;
        }

        (*recv_peer_num) += 1;
        //看远端地址是否变化了,如果变化了，更新peer_addr_set_ 表
        zce::sockaddr_any old_remote;
        bool change = false;
        //将接收的数据调教给PEER
        peer::RECV_NEXT_CALL next_call = peer::RECV_NEXT_CALL::INVALID;
        recv_peer->deliver_recv(&remote_ip,
                                frame,
                                (size_t)ssz_recv,
                                &change,
                                &old_remote,
                                &next_call);
        if (change && next_call != peer::RECV_NEXT_CALL::RESET_PEER
            && next_call != peer::RECV_NEXT_CALL::BE_RESET)
        {
            peer_addr_set_.erase(old_remote);
            peer_addr_set_.insert(std::make_pair(remote_ip,
                                  recv_peer->session_id_));
        }

        if (next_call == peer::RECV_NEXT_CALL::RESET_PEER)
        {
            recv_peer->send_frame_to(FLAG::RST);
            recv_peer->reset();
            close_peer(recv_peer);
        }
        else if (next_call == peer::RECV_NEXT_CALL::BE_RESET)
        {
            recv_peer->reset();
            close_peer(recv_peer);
        }
        else if (next_call == peer::RECV_NEXT_CALL::RECEIVE && is_callbak_recv_)
        {
            ++recv_peer->need_callback_recv_;
            once_callback_rcv_[once_callback_rcv_num_] = recv_peer->session_id_;
            once_callback_rcv_num_++;
            recv_peer->record_ack();
            once_sendback_ack_[once_sendback_ack_num_] = recv_peer->session_id_;
            once_sendback_ack_num_++;
        }
        //不直接处理RESET可能很好理解，但为什么不立即返回ACK呢。
        //如果一次接受的大量数据都是一个PEER的，那么会回复很多ACK和UNO，反而导致重发的风暴
        else if (next_call == peer::RECV_NEXT_CALL::SENDBACK_ACK)
        {
            recv_peer->record_ack();
            once_sendback_ack_[once_sendback_ack_num_] = recv_peer->session_id_;
            once_sendback_ack_num_++;
        }
        *recv_bytes += (size_t)ssz_recv;
    }

    //回调接收函数
    if (is_callbak_recv_)
    {
        for (size_t q = 0; q < once_callback_rcv_num_; ++q)
        {
            auto iter = peer_map_.find(once_callback_rcv_[q]);
            if (iter == peer_map_.end())
            {
                //不直接保存指针，而保存session id，因为在循环过程，可能部分PEER都退出了，指针会时效
                continue;
            }
            zce::rudp::server_peer *rc_peer = iter->second;
            if (rc_peer->need_callback_recv_)
            {
                callbak_recv_(rc_peer);
                rc_peer->need_callback_recv_ = 0;
            }
        }
    }
    //处理回送ACK
    for (size_t u = 0; u < once_sendback_ack_num_; ++u)
    {
        auto iter = peer_map_.find(once_sendback_ack_[u]);
        if (iter == peer_map_.end())
        {
            continue;
        }
        zce::rudp::server_peer *ack_peer = iter->second;
        if (ack_peer->need_sendback_ack_)
        {
            ack_peer->send_ack();
        }
    }
    return 0;
}

int server_core::receive_timeout(zce::time_value* timeout_tv,
                                 size_t *recv_peer_num,
                                 size_t *accpet_peer_num,
                                 size_t *recv_bytes)
{
    int ret = 0;
    //进行超时处理
    ret = zce::handle_ready(core_socket_,
                            timeout_tv,
                            zce::HANDLE_READY::READ);

    const int HANDLE_READY_ONE = 1;
    if (ret != HANDLE_READY_ONE)
    {
        return -1;
    }
    ret = batch_receive(recv_peer_num,
                        accpet_peer_num,
                        recv_bytes);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

int server_core::accept(const zce::sockaddr_any *remote_ip,
                        zce::rudp::server_peer *& new_peer)
{
    int ret = 0;
    const size_t buf_size = 64;
    char remote_str[buf_size];
    //用这个IP找找，如果地址库里有，就找出对应那个关闭
    auto iter_addr = peer_addr_set_.find(*remote_ip);
    if (iter_addr != peer_addr_set_.end())
    {
        uint32_t session_id = iter_addr->second;
        auto iter_peer = peer_map_.find(session_id);
        if (iter_peer != peer_map_.end())
        {
            server_peer * old_peer = iter_peer->second;
            old_peer->close();
        }
        else
        {
            ZCE_LOG(RS_ERROR,
                    "[RUDP] accept_peer find remote addr [%s] to session id[%u],"
                    "but session id have't map to peer.",
                    zce::get_host_addr_port((sockaddr *)&remote_ip, remote_str, buf_size),
                    session_id);
            peer_addr_set_.erase(iter_addr);
        }
    }
    new_peer = new server_peer();
    //得到非0的session id或者序列id
    uint32_t session_id = 0, serial_id = 0;
    do
    {
        //random_gen_的随机性还是挺好的，反探测的能力也一流
        session_id = random_gen_();
        auto iter = peer_map_.find(session_id);
        if (iter != peer_map_.end())
        {
            continue;
        }
    } while (session_id == 0);
    do
    {
        serial_id = random_gen_();
    } while (serial_id == 0);
    ret = new_peer->open(this,
                         session_id,
                         serial_id,
                         core_socket_,
                         (sockaddr *)remote_ip,
                         send_buffer_,
                         peer_send_wnd_size_,
                         peer_recv_wnd_size_);
    if (ret != 0)
    {
        return -1;
    }

    peer_map_[session_id] = new_peer;
    peer_addr_set_[*remote_ip] = session_id;
    return 0;
}

//删除对应的PEER
void server_core::close_peer(server_peer * del_peer)
{
    uint32_t session_id = del_peer->session_id_;
    auto iter = peer_map_.find(session_id);
    if (iter == peer_map_.end())
    {
        return;
    }
    peer_addr_set_.erase(del_peer->remote_addr_);
    peer_map_.erase(session_id);
    delete del_peer;
    del_peer = nullptr;
}

//超时处理
void server_core::time_out()
{
    uint64_t now_clock_ms = zce::clock_ms();
    //这儿我抉择过一段时间。到底要不要用一些其他unorder_map的遍历
    for (auto iter = peer_map_.begin(); iter != peer_map_.end(); ++iter)
    {
        bool no_alive = false;
        bool nouse_connectfail = false;
        iter->second->time_out(now_clock_ms, &no_alive, &nouse_connectfail);
        if (no_alive)
        {
            auto del_iter = iter;
            ++iter;
            del_iter->second->close();
            if (iter == peer_map_.end())
            {
                break;
            }
        }
    }
    return;
}
}