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
    sack_[0] = htonl(sack_[0]);
    sack_[1] = htonl(sack_[1]);
    sack_[2] = htonl(sack_[2]);
}

//将所有的uint16_t,uint32_t转换为本地序
void RUDP_HEAD::ntoh()
{
    u32_1_copy_ = ntohl(u32_1_copy_);
    session_id_ = ntohl(session_id_);
    sequence_num_ = ntohl(sequence_num_);
    ack_id_ = ntohl(ack_id_);
    windows_size_ = ntohl(windows_size_);
    sack_[0] = ntohl(sack_[0]);
    sack_[1] = ntohl(sack_[1]);
    sack_[2] = ntohl(sack_[2]);
}

void RUDP_HEAD::clear()
{
    u32_1_copy_ = 0;
    session_id_ = 0;
    sequence_num_ = 0;
    ack_id_ = 0;
    windows_size_ = 0;
    sack_[0] = 0;
    sack_[1] = 0;
    sack_[2] = 0;
}

//填充Data数据到Frame
int RUDP_FRAME::fill_data(const size_t szdata, const char* vardata)
{
    //填写数据区的长度
    ::memcpy(data_, vardata, szdata);
    u32_1_.len_ = static_cast<uint32_t>(sizeof(RUDP_HEAD) + szdata);
    return 0;
}

RUDP_FRAME *RUDP_FRAME::new_frame(size_t frame_len)
{
    assert(frame_len > sizeof(RUDP_HEAD));
    return (RUDP_FRAME *)new char[frame_len];
}

///删除回收一个new的frame
void RUDP_FRAME::delete_frame(RUDP_FRAME *frame)
{
    delete[](char *)frame;
}

//=================================================================================================
//class BASE
std::mt19937 BASE::random_gen_(19190504 + (uint32_t)::time(NULL));

uint32_t BASE::random()
{
    return random_gen_();
}

//=================================================================================================
//class PEER
double PEER::blocking_rto_ratio_ = 1.5;

time_t PEER::min_rto_ = 80;
//10分钟
time_t PEER::noalive_time_to_close_ = 600000;

//服务器端CORE打开一个PEER
int PEER::open(CORE *core,
               uint32_t session_id,
               uint32_t sequence_num,
               ZCE_SOCKET peer_socket,
               const sockaddr *remote_addr,
               char *send_buffer,
               size_t send_rec_list_size,
               size_t send_wnd_size,
               size_t recv_wnd_size,
               std::function<ssize_t(uint32_t, PEER *)> &callbak_recv)
{
    bool bret = false;
    model_ = MODEL::PEER_CORE_CREATE;
    core_ = core;
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
    bret = send_rec_list_.initialize(send_rec_list_size);
    if (!bret)
    {
        return -2;
    }
    bret = send_windows_.initialize(send_wnd_size);
    if (!bret)
    {
        return -3;
    }
    bret = recv_windows_.initialize(recv_wnd_size);
    if (!bret)
    {
        return -4;
    }
    selective_ack_num_ = 0;
    selective_ack_ary_[0] = RUDP_FRAME::new_frame(MAX_BUFFER_LEN);
    selective_ack_ary_[1] = RUDP_FRAME::new_frame(MAX_BUFFER_LEN);
    selective_ack_ary_[2] = RUDP_FRAME::new_frame(MAX_BUFFER_LEN);
    //peer_live_clock_服务端打开的 PEER是收到数据创建的，所以后面会更新peer_live_clock_

    callbak_recv_ = callbak_recv;

    return 0;
}

//客户端打开一个
int PEER::open(const sockaddr *remote_addr,
               size_t send_rec_list_size,
               size_t send_windows_capacity,
               size_t recv_windows_capacity,
               std::function<ssize_t(uint32_t, PEER *)> &callbak_recv,
               bool link_test_mtu)
{
    int ret = 0;
    model_ = MODEL::PEER_CLIENT;
    my_seq_num_counter_ = BASE::random();

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

    bool bret = send_rec_list_.initialize(send_rec_list_size);
    if (!bret)
    {
        return -2;
    }
    bret = send_windows_.initialize(send_windows_capacity);
    if (!bret)
    {
        return -3;
    }
    bret = recv_windows_.initialize(recv_windows_capacity);
    if (!bret)
    {
        return -4;
    }
    recv_buffer_ = new char[MAX_BUFFER_LEN];
    send_buffer_ = new char[MAX_BUFFER_LEN];
    callbak_recv_ = callbak_recv;

    if (link_test_mtu)
    {
        send_frame_to(FLAG::SYN | FLAG::LMT);
    }
    else
    {
        send_frame_to(FLAG::SYN);
    }
    selective_ack_num_ = 0;
    selective_ack_ary_[0] = RUDP_FRAME::new_frame(MAX_BUFFER_LEN);
    selective_ack_ary_[1] = RUDP_FRAME::new_frame(MAX_BUFFER_LEN);
    selective_ack_ary_[2] = RUDP_FRAME::new_frame(MAX_BUFFER_LEN);
    peer_live_clock_ = zce::clock_ms();
    return 0;
}

void PEER::close()
{
    established_ = false;
    selective_ack_num_ = 0;
    RUDP_FRAME::delete_frame(selective_ack_ary_[0]);
    RUDP_FRAME::delete_frame(selective_ack_ary_[1]);
    RUDP_FRAME::delete_frame(selective_ack_ary_[2]);
    selective_ack_ary_[0] = nullptr;
    selective_ack_ary_[1] = nullptr;
    selective_ack_ary_[2] = nullptr;
    if (model_ == MODEL::PEER_CLIENT)
    {
        zce::close_socket(peer_socket_);

        delete[] recv_buffer_;
        recv_buffer_ = nullptr;
        delete[] send_buffer_;
        send_buffer_ = nullptr;
    }
    else if (model_ == MODEL::PEER_CORE_CREATE)
    {
        core_->delete_peer(this);
    }
    else
    {
    }
    return;
}

int PEER::reset()
{
    established_ = false;
    selective_ack_num_ = 0;
    if (model_ == MODEL::PEER_CLIENT)
    {
        send_rec_list_.clear();
        send_windows_.clear();
        recv_windows_.clear();
        session_id_ = 0;
        my_seq_num_counter_ = BASE::random();
        my_seq_num_ack_ = 0;
        peer_expect_seq_num_ = 0;
        send_frame_to(FLAG::SYN);
    }
    else if (model_ == MODEL::PEER_CORE_CREATE)
    {
        core_->delete_peer(this);
    }
    return 0;
}

//给外部调用的接收接口
int PEER::outer_recv(char* buf,
                     size_t &len)
{
    if (len > recv_windows_.size())
    {
        len = recv_windows_.size();
    }
    recv_windows_.pop_front(buf, len);
    //窗口变化了通知对方
    send_frame_to(FLAG::ACK);
    return 0;
}

//给外部调用的发送接口
int PEER::outer_send(const char* buf,
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
    else
    {
        assert(false);
    }
    //发送数据太长，臣妾搞不掂呀
    size_t snd_rec_free = send_rec_list_.free();
    size_t snd_wnd_free = send_windows_.free();

    const size_t buf_size = 64;
    char remote_str[buf_size];
    ZCE_LOG_DEBUG(RS_DEBUG,
                  "[RUDP] outer_send model[%d] session[%u] remote [%s] mtu[%d] frame_max_len[%u] "
                  "send windows free [%u] send record [%u] my sn counter[%u] my ack sn[%u] peer ack [%u] ",
                  model_,
                  session_id_,
                  zce::get_host_addr_port((sockaddr *)&remote_addr_, remote_str, buf_size),
                  mtu_type_,
                  frame_max_len,
                  snd_wnd_free,
                  snd_rec_free,
                  my_seq_num_counter_,
                  my_seq_num_ack_,
                  peer_expect_seq_num_);

    if (snd_wnd_free == 0 || snd_rec_free == 0)
    {
        zce::last_error(EWOULDBLOCK);
        ZCE_LOG(RS_INFO, "[RUDP] snd_rec_free [%u] snd_wnd_free[%u]",
                snd_rec_free,
                snd_wnd_free);
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
                            true,
                            buf + process_len,
                            one_process);
        if (ret != 0)
        {
            break;
        }
        remain_len -= one_process;
        snd_wnd_free -= one_process;
        peer_windows_size_ -= one_process;
        --snd_rec_free;
        process_len += one_process;

        ZCE_LOG_DEBUG(RS_DEBUG,
                      "[RUDP] remain_len [%u] one_process[%u] peer_windows_size_[%u]"
                      "frame_max_len [%u] snd_wnd_free[%u] snd_rec_free[%u]",
                      remain_len,
                      one_process,
                      peer_windows_size_,
                      frame_max_len,
                      snd_wnd_free,
                      snd_rec_free);
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
    const size_t buf_size = 64;
    char new_remote_str[buf_size], old_remote_str[buf_size];
    ZCE_LOG_DEBUG(RS_DEBUG,
                  "[RUDP] deliver_recv model[%u] session[%u] remote new[%s] old [%s] "
                  "my sn counter[%u] my ack sn[%u] peer ack [%u] "
                  "recv frame len[%u] flag[%u] session[%u] sn[%u] ack[%u] "
                  "recv windows size[%u] free[%u] ",
                  model_,
                  session_id_,
                  zce::get_host_addr_port((sockaddr *)remote_addr, new_remote_str, buf_size),
                  zce::get_host_addr_port((sockaddr *)&remote_addr_, old_remote_str, buf_size),
                  my_seq_num_counter_,
                  my_seq_num_ack_,
                  peer_expect_seq_num_,
                  recv_frame->u32_1_.len_,
                  recv_frame->u32_1_.flag_,
                  recv_frame->session_id_,
                  recv_frame->sequence_num_,
                  recv_frame->ack_id_,
                  recv_windows_.size(),
                  recv_windows_.free());
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
    if ((model_ == MODEL::PEER_CORE_CREATE) && new_remote != remote_addr_)
    {
        *remote_change = true;
        *old_remote = remote_addr_;
        remote_addr_ = new_remote;
        ZCE_LOG(RS_INFO, "[RUDP]Remote address [%s] already modify [%s].Please pay attention to.",
                zce::get_host_addr_port((sockaddr *)&remote_addr_, old_remote_str, buf_size),
                zce::get_host_addr_port((sockaddr *)remote_addr, new_remote_str, buf_size));
    }
    peer_windows_size_ = recv_frame->windows_size_;
    mtu_type_ = static_cast<MTU_TYPE>(recv_frame->u32_1_.mtu_type_);
    uint32_t flag = recv_frame->u32_1_.flag_;
    //记录服务器分配的session id
    if (model_ == MODEL::PEER_CLIENT && session_id_ == 0)
    {
        session_id_ = recv_frame->session_id_;
    }
    else
    {
        if (session_id_ != recv_frame->session_id_ &&
            (flag & FLAG::SYN) == 0)
        {
            ZCE_LOG(RS_ERROR, "Session id not eaqul, receive session id[%u] init session[%u].",
                    recv_frame->session_id_,
                    session_id_);
            return -1;
        }
    }

    if (flag & FLAG::RST)
    {
        return reset();
    }

    //对方有数据
    bool already_processed = false, advance_arrive = false;
    if (flag & FLAG::PSH)
    {
        //如果连接没有建立，
        if (!established_)
        {
            ZCE_LOG_DEBUG(RS_DEBUG, "[RUDP]link session[%u] not established, but receive push data,"
                          "discard it. peer_expect_seq_num_[%u] recv sn[%u].",
                          session_id_,
                          peer_expect_seq_num_,
                          recv_frame->sequence_num_);
            return -1;
        }
        process_push_data(recv_frame,
                          &already_processed,
                          &advance_arrive);
        //如果是跳跃数据。
        if (advance_arrive)
        {
            record_selective(recv_frame);
        }
    }
    else if (flag & FLAG::SYN)
    {
        //记录对方的sequence number，期待下一个的sn = 当前的sn +1
        peer_expect_seq_num_ = recv_frame->sequence_num_ + 1;
        if (flag & FLAG::ACK && model_ == MODEL::PEER_CLIENT)
        {
            send_frame_to(FLAG::ACK);
        }
        else if (model_ == MODEL::PEER_CORE_CREATE)
        {
            send_frame_to(FLAG::ACK & FLAG::SYN);
        }
        else
        {
            ZCE_ASSERT(false);
        }
        established_ = true;
    }
    //心跳情况下，什么都不做，就是保证有应答
    else if (flag & FLAG::KPL)
    {
        send_frame_to(FLAG::ACK);
    }
    uint64_t now_clock = zce::clock_ms();
    //有ACK 标志处理,（而且没有处理过）
    if ((flag & FLAG::ACK) && already_processed == false)
    {
        ret = acknowledge_send(recv_frame,
                               now_clock);
        if (ret != 0)
        {
            return ret;
        }
    }
    peer_live_clock_ = now_clock;
    return 0;
}

///
bool PEER::process_push_data(RUDP_FRAME *recv_frame,
                             bool *already_processed,
                             bool *advance_arrive)
{
    *already_processed = false;
    *advance_arrive = false;
    size_t data_len = recv_frame->u32_1_.len_ - sizeof(RUDP_HEAD);
    if (recv_windows_.free() < data_len)
    {
        ZCE_LOG_DEBUG(RS_DEBUG, "[RUDP]link session[%u] recv frame len[%u] data len[%u],"
                      "recv windows size [%u] free[%u],discard it. .",
                      session_id_,
                      recv_frame->u32_1_.len_,
                      data_len,
                      recv_windows_.size(),
                      recv_windows_.free());
        return false;
    }
    else
    {
        uint32_t recv_seq_num = recv_frame->sequence_num_;
        int32_t difference = (int32_t)(recv_seq_num - peer_expect_seq_num_);
        if (difference < 0)
        {
            *already_processed = true;
            return false;
        }
        //发生了跳跃,
        else if (difference > 0)
        {
            *advance_arrive = true;
            return false;
        }
        //和期待的seq num 一致，
        else
        {
            char *wt_ptr = nullptr;
            recv_windows_.push_end(recv_frame->data_, data_len, wt_ptr);
            peer_expect_seq_num_ = peer_expect_seq_num_ + static_cast<uint32_t>(data_len);
            send_frame_to(FLAG::ACK);
            callbak_recv_(session_id_, this);
            return true;
        }
    }
    //return false;
}

void PEER::proces_selective()
{
    size_t process_num = 0;
    for (size_t i = 0; i < selective_ack_num_; ++i)
    {
        bool already_processed = false, advance_arrive = false;
        RUDP_FRAME *selective_frame = selective_ack_ary_[i];
        bool processed = process_push_data(selective_frame,
                                           &already_processed,
                                           &advance_arrive);
        if (processed)
        {
            ++process_num;
        }
        if (advance_arrive)
        {
            break;
        }
    }
    selective_ack_num_ -= process_num;
}

//记录收到的跳跃数据。代码丑的一比
void PEER::record_selective(RUDP_FRAME *selective_frame)
{
    uint32_t s_s_num = selective_frame->sequence_num_;
    if (selective_ack_num_ == 0)
    {
        ::memcpy(selective_ack_ary_[0], selective_frame, selective_frame->u32_1_.len_);
        ++selective_ack_num_;
    }
    uint32_t s_a_a0_num = selective_ack_ary_[0]->sequence_num_;
    if (selective_ack_num_ == 1)
    {
        if (s_s_num == s_a_a0_num)
        {
            return;
        }
        if (int32_t(s_s_num - s_a_a0_num) > 0)
        {
            ::memcpy(selective_ack_ary_[1], selective_frame, selective_frame->u32_1_.len_);
            ++selective_ack_num_;
        }
        else
        {
            RUDP_FRAME *temp = selective_ack_ary_[1];
            selective_ack_ary_[1] = selective_ack_ary_[0];
            selective_ack_ary_[0] = temp;
            ::memcpy(selective_ack_ary_[0], selective_frame, selective_frame->u32_1_.len_);
            ++selective_ack_num_;
        }
    }
    uint32_t s_a_a1_num = selective_ack_ary_[1]->sequence_num_;
    if (selective_ack_num_ == 2)
    {
        if (s_s_num == selective_ack_ary_[0]->sequence_num_ ||
            s_s_num == selective_ack_ary_[1]->sequence_num_)
        {
            return;
        }
        if (int32_t(s_s_num - s_a_a1_num) > 0)
        {
            ::memcpy(selective_ack_ary_[2], selective_frame, selective_frame->u32_1_.len_);
            ++selective_ack_num_;
        }
        else if (int32_t(s_s_num - s_a_a0_num))
        {
            RUDP_FRAME *temp = selective_ack_ary_[2];
            selective_ack_ary_[2] = selective_ack_ary_[1];
            selective_ack_ary_[1] = temp;
            memcpy(selective_ack_ary_[0], selective_frame, selective_frame->u32_1_.len_);
            ++selective_ack_num_;
        }
        else
        {
            RUDP_FRAME *temp = selective_ack_ary_[2];
            selective_ack_ary_[2] = selective_ack_ary_[1];
            selective_ack_ary_[1] = selective_ack_ary_[0];
            selective_ack_ary_[0] = temp;
            ::memcpy(selective_ack_ary_[0], selective_frame, selective_frame->u32_1_.len_);
            ++selective_ack_num_;
        }
    }
    uint32_t s_a_a2_num = selective_ack_ary_[1]->sequence_num_;
    if (selective_ack_num_ == 3)
    {
        if (s_s_num == selective_ack_ary_[0]->sequence_num_ ||
            s_s_num == selective_ack_ary_[1]->sequence_num_ ||
            s_s_num == selective_ack_ary_[2]->sequence_num_)
        {
            return;
        }
        if (int32_t(s_s_num - s_a_a2_num) > 0)
        {
            //很无奈，空间保存不了
            return;
        }
        else if (int32_t(s_s_num - s_a_a1_num) > 0)
        {
            ::memcpy(selective_ack_ary_[2], selective_frame, selective_frame->u32_1_.len_);
        }
        else if (int32_t(s_s_num - s_a_a0_num) > 0)
        {
            RUDP_FRAME *temp = selective_ack_ary_[2];
            selective_ack_ary_[2] = selective_ack_ary_[1];
            selective_ack_ary_[1] = temp;
            ::memcpy(selective_ack_ary_[1], selective_frame, selective_frame->u32_1_.len_);
        }
        else
        {
            RUDP_FRAME *temp = selective_ack_ary_[2];
            selective_ack_ary_[2] = selective_ack_ary_[1];
            selective_ack_ary_[1] = selective_ack_ary_[0];
            selective_ack_ary_[0] = temp;
            ::memcpy(selective_ack_ary_[0], selective_frame, selective_frame->u32_1_.len_);
        }
    }
}

//客户端调用的接收函数
ssize_t PEER::recv()
{
    assert(model_ == MODEL::PEER_CLIENT);
    if (model_ != MODEL::PEER_CLIENT)
    {
        return -1;
    }
    const size_t buf_size = 64;
    char out_buf[buf_size];
    zce::sockaddr_ip remote_ip;
    socklen_t sz_addr = sizeof(zce::sockaddr_ip);
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
            return 0;
        }
        else
        {
            ZCE_LOG(RS_ERROR,
                    "[RUDP]zce::recvfrom return error ret = [%d] remote[%s] errno=[%d]",
                    ssz_recv,
                    zce::get_host_addr_port((sockaddr *)&remote_addr_, out_buf, buf_size),
                    zce::last_error());
            return -1;
        }
    }
    else
    {
        //收到的数据长度不可能大于以太网的MSS
        if (ssz_recv > MAX_FRAME_LEN || ssz_recv < MIN_FRAME_LEN)
        {
            ZCE_LOG(RS_ERROR,
                    "[RUDP]zce::recvfrom ssz_recv [%u] session id [%u] remote[%s]",
                    ssz_recv,
                    session_id_,
                    zce::get_host_addr_port((sockaddr *)&remote_addr_, out_buf, buf_size));
            return -2;
        }
    }
    zce::sockaddr_ip old_remote;
    bool change = false;
    RUDP_FRAME *recv_frame = (RUDP_FRAME *)recv_buffer_;
    recv_frame->ntoh();
    //
    int ret = deliver_recv(&remote_ip,
                           recv_frame,
                           (uint32_t)ssz_recv,
                           &change,
                           &old_remote);
    if (ret != 0)
    {
        return -1;
    }
    return ssz_recv;
}

ssize_t PEER::recv_timeout(zce::Time_Value* timeout_tv)
{
    int ret = 0;
    //进行超时处理
    ret = zce::handle_ready(peer_socket_,
                            timeout_tv,
                            zce::HANDLE_READY::READ);

    const int HANDLE_READY_ONE = 1;
    if (ret != HANDLE_READY_ONE)
    {
        return -1;
    }
    // 尽量读取
    ssize_t recv_couter = 0;
    const size_t ONCE_PROCESS_TIMES = 64;
    for (size_t k = 0; k < ONCE_PROCESS_TIMES; ++k)
    {
        ssize_t recv_len = recv();
        if (recv_len < 0)
        {
            return -1;
        }
        else if (recv_len == 0)
        {
            break;
        }
        else
        {
            recv_couter += recv_len;
        }
    }
    return recv_couter;
}

int PEER::send_frame_to(int flag,
                        bool first_send,
                        const char *data,
                        size_t sz_data,
                        SEND_RECORD *old_snd_rec)
{
    RUDP_FRAME *frame = (RUDP_FRAME *)send_buffer_;
    frame->clear();
    size_t sz_frame = sizeof(RUDP_HEAD) + sz_data;
    //如果PSH，就带上ACK
    if (flag & FLAG::PSH)
    {
        flag &= FLAG::ACK;
    }
    frame->u32_1_.flag_ = flag;
    //如果进行链路探测，填写最大的帧长进行发送，
    if (flag & FLAG::SYN && flag & FLAG::LMT)
    {
        sz_frame = MAX_FRAME_LEN;
        ::memset(send_buffer_ + sizeof(RUDP_HEAD),
                 0,
                 MAX_FRAME_LEN - sizeof(RUDP_HEAD));
    }
    frame->u32_1_.len_ = sz_frame;
    frame->session_id_ = session_id_;

    if (sz_data > 0)
    {
        ::memcpy(send_buffer_ + sizeof(RUDP_HEAD), data, sz_data);
    }
    if (flag & FLAG::ACK)
    {
        frame->ack_id_ = peer_expect_seq_num_;
        frame->u32_1_.selective_num_ = (uint32_t)selective_ack_num_;
        for (size_t l = 0; l < selective_ack_num_; ++l)
        {
            frame->sack_[l] = selective_ack_ary_[l]->sequence_num_;
        }
    }
    frame->u32_1_.mtu_type_ = (uint32_t)mtu_type_;
    frame->windows_size_ = static_cast<uint32_t>(recv_windows_.free());
    frame->session_id_ = session_id_;

    uint64_t now_clock = zce::clock_ms();
    //这几种情况的发送需要ACK确认,会填写sequence_num_
    if ((flag & FLAG::PSH) || (flag & FLAG::SYN))
    {
        if (first_send == true)
        {
            SEND_RECORD first_snd_rec;
            first_snd_rec.flag_ = flag;
            first_snd_rec.len_ = sz_data;
            frame->sequence_num_ = my_seq_num_counter_;
            first_snd_rec.sequence_num_ = my_seq_num_counter_;
            if (sz_data > 0)
            {
                send_windows_.push_end(data, sz_data, first_snd_rec.buf_pos_);
                my_seq_num_counter_ += static_cast<uint32_t>(sz_data);
            }
            else
            {
                ++my_seq_num_counter_;
            }

            first_snd_rec.send_clock_ = now_clock;
            first_snd_rec.timeout_clock_ = first_snd_rec.send_clock_ +
                (rto_ > min_rto_ ? rto_ : min_rto_);
            ++first_snd_rec.send_num_;
            send_rec_list_.push_back(first_snd_rec);
        }
        else
        {
            frame->sequence_num_ = old_snd_rec->sequence_num_;
            if (old_snd_rec->len_ > 0)
            {
                send_windows_.get_data(old_snd_rec->buf_pos_,
                                       send_buffer_ + sizeof(RUDP_HEAD),
                                       old_snd_rec->len_);
            }
            old_snd_rec->timeout_clock_ = now_clock +
                (rto_ > min_rto_ ? rto_ : min_rto_);
            ++old_snd_rec->send_num_;
        }
    }

    const size_t buf_size = 64;
    char remote_str[buf_size];
    ZCE_LOG_DEBUG(RS_DEBUG,
                  "[RUDP] send_frame_to model[%u] session[%u] remote [%s] "
                  "my sn counter[%u] my ack sn[%u] peer ack [%u] "
                  "send frame len[%u] flag[%u] sn[%u] ack[%u]",
                  model_,
                  session_id_,
                  zce::get_host_addr_port((sockaddr *)&remote_addr_,
                  remote_str, buf_size),
                  my_seq_num_counter_,
                  my_seq_num_ack_,
                  peer_expect_seq_num_,
                  frame->u32_1_.len_,
                  frame->u32_1_.flag_,
                  frame->sequence_num_,
                  frame->ack_id_);
    frame->hton();
    ssize_t ssend = zce::sendto(peer_socket_,
                                (void *)frame,
                                sz_frame,
                                0,
                                (sockaddr *)&remote_addr_,
                                sizeof(zce::sockaddr_ip));
    if (ssend != (ssize_t)sz_frame)
    {
        char buf[buf_size];
        ZCE_LOG(RS_ERROR,
                "zce::sendto return error ret = [%d] frame len[%u] remote[%s]",
                ssend,
                sz_frame,
                zce::get_host_addr_port(
                (sockaddr *)&remote_addr_, buf, buf_size));
        return -1;
    }

    return 0;
}

int PEER::acknowledge_send(RUDP_FRAME *recv_frame,
                           uint64_t now_clock)
{
    //收到了2次相同的ack id
    uint32_t recv_ack_id = recv_frame->ack_id_;
    if (my_seq_num_ack_ == recv_ack_id)
    {
        return 0;
    }

    size_t size_snd_rec = send_rec_list_.size();
    int32_t difference = 0;
    for (size_t i = 0; i < size_snd_rec; ++i)
    {
        //一直处理第一个
        SEND_RECORD &snd_rec = send_rec_list_[0];
        uint32_t snd_serial_id = snd_rec.sequence_num_;

        //这儿有一点技巧，避免 wrap around引发麻烦,其实就是利用了补码
        difference = (int32_t)(recv_ack_id - snd_serial_id);
        if (difference < 0)
        {
            //只对没有超时处理的发送计算rto
            if (snd_rec.send_num_ == 1)
            {
                calculate_rto(snd_rec.send_clock_,
                              now_clock);
            }

            send_windows_.pop_front(snd_rec.len_);
            send_rec_list_.pop_front();
            continue;
        }
        else
        {
            break;
        }
    }

    my_seq_num_ack_ = recv_frame->ack_id_;

    //
    if ((difference == 0 && send_rec_list_.size() > 0)
        || (difference > 0 && send_rec_list_.size() == 0
        && recv_ack_id == my_seq_num_counter_))
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
void PEER::calculate_rto(uint64_t send_clock,
                         uint64_t now_clock)
{
    //α = 0.125，β = 0.25， μ = 1，∂ = 4
    const int mu = 1;
    const int dee = 4;
    const double alpha = 0.125;
    const double beta = 0.25;
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

//超时处理
void PEER::time_out(uint64_t now_clock_ms)
{
    int ret = 0;
    size_t size_snd_rec = send_rec_list_.size();
    for (size_t i = 0; i < size_snd_rec; ++i)
    {
        SEND_RECORD &snd_rec = send_rec_list_[i];
        //有超时发生
        if (snd_rec.timeout_clock_ <= now_clock_ms)
        {
            if (snd_rec.flag_)
            {
            }

            rto_ = (time_t)blocking_rto_ratio_ * rto_;
            //重发
            ret = send_frame_to(snd_rec.flag_,
                                false,
                                nullptr,
                                0,
                                &snd_rec);
            if (ret != 0)
            {
                break;
            }
        }
    }
    //如果长时间没有反应。
    if (now_clock_ms - peer_live_clock_ > (uint64_t)noalive_time_to_close_)
    {
        close();
    }
}

//=================================================================================================
//class CORE

CORE::~CORE()
{
    close();
}

int CORE::open(const sockaddr *core_addr,
               size_t max_num_of_peer,
               size_t peer_send_rec_list_size,
               size_t peer_send_list_num,
               size_t peer_recv_list_num,
               std::function<ssize_t(uint32_t, PEER *)> &peer_callbak_recv)
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

    peer_send_wnd_size_ = peer_send_list_num;
    peer_recv_wnd_size_ = peer_recv_list_num;
    peer_send_rec_list_size_ = peer_send_rec_list_size;
    peer_callbak_recv_ = peer_callbak_recv;
    return 0;
}

void CORE::close()
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

ssize_t CORE::recv(PEER *& recv_rudp,
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
        if (ssz_recv > MAX_FRAME_LEN || ssz_recv < MIN_FRAME_LEN)
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
            if (peer_map_.size() > max_num_of_peer_)
            {
                ZCE_LOG(RS_ERROR, "[RUDP] peer_map_ size[%u] > "
                        " max_num_of_peer_[%u]",
                        peer_map_.size(),
                        max_num_of_peer_);
                return -1;
            }
            *new_rudp = true;
            ret = create_peer(&remote_ip, recv_rudp);
            if (ret != 0)
            {
                return ret;
            }
        }
        else
        {
            ZCE_LOG_DEBUG(RS_ERROR, "[RUDP] SYN error, flag [%u] but "
                          "session id!=0,[%u]",
                          frame->u32_1_.flag_,
                          frame->session_id_);
            return -1;
        }
    }
    else
    {
        auto iter = peer_map_.find(frame->session_id_);
        if (iter == peer_map_.end())
        {
            ZCE_LOG_DEBUG(RS_ERROR, "[RUDP] ");
            return -1;
        }
        recv_rudp = iter->second;
    }
    //看远端地址是否变化了,如果变化了，更新peer_addr_set_ 表
    zce::sockaddr_ip old_remote;
    bool change = false;
    //将接收的数据调教给PEER
    recv_rudp->deliver_recv(&remote_ip,
                            frame,
                            (uint32_t)ssz_recv,
                            &change,
                            &old_remote);
    if (change)
    {
        peer_addr_set_.erase(old_remote);
        peer_addr_set_.insert(std::make_pair(remote_ip,
                              recv_rudp->session_id_));
    }
    return ssz_recv;
}

ssize_t CORE::recv_timeout(zce::Time_Value* timeout_tv)
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

    ssize_t recv_couter = 0;
    const size_t ONCE_PROCESS_TIMES = 256;
    for (size_t k = 0; k < ONCE_PROCESS_TIMES; ++k)
    {
        PEER *recv_rudp = nullptr;
        bool new_rudp = false;
        ssize_t recv_len = recv(recv_rudp, &new_rudp);
        if (recv_len < 0)
        {
            return -1;
        }
        else if (recv_len == 0)
        {
            break;
        }
        else
        {
            recv_couter += recv_len;
        }
    }
    return recv_couter;
}

int CORE::create_peer(const zce::sockaddr_ip *remote_ip,
                      PEER *& new_peer)
{
    int ret = 0;
    //用这个IP找找，如果地址库里有，就找出对应那个关闭
    auto iter_addr = peer_addr_set_.find(*remote_ip);
    if (iter_addr != peer_addr_set_.end())
    {
        uint32_t session_id = iter_addr->second;
        auto iter_peer = peer_map_.find(session_id);
        if (iter_peer != peer_map_.end())
        {
            PEER * old_peer = iter_peer->second;
            delete_peer(old_peer);
        }
        else
        {
            ZCE_LOG(RS_ERROR, "");
        }
    }
    new_peer = new PEER();
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
                         peer_send_rec_list_size_,
                         peer_send_wnd_size_,
                         peer_recv_wnd_size_,
                         peer_callbak_recv_);
    if (ret != 0)
    {
        return -1;
    }
    return 0;
}

//删除对应的PEER
void CORE::delete_peer(PEER * del_peer)
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
void CORE::time_out()
{
    uint64_t now_clock_ms = zce::clock_ms();
    //这儿我抉择过一段时间。到底要不要用一些其他unorder_map的遍历
    for (auto iter : peer_map_)
    {
        iter.second->time_out(now_clock_ms);
    }
    return;
}

//=================================================================================================
}