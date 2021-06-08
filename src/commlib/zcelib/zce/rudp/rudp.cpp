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
    serial_number_ = htonl(serial_number_);
    ack_ = htonl(ack_);
    uno1_ = htonl(uno1_);
    uno2_ = htonl(uno2_);
}

//将所有的uint16_t,uint32_t转换为本地序
void RUDP_HEAD::ntoh()
{
    u32_1_copy_ = ntohl(u32_1_copy_);
    session_id_ = ntohl(session_id_);
    serial_number_ = ntohl(serial_number_);
    ack_ = ntohl(ack_);
    uno1_ = ntohl(uno1_);
    uno2_ = ntohl(uno2_);
}

//=================================================================================================
//class CORE

int CORE::initialize(int family,
                     size_t send_pool_num,
                     size_t recv_pool_num)
{
    recv_buffer_ = new char[MAX_FRAME_LEN];
    send_buffer_ = new char[MAX_FRAME_LEN];
    family_ = family;
    return 0;
}

int CORE::receive(PEER *& recv_rudp,
                  bool *new_rudp)
{
    int ret = 0;
    zce::sockaddr_ip remote_ip;
    ssize_t ssz_recv = zce::recvfrom(udp_socket_,
                                     recv_buffer_,
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
    if (head->u32_1_.len_ != ssz_recv)
    {
        return -2;
    }

    return 0;
}

//=================================================================================================
}