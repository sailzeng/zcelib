#include "zce/predefine.h"
#include "zce/rudp/rudp.h"

namespace zce::rudp
{
//=================================================================================================

//=================================================================================================
int core_init(CORE *core,
              size_t /*send_pool_num*/,
              size_t /*recv_pool_num*/,
              int family)
{
    core->family_ = family;
    return 0;
}

int core_receive(CORE * core,
                 ZCE_SOCKET socket,
                 PEER *&recv_rudp,
                 bool *new_rudp)
{
    HANDLE handle;
    handle.udp_socket_ = socket;
    int ret = 0;
    zce::sockaddr_ip local_ip;
    socklen_t sz_addr = sizeof(zce::sockaddr_ip);
    ret = zce::getsockname(socket, (sockaddr*)&local_ip, &sz_addr);
    if (ret)
    {
        return ret;
    }
    handle.local_ = local_ip;
    zce::sockaddr_ip remote_ip;
    ssize_t ssz_recv = zce::recvfrom(socket,
                                     core->receive_buffer_,
                                     MAX_PROCESS_LEN,
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
        if (ssz_recv > MSS_ETHERNET)
        {
            return -1;
        }
    }
    core->receive_len_ = ssz_recv;
    handle.remote_ = remote_ip;
    auto iter = core->rudp_map_.find(handle);
    if (iter != core->rudp_map_.end())
    {
        recv_rudp = iter.second();
    }
    else
    {
        *new_rudp = true;
        recv_rudp = new PEER;
        recv_rudp->hanlde_ = handle;
        recv_rudp->core
            core->rudp_map_.insert();
    }
    return 0;
}

//=================================================================================================
}