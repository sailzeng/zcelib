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
//class PEER

//服务器端CORE打开一个PEER
int PEER::open(uint32_t session_id,
               uint32_t serial_id,
               ZCE_SOCKET peer_socket,
               const sockaddr *remote_addr,
               size_t send_list_num,
               size_t recv_list_num,
               zce::buffer_pool *buf_pool)
{
    int ret = 0;
    model_ = MODEL::PEER_CORE_CREATE;
    session_id_ = session_id;
    serial_id_ = serial_id;
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
    ret = send_list_.initialize(send_list_num);
    if (ret != 0)
    {
        return ret;
    }
    ret = recv_list_.initialize(recv_list_num);
    if (ret != 0)
    {
        return ret;
    }
    buf_pool_ = buf_pool;
    return 0;
}

//客户端打开一个
int PEER::open(const sockaddr *remote_addr,
               size_t send_list_num,
               size_t recv_list_num)
{
    int ret = 0;
    model_ = MODEL::PEER_CLIENT;

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
    ret = zce::open_socket(peer_socket_,
                           SOCK_DGRAM);
    if (ret != 0)
    {
        return ret;
    }
    ret = send_list_.initialize(send_list_num);
    if (ret != 0)
    {
        return ret;
    }
    ret = recv_list_.initialize(recv_list_num);
    if (ret != 0)
    {
        return ret;
    }
    //客户端模式用自己的内存分配池
    buf_pool_ = new zce::buffer_pool();
    buf_pool_->initialize(BUF_BUCKET_NUM,
                          BUF_BUCKET_SIZE_ARY,
                          2,
                          16);
    return 0;
}

//=================================================================================================
//class CORE
CORE::~CORE()
{
    terminate();
}

int CORE::initialize(const sockaddr *core_addr,
                     size_t max_num_of_peer,
                     size_t peer_send_list_num,
                     size_t peer_recv_list_num)
{
    int ret = 0;
    socklen_t socket_len = 0;
    if (core_addr->sa_family == AF_INET)
    {
        ::memcpy(&core_addr_, core_addr, sizeof(sockaddr_in));
        socket_len = sizeof(sockaddr_in);
    }
    else if (core_addr->sa_family == AF_INET6)
    {
        ::memcpy(&core_addr_, core_addr, sizeof(sockaddr_in6));
        socket_len = sizeof(sockaddr_in6);
    }
    else
    {
        assert(false);
        return -1;
    }
    ret = zce::open_socket(core_socket_,
                           SOCK_DGRAM,
                           core_addr,
                           socket_len);
    if (ret != 0)
    {
        return ret;
    }
    recv_buffer_ = new char[MAX_FRAME_LEN];
    send_buffer_ = new char[MAX_FRAME_LEN];

    std::random_device rd;
    random_gen_.seed(rd());
    peer_send_list_num_ = peer_send_list_num;
    peer_recv_list_num_ = peer_recv_list_num;
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