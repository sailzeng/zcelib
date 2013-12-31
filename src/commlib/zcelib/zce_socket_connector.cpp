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

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011年6月21日
Function        :
Return          : int
Parameter List  :
  Param1: ZCE_Socket_Stream& new_stream
  Param2: const sockaddr* remote_addr
  Param3: socklen_t remote_addr_len
  Param4: const timeval& timeout
  Param5: int reuse_addr
  Param6: int protocol
  Param7: const sockaddr* local_addr
Description     : 进行超时连接处理
Calls           :
Called By       :
Other           :
Modify Record   :
************************************************************************************************************/
int ZCE_Socket_Connector::connect (ZCE_Socket_Stream &new_stream,
                                   const ZCE_Sockaddr *remote_addr ,
                                   ZCE_Time_Value &timeout,
                                   bool reuse_addr ,
                                   int protocol ,
                                   const ZCE_Sockaddr *local_addr)
{

    int ret = 0;

    //清理最后的错误值
    ZCE_OS::clear_last_error();

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
    ret = ZCE_OS::connect(new_stream.get_handle(),
                          remote_addr->sockaddr_ptr_,
                          remote_addr->sockaddr_size_);

    //
    if (ret != 0  )
    {
        //WINDOWS下返回EWOULDBLOCK，LINUX下返回EINPROGRESS
        int last_err =  ZCE_OS::last_error();

        if ( EINPROGRESS != last_err &&  EWOULDBLOCK != last_err )
        {
            new_stream.close();
            return ret;
        }
    }

    //进行超时处理
    ret = ZCE_OS::handle_ready(new_stream.get_handle(),
                               &timeout,
                               ZCE_OS::HANDLE_READY_CONNECT_SUCCESS);

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

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011年6月21日
Function        :
Return          : int
Parameter List  :
  Param1: ZCE_Socket_Stream& new_stream 连接的SOCKET 句柄类
  Param2: const sockaddr* remote_addr   远端的地址，实际结构是sockadd_in，sockadd_in6等
  Param3: socklen_t remote_addr_len 地址的长度，表示前面的类型是什么，C语言做法
  Param4: bool non_blocing 是否阻塞进行连接。
  Param5: int reuse_addr   是否置为reuse_addr选项
  Param6: int protocol     协议，用于RAW Socket，基本没用
  Param7: const sockaddr* local_addr 本地的BIND地址信息，CONNECT基本没用
Description     : 进行连接处理，可以进行非阻塞连接处理，
Calls           :
Called By       :
Other           :
Modify Record   :
************************************************************************************************************/
int ZCE_Socket_Connector::connect (ZCE_Socket_Stream &new_stream,
                                   const ZCE_Sockaddr *remote_addr ,
                                   bool non_blocing,
                                   bool reuse_addr,
                                   int protocol,
                                   const ZCE_Sockaddr *local_addr)
{
    int ret = 0;

    //清理最后的错误值
    ZCE_OS::clear_last_error();

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
    ret = ZCE_OS::connect(new_stream.get_handle(),
                          remote_addr->sockaddr_ptr_,
                          remote_addr->sockaddr_size_);

    //进行非阻塞的连接，一般都是返回错误。但是UNIX 网络卷一也提到了过本地连接立即返回0，我自己测试过好像都是返回-1
    if (ret != 0 )
    {
        //WINDOWS下返回EWOULDBLOCK，LINUX下返回EINPROGRESS
        int last_error = ZCE_OS::last_error();

        if ( non_blocing && ( EINPROGRESS == last_error ||  EWOULDBLOCK == last_error ) )
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

