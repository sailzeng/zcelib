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

//填充Data数据到Frame
int RUDP_FRAME::fill_data(const size_t szdata, const char* vardata)
{
    //填写数据区的长度
    memcpy(data_, vardata, szdata);
    u32_1_.len_ = static_cast<uint32_t>(sizeof(RUDP_HEAD) + szdata);
    return 0;
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

    remote_addr_ = remote_addr;
    ret = zce::open_socket(&peer_socket_,
                           SOCK_DGRAM,
                           remote_addr->sa_family);
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

void PEER::close()
{
    if (model_ == MODEL::PEER_CLIENT)
    {
        zce::close_socket(peer_socket_);
    }
    //CORE 产生的
    return;
}

//远端地址是否发生了变化
bool PEER::remote_change(const zce::sockaddr_ip &new_remote,
                         zce::sockaddr_ip &old_remote)
{
    if (new_remote == remote_addr_)
    {
        return false;
    }
    else
    {
        old_remote = remote_addr_;
        return true;
    }
}

int PEER::receive(const sockaddr *remote_addr,
                  RUDP_FRAME *rudp_frame,
                  size_t frame_len)
{
    //远端地址如果可能发生改变，CORE用remote_change自己判定是否有变化
    remote_addr_ = remote_addr;
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

    RUDP_FRAME * frame = (RUDP_FRAME *)recv_buffer_;
    frame->ntoh();
    if (frame->u32_1_.len_ != (uint32_t)ssz_recv)
    {
        return -2;
    }
    if (frame->session_id_ == 0)
    {
        if (frame->u32_1_.flag_ & SYN)
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
    bool change = recv_rudp->remote_change(remote_ip,
                                           old_remote);
    if (change)
    {
        peer_addr_set_.erase(old_remote);
        peer_addr_set_.insert(std::make_pair(remote_ip,
                              recv_rudp->seesion_id()));
    }
    //
    recv_rudp->receive((sockaddr *)&remote_ip,
                       frame,
                       (uint32_t)ssz_recv);
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
                         peer_send_list_num_,
                         peer_recv_list_num_,
                         &buf_pool_);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

//=================================================================================================
}