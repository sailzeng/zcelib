#include "zce/predefine.h"
#include "zce/rudp/base.h"
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
//class base

//=================================================================================================
//class base
std::mt19937 base::random_gen_(19190504 + (uint32_t)::time(nullptr));

uint32_t base::random()
{
    return random_gen_();
}
//!
double base::blocking_rto_ratio_ = 1.5;
//!
time_t base::min_rto_ = 80;
//10分钟
time_t base::noalive_time_to_close_ = 600000;

void base::min_rto(time_t rto)
{
    min_rto_ = rto;
}

void base::blocking_rto_ratio(double rto_ratio)
{
    blocking_rto_ratio_ = rto_ratio;
}

void base::noalive_time_to_close(time_t to_close_time)
{
    noalive_time_to_close_ = to_close_time;
}
} //