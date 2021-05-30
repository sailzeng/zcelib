#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/socket/addr_base.h"
#include "zce/os_adapt/socket.h"
#include "zce/logger/logging.h"
#include "zce/socket/socket_base.h"

namespace zce
{
//构造函数
Socket_Base::Socket_Base() :
    socket_handle_(ZCE_INVALID_SOCKET)
{
}

Socket_Base::Socket_Base(const ZCE_SOCKET& socket_handle) :
    socket_handle_(socket_handle)
{
}

//析构函数，会调用close，
Socket_Base::~Socket_Base()
{
    close();
}

//
void Socket_Base::set_handle(const ZCE_SOCKET& socket_handle)
{
    socket_handle_ = socket_handle;
}

//
ZCE_SOCKET Socket_Base::get_handle() const
{
    return socket_handle_;
}

//Open SOCK句柄，不BIND本地地址的方式
int Socket_Base::open(int type,
                      int family,
                      int protocol,
                      bool reuse_addr)
{
    //防止你干坏事，重复调用，造成资源无法释放
    assert(socket_handle_ == ZCE_INVALID_SOCKET);
    int ret = zce::open_socket(&socket_handle_,
                               type,
                               family,
                               protocol,
                               reuse_addr);
    return ret;
}

//Open SOCK句柄，BIND地址的方式
int Socket_Base::open(int type,
                      const Sockaddr_Base* local_addr,
                      int protocol,
                      bool reuse_addr)
{
    assert(socket_handle_ == ZCE_INVALID_SOCKET);
    int ret = zce::open_socket(&socket_handle_,
                               type,
                               local_addr->get_addr(),
                               local_addr->get_size(),
                               protocol,
                               reuse_addr);
    return ret;
}

//关闭之
int Socket_Base::close()
{
    int ret = close_socket(socket_handle_);

    if (0 == ret)
    {
        socket_handle_ = ZCE_INVALID_SOCKET;
    }

    return ret;
}

//释放对句柄的管理，
void Socket_Base::release_noclose()
{
    socket_handle_ = ZCE_INVALID_SOCKET;
}

int Socket_Base::bind(const Sockaddr_Base* add_name) const
{
    return zce::bind(socket_handle_,
                     add_name->sockaddr_ptr_,
                     add_name->sockaddr_size_);
}

//打开某些选项，WIN32目前只支持O_NONBLOCK
int Socket_Base::sock_enable(int value) const
{
    return zce::sock_enable(socket_handle_, value);
}

//关闭某些选项，WIN32目前只支持O_NONBLOCK
int Socket_Base::sock_disable(int value) const
{
    return zce::sock_disable(socket_handle_, value);
}

//获取Socket的选项
int Socket_Base::getsockopt(int level,
                            int optname,
                            void* optval,
                            socklen_t* optlen)  const
{
    return zce::getsockopt(socket_handle_, level, optname, optval, optlen);
}

//设置Socket的选项
int Socket_Base::setsockopt(int level,
                            int optname,
                            const void* optval,
                            int optlen) const
{
    return zce::setsockopt(socket_handle_,
                           level,
                           optname,
                           optval,
                           optlen);
}

//取得对端的地址信息
int Socket_Base::getpeername(Sockaddr_Base* addr)  const
{
    return zce::getpeername(socket_handle_,
                            addr->sockaddr_ptr_,
                            &addr->sockaddr_size_);
}

//取得本地的地址信息
int Socket_Base::getsockname(Sockaddr_Base* addr)  const
{
    return zce::getsockname(socket_handle_,
                            addr->sockaddr_ptr_,
                            &addr->sockaddr_size_);
}

//connect某个地址
int Socket_Base::connect(const Sockaddr_Base* addr) const
{
    return zce::connect(socket_handle_,
                        addr->sockaddr_ptr_,
                        addr->sockaddr_size_);
}

//接收数据，根据阻塞状态决定行为
ssize_t Socket_Base::recv(void* buf,
                          size_t len,
                          int flags) const
{
    return zce::recv(socket_handle_,
                     buf,
                     len,
                     flags);
}

//发送数据，根据阻塞状态决定行为
ssize_t Socket_Base::send(const void* buf,
                          size_t len,
                          int flags) const
{
    return zce::send(socket_handle_,
                     buf,
                     len,
                     flags);
}
}