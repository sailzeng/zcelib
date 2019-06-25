#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_socket.h"
#include "zce_os_adapt_time.h"
#include "zce_socket_addr_base.h"
#include "zce_socket_base.h"
#include "zce_socket_stream.h"
#include "zce_socket_connector.h"

/************************************************************************************************************
Class           : ZCE_Socket_Acceptor
************************************************************************************************************/

ZCE_Socket_Connector::ZCE_Socket_Connector()
{
}

ZCE_Socket_Connector::~ZCE_Socket_Connector()
{
}


int ZCE_Socket_Connector::connect (ZCE_Socket_Stream &new_stream,
                                   const ZCE_Sockaddr *remote_addr,
                                   ZCE_Time_Value &timeout,
                                   bool reuse_addr,
                                   int protocol,
                                   const ZCE_Sockaddr *local_addr)
{

    int ret = 0;

    //清理最后的错误值
    ZCE_LIB::clear_last_error();

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
    ret = ZCE_LIB::connect(new_stream.get_handle(),
                           remote_addr->sockaddr_ptr_,
                           remote_addr->sockaddr_size_);

    //
    if (ret != 0  )
    {
        //WINDOWS下返回EWOULDBLOCK，LINUX下返回EINPROGRESS
        int last_err =  ZCE_LIB::last_error();

        if ( EINPROGRESS != last_err &&  EWOULDBLOCK != last_err )
        {
            new_stream.close();
            return ret;
        }
    }

    //进行超时处理
    ret = ZCE_LIB::handle_ready(new_stream.get_handle(),
                                &timeout,
                                ZCE_LIB::HANDLE_READY_CONNECTED);

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
int ZCE_Socket_Connector::connect (ZCE_Socket_Stream &new_stream,
                                   const ZCE_Sockaddr *remote_addr,
                                   bool non_blocing,
                                   bool reuse_addr,
                                   int protocol,
                                   const ZCE_Sockaddr *local_addr)
{
    int ret = 0;

    //清理最后的错误值
    ZCE_LIB::clear_last_error();

    zcecket，如果需要绑定，会绑定IP和端口
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
    ret = ZCE_LIB::connect(new_stream.get_handle(),
                           remote_addr->sockaddr_ptr_,
                           zce          remote_addr->sockaddr_size_);

    //进行非阻塞的连接，一般都是返回错误。但是UNIX 网络卷一也提到了过本地连接立即返回0，我自己测试过好像都是返回-1
    if (ret != 0 )
    {
        //WINDOWS下返回EWOULDBLOCK，LINUX下返回EINPROGRESS
        int last_error = ZCE_LIB::last_error();

        if ( non_blocing zceNPROGRESS == last_error ||  EWOULDBLOCK == last_error ) )
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

