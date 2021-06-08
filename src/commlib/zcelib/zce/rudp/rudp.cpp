#include "zce/predefine.h"
#include "zce/rudp/rudp.h"

namespace zce::rudp
{
//=================================================================================================
//class RUDP_HEAD

void RUDP_HEAD::hton()
{
    u32_1_copy_ = htonl(u32_1_copy_);
    session_id_ = htonl(session_id_);
    serial_id_ = htonl(serial_id_);
    ack_ = htonl(ack_);
    uno1_ = htonl(uno1_);
    uno2_ = htonl(uno2_);
}

//将所有的uint16_t,uint32_t转换为本地序
void RUDP_HEAD::ntoh()
{
    u32_1_copy_ = ntohl(u32_1_copy_);
    session_id_ = ntohl(session_id_);
    serial_id_ = ntohl(serial_id_);
    ack_ = ntohl(ack_);
    uno1_ = ntohl(uno1_);
    uno2_ = ntohl(uno2_);
}

//=================================================================================================
//class CORE

int CORE::initialize(int family,
                     size_t /*send_pool_num*/,
                     size_t /*recv_pool_num*/)
{
    recv_buffer_ = new char[MAX_FRAME_LEN];
    send_buffer_ = new char[MAX_FRAME_LEN];
    family_ = family;
    std::random_device rd;
    random_gen_.seed(rd());
    return 0;
}

void CORE::terminate()
{
    delete recv_buffer_;
    recv_buffer_ = nullptr;
    delete send_buffer_;
    send_buffer_ = nullptr;
}

int CORE::receive(PEER *& /*recv_rudp*/,
                  bool * /*new_rudp*/)
{
    zce::sockaddr_ip remote_ip;
    socklen_t sz_addr = sizeof(zce::sockaddr_ip);
    ssize_t ssz_recv = zce::recvfrom(udp_socket_,
                                     (void *)recv_buffer_,
                                     MAX_FRAME_LEN,
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

    RUDP_HEAD * head = (RUDP_HEAD *)recv_buffer_;
    head->ntoh();
    if (head->u32_1_.len_ != (uint32_t)ssz_recv)
    {
        return -2;
    }
    if (head->session_id_ == 0)
    {
        if (head->u32_1_.flag_ & SYN)
        {
            uint32_t session_id = random_gen_();
            uint32_t serial_id = random_gen_();
        }
        else
        {
        }
    }
    else
    {
        auto iter = peer_map_.find(head->session_id_);
        if (iter == peer_map_.end())
        {
        }
        //PEER * peer = iter->second;
    }
    return 0;
}

//=================================================================================================
}