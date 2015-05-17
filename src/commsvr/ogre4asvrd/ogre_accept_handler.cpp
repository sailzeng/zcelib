#include "ogre_predefine.h"
#include "ogre_tcp_ctrl_handler.h"
#include "ogre_configure.h"
#include "ogre_ip_restrict.h"
#include "ogre_accept_handler.h"

/****************************************************************************************************
class  OgreTCPAcceptHandler TCP Accept 处理的EventHandler,
****************************************************************************************************/
Ogre_TCPAccept_Hdl::Ogre_TCPAccept_Hdl(const TCP_PEER_CONFIG_INFO &config_info,
                                       ZCE_Reactor *reactor):
    ZCE_Event_Handler(reactor),
    ip_restrict_(Ogre_IPRestrict_Mgr::instance())
{
    peer_module_info_.peer_info_ = config_info;
    ZCE_ASSERT(peer_module_info_.fp_judge_whole_frame_);
}

//自己清理的类型，统一关闭在handle_close,这个地方不用关闭
Ogre_TCPAccept_Hdl::~Ogre_TCPAccept_Hdl()
{
    peer_module_info_.close_module();
}

//
int Ogre_TCPAccept_Hdl::create_listenpeer()
{

    int ret = 0;
    ret = peer_module_info_.open_module();
    if (ret != 0)
    {
        return ret;
    }

    peer_acceptor_.sock_enable(O_NONBLOCK);

    socklen_t opval = 32 * 1024;
    socklen_t opvallen = sizeof(socklen_t);

    socklen_t sndbuflen, rcvbuflen;
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&rcvbuflen), &opvallen);
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_INFO, "Get Listen Peer SO_RCVBUF:%u SO_SNDBUF %u.\n", rcvbuflen, sndbuflen);

    //设置一个SND,RCV BUFFER,
    peer_acceptor_.setsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&opval), opvallen);
    peer_acceptor_.setsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&opval), opvallen);

    peer_acceptor_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&rcvbuflen), &opvallen);
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_INFO, "Set Listen Peer SO_RCVBUF:%u SO_SNDBUF %u.\n", rcvbuflen, sndbuflen);

    ret = peer_acceptor_.open(&peer_module_info_.peer_info_.peer_socketin_);

    //如果不能Bind相应的端口
    if (ret != 0)
    {
        int last_err = ZCE_LIB::last_error();
        ZCE_LOG(RS_ERROR, "Bind Listen IP|Port :[%s|%u] Fail.Error: %u|%s.\n",
                peer_module_info_.peer_info_.peer_socketin_.get_host_addr(),
                peer_module_info_.peer_info_.peer_socketin_.get_port_number(),
                last_err,
                strerror(last_err));
        return SOAR_RET::ERR_OGRE_INIT_LISTEN_PORT_FAIL;
    }

    ZCE_LOG(RS_INFO, "Bind listen IP|Port : [%s|%u] Success.\n",
            peer_module_info_.peer_info_.peer_socketin_.get_host_addr(),
            peer_module_info_.peer_info_.peer_socketin_.get_port_number());

    peer_acceptor_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&rcvbuflen), &opvallen);
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_INFO, "Get listen peer SO_RCVBUF:%u SO_SNDBUF %u.\n", rcvbuflen, sndbuflen);

    //设置一个SND,RCV BUFFER,
    peer_acceptor_.setsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&opval), opvallen);
    peer_acceptor_.setsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&opval), opvallen);

    peer_acceptor_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&rcvbuflen), &opvallen);
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_INFO, "Set Listen Peer SO_RCVBUF:%u SO_SNDBUF %u.\n", rcvbuflen, sndbuflen);

#ifndef WIN32
    //避免DELAY发送这种情况
    int NODELAY = 1;
    opvallen = sizeof(int);
    peer_acceptor_.setsockopt(SOL_TCP, TCP_NODELAY, reinterpret_cast<void *>(&NODELAY), opvallen);
#endif

    //
    reactor()->register_handler(this, ZCE_Event_Handler::ACCEPT_MASK);

    return 0;
}

//
int Ogre_TCPAccept_Hdl::handle_input(ZCE_HANDLE /*handle*/)
{
    ZCE_Socket_Stream  sockstream;
    ZCE_Sockaddr_In   remoteaddress;
    int ret = 0;
    ret = peer_acceptor_.accept(sockstream, &remoteaddress);

    //如果出现错误,如何处理? return -1?
    if (ret != 0)
    {

        sockstream.close();

        //记录错误
        int accept_error =  ZCE_LIB::last_error();
        ZCE_LOG(RS_ERROR, "Accept [%s|%u] handler fail! peer_acceptor_.accept ret =%d  errno=%u|%s \n",
                remoteaddress.get_host_addr(),
                remoteaddress.get_port_number(),
                ret,
                accept_error,
                strerror(accept_error));

        //如果是这些错误继续。
        if ( accept_error == EWOULDBLOCK || accept_error == EINVAL
             || accept_error == ECONNABORTED || accept_error == EPROTOTYPE )
        {
            return 0;
        }

        //这儿应该退出进程
        //return -1;
        return 0;

    }

    //如果允许的连接的服务器地址中间没有.或者在拒绝的服务列表中... kill
    ret =  ip_restrict_->check_ip_restrict(remoteaddress) ;

    if (ret != 0)
    {
        return ret;
    }

    Ogre_TCP_Svc_Handler *phandler = Ogre_TCP_Svc_Handler::alloc_svchandler_from_pool(Ogre_TCP_Svc_Handler::HANDLER_MODE_ACCEPTED);

    if (phandler != NULL)
    {
        phandler->init_tcp_svc_handler(sockstream, peer_module_info_.fp_judge_whole_frame_);
    }
    else
    {
        sockstream.close();
    }

    return 0;
}
//
ZCE_HANDLE Ogre_TCPAccept_Hdl::get_handle(void) const
{
    return (ZCE_HANDLE)peer_acceptor_.get_handle();
}



//
int Ogre_TCPAccept_Hdl::handle_close ()
{
    //
    if (peer_acceptor_.get_handle () != ZCE_INVALID_SOCKET)
    {
        reactor()->remove_handler (this, true);
        peer_acceptor_.close ();
    }

    //删除自己
    delete this;

    return 0;
}

