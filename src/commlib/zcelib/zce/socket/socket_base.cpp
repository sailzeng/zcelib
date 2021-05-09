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
    int ret = 0;

    //防止你干坏事，重复调用，造成资源无法释放
    assert(socket_handle_ == ZCE_INVALID_SOCKET);

    socket_handle_ = socket(family, type, protocol);
    if (ZCE_INVALID_SOCKET == socket_handle_)
    {
        int last_err = last_error();
        ZCE_LOG(RS_ERROR, "socket return fail last error %d|%s.",
                last_err,
                strerror(last_err));
        return -1;
    }

    //如果要REUSE这个地址
    if (reuse_addr)
    {
        int one = 1;
        ret = zce::setsockopt(socket_handle_,
                              SOL_SOCKET,
                              SO_REUSEADDR,
                              &one,
                              sizeof(int));

        if (ret != 0)
        {
            zce::closesocket(socket_handle_);
            return -1;
        }
    }

    return 0;
}

//Open SOCK句柄，BIND地址的方式
int Socket_Base::open(int type,
                      const Sockaddr_Base* local_addr,
                      int family,
                      int protocol,
                      bool reuse_addr)
{
    int ret = 0;

    //如果没有标注协议簇，用bind的本地地址的协议簇标识
    if (local_addr && family == AF_UNSPEC)
    {
        family = local_addr->sockaddr_ptr_->sa_family;
    }

    //如果地址协议和socket的协议簇不一样，
    if (local_addr && family != local_addr->sockaddr_ptr_->sa_family)
    {
        assert(false);
        return -1;
    }

    ret = this->open(type,
                     family,
                     protocol,
                     reuse_addr);

    //如果要绑定本地地址，一般SOCKET无须此步
    if (local_addr)
    {
        ret = Socket_Base::bind(local_addr);

        if (ret != 0)
        {
            close();
            return ret;
        }
    }

    return 0;
}

//关闭之
int Socket_Base::close()
{
    int ret = closesocket(socket_handle_);

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