#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/socket/addr_base.h"
#include "zce/os_adapt/socket.h"
#include "zce/logger/logging.h"
#include "zce/socket/socket_base.h"

namespace zce::skt
{
//构造函数
socket_base::socket_base() :
    socket_handle_(ZCE_INVALID_SOCKET)
{
}

socket_base::socket_base(const ZCE_SOCKET& socket_handle) :
    socket_handle_(socket_handle)
{
}

//析构函数，会调用close，
socket_base::~socket_base()
{
    close();
}

//
void socket_base::set_handle(const ZCE_SOCKET& socket_handle)
{
    socket_handle_ = socket_handle;
}

//
ZCE_SOCKET socket_base::get_handle() const
{
    return socket_handle_;
}

//Open SOCK句柄，不BIND本地地址的方式
int socket_base::open(int type,
                      int family,
                      bool nonblock,
                      int protocol,
                      bool reuse_addr)
{
    //防止你干坏事，重复调用，造成资源无法释放
    assert(socket_handle_ == ZCE_INVALID_SOCKET);
    int ret = zce::open_socket(&socket_handle_,
                               type,
                               family,
                               nonblock,
                               protocol,
                               reuse_addr);
    return ret;
}

//Open SOCK句柄，BIND地址的方式
int socket_base::open(int type,
                      const zce::skt::addr_base* local_addr,
                      bool nonblock,
                      int protocol,
                      bool reuse_addr)
{
    assert(socket_handle_ == ZCE_INVALID_SOCKET);
    int ret = zce::open_socket(&socket_handle_,
                               type,
                               local_addr->get_addr(),
                               local_addr->get_size(),
                               nonblock,
                               protocol,
                               reuse_addr);
    return ret;
}

//关闭之
int socket_base::close()
{
    int ret = close_socket(socket_handle_);

    if (0 == ret)
    {
        socket_handle_ = ZCE_INVALID_SOCKET;
    }

    return ret;
}

//释放对句柄的管理，
void socket_base::release_noclose()
{
    socket_handle_ = ZCE_INVALID_SOCKET;
}

int socket_base::bind(const zce::skt::addr_base* add_name) const
{
    return zce::bind(socket_handle_,
                     add_name->sockaddr_ptr_,
                     add_name->sockaddr_size_);
}

//打开某些选项，WIN32目前只支持O_NONBLOCK
int socket_base::sock_enable(int value) const
{
    return zce::sock_enable(socket_handle_, value);
}

//关闭某些选项，WIN32目前只支持O_NONBLOCK
int socket_base::sock_disable(int value) const
{
    return zce::sock_disable(socket_handle_, value);
}

//获取Socket的选项
int socket_base::getsockopt(int level,
                            int optname,
                            void* optval,
                            socklen_t* optlen)  const
{
    return zce::getsockopt(socket_handle_, level, optname, optval, optlen);
}

//设置Socket的选项
int socket_base::setsockopt(int level,
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
int socket_base::getpeername(zce::skt::addr_base* addr)  const
{
    return zce::getpeername(socket_handle_,
                            addr->sockaddr_ptr_,
                            &addr->sockaddr_size_);
}

//取得本地的地址信息
int socket_base::getsockname(zce::skt::addr_base* addr)  const
{
    return zce::getsockname(socket_handle_,
                            addr->sockaddr_ptr_,
                            &addr->sockaddr_size_);
}

//connect某个地址
int socket_base::connect(const zce::skt::addr_base* addr) const
{
    return zce::connect(socket_handle_,
                        addr->sockaddr_ptr_,
                        addr->sockaddr_size_);
}

//接收数据，根据阻塞状态决定行为
ssize_t socket_base::recv(void* buf,
                          size_t len,
                          int flags) const
{
    return zce::recv(socket_handle_,
                     buf,
                     len,
                     flags);
}

//发送数据，根据阻塞状态决定行为
ssize_t socket_base::send(const void* buf,
                          size_t len,
                          int flags) const
{
    return zce::send(socket_handle_,
                     buf,
                     len,
                     flags);
}
}