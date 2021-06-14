#include "zce/predefine.h"
#include "zce/rudp/rudp.h"
#include "zce/os_adapt/socket.h"

namespace zce::rudp
{
//=================================================================================================
//class RUDP_HEAD

void RUDP_HEAD::hton()
{
    u32_1_copy_ = htonl(u32_1_copy_);
    session_id_ = htonl(session_id_);
    sequence_num_ = htonl(sequence_num_);
    ack_id_ = htonl(ack_id_);
    windows_size_ = htonl(windows_size_);
    uno1_ = htonl(uno1_);
    uno2_ = htonl(uno2_);
}

//将所有的uint16_t,uint32_t转换为本地序
void RUDP_HEAD::ntoh()
{
    u32_1_copy_ = ntohl(u32_1_copy_);
    session_id_ = ntohl(session_id_);
    sequence_num_ = ntohl(sequence_num_);
    ack_id_ = ntohl(ack_id_);
    windows_size_ = ntohl(windows_size_);
    uno1_ = ntohl(uno1_);
    uno2_ = ntohl(uno2_);
}

void RUDP_HEAD::clear()
{
    u32_1_copy_ = 0;
    session_id_ = 0;
    sequence_num_ = 0;
    ack_id_ = 0;
    windows_size_ = 0;
    uno1_ = 0;
    uno2_ = 0;
}

//填充Data数据到Frame
int RUDP_FRAME::fill_data(const size_t szdata, const char* vardata)
{
    //填写数据区的长度
    ::memcpy(data_, vardata, szdata);
    u32_1_.len_ = static_cast<uint32_t>(sizeof(RUDP_HEAD) + szdata);
    return 0;
}

//=================================================================================================
//class PEER

//服务器端CORE打开一个PEER
int PEER::open(uint32_t session_id,
               uint32_t sequence_num,
               ZCE_SOCKET peer_socket,
               const sockaddr *remote_addr,
               char *send_buffer,
               size_t send_rec_list_size,
               size_t send_windows_capacity,
               size_t recv_windows_capacity)
{
    int ret = 0;
    model_ = MODEL::PEER_CORE_CREATE;
    rudp_state_ = STATE::SYN_RCVD;
    session_id_ = session_id;
    snd_seq_num_counter_ = sequence_num;
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
    ret = send_rec_list_.initialize(send_rec_list_size);
    if (ret != 0)
    {
        return ret;
    }
    ret = send_windows_.initialize(send_windows_capacity);
    if (ret != 0)
    {
        return ret;
    }
    ret = recv_windows_.initialize(recv_windows_capacity);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

//客户端打开一个
int PEER::open(const sockaddr *remote_addr,
               size_t send_rec_list_size,
               size_t send_windows_capacity,
               size_t recv_windows_capacity)
{
    int ret = 0;
    model_ = MODEL::PEER_CLIENT;
    std::mt19937  random_gen;
    random_gen.seed((unsigned int)time(NULL));
    snd_seq_num_counter_ = random_gen();

    remote_addr_ = remote_addr;
    ret = zce::open_socket(&peer_socket_,
                           SOCK_DGRAM,
                           remote_addr->sa_family);
    if (ret != 0)
    {
        return ret;
    }
    ret = send_rec_list_.initialize(send_rec_list_size);
    if (ret != 0)
    {
        return ret;
    }
    ret = send_windows_.initialize(send_windows_capacity);
    if (ret != 0)
    {
        return ret;
    }
    ret = recv_windows_.initialize(recv_windows_capacity);
    if (ret != 0)
    {
        return ret;
    }
    recv_buffer_ = new char[MAX_BUFFER_LEN];
    send_buffer_ = new char[MAX_BUFFER_LEN];

    rudp_state_ = STATE::SYS_SEND;
    send_frame_to(FLAG::SYN);
    return 0;
}

int PEER::reset()
{
    return 0;
}

void PEER::close()
{
    if (model_ == MODEL::PEER_CLIENT)
    {
        zce::close_socket(peer_socket_);

        delete[] recv_buffer_;
        recv_buffer_ = nullptr;
        delete[] send_buffer_;
        send_buffer_ = nullptr;
    }
    //else CORE 产生的 不进行这些处理

    rudp_state_ = STATE::CLOSE;

    return;
}

//给外部调用的接收接口
int PEER::recv(char* buf,
               size_t &len)
{
    if (len > recv_windows_.size())
    {
        len = recv_windows_.size();
    }
    recv_windows_.pop_front(buf, len);

    return 0;
}

//给外部调用的发送接口
int PEER::send(const char* buf,
               size_t &len)
{
    int ret = 0;
    size_t frame_max_len = MSS_ETHERNET_RUDP;
    if (mtu_type_ == MTU_TYPE::ETHERNET)
    {
        frame_max_len = MSS_ETHERNET_RUDP;
    }
    else if (mtu_type_ == MTU_TYPE::WAN)
    {
        frame_max_len = MSS_WAN_RUDP;
    }
    //else

    //发送数据太长，臣妾搞不掂呀
    size_t snd_rec_free = send_rec_list_.free();
    size_t snd_wnd_free = send_windows_.free();
    if (send_windows_.free() == 0 || snd_rec_free == 0)
    {
        zce::last_error(EWOULDBLOCK);
        ZCE_LOG(RS_INFO, "");
        return -1;
    }

    size_t remain_len = len;
    size_t one_process = 0, process_len = 0;
    while (remain_len > 0 && snd_rec_free > 0 &&
           snd_wnd_free > 0 && peer_windows_size_ > 0)
    {
        //得到这一次最大能处理的数据大小
        one_process = remain_len > frame_max_len ?
            frame_max_len : remain_len;
        one_process = one_process > snd_wnd_free ?
            snd_wnd_free : one_process;
        one_process = one_process > peer_windows_size_ ?
            peer_windows_size_ : one_process;

        ret = send_frame_to(FLAG::PSH,
                            snd_seq_num_counter_,
                            true,
                            buf + process_len,
                            one_process);
        if (ret != 0)
        {
            break;
        }
        snd_seq_num_counter_ += one_process;

        remain_len -= one_process;
        snd_wnd_free -= one_process;
        peer_windows_size_ -= one_process;
        --snd_rec_free;
        process_len += one_process;

        ZCE_LOG(RS_DEBUG, "");
    }
    len = process_len;

    //是否要把超时的都拎出来发送一次？

    return 0;
}

int PEER::deliver_recv(const zce::sockaddr_ip *remote_addr,
                       RUDP_FRAME *recv_frame,
                       size_t frame_len,
                       bool *remote_change,
                       zce::sockaddr_ip *old_remote)
{
    int ret = 0;
    if (frame_len <= 0)
    {
        return -1;
    }
    else
    {
        //收到的数据长度不可能大于以太网的MSS
        if (frame_len > MSS_ETHERNET || frame_len < MIN_FRAME_LEN)
        {
            ZCE_LOG(RS_ERROR, "");
            return -2;
        }
    }
    //远端地址可能发生改变，
    zce::sockaddr_ip new_remote = *remote_addr;
    if (new_remote != remote_addr_)
    {
        *remote_change = true;
        *old_remote = remote_addr_;
        remote_addr_ = new_remote;
        ZCE_LOG(RS_INFO, "");
    }
    peer_windows_size_ = recv_frame->windows_size_;
    mtu_type_ = static_cast<MTU_TYPE>(recv_frame->u32_1_.mtu_type_);
    //记录服务器分配的session id
    if (model_ == MODEL::PEER_CLIENT && session_id_ == 0)
    {
        session_id_ = recv_frame->session_id_;
    }
    else
    {
        if (session_id_ != recv_frame->session_id_)
        {
            ZCE_LOG(RS_ERROR, "");
            return -1;
        }
    }

    ret = state_changes(recv_frame);
    if (ret != 0)
    {
        return ret;
    }

    //对方有数据
    if (recv_frame->u32_1_.flag_ & FLAG::PSH)
    {
        size_t data_len = frame_len - sizeof(RUDP_HEAD);
        if (recv_windows_.free() < data_len)
        {
        }

        uint32_t recv_seq_num = recv_frame->sequence_num_;

        //这儿有一点技巧，避免 wraparound引发麻烦,其实就是利用了补码
        int32_t difference = (int32_t)(recv_seq_num - rcv_expect_seq_num_);

        //收取重复数据
        if (difference > 0)
        {
            ZCE_LOG(RS_DEBUG, "");
            return 0;
        }
        //发生了跳跃
        else if (difference < 0)
        {
        }
        //和期待的seq num 一致，
        else
        {
            char *wt_ptr = nullptr;
            recv_windows_.push_end(recv_frame->data_, data_len, wt_ptr);
            rcv_expect_seq_num_ = rcv_expect_seq_num_ + data_len;
        }
    }
    else if (recv_frame->u32_1_.flag_ & FLAG::LMT)
    {
        if ((recv_frame->u32_1_.flag_ & FLAG::ACK) == 0)
        {
            //什么都不做，回复一个ACK
            send_frame_to(FLAG::ACK);
        }
    }
    else if (recv_frame->u32_1_.flag_ & FLAG::RST)
    {
    }

    //
    if (recv_frame->u32_1_.flag_ & FLAG::ACK)
    {
        ret = acknowledge_send(recv_frame->ack_id_);
    }
    return 0;
}

int PEER::state_changes(RUDP_FRAME *recv_frame)
{
    if (rudp_state_ == STATE::SYN_RCVD)
    {
        if (recv_frame->u32_1_.flag_ & FLAG::ACK &&
            recv_frame->ack_id_ == snd_seq_num_counter_ + 1)
        {
            rudp_state_ = STATE::ESTABLISHED;
            ++snd_seq_num_counter_;
            rcv_expect_seq_num_ = recv_frame->sequence_num_ + 1;
            send_frame_to(FLAG::ACK & FLAG::SYN);
        }
        else
        {
            return -1;
        }
    }
    else if (rudp_state_ == STATE::SYS_SEND)
    {
        if (recv_frame->u32_1_.flag_ & FLAG::SYN &&
            recv_frame->u32_1_.flag_ & FLAG::ACK &&
            recv_frame->ack_id_ == snd_seq_num_counter_ + 1)
        {
            rudp_state_ = STATE::ESTABLISHED;
            ++snd_seq_num_counter_;
            rcv_expect_seq_num_ = recv_frame->sequence_num_ + 1;
            send_frame_to(FLAG::ACK);
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else if (rudp_state_ == STATE::CLOSE)
    {
        send_frame_to(FLAG::RST);
        return -1;
    }
    return 0;
}

int PEER::recvfrom()
{
    assert(model_ == MODEL::PEER_CLIENT);
    if (model_ != MODEL::PEER_CLIENT)
    {
        return -1;
    }
    zce::sockaddr_ip remote_ip;
    socklen_t sz_addr = sizeof(zce::sockaddr_ip);
    ssize_t ssz_recv = zce::recvfrom(peer_socket_,
                                     (void *)recv_buffer_,
                                     MAX_FRAME_LEN,
                                     0,
                                     (sockaddr*)&remote_ip,
                                     &sz_addr);

    zce::sockaddr_ip old_remote;
    bool change = false;
    //
    deliver_recv(&remote_ip,
                 (RUDP_FRAME *)recv_buffer_,
                 (uint32_t)ssz_recv,
                 &change,
                 &old_remote);
    return 0;
}

int PEER::send_frame_to(int flag,
                        uint32_t sequence_num,
                        bool first_send,
                        const char *data,
                        size_t sz_data)
{
    int ret = 0;
    RUDP_FRAME *frame = (RUDP_FRAME *)send_buffer_;
    size_t sz_frame = sizeof(RUDP_HEAD) + sz_data;
    frame->session_id_ = session_id_;

    if (rudp_state_ != STATE::ESTABLISHED)
    {
    }
    frame->u32_1_.flag_ = flag;
    frame->clear();
    frame->u32_1_.len_ = sz_frame;
    if (sz_data > 0)
    {
        ::memcpy(send_buffer_ + sizeof(RUDP_HEAD), data, sz_data);
        ;
    }

    if ((flag & FLAG::PSH || flag & FLAG::SYN)
        && first_send == true)
    {
        SEND_RECORD snd_rec;
        snd_rec.len_ = sz_frame;
        if (sz_data > 0)
        {
            send_windows_.push_end(data, sz_data, snd_rec.buf_pos_);
        }
        frame->sequence_num_ = sequence_num;
        snd_rec.send_clock_ = zce::clock_ms();
        snd_rec.timeout_clock_ = snd_rec.send_clock_ + rto_;
        ++snd_rec.send_num_;
        send_rec_list_.push_back(snd_rec);
    }

    if (flag & FLAG::ACK)
    {
        frame->ack_id_ = rcv_expect_seq_num_;
    }
    frame->u32_1_.flag_ &= FLAG::ACK;
    frame->ack_id_ = rcv_expect_seq_num_;

    frame->u32_1_.mtu_type_ = (uint32_t)mtu_type_;
    frame->windows_size_ = recv_windows_.free();
    frame->session_id_ = session_id_;

    ret = zce::sendto(peer_socket_,
                      (void *)frame,
                      sz_frame,
                      0,
                      (sockaddr *)&remote_addr_,
                      sizeof(zce::sockaddr_ip));
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "");
        return ret;
    }

    return 0;
}

int PEER::acknowledge_send(uint32_t recv_ack_id)
{
    //收到了2次相同的ack id
    if (snd_seq_num_ack_ == recv_ack_id)
    {
        return 0;
    }

    size_t size_snd_rec = send_rec_list_.size();
    int32_t difference = 0;
    for (size_t i = 0; i < size_snd_rec; ++i)
    {
        //一直处理第一个
        SEND_RECORD &snd_rec = send_rec_list_[0];
        size_t snd_serial_id = snd_rec.sequence_num_;

        //这儿有一点技巧，避免 wraparound引发麻烦,其实就是利用了补码
        difference = (int32_t)(recv_ack_id - snd_serial_id);
        if (difference < 0)
        {
            //只对没有超时处理的发送计算rto
            if (snd_rec.send_num_ == 1)
            {
                calculate_rto(snd_rec.send_clock_);
            }
            //else 多次发送的在超时处理时计算
            send_windows_.pop_front(snd_rec.len_);
            send_rec_list_.pop_front();
            continue;
        }
        else
        {
            break;
        }
    }

    snd_seq_num_ack_ = recv_ack_id;

    if ((difference == 0 && send_rec_list_.size() > 0)
        || (difference > 0 && send_rec_list_.size() == 0 && recv_ack_id == snd_seq_num_counter_))
    {
        //正常，如果收到的ack id
    }
    else
    {
        //其实这儿存在错误
        ZCE_LOG(RS_ERROR, "");
    }
    return 0;
}

//计算rto
void PEER::calculate_rto(uint64_t send_clock)
{
    //α = 0.125，β = 0.25， μ = 1，∂ = 4
    const int mu = 1;
    const int dee = 4;
    const double alpha = 0.125;
    const double beta = 0.25;
    uint64_t now_clock = zce::clock_ms();
    //第一次计算
    if (rto_ == 0)
    {
        time_t r1 = now_clock - send_clock;
        rto_ = mu * r1 + dee * (r1 / 2);
    }
    else
    {
        time_t r2 = now_clock - send_clock;
        time_t srtt = (time_t)(rto_ + alpha * (r2 - rto_));
        time_t devrtt = (time_t)((1 - beta) * (rto_ / 2) + beta * abs(r2 - rto_));
        rto_ = mu * srtt + dee * devrtt;
    }
}

//=================================================================================================
//class CORE
CORE::~CORE()
{
    terminate();
}

int CORE::initialize(const sockaddr *core_addr,
                     size_t max_num_of_peer,
                     size_t peer_send_rec_list_size,
                     size_t peer_send_list_num,
                     size_t peer_recv_list_num)
{
    int ret = 0;
    socklen_t socket_len = 0;
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
    recv_buffer_ = new char[MAX_BUFFER_LEN];
    send_buffer_ = new char[MAX_BUFFER_LEN];

    std::random_device rd;
    random_gen_.seed(rd());
    peer_send_list_num_ = peer_send_list_num;
    peer_recv_list_num_ = peer_recv_list_num;
    peer_send_rec_list_size_ = peer_send_rec_list_size;
    return 0;
}

void CORE::terminate()
{
    if (recv_buffer_)
    {
        delete recv_buffer_;
        recv_buffer_ = nullptr;
    }
    if (send_buffer_)
    {
        delete send_buffer_;
        send_buffer_ = nullptr;
    }
    zce::close_socket(core_socket_);
}

int CORE::receive(PEER *& recv_rudp,
                  bool * new_rudp)
{
    assert(recv_rudp == nullptr);
    *new_rudp = false;
    int ret = 0;
    zce::sockaddr_ip remote_ip;
    socklen_t sz_addr = sizeof(zce::sockaddr_ip);
    ssize_t ssz_recv = zce::recvfrom(core_socket_,
                                     (void *)recv_buffer_,
                                     MAX_BUFFER_LEN,
                                     0,
                                     (sockaddr*)&remote_ip,
                                     &sz_addr);
    if (ssz_recv <= 0)
    {
        return -1;
    }
    else
    {
        //收到的数据长度不可能大于以太网的MSS
        if (ssz_recv > MSS_ETHERNET || ssz_recv < MIN_FRAME_LEN)
        {
            return -2;
        }
    }

    RUDP_FRAME * frame = (RUDP_FRAME *)recv_buffer_;
    frame->ntoh();
    if (frame->u32_1_.len_ != (uint32_t)ssz_recv)
    {
        return -2;
    }
    if (frame->u32_1_.flag_ & SYN)
    {
        if (frame->session_id_ == 0)
        {
            *new_rudp = true;
            ret = create_peer(&remote_ip, recv_rudp);
            if (ret != 0)
            {
                return ret;
            }
        }
        else
        {
            ZCE_LOG(RS_ERROR, "");
            return -1;
        }
    }
    else
    {
        auto iter = peer_map_.find(frame->session_id_);
        if (iter == peer_map_.end())
        {
            return -1;
        }
        recv_rudp = iter->second;
    }
    //看远端地址是否变化了,如果变化了，更新peer_addr_set_ 表
    zce::sockaddr_ip old_remote;
    bool change = false;
    //
    recv_rudp->deliver_recv(&remote_ip,
                            frame,
                            (uint32_t)ssz_recv,
                            &change,
                            &old_remote);
    if (change)
    {
        peer_addr_set_.erase(old_remote);
        peer_addr_set_.insert(std::make_pair(remote_ip,
                              recv_rudp->seesion_id()));
    }
    return 0;
}

int CORE::create_peer(const zce::sockaddr_ip *remote_ip,
                      PEER *& new_peer)
{
    int ret = 0;
    auto iter_addr = peer_addr_set_.find(*remote_ip);
    if (iter_addr != peer_addr_set_.end())
    {
        uint32_t session_id = iter_addr->second;
        auto iter_peer = peer_map_.find(session_id);
        if (iter_peer == peer_map_.end())
        {
            new_peer = iter_peer->second;
            new_peer->reset();
            //
            return 0;
        }
        else
        {
        }
    }
    new_peer = new PEER();
    uint32_t session_id = random_gen_();
    uint32_t serial_id = random_gen_();
    ret = new_peer->open(session_id,
                         serial_id,
                         core_socket_,
                         (sockaddr *)remote_ip,
                         send_buffer_,
                         peer_send_rec_list_size_,
                         peer_send_list_num_,
                         peer_recv_list_num_);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

//=================================================================================================
}