#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/socket.h"
#include "zce/os_adapt/time.h"
#include "zce/socket/addr_base.h"
#include "zce/socket/socket_base.h"
#include "zce/socket/stream.h"
#include "zce/socket/connector.h"

namespace zce
{
/************************************************************************************************************
Class           : zce::Socket_Acceptor
************************************************************************************************************/

Socket_Connector::Socket_Connector()
{
}

Socket_Connector::~Socket_Connector()
{
}


int Socket_Connector::connect (zce::Socket_Stream &new_stream,
                                   const zce::Sockaddr_Base *remote_addr,
                                   zce::Time_Value &timeout,
                                   bool reuse_addr,
                                   int protocol,
                                   const Sockaddr_Base *local_addr)
{

    int ret = 0;

    //清理最后的错误值
    zce::clear_last_error();

    //如果没有初始化
    if (ZCE_INVALID_SOCKET == new_stream.get_handle () )
    {
        if (local_addr)
        {
            ret = new_stream.open (local_addr,
                                   local_addr->sockaddr_ptr_->sa_family,
                                   protocol,
                                   reuse_addr );
        }
        else
        {
            ret = new_stream.open (remote_addr->sockaddr_ptr_->sa_family,
                                   protocol,
                                   reuse_addr );
        }

        if (ret != 0)
        {
            return ret;
        }
    }

    //不能用阻塞状态的SOCKET进行超时尝试
    ret = new_stream.sock_enable(O_NONBLOCK);

    if (ret != 0)
    {
        new_stream.close();
        return ret;
    }

    //进行连接尝试
    ret = zce::connect(new_stream.get_handle(),
                       remote_addr->sockaddr_ptr_,
                       remote_addr->sockaddr_size_);

    //
    if (ret != 0  )
    {
        //WINDOWS下返回EWOULDBLOCK，LINUX下返回EINPROGRESS
        int last_err =  zce::last_error();

        if ( EINPROGRESS != last_err &&  EWOULDBLOCK != last_err )
        {
            new_stream.close();
            return ret;
        }
    }

    //进行超时处理
    ret = zce::handle_ready(new_stream.get_handle(),
                            &timeout,
                            zce::HANDLE_READY::CONNECTED);

    const int HANDLE_READY_ONE = 1;

    if (ret != HANDLE_READY_ONE)
    {
        new_stream.close();
        return -1;
    }

    //关闭非阻塞状态
    ret = new_stream.sock_disable(O_NONBLOCK);
    if (ret != 0)
    {
        new_stream.close();
        return -1;
    }

    return 0;
}

//进行连接处理，可以进行非阻塞连接处理，
int Socket_Connector::connect (zce::Socket_Stream &new_stream,
                                   const Sockaddr_Base *remote_addr,
                                   bool non_blocing,
                                   bool reuse_addr,
                                   int protocol,
                                   const Sockaddr_Base *local_addr)
{
    int ret = 0;

    //清理最后的错误值
    zce::clear_last_error();

    //初始化Socket，如果需要绑定，会绑定IP和端口
    //如果没有初始化
    if (ZCE_INVALID_SOCKET == new_stream.get_handle () )
    {
        ret = new_stream.open (local_addr,
                               remote_addr->sockaddr_ptr_->sa_family,
                               protocol,
                               reuse_addr);

        if (ret != 0)
        {
            return ret;
        }
    }

    //进行非阻塞连接
    if (non_blocing)
    {
        ret = new_stream.sock_enable(O_NONBLOCK);

        if (ret != 0)
        {
            new_stream.close();
            return ret;
        }
    }

    //errno = 0;

    //进行连接
    ret = zce::connect(new_stream.get_handle(),
                       remote_addr->sockaddr_ptr_,
                       remote_addr->sockaddr_size_);

    //进行非阻塞的连接，一般都是返回错误。但是UNIX 网络卷一也提到了过本地连接立即返回0，我自己测试过好像都是返回-1
    if (ret != 0 )
    {
        //WINDOWS下返回EWOULDBLOCK，LINUX下返回EINPROGRESS
        int last_error = zce::last_error();

        if (non_blocing && (EINPROGRESS == last_error || EWOULDBLOCK == last_error))
        {
            //不关闭socket stream
            return -1;
        }
        else
        {
            new_stream.close();
            return ret;
        }
    }

    return 0;
}

}
