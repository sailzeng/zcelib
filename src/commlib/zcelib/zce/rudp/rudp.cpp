#include "zce/predefine.h"
#include "zce/rudp/rudp.h"
#include "zce/os_adapt/socket.h"

#define RUDP_CHECK 1

//ZNO_LOG 很多日志用 ZNO_LOG关闭了。你可以自己改为 ZCE_LOG 打开

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
    una_[0] = htonl(una_[0]);
    una_[1] = htonl(una_[1]);
    una_[2] = htonl(una_[2]);
}

//将所有的uint16_t,uint32_t转换为本地序
void RUDP_HEAD::ntoh()
{
    u32_1_copy_ = ntohl(u32_1_copy_);
    session_id_ = ntohl(session_id_);
    sequence_num_ = ntohl(sequence_num_);
    ack_id_ = ntohl(ack_id_);
    windows_size_ = ntohl(windows_size_);
    una_[0] = ntohl(una_[0]);
    una_[1] = ntohl(una_[1]);
    una_[2] = ntohl(una_[2]);
}

void RUDP_HEAD::clear()
{
    u32_1_copy_ = 0;
    session_id_ = 0;
    sequence_num_ = 0;
    ack_id_ = 0;
    windows_size_ = 0;
    una_[0] = 0;
    una_[1] = 0;
    una_[2] = 0;
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

//=================================================================================================
//class BASE
std::mt19937 BASE::random_gen_(19190504 + (uint32_t)::time(NULL));

uint32_t BASE::random()
{
    return random_gen_();
}
//!
double BASE::blocking_rto_ratio_ = 1.5;
//!
time_t BASE::min_rto_ = 80;
//10分钟
time_t BASE::noalive_time_to_close_ = 600000;

void BASE::min_rto(time_t rto)
{
    min_rto_ = rto;
}

void BASE::blocking_rto_ratio(double rto_ratio)
{
    blocking_rto_ratio_ = rto_ratio;
}

void BASE::noalive_time_to_close(time_t to_close_time)
{
    noalive_time_to_close_ = to_close_time;
}

//=================================================================================================
//class PEER

void PEER::close()
{
    established_ = false;
    return;
}

void PEER::reset()
{
    established_ = false;
    return;
}

//给外部调用的接收接口
int PEER::recv(char* buf,
               size_t *recv_len)
{
    uint32_t can_read = rcv_wnd_series_end_ - rcv_wnd_first_;
    size_t read_len = *recv_len;
    *recv_len = 0;
    if (read_len > can_read)
    {
        read_len = can_read;
    }
    const size_t NOTIFY_PEER_WINDOWS_THRESHOLD = MSS_ETHERNET_RUDP;
    bool send_iao = false;
    if (recv_windows_.free() <= NOTIFY_PEER_WINDOWS_THRESHOLD)
    {
        send_iao = true;
    }
    recv_windows_.pop_front(buf, read_len);
    *recv_len = read_len;
    //窗口变化了通知对方
    if (send_iao)
    {
        send_frame_to(FLAG::IAO);
    }
    rcv_wnd_first_ += can_read;
    return 0;
}

//给外部调用的发送接口
int PEER::send(const char* buf,
               size_t *send_len)
{
    int ret = 0;
    size_t remain_len = *send_len;
    *send_len = 0;
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
    ZNO_LOG(RS_DEBUG,
            "[RUDP] send start. model[%d] session[%u] remote [%s] mtu[%d] frame_max_len[%u] "
            "peer_windows_size_[%u] send windows [%u|%u] send record [%u|%u]"
            "send wnd[%u|%u] send rec [%u]  recv wnd[%u|%u|%u] ",
            model_,
            session_id_,
            zce::get_host_addr_port((sockaddr *)&remote_addr_, remote_str, buf_size),
            mtu_type_,
            frame_max_len,
            peer_windows_size_,
            send_windows_.size(),
            send_windows_.free(),
            send_rec_list_.size(),
            send_rec_list_.free(),
            my_seq_num_counter_,
            my_seq_num_ack_,
            rcv_wnd_first_,
            rcv_wnd_series_end_,
            rcv_wnd_last_);

    if (snd_wnd_free == 0 || snd_rec_free == 0 || peer_windows_size_ == 0)
    {
        zce::last_error(EWOULDBLOCK);
        ZCE_LOG(RS_ERROR, "[RUDP] send fail. [%u] room not enough. "
                " snd_rec_free [%u] snd_wnd_free[%u] peer_windows_size_ [%u]",
                session_id_,
                snd_rec_free,
                snd_wnd_free,
                peer_windows_size_);
        //ZWP Zero Window Probe
        if (peer_windows_size_ == 0)
        {
            send_frame_to(FLAG::AYO);
        }
        return -1;
    }

    size_t one_process = 0, process_len = 0, z = 0;
    while (remain_len > 0 && snd_rec_free > 0 &&
           snd_wnd_free > 0 && peer_windows_size_ > 0 &&
           z < congestion_window_)
    {
        //得到这一次最大能处理的数据大小
        one_process = remain_len > frame_max_len ?
            frame_max_len : remain_len;
        one_process = one_process > snd_wnd_free ?
            snd_wnd_free : one_process;
        one_process = one_process > peer_windows_size_ ?
            peer_windows_size_ : one_process;

        ret = send_frame_to(FLAG::PSH,
                            false,
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
        ++z;
        ZNO_LOG(RS_DEBUG,
                "[RUDP]send loop. seesion[%u] one_process [%u] process_len[%u] remain_len [%u] "
                "peer_windows_size_[%u] frame_max_len [%u] ",
                session_id_,
                one_process,
                process_len,
                remain_len,
                peer_windows_size_,
                frame_max_len);
    }
    *send_len = process_len;
    ZNO_LOG(RS_DEBUG,
            "[RUDP]send end. seesion[%u] process_len(send) [%u] remain_len [%u] "
            "peer_windows_size_[%u] snd_wnd[%u|%u] snd_rec[%u|%u]",
            session_id_,
            process_len,
            remain_len,
            peer_windows_size_,
            send_windows_.size(),
            send_windows_.free(),
            send_rec_list_.size(),
            send_rec_list_.free());
    //是否要把超时的都拎出来发送一次？

    return 0;
}

//!有数据抵达时的函数
int PEER::deliver_recv(const zce::sockaddr_any *remote_addr,
                       RUDP_FRAME *recv_frame,
                       size_t recv_len,
                       bool *remote_change,
                       zce::sockaddr_any *old_remote,
                       RECV_NEXT_CALL *next_call)
{
    *next_call = RECV_NEXT_CALL::INVALID;
    const size_t buf_size = 64;
    char new_remote_str[buf_size], old_remote_str[buf_size];

    ZNO_LOG(RS_DEBUG,
            "[RUDP] deliver_recv start.model[%u] session[%u] recv len[%u] "
            "remote [%s].[%s] recv frame len[%u] flag[%u] session[%u] sn[%u] ack[%u] wnd [%u]"
            "peer_windows_size_[%u] recv seq_ [%u|%u|%u][%u|%u] send seq_[%u|%u][%u]"
            "recv wnd [%u|%u] rec [%u|%u] send rec[%u|%u].recv bytes[%llu][%llu]",
            model_,
            session_id_,
            recv_len,
            zce::get_host_addr_port((sockaddr *)&remote_addr_, old_remote_str, buf_size),
            zce::get_host_addr_port((sockaddr *)remote_addr, new_remote_str, buf_size),
            recv_frame->u32_1_.len_,
            recv_frame->u32_1_.flag_,
            recv_frame->session_id_,
            recv_frame->sequence_num_,
            recv_frame->ack_id_,
            recv_frame->windows_size_,
            peer_windows_size_,
            rcv_wnd_first_,
            rcv_wnd_series_end_,
            rcv_wnd_last_,
            rcv_wnd_series_end_ - rcv_wnd_first_,
            rcv_wnd_last_ - rcv_wnd_last_,
            my_seq_num_ack_,
            my_seq_num_counter_,
            my_seq_num_counter_ - my_seq_num_ack_,
            recv_windows_.size(),
            recv_windows_.free(),
            recv_rec_list_.size(),
            recv_rec_list_.free(),
            send_rec_list_.size(),
            send_rec_list_.free(),
            recv_bytes_,
            rerecv_bytes_);

    //收到的数据长度不可能大于以太网的MSS
    if (recv_len <= 0 ||
        recv_len > MSS_ETHERNET ||
        recv_len < MIN_FRAME_LEN ||
        recv_len != recv_frame->u32_1_.len_)
    {
        ZCE_LOG(RS_ERROR,
                "[RUDP] deliver_recv. Frame len error. model[%u] session[%u] mtu[%u] "
                "recv len [%u] frame len [%u]in head MSS_ETHERNET [%u] MIN_FRAME_LEN [%u]",
                model_,
                session_id_,
                mtu_type_,
                recv_len,
                recv_frame->u32_1_.len_,
                MSS_ETHERNET,
                MIN_FRAME_LEN);
        *next_call = RECV_NEXT_CALL::RESET_PEER;
        zce::last_error(EMSGSIZE);
        return -1;
    }
    //远端地址可能发生改变，
    zce::sockaddr_any new_remote = *remote_addr;
    if (new_remote != remote_addr_)
    {
        //客户端的地址可能会变化。属于正常情况，记录一下
        if (model_ == MODEL::PEER_CORE_CREATE)
        {
            *remote_change = true;
            *old_remote = remote_addr_;
            remote_addr_ = new_remote;
            ZCE_LOG(RS_INFO,
                    "[RUDP] deliver_recv. model[%u] session[%u] remote address [%s] "
                    "already modify [%s].Please pay attention to.",
                    model_,
                    session_id_,
                    zce::get_host_addr_port((sockaddr *)&remote_addr_, old_remote_str, buf_size),
                    zce::get_host_addr_port((sockaddr *)remote_addr, new_remote_str, buf_size));
        }
        //服务端地址不会变化，也不能接受其他人塞过来的数据
        else if (model_ == MODEL::PEER_CLIENT)
        {
            ZCE_LOG(RS_ERROR,
                    "[RUDP] deliver_recv. model[%u] session[%u] recv remote address [%s] data."
                    "but core remote addr [%s].Please pay attention to.",
                    model_,
                    session_id_,
                    zce::get_host_addr_port((sockaddr *)remote_addr, new_remote_str, buf_size),
                    zce::get_host_addr_port((sockaddr *)&remote_addr_, old_remote_str, buf_size));
            zce::last_error(ENOTCONN);
            return -1;
        }
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
            ZCE_LOG(RS_ERROR,
                    "[RUDP] deliver_recv.Session id [%u] not eaqul, receive session id[%u] .",
                    session_id_,
                    recv_frame->session_id_);
            *next_call = RECV_NEXT_CALL::RESET_PEER;
            return -1;
        }
    }

    if (flag & FLAG::RST)
    {
        *next_call = RECV_NEXT_CALL::BE_RESET;
        return 0;
    }

    //对方有数据
    if (flag & FLAG::PSH)
    {
        //如果连接没有建立，
        if (!established_)
        {
            ZCE_LOG(RS_ERROR,
                    "[RUDP] deliver_recv.session[%u] link not established, but receive "
                    " push data,discard it. recv wnd[%u|%u|%u] recv sn[%u].",
                    session_id_,
                    rcv_wnd_first_,
                    rcv_wnd_series_end_,
                    rcv_wnd_last_,
                    recv_frame->sequence_num_);
            *next_call = RECV_NEXT_CALL::RESET_PEER;
            return -1;
        }
        if (recv_len == sizeof(RUDP_HEAD))
        {
            ZCE_LOG(RS_ERROR,
                    "[RUDP] deliver_recv.session[%u] receive PSH flag ,but frame len == "
                    "sizeof(RUDP_HEAD). ",
                    session_id_);
            *next_call = RECV_NEXT_CALL::RESET_PEER;
            return -1;
        }
        RECV_DATA_LOCATION op = RECV_DATA_LOCATION::INVALID;
        int ret = process_recv_data(recv_frame,
                                    &op);
        ZNO_LOG(RS_DEBUG,
                "[RUDP] deliver_recv session[%u] process recv ret[%d]  operation[%u]."
                "len [%u] flag [%u] sn[%u] "
                "recv wnd [%u|%u|%u] recv bytes[%llu][%llu] ",
                session_id_,
                ret,
                op,
                recv_len,
                recv_frame->u32_1_.flag_,
                recv_frame->sequence_num_,
                rcv_wnd_first_,
                rcv_wnd_series_end_,
                rcv_wnd_last_,
                recv_bytes_,
                rerecv_bytes_);
        //成功接收，而且非重复数据才记录
        if (ret == 0)
        {
            *next_call = RECV_NEXT_CALL::SENDBACK_ACK;
            if (op == RECV_DATA_LOCATION::REPEAT || op == RECV_DATA_LOCATION::NOROOM)
            {
                rerecv_bytes_ += (recv_len - sizeof(RUDP_HEAD));
            }
            else if (op == RECV_DATA_LOCATION::FILL || op == RECV_DATA_LOCATION::TAIL
                     || op == RECV_DATA_LOCATION::ADVANCE || op == RECV_DATA_LOCATION::SERIES)
            {
                recv_bytes_ += (recv_len - sizeof(RUDP_HEAD));
            }
            if (op == RECV_DATA_LOCATION::SERIES)
            {
                *next_call = RECV_NEXT_CALL::RECEIVE;
            }
        }
    }
    else if (flag & FLAG::SYN)
    {
        //记录对方的sequence number，期待下一个的sn = 当前的sn +1
        rcv_wnd_series_end_ = recv_frame->sequence_num_ + 1;
        rcv_wnd_first_ = rcv_wnd_series_end_;
        rcv_wnd_last_ = rcv_wnd_series_end_;
        if (flag & FLAG::ACK && model_ == MODEL::PEER_CLIENT)
        {
            *next_call = RECV_NEXT_CALL::CONNECT;
            send_frame_to(FLAG::ACK);
        }
        else if (model_ == MODEL::PEER_CORE_CREATE)
        {
            send_frame_to(FLAG::ACK | FLAG::SYN);
        }
        else
        {
            ZCE_ASSERT(false);
        }
        established_ = true;
    }
    //心跳情况下，什么都不做，就是保证有应答
    else if (flag & FLAG::AYO)
    {
        send_frame_to(FLAG::IAO);
    }
    uint64_t now_clock = zce::clock_ms();
    //有ACK 标志处理,（而且没有处理过）
    if ((flag & FLAG::ACK))
    {
        acknowledge_send(recv_frame,
                         now_clock);
    }
    peer_live_clock_ = now_clock;

    ZNO_LOG(RS_DEBUG,
            "[RUDP] deliver_recv end. model[%u] session[%u] recv len [%u] "
            "remote [%s] peer_windows_size_[%u] recv seq_ [%u|%u|%u][%u|%u] send seq_[%u|%u][%u] "
            "recv wnd [%u|%u] rec [%u|%u] send rec[%u|%u].recv bytes[%llu][%llu]",
            model_,
            session_id_,
            recv_len,
            zce::get_host_addr_port((sockaddr *)&remote_addr_, old_remote_str, buf_size),
            peer_windows_size_,
            rcv_wnd_first_,
            rcv_wnd_series_end_,
            rcv_wnd_last_,
            rcv_wnd_series_end_ - rcv_wnd_first_,
            rcv_wnd_last_ - rcv_wnd_last_,
            my_seq_num_ack_,
            my_seq_num_counter_,
            my_seq_num_counter_ - my_seq_num_ack_,
            recv_windows_.size(),
            recv_windows_.free(),
            recv_rec_list_.size(),
            recv_rec_list_.free(),
            send_rec_list_.size(),
            send_rec_list_.free(),
            recv_bytes_,
            rerecv_bytes_);

    return 0;
}

//
int PEER::process_recv_data(const RUDP_FRAME *recv_frame,
                            RECV_DATA_LOCATION *op)
{
    bool already_proc = false;
    uint32_t data_len = recv_frame->u32_1_.len_ - sizeof(RUDP_HEAD);
    uint32_t new_start = recv_frame->sequence_num_;
    uint32_t new_end = recv_frame->sequence_num_ + data_len;

    ZNO_LOG(RS_DEBUG,
            "[RUDP]process_recv_data start.session[%u],"
            "recv frame len[%u] flag[%u] sn[%u|%u] data len[%u] "
            "recv windows [%u|%u] recv_rec [%u|%u],recv seq_[%u|%u|%u].",
            session_id_,
            recv_frame->u32_1_.len_,
            recv_frame->u32_1_.flag_,
            new_start,
            new_end,
            data_len,
            recv_windows_.size(),
            recv_windows_.free(),
            recv_rec_list_.size(),
            recv_rec_list_.free(),
            rcv_wnd_first_,
            rcv_wnd_series_end_,
            rcv_wnd_last_);
    //接收记录空间不够
    if (recv_rec_list_.free() == 0)
    {
        ZCE_LOG_DEBUG(RS_ERROR,
                      "[RUDP]process_recv_data session[%u].recv record size not enought."
                      "recv_rec_list_[%u|%u]",
                      session_id_,
                      recv_rec_list_.size(),
                      recv_rec_list_.free());
        *op = RECV_DATA_LOCATION::NOROOM;
        return 0;
    }
    if ((int32_t)(new_start - rcv_wnd_series_end_) < 0)
    {
        *op = RECV_DATA_LOCATION::REPEAT;
        return 0;
    }
    auto r = recv_rec_list_.begin();
    char *write_pos = nullptr;
    for (; r != recv_rec_list_.end(); ++r)
    {
        int32_t dif_rs_ne = (int32_t)(r->seq_start_ - new_end);
        int32_t dif_re_ns = (int32_t)(r->seq_end_ - new_start);
        int32_t dif_rs_ns = (int32_t)(r->seq_start_ - new_start);
        int32_t dif_re_ne = (int32_t)(r->seq_end_ - new_end);
        if (dif_rs_ne >= 0)
        {
            RECV_RECORD rcv_rec;
            rcv_rec.seq_start_ = new_start;
            rcv_rec.seq_end_ = new_end;
            recv_rec_list_.insert(r, rcv_rec);
            uint32_t write_start = new_start - rcv_wnd_first_;

            recv_windows_.set_data(write_start, recv_frame->data_, data_len, write_pos);
            *op = RECV_DATA_LOCATION::FILL;
            already_proc = true;
            break;
        }
        else if (dif_re_ns <= 0)
        {
            continue;
        }
        else if (dif_rs_ns == 0 && dif_re_ne == 0)
        {
            *op = RECV_DATA_LOCATION::REPEAT;
            already_proc = true;
            break;
        }
        else
        {
            ZCE_LOG(RS_ERROR,
                    "[RUDP]process_recv_data session[%u].recv sequence error."
                    "recv frame sn[%u|%u] data len[%u].",
                    session_id_,
                    new_start,
                    new_end,
                    data_len);
            *op = RECV_DATA_LOCATION::ERR;
            return -1;
        }
    }
    //如果数据还没有处理，是要在尾部插入
    if (!already_proc)
    {
        uint32_t write_len = (new_end - rcv_wnd_last_);
        if (recv_windows_.free() < write_len)
        {
            ZCE_LOG(RS_ERROR,
                    "[RUDP]process_recv_data session[%u].recv windows size not enought."
                    "recv frame len[%u] data len[%u] write len[%u] recv_windows_ [%u|%u].",
                    session_id_,
                    recv_frame->u32_1_.len_,
                    data_len,
                    write_len,
                    recv_windows_.size(),
                    recv_windows_.free());
            *op = RECV_DATA_LOCATION::NOROOM;
            return 0;
        }
        //不连续
        if (rcv_wnd_last_ != new_start)
        {
            *op = RECV_DATA_LOCATION::ADVANCE;
            rcv_wnd_last_ = new_start;
            recv_windows_.push_end('\0', write_len - data_len, write_pos);
        }
        else
        {
            *op = RECV_DATA_LOCATION::TAIL;
        }
        rcv_wnd_last_ = new_end;
        RECV_RECORD rcv_rec;
        rcv_rec.seq_start_ = new_start;
        rcv_rec.seq_end_ = new_end;
        recv_rec_list_.insert(r, rcv_rec);
        recv_windows_.push_end(recv_frame->data_, data_len, write_pos);
    }

    if (rcv_wnd_series_end_ == new_start)
    {
        *op = RECV_DATA_LOCATION::SERIES;
    }

    //检查接收的数据是否是连续的，
    auto t = recv_rec_list_.begin();
    while (t != recv_rec_list_.end())
    {
        if (t->seq_start_ == rcv_wnd_series_end_)
        {
            rcv_wnd_series_end_ = t->seq_end_;
            auto del_iter = t;
            recv_rec_list_.erase(del_iter);
            t = recv_rec_list_.begin();
            continue;
        }
        else
        {
            break;
        }
    }
#if defined RUDP_CHECK && RUDP_CHECK ==1
    if (recv_windows_.size() != (rcv_wnd_last_ - rcv_wnd_first_) ||
        (rcv_wnd_series_end_ - rcv_wnd_first_) > (rcv_wnd_last_ - rcv_wnd_first_))
    {
        ZCE_LOG(RS_ERROR,
                "[RUDP]process_recv_data error.session[%u] please check. "
                "recv wind[%u]?=[%u] seq_ [%u]?<=[%u]",
                session_id_,
                recv_windows_.size(),
                (rcv_wnd_last_ - rcv_wnd_first_),
                (rcv_wnd_series_end_ - rcv_wnd_first_),
                (rcv_wnd_last_ - rcv_wnd_first_));
        t = recv_rec_list_.begin();
        for (size_t e = 0; t != recv_rec_list_.end(); ++e, ++t)
        {
            ZCE_LOG(RS_ERROR,
                    "[RUDP]process_recv_data error.recv_rec_list_[%u] data[%u][%u]",
                    e,
                    t->seq_start_,
                    t->seq_end_);
        }
    }
#endif

    ZNO_LOG(RS_DEBUG,
            "[RUDP]process_recv_data end.session[%u] op[%d]"
            "recv_windows_[%u|%u] recv_rec_list_ [%u|%u],recv seq_[%u|%u|%u][%u|%u].",
            session_id_,
            *op,
            recv_windows_.size(),
            recv_windows_.free(),
            recv_rec_list_.size(),
            recv_rec_list_.free(),
            rcv_wnd_first_,
            rcv_wnd_series_end_,
            rcv_wnd_last_,
            rcv_wnd_series_end_ - rcv_wnd_first_,
            rcv_wnd_last_ - rcv_wnd_first_);

    return 0;
}

//发送FRAME数据去远端，
int PEER::send_frame_to(int flag,
                        bool prev_rec_ack,
                        bool first_send,
                        const char *data,
                        size_t sz_data,
                        SEND_RECORD *old_snd_rec)
{
    RUDP_FRAME *frame = (RUDP_FRAME *)send_buffer_;
    frame->clear();
    size_t sz_frame = 0;
    if (first_send)
    {
        sz_frame = sizeof(RUDP_HEAD) + sz_data;
        if (sz_data > 0)
        {
            ZCE_ASSERT(data);
            ::memcpy(send_buffer_ + sizeof(RUDP_HEAD), data, sz_data);
        }
        send_bytes_ += sz_data;
    }
    else
    {
        ZCE_ASSERT(old_snd_rec);
        sz_frame = sizeof(RUDP_HEAD) + old_snd_rec->len_;
        if (old_snd_rec->len_ > 0)
        {
            bool succ = send_windows_.acquire_data(old_snd_rec->buf_pos_,
                                                   send_buffer_ + sizeof(RUDP_HEAD),
                                                   old_snd_rec->len_);
            if (!succ)
            {
                ZCE_LOG(RS_ERROR,
                        "[RUDP]send_frame_to erro. session[%u], acquire_data fail."
                        "Please check code.",
                        session_id_);
            }
            resend_bytes_ += old_snd_rec->len_;
        }
    }
    //如果PSH，就带上ACK
    if (flag & FLAG::PSH)
    {
        flag |= FLAG::ACK;
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

    //ACK标志
    if (flag & FLAG::ACK)
    {
        frame->ack_id_ = rcv_wnd_series_end_;
        if (prev_rec_ack)
        {
            frame->ack_id_ = record_prev_ack_;
        }

        if (flag & FLAG::UNA)
        {
            uint32_t resend_num = 0;
            for (auto iter1 = recv_rec_list_.begin();
                 iter1 != recv_rec_list_.end(); ++iter1)
            {
                auto iter2 = ++iter1;
                if (iter2 == recv_rec_list_.end())
                {
                    break;
                }
                //收到的数据不连续
                if (iter1->seq_end_ != iter2->seq_start_)
                {
                    frame->una_[resend_num] = iter2->seq_start_;
                    if (resend_num >= RUDP_FRAME::MAX_UNA_NUMBER)
                    {
                        break;
                    }
                    ++resend_num;
                }
            }
            frame->u32_1_.una_num_ = resend_num;
        }
    }
    frame->u32_1_.mtu_type_ = (uint32_t)mtu_type_;
    frame->windows_size_ = static_cast<uint32_t>(recv_windows_.free());

    uint64_t now_clock = zce::clock_ms();
    //这几种情况的发送需要ACK确认,会填写sequence_num_
    if ((flag & FLAG::PSH) || (flag & FLAG::SYN))
    {
        if (!established_ && (flag & FLAG::PSH))
        {
            ZCE_LOG_DEBUG(RS_DEBUG,
                          "[RUDP]link session[%u] not established, but send push data,"
                          "discard it. rcv_wnd_series_end_[%u] .",
                          session_id_,
                          rcv_wnd_series_end_);
            return -1;
        }
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
            //超时发送,RTO会不断的扩展，但这个比TCP的扩展速率低很多
            old_snd_rec->timeout_clock_ = now_clock +
                (uint64_t)((rto_ > min_rto_ ? rto_ : min_rto_) *
                           pow(blocking_rto_ratio_, old_snd_rec->send_num_));
            ++old_snd_rec->send_num_;
        }
    }

    const size_t buf_size = 64;
    char remote_str[buf_size];
    ZNO_LOG(RS_DEBUG,
            "[RUDP]send_frame_to start. model[%u] session[%u] first send[%u] remote [%s] "
            "send frame len[%u] flag[%u] sn[%u] ack[%u] wnd[%u] send seq_[%u|%u][%u]"
            " send wnd [%u|%u] rec[%u|%u] send bytes[%llu][%llu]",
            model_,
            session_id_,
            first_send,
            zce::get_host_addr_port((sockaddr *)&remote_addr_, remote_str, buf_size),
            frame->u32_1_.len_,
            frame->u32_1_.flag_,
            frame->sequence_num_,
            frame->ack_id_,
            frame->windows_size_,
            my_seq_num_ack_,
            my_seq_num_counter_,
            my_seq_num_counter_ - my_seq_num_ack_,
            send_windows_.size(),
            send_windows_.free(),
            send_rec_list_.size(),
            send_rec_list_.free(),
            send_bytes_,
            resend_bytes_);
    frame->hton();
    ssize_t ssend = sz_frame;
    if (random() % 100 > 3)
    {
        ssend = zce::sendto(peer_socket_,
                            (void *)frame,
                            sz_frame,
                            0,
                            (sockaddr *)&remote_addr_,
                            sizeof(zce::sockaddr_any));
        return 0;
    }

    if (ssend != (ssize_t)sz_frame)
    {
        char buf[buf_size];
        ZCE_LOG(RS_ERROR,
                "[RUDP]zce::sendto return error ret = [%d] frame len[%u] remote[%s]",
                ssend,
                sz_frame,
                zce::get_host_addr_port(
                (sockaddr *)&remote_addr_, buf, buf_size));
        return -1;
    }

    ZNO_LOG(RS_DEBUG,
            "[RUDP] send_frame_to send end. model[%u] session[%u] first send[%u] remote [%s] "
            "send len [%u]send seq_[%u|%u][%u] send wnd [%u|%u] rec[%u|%u] send bytes[%llu][%llu]",
            model_,
            session_id_,
            first_send,
            zce::get_host_addr_port((sockaddr *)&remote_addr_, remote_str, buf_size),
            sz_frame,
            my_seq_num_ack_,
            my_seq_num_counter_,
            my_seq_num_counter_ - my_seq_num_ack_,
            send_windows_.size(),
            send_windows_.free(),
            send_rec_list_.size(),
            send_rec_list_.free(),
            send_bytes_,
            resend_bytes_);
    return 0;
}

//
int PEER::acknowledge_send(const RUDP_FRAME *recv_frame,
                           uint64_t now_clock)
{
    ZNO_LOG(RS_DEBUG,
            "[RUDP]acknowledge_send start.session[%u] recv frame ack[%u] my_seq_num_ack_[%u] "
            "una number [%u] uno [%u][%u][%u] send list[%u][%u].",
            session_id_,
            recv_frame->ack_id_,
            my_seq_num_ack_,
            recv_frame->u32_1_.una_num_,
            recv_frame->una_[0],
            recv_frame->una_[1],
            recv_frame->una_[2],
            send_rec_list_.size(),
            send_rec_list_.free());
    //收到了2次相同的ack id，很可能丢包
    uint32_t recv_ack_id = recv_frame->ack_id_;
    size_t size_snd_rec = send_rec_list_.size();
    if (my_seq_num_ack_ == recv_ack_id)
    {
        if (size_snd_rec > 0)
        {
            SEND_RECORD &snd_rec = send_rec_list_[0];
            if (snd_rec.sequence_num_ == recv_ack_id)
            {
                send_frame_to(snd_rec.flag_,
                              false,
                              false,
                              nullptr,
                              0,
                              &snd_rec);
            }
            else
            {
            }
        }
    }
    else
    {
        int32_t difference = 0;
        for (size_t i = 0; i < size_snd_rec; ++i)
        {
            //一直处理第一个
            SEND_RECORD &snd_rec = send_rec_list_[0];
            uint32_t snd_serial_id = snd_rec.sequence_num_;

            //这儿有一点技巧，避免 wrap around引发麻烦,其实就是利用了补码
            difference = (int32_t)(snd_serial_id - recv_ack_id);
            if (difference < 0)
            {
                //只对没有超时处理的发送计算rto
                if (snd_rec.send_num_ == 1)
                {
                    calculate_rto(snd_rec.send_clock_,
                                  now_clock);
                }
                //已经的得到确认，从发送窗口pop出数据
                if (snd_rec.len_ > 0)
                {
                    send_windows_.pop_front(snd_rec.len_);
                }
                send_rec_list_.pop_front();
                adjust_cwnd(CWND_EVENT::ACK);
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
            || (difference < 0 && send_rec_list_.size() == 0
            && recv_ack_id == my_seq_num_counter_))
        {
            //正常，如果收到的ack id
        }
        else
        {
            //其实这儿存在错误
            ZCE_LOG(RS_ERROR, "[RUDP]code error? difference [%d] send rec [%u] "
                    "recv_ack_id[%u] my_seq_num_counter_[%u]",
                    difference,
                    send_rec_list_.size(),
                    recv_ack_id,
                    my_seq_num_counter_);
        }
    }

    //处理重新发送请求，
    size_t resend_num = recv_frame->u32_1_.una_num_;
    if (resend_num > 0 && resend_num <= RUDP_FRAME::MAX_UNA_NUMBER)
    {
        size_t y = 0;
        size_snd_rec = send_rec_list_.size();
        for (size_t x = 0; x < size_snd_rec; ++x)
        {
            SEND_RECORD &snd_rec = send_rec_list_[x];
            //req_resend_seq_ 应该是有序的，下面的代码基于这个假定
            if (snd_rec.sequence_num_ == recv_frame->una_[y])
            {
                //重发，重发不检查窗口大小
                send_frame_to(snd_rec.flag_,
                              false,
                              false,
                              nullptr,
                              0,
                              &snd_rec);
                ++y;
                if (y >= resend_num)
                {
                    break;
                }
            }
        }
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
void PEER::time_out(uint64_t now_clock_ms,
                    bool *not_alive,
                    bool *connect_fail)
{
    int ret = 0;
    *not_alive = false;
    *connect_fail = false;
    size_t resend_num = 0;
    size_t size_snd_rec = send_rec_list_.size();
    for (size_t i = 0; i < size_snd_rec; ++i)
    {
        SEND_RECORD &snd_rec = send_rec_list_[i];
        //有超时发生
        if (snd_rec.timeout_clock_ <= now_clock_ms)
        {
            if (snd_rec.flag_ & FLAG::SYN && model_ == MODEL::PEER_CLIENT &&
                now_clock_ms - snd_rec.send_clock_ >= 60000)
            {
                *connect_fail = true;
                break;
            }

            //重发
            ret = send_frame_to(snd_rec.flag_,
                                false,
                                false,
                                nullptr,
                                0,
                                &snd_rec);
            if (ret != 0)
            {
                break;
            }
            ++resend_num;
        }
    }
    //如果长时间没有反应。
    if (now_clock_ms - peer_live_clock_ > (uint64_t)noalive_time_to_close_)
    {
        *not_alive = true;
    }
}

//再次声明，这儿的cwnd和TCP的拥塞窗口不是一样，我只用于控制一次的发送大小
void PEER::adjust_cwnd(CWND_EVENT event)
{
    //!CWND最小值
    static constexpr size_t MIN_CWND_SIZE = 4;
    //!CWND最大值
    static constexpr size_t MAX_CWND_SIZE = 64;
    //!慢启动CWND阈值
    static constexpr size_t CWND_SSTHRESH = 32;

    switch (event)
    {
        case CWND_EVENT::ACK:
            if (congestion_window_ < CWND_SSTHRESH)
            {
                congestion_window_ += 8;
            }
            else
            {
                congestion_window_ += 1;
            }
            break;
        case CWND_EVENT::FAST_RECOVERY:
            if (congestion_window_ > 2)
            {
                congestion_window_ -= 2;
            }
            break;
        case CWND_EVENT::RTO_RECOVERY:
            if (congestion_window_ > 4)
            {
                congestion_window_ -= 4;
            }
            break;
        case CWND_EVENT::SWND_CHANGE:
            if (send_windows_.size() < 4)
            {
                congestion_window_ = 4;
            }
            break;
    }
    congestion_window_ = congestion_window_ <= MAX_CWND_SIZE ? congestion_window_ : MAX_CWND_SIZE;
    congestion_window_ = congestion_window_ >= MIN_CWND_SIZE ? congestion_window_ : MIN_CWND_SIZE;
}

//!发送ACK
void PEER::send_ack()
{
    if (need_sendback_ack_ > 1)
    {
        send_frame_to(FLAG::ACK, true);
    }
    send_frame_to(FLAG::ACK | FLAG::UNA);
    need_sendback_ack_ = 0;
}

void PEER::dump_info(const char *some_thing, LOG_PRIORITY log_priority)
{
    const size_t buf_size = 64;
    char remote_str[buf_size];
    ZCE_LOG(log_priority,
            "[RUDP].info[%s].model[%u] session[%u] remote [%s] "
            "send seq_[%u|%u][%u]send wnd [%u|%u] rec[%u|%u] send bytes[%llu][%llu]"
            "recv seq_[%u|%u|%u][%u|%u]recv windows[%u|%u] rec [%u|%u],recv bytes[%llu][%llu].",
            some_thing,
            model_,
            session_id_,
            zce::get_host_addr_port((sockaddr *)&remote_addr_, remote_str, buf_size),
            my_seq_num_ack_,
            my_seq_num_counter_,
            my_seq_num_counter_ - my_seq_num_ack_,
            send_windows_.size(),
            send_windows_.free(),
            send_rec_list_.size(),
            send_rec_list_.free(),
            send_bytes_,
            resend_bytes_,
            rcv_wnd_first_,
            rcv_wnd_series_end_,
            rcv_wnd_last_,
            rcv_wnd_series_end_ - rcv_wnd_first_,
            rcv_wnd_last_ - rcv_wnd_first_,
            recv_windows_.size(),
            recv_windows_.free(),
            recv_rec_list_.size(),
            recv_rec_list_.free(),
            recv_bytes_,
            rerecv_bytes_);
}

//=================================================================================================
//客户端调用的接收数据到内部的函数
//打开一个客户端
int CLIENT::open(const sockaddr *remote_addr,
                 size_t send_wnd_size,
                 size_t recv_wnd_size,
                 std::function<ssize_t(CLIENT *)> *callbak_recv,
                 std::function<int(CLIENT *, bool)> *callbak_connect)
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

void CLIENT::close()
{
    PEER::close();
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

void CLIENT::reset()
{
    PEER::close();
    send_rec_list_.clear();
    send_windows_.clear();
    recv_rec_list_.clear();
    recv_windows_.clear();
    session_id_ = 0;
    my_seq_num_counter_ = BASE::random();
    my_seq_num_ack_ = 0;
    rcv_wnd_first_ = 0;
    rcv_wnd_last_ = 0;
    rcv_wnd_series_end_ = 0;
    return;
}

//客户端无阻塞（无等待）收取数据,收取数据到内部接收窗口
int CLIENT::batch_receive(size_t *recv_size)
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
            if (ssz_recv > MAX_FRAME_LEN || ssz_recv < MIN_FRAME_LEN)
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
        else if (next_call == PEER::RECV_NEXT_CALL::RESET_PEER)
        {
            send_frame_to(FLAG::RST);
            reset();
            close();
        }
        else if (next_call == PEER::RECV_NEXT_CALL::BE_RESET)
        {
            reset();
            close();
        }
        else if (next_call == PEER::RECV_NEXT_CALL::RECEIVE && is_callbak_recv_)
        {
            ++need_callback_recv_;
            record_ack();
        }
        //不直接处理RESET可能很好理解，但为什么不立即返回ACK呢。
        //如果一次接受的大量数据都是一个PEER的，那么会回复很多ACK和UNO，反而导致重发的风暴
        else if (next_call == PEER::RECV_NEXT_CALL::SENDBACK_ACK)
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

int CLIENT::receive_timeout(zce::Time_Value* timeout_tv,
                            size_t *recv_size)
{
    *recv_size = 0;
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
    batch_receive(recv_size);

    return 0;
}

//! 异步（非阻塞）连接，返回0并不表示真正成功，还没有对方确认
int CLIENT::connect(bool link_test_mtu)
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
int CLIENT::connect_timeout(zce::Time_Value* timeout_tv,
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
void CLIENT::time_out()
{
    uint64_t now_clock_ms = zce::clock_ms();

    bool no_alive = false;
    bool connect_fail = false;
    PEER::time_out(now_clock_ms, &no_alive, &connect_fail);
    if (no_alive)
    {
        close();
    }
    if (connect_fail && callbak_connect_)
    {
        callbak_connect_(this, false);
    }
}

//=================================================================================================

//服务器端CORE打开一个PEER
int ACCEPT::open(CORE *core,
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

void ACCEPT::close()
{
    PEER::close();
    return;
}

void ACCEPT::reset()
{
    PEER::close();
    return;
}

//=================================================================================================
//class CORE

int CORE::open(const sockaddr *core_addr,
               size_t max_num_of_peer,
               size_t peer_send_list_num,
               size_t peer_recv_list_num,
               std::function<ssize_t(ACCEPT *)> *callbak_recv,
               std::function<int(ACCEPT *)> *callbak_accept)
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

void CORE::close()
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

int CORE::batch_receive(size_t *recv_peer_num,
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
        ACCEPT *recv_peer = nullptr;
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
                        "[RUDP][CORE]zce::recvfrom return error ret = [%d] errno=[%d]",
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
                ZCE_LOG(RS_ERROR, "[RUDP][CORE] batch_receive ssz_recv [%u] error. "
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
                    "[RUDP][CORE]batch_receive frame len error. ssz_recv [%d] frame len[%d]",
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
                    ZCE_LOG(RS_ERROR, "[RUDP][CORE]batch_receive already process max. peer_map_ size[%u] > "
                            " max_num_of_peer_[%u]",
                            peer_map_.size(),
                            max_num_of_peer_);
                    break;
                }
                ret = accept_peer(&remote_ip, recv_peer);
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
                ZCE_LOG_DEBUG(RS_ERROR, "[RUDP][CORE] SYN error, flag [%u] but "
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
                ZCE_LOG_DEBUG(RS_ERROR, "[RUDP][CORE] session[%u] not map to peer.",
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
        PEER::RECV_NEXT_CALL next_call = PEER::RECV_NEXT_CALL::INVALID;
        recv_peer->deliver_recv(&remote_ip,
                                frame,
                                (size_t)ssz_recv,
                                &change,
                                &old_remote,
                                &next_call);
        if (change && next_call != PEER::RECV_NEXT_CALL::RESET_PEER
            && next_call != PEER::RECV_NEXT_CALL::BE_RESET)
        {
            peer_addr_set_.erase(old_remote);
            peer_addr_set_.insert(std::make_pair(remote_ip,
                                  recv_peer->session_id_));
        }

        if (next_call == PEER::RECV_NEXT_CALL::RESET_PEER)
        {
            recv_peer->send_frame_to(FLAG::RST);
            recv_peer->reset();
            close_peer(recv_peer);
        }
        else if (next_call == PEER::RECV_NEXT_CALL::BE_RESET)
        {
            recv_peer->reset();
            close_peer(recv_peer);
        }
        else if (next_call == PEER::RECV_NEXT_CALL::RECEIVE && is_callbak_recv_)
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
        else if (next_call == PEER::RECV_NEXT_CALL::SENDBACK_ACK)
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
            ACCEPT *rc_peer = iter->second;
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
        ACCEPT *ack_peer = iter->second;
        if (ack_peer->need_sendback_ack_)
        {
            ack_peer->send_ack();
        }
    }
    return 0;
}

int CORE::receive_timeout(zce::Time_Value* timeout_tv,
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

int CORE::accept_peer(const zce::sockaddr_any *remote_ip,
                      ACCEPT *& new_peer)
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
            ACCEPT * old_peer = iter_peer->second;
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
    new_peer = new ACCEPT();
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
void CORE::close_peer(ACCEPT * del_peer)
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

//=================================================================================================
}