#include "zce/predefine.h"
#include "zce/rudp/rudp.h"

namespace zce::rudp
{
int core_init(CORE *core,
              size_t /*send_pool_num*/,
              size_t /*recv_pool_num*/,
              int family)
{
    core->family_ = family;
    return 0;
}

int core_recieve(CORE * /*core*/,
                 ZCE_SOCKET socket,
                 RUDP *& /*rudp*/,
                 RUDP */*new_rudp*/)
{
    HANDLE handle;
    handle.udp_socket_ = socket;
    int ret = 0;
    zce::sockaddr_ip local_ip;
    socklen_t sz = sizeof(zce::sockaddr_ip);
    ret = zce::getsockname(socket, (sockaddr*)&local_ip, &sz);
    if (ret)
    {
        return ret;
    }
    handle.local_ = local_ip;
    //zce::sockaddr_ip remote_ip;
    //ret = zce::recvfrom(socket, )
    return 0;
}
}