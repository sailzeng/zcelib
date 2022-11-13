#include "ogre/predefine.h"
#include "ogre/svc_tcp.h"
#include "ogre/configure.h"
#include "ogre/ip_restrict.h"
#include "ogre/svc_accept.h"

namespace ogre
{
svc_accept::svc_accept(const TCP_PEER_CONFIG_INFO& config_info,
                       zce::reactor* reactor) :
    zce::event_handler(reactor),
    ip_restrict_(ip_restrict::instance())
{
    peer_module_info_.peer_info_ = config_info;
    ZCE_ASSERT(peer_module_info_.fp_judge_whole_frame_);
}

//自己清理的类型，统一关闭在event_close,这个地方不用关闭
svc_accept::~svc_accept()
{
    peer_module_info_.close_module();
}

//
int svc_accept::create_listenpeer()
{
    int ret = 0;

    const size_t IP_ADDR_LEN = 31;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;

    ret = peer_module_info_.open_module();
    if (ret != 0)
    {
        return ret;
    }

    peer_acceptor_.sock_enable(O_NONBLOCK);

    socklen_t opval = 32 * 1024;
    socklen_t opvallen = sizeof(socklen_t);

    socklen_t sndbuflen, rcvbuflen;
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void*>(&rcvbuflen), &opvallen);
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void*>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_INFO, "Get Listen Peer SO_RCVBUF:%u SO_SNDBUF %u.\n", rcvbuflen, sndbuflen);

    //设置一个SND,RCV BUFFER,
    peer_acceptor_.setsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void*>(&opval), opvallen);
    peer_acceptor_.setsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void*>(&opval), opvallen);

    peer_acceptor_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void*>(&rcvbuflen), &opvallen);
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void*>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_INFO, "Set Listen Peer SO_RCVBUF:%u SO_SNDBUF %u.\n", rcvbuflen, sndbuflen);

    ret = peer_acceptor_.open(&peer_module_info_.peer_info_.peer_socketin_);

    //如果不能Bind相应的端口
    if (ret != 0)
    {
        int last_err = zce::last_error();
        ZCE_LOG(RS_ERROR, "Bind Listen IP|Port :[%s] Fail.Error: %u|%s.\n",
                peer_module_info_.peer_info_.peer_socketin_.to_string(ip_addr_str, IP_ADDR_LEN, use_len),
                last_err,
                strerror(last_err));
        return SOAR_RET::ERR_OGRE_INIT_LISTEN_PORT_FAIL;
    }

    ZCE_LOG(RS_INFO, "Bind listen IP|Port : [%s|%u] Success.\n",
            peer_module_info_.peer_info_.peer_socketin_.to_string(ip_addr_str, IP_ADDR_LEN, use_len));

    peer_acceptor_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void*>(&rcvbuflen), &opvallen);
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void*>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_INFO, "Get listen peer SO_RCVBUF:%u SO_SNDBUF %u.\n", rcvbuflen, sndbuflen);

    //设置一个SND,RCV BUFFER,
    peer_acceptor_.setsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void*>(&opval), opvallen);
    peer_acceptor_.setsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void*>(&opval), opvallen);

    peer_acceptor_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void*>(&rcvbuflen), &opvallen);
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void*>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_INFO, "Set Listen Peer SO_RCVBUF:%u SO_SNDBUF %u.\n", rcvbuflen, sndbuflen);

#ifndef WIN32
    //避免DELAY发送这种情况
    int NODELAY = 1;
    opvallen = sizeof(int);
    peer_acceptor_.setsockopt(SOL_TCP, TCP_NODELAY, reinterpret_cast<void*>(&NODELAY), opvallen);
#endif

    //
    reactor()->register_handler(this, zce::ACCEPT_MASK);

    return 0;
}

//
void svc_accept::accept_event()
{
    const size_t IP_ADDR_LEN = 31;
    char ip_addr_str[IP_ADDR_LEN + 1];
    size_t use_len = 0;

    zce::skt::stream  sockstream;
    zce::skt::addr_in   remoteaddress;
    int ret = 0;
    ret = peer_acceptor_.accept(sockstream, &remoteaddress);

    //如果出现错误,如何处理? return -1?
    if (ret != 0)
    {
        sockstream.close();

        //记录错误
        int accept_error = zce::last_error();
        ZCE_LOG(RS_ERROR, "Accept [%s] handler fail! peer_acceptor_.accept ret =%d  errno=%u|%s \n",
                remoteaddress.to_string(ip_addr_str, IP_ADDR_LEN, use_len),
                ret,
                accept_error,
                strerror(accept_error));

        //如果是这些错误继续。
        if (accept_error == EWOULDBLOCK || accept_error == EINVAL
            || accept_error == ECONNABORTED || accept_error == EPROTOTYPE)
        {
            return;
        }

        //这儿应该退出进程
        return;
    }

    //如果允许的连接的服务器地址中间没有.或者在拒绝的服务列表中... kill
    ret = ip_restrict_->check_ip_restrict(remoteaddress);
    if (ret != 0)
    {
        sockstream.close();
        return;
    }

    svc_tcp* phandler = svc_tcp::alloc_svchandler_from_pool(svc_tcp::HANDLER_MODE_ACCEPTED);

    if (phandler != nullptr)
    {
        phandler->init_tcp_svc_handler(std::move(sockstream),
                                       peer_module_info_.fp_judge_whole_frame_);
    }
    else
    {
        sockstream.close();
    }

    return;
}
//
ZCE_HANDLE svc_accept::get_handle(void) const
{
    return (ZCE_HANDLE)peer_acceptor_.get_handle();
}

//
void svc_accept::close_handle()
{
    //
    if (peer_acceptor_.get_handle() != ZCE_INVALID_SOCKET)
    {
        reactor()->remove_handler(this, true);
        peer_acceptor_.close();
    }

    //删除自己
    delete this;

    return;
}
}