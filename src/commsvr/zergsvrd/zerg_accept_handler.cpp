#include "zerg_predefine.h"
#include "zerg_accept_handler.h"
#include "zerg_tcp_ctrl_handler.h"
#include "zerg_ip_restrict.h"



//TCP Accept 处理的EventHandler,
TCP_Accept_Handler::TCP_Accept_Handler(const SERVICES_ID &svcid,
                                       const ZCE_Sockaddr_In &addr):
    ZCE_Event_Handler(ZCE_Reactor::instance()),
    my_svc_info_(svcid),
    accept_bind_addr_(addr),
    ip_restrict_(Zerg_IPRestrict_Mgr::instance())
{
}

//自己清理的类型，统一关闭在handle_close,这个地方不用关闭
TCP_Accept_Handler::~TCP_Accept_Handler()
{
}

//创建监听端口
int TCP_Accept_Handler::create_listen()
{
    //直接把backlog干上一个很大的值
    const int DEF_ZERG_BACKLOG = 512;
    //
    const socklen_t opval = ZERG_SND_RCV_BUF_OPVAL;
    socklen_t opvallen = sizeof(socklen_t);

    //
    socklen_t sndbuflen = 0, rcvbuflen = 0;
    int ret = peer_acceptor_.open(&accept_bind_addr_, true, AF_UNSPEC, DEF_ZERG_BACKLOG);

    //如果不能Bind相应的端口
    if (ret != 0)
    {

        ZCE_LOG(RS_ERROR, "[zergsvr] Bind Listen IP|Port :[%s|%u] Fail.Error: %d|%s.",
                accept_bind_addr_.get_host_addr(),
                accept_bind_addr_.get_port_number(),
                ZCE_LIB::last_error(),
                strerror(ZCE_LIB::last_error()));
        return SOAR_RET::ERR_ZERG_INIT_LISTEN_PORT_FAIL;
    }

    peer_acceptor_.sock_enable(O_NONBLOCK);

    ZCE_LOG(RS_INFO, "[zergsvr] Bind Listen IP|Port : [%s|%u] Success.",
            accept_bind_addr_.get_host_addr(),
            accept_bind_addr_.get_port_number());

    //被Accept的端口会继承这些选项
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&rcvbuflen), &opvallen);
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_INFO, "[zergsvr] Get Listen Peer SO_RCVBUF:%u SO_SNDBUF %u.", rcvbuflen, sndbuflen);

    //设置一个SND,RCV BUFFER,
    peer_acceptor_.setsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const void *>(&opval), opvallen);
    peer_acceptor_.setsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const void *>(&opval), opvallen);

    peer_acceptor_.getsockopt(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<void *>(&rcvbuflen), &opvallen);
    peer_acceptor_.getsockopt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void *>(&sndbuflen), &opvallen);
    ZCE_LOG(RS_INFO, "[zergsvr] Set Listen Peer SO_RCVBUF:%u SO_SNDBUF %u.", rcvbuflen, sndbuflen);

#ifndef ZCE_OS_WINDOWS

    //避免DELAY发送这种情况
    int NODELAY = 1;
    opvallen = sizeof(int);
    peer_acceptor_.setsockopt(SOL_TCP, TCP_NODELAY, reinterpret_cast<void *>(&NODELAY), opvallen);

    //TCP_DEFER_ACCEPT,这个选项我暂时不开，但是这个选项是一个很好的避免攻击的手段。
    //int val = 1;
    //ret = peer_acceptor_.set_option(IPPROTO_TCP, TCP_DEFER_ACCEPT, &val, sizeof(val));
    //ZCE_LOG(RS_INFO,"[zergsvr]  setsockopt TCP_DEFER_ACCEPT val(%d) ret(%d). ", val, ret);

#endif

    //
    reactor()->register_handler(this, ZCE_Event_Handler::ACCEPT_MASK);

    return 0;
}

//事件触发处理，表示有一个accept 的数据
int TCP_Accept_Handler::handle_input(/*handle*/)
{
    ZCE_Socket_Stream  sockstream;
    ZCE_Sockaddr_In       remote_address;
    int ret = peer_acceptor_.accept(sockstream, &remote_address);

    //如果出现错误,如何处理? return -1?
    if (ret != 0)
    {
        //由于该死的C/C++的返回静态指针的问题，这儿要输出两个地址，所以只能先打印到其他地方
        const size_t TMP_ADDR_LEN = 64;
        char str_local_addr[TMP_ADDR_LEN], str_remote_addr[TMP_ADDR_LEN];

        int accept_error =  ZCE_LIB::last_error();
        ZCE_LOG(RS_ERROR, "[zergsvr] Local peer[%s|%u] Accept remote [%s|%u] handler fail! peer_acceptor_.accept ret =%d  errno=%d|%s ",
                accept_bind_addr_.get_host_addr(str_local_addr, TMP_ADDR_LEN),
                accept_bind_addr_.get_port_number(),
                remote_address.get_host_addr(str_remote_addr, TMP_ADDR_LEN),
                remote_address.get_port_number(),
                ret,
                accept_error,
                strerror(accept_error));
        sockstream.close();

        //如果是这些错误继续。
        if ( accept_error == EWOULDBLOCK || accept_error == EINVAL
             || accept_error == ECONNABORTED || accept_error == EPROTOTYPE )
        {

            return 0;
        }

        //这儿应该退出进程???,还是继续把。哈哈。
        //return -1;
        return 0;

    }

    ret = ip_restrict_->check_iprestrict(remote_address);

    if (ret != 0)
    {
        sockstream.close();
        return 0;
    }

    TCP_Svc_Handler *phandler = TCP_Svc_Handler::alloce_hdl_from_pool(TCP_Svc_Handler::HANDLER_MODE_ACCEPTED);

    if (phandler != NULL)
    {
        phandler->init_tcpsvr_handler(my_svc_info_, sockstream, remote_address);
        //避免析构的时候close句柄
        sockstream.release_noclose();
    }
    else
    {
        sockstream.close();
    }

    return 0;
}

//返回句柄ID
ZCE_HANDLE TCP_Accept_Handler::get_handle(void) const
{
    return (ZCE_HANDLE)peer_acceptor_.get_handle();
}

//退出处理
int TCP_Accept_Handler::handle_close ()
{
    //
    if (peer_acceptor_.get_handle () != ZCE_INVALID_SOCKET)
    {
        reactor()->remove_handler (this, false);
        peer_acceptor_.close ();
    }

    //删除自己
    delete this;

    return 0;
}

