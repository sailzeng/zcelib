#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/common.h"
#include "zce/os_adapt/error.h"
#include "zce/time/time_value.h"
#include "zce/os_adapt/socket.h"
#include "zce/bytes/hash_value.h"

namespace zce
{
//===============================================
//sockaddr_ip，兼容sockaddr_in sockaddr_in6的地址信息

sockaddr_any::sockaddr_any()
{
    memset(&in6_, 0, sizeof(sockaddr_in6));
}
sockaddr_any::~sockaddr_any()
{
}

sockaddr_any::sockaddr_any(const ::sockaddr_in& sa)
{
    ::memcpy(&in_, &sa, sizeof(sockaddr_in));
}

sockaddr_any::sockaddr_any(const ::sockaddr_in6& sa)
{
    ::memcpy(&in6_, &sa, sizeof(sockaddr_in6));
}

sockaddr_any::sockaddr_any(const ::sockaddr* sa, socklen_t sa_len)
{
    set(sa, sa_len);
}

bool sockaddr_any::operator == (const sockaddr_any& others) const
{
    const sockaddr* addr_ptr = (const sockaddr*)&others;
    if (addr_ptr->sa_family == AF_INET)
    {
        //避免比较sin_zero一类数据
        const sockaddr_in* s = (sockaddr_in*)this;
        const sockaddr_in* o = (sockaddr_in*)&others;
        //比较地址协议簇，地址，端口
        if (o->sin_family == s->sin_family &&
            o->sin_addr.s_addr == s->sin_addr.s_addr &&
            o->sin_port == s->sin_port)
        {
            return true;
        }
    }
    else if (addr_ptr->sa_family == AF_INET6)
    {
        const sockaddr_in6* s = (sockaddr_in6*)this;
        const sockaddr_in6* o = (sockaddr_in6*)&others;
        //比较地址协议簇，地址，端口
        if (o->sin6_family == s->sin6_family &&
            0 == ::memcmp(&(o->sin6_addr), &(s->sin6_addr), sizeof(in6_addr)) &&
            o->sin6_port == s->sin6_port)
        {
            return true;
        }
    }
    else
    {
        assert(false);
    }
    return false;
}

sockaddr_any& sockaddr_any::operator = (const ::sockaddr* sa)
{
    if (sa->sa_family == AF_INET)
    {
        ::memcpy(&in_, sa, sizeof(sockaddr_in));
    }
    else if (sa->sa_family == AF_INET6)
    {
        ::memcpy(&in6_, sa, sizeof(sockaddr_in6));
    }
    else
    {
        assert(false);
    }
    return *this;
}
sockaddr_any& sockaddr_any::operator = (const ::sockaddr_in& sa)
{
    ::memcpy(&this->in_, &sa, sizeof(sockaddr_in));
    return *this;
}
sockaddr_any& sockaddr_any::operator = (const ::sockaddr_in6& sa)
{
    ::memcpy(&this->in6_, &sa, sizeof(sockaddr_in6));
    return *this;
}

void sockaddr_any::set(const ::sockaddr* sa, socklen_t sa_len)
{
    if (sa_len == sizeof(::sockaddr_in))
    {
        ::memcpy(&in_, sa, sizeof(::sockaddr_in));
        in_.sin_family = AF_INET;
        in_.sin_port = 0;
    }
    else if (sa_len == sizeof(::sockaddr_in6))
    {
        ::memcpy(&in6_, sa, sizeof(::sockaddr_in6));
        in6_.sin6_family = AF_INET6;
        in6_.sin6_port = 0;
    }
    else
    {
        assert(false);
    }
}

void sockaddr_any::set_family(int family)
{
    in_.sin_family = (ADDRESS_FAMILY)family;
}

int sockaddr_any::get_family() const
{
    //因为用的是union，所以in_.sin_family和 in6_.sin6_family 一个德行
    return in_.sin_family;
}

size_t sockaddr_ip_hash::operator()(const zce::sockaddr_any& s) const
{
    const sockaddr* addr_ptr = (const sockaddr*)&s;
    if (addr_ptr->sa_family == AF_INET)
    {
        return zce::hash_js((char*)addr_ptr, sizeof(sockaddr_in));
    }
    else if (addr_ptr->sa_family == AF_INET6)
    {
        return zce::hash_js((char*)addr_ptr, sizeof(sockaddr_in6));
    }
    else
    {
        assert(false);
    }
    return 0;
}

//===============================================

//初始化Socket，
int socket_init(int version_high, int version_low)
{
#if defined (ZCE_OS_WINDOWS)

    WORD version_requested = MAKEWORD(version_high, version_low);
    WSADATA wsa_data;
    ::WSASetLastError(0);
    int error = ::WSAStartup(version_requested, &wsa_data);

    if (error != 0)
    {
        errno = ::WSAGetLastError();
        ::fprintf(stderr,
                  "zce::socket_init; WSAStartup failed, "
                  "WSAGetLastError returned %d\n",
                  errno);
        WSACleanup();
        return -1;
    }
    return 0;
#else
    ZCE_UNUSED_ARG(version_high);
    ZCE_UNUSED_ARG(version_low);
    return 0;
#endif
}

//打开socket 句柄，简化处理的函数，非标准，通常用于客户端本地端口
int open_socket(ZCE_SOCKET* handle,
                int type,
                int family,
                int protocol,
                bool reuse_addr)
{
    int ret = 0;

    *handle = socket(family, type, protocol);
    if (ZCE_INVALID_SOCKET == *handle)
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
        ret = zce::setsockopt(*handle,
                              SOL_SOCKET,
                              SO_REUSEADDR,
                              &one,
                              sizeof(int));

        if (ret != 0)
        {
            int last_err = last_error();
            ZCE_LOG(RS_ERROR, "setsockopt return fail last error %d|%s.",
                    last_err,
                    strerror(last_err));
            zce::close_socket(*handle);
            return -1;
        }
    }
    return 0;
}

//打开socket 句柄，同时绑定本地地址，简化处理的函数，非标准，通常用于监听端口
int zce::open_socket(ZCE_SOCKET* handle,
                     int type,
                     const sockaddr* local_addr,
                     socklen_t addr_len,
                     int protocol,
                     bool reuse_addr)
{
    int ret = 0;

    //如果没有标注协议簇，用bind的本地地址的协议簇标识
    int family = local_addr->sa_family;
    ret = zce::open_socket(handle,
                           type,
                           family,
                           protocol,
                           reuse_addr);

    //如果要绑定本地地址，一般SOCKET无须此步
    if (local_addr)
    {
        ret = zce::bind(*handle, local_addr, addr_len);
        if (ret != 0)
        {
            close_socket(*handle);
            return ret;
        }
    }

    return 0;
}

//关闭Socket的支持
int socket_terminate(void)
{
#if defined (ZCE_OS_WINDOWS)

    if (::WSACleanup() != 0)
    {
        errno = ::WSAGetLastError();

        ::fprintf(stderr,
                  "zce::socket_fini; WSACleanup failed, "
                  "WSAGetLastError returned %d\n",
                  errno);
    }

#endif
    return 0;
}

//
ssize_t writev(ZCE_SOCKET handle,
               const iovec* buffers,
               int iovcnt)
{
#if defined (ZCE_OS_WINDOWS)

    assert(iovcnt <= IOV_MAX);

    WSABUF wsa_buf[IOV_MAX];

    for (int i = 0; i < iovcnt; ++i)
    {
        wsa_buf[i].buf = static_cast<CHAR*>(buffers[i].iov_base);
        wsa_buf[i].len = static_cast<ULONG>(buffers[i].iov_len);
    }

    DWORD bytes_sent = 0;
    int zce_result = ::WSASend((SOCKET)handle,
                               wsa_buf,
                               iovcnt,
                               &bytes_sent,
                               0,
                               0,
                               0);

    if (zce_result == SOCKET_ERROR)
    {
        errno = ::WSAGetLastError();
        return -1;
    }

    return (ssize_t)bytes_sent;

#elif defined (ZCE_OS_LINUX)

    return ::writev(handle, buffers, iovcnt);
#endif
}

//
ssize_t readv(ZCE_SOCKET handle,
              iovec* buffers,
              int iovcnt)
{
#if defined (ZCE_OS_WINDOWS)

    DWORD bytes_received = 0;
    int result = 1;

    // Winsock 2 has WSARecv and can do this directly, but Winsock 1 needs
    // to do the recvs piece-by-piece.

    //IOV_MAX根据各个平台不太一样
    assert(iovcnt <= IOV_MAX);

    WSABUF wsa_buf[IOV_MAX];

    for (int i = 0; i < iovcnt; ++i)
    {
        wsa_buf[i].buf = static_cast<CHAR*>(buffers[i].iov_base);
        wsa_buf[i].len = static_cast<ULONG>(buffers[i].iov_len);
    }

    DWORD flags = 0;
    result = ::WSARecv(handle,
                       wsa_buf,
                       iovcnt,
                       &bytes_received,
                       &flags,
                       0,
                       0);

    if (result == SOCKET_ERROR)
    {
        errno = ::WSAGetLastError();
        return -1;
    }

    return (ssize_t)bytes_received;

#elif defined (ZCE_OS_LINUX)

    return ::readv(handle, buffers, iovcnt);

#endif
}

//
ssize_t recvmsg(ZCE_SOCKET handle,
                msghdr* msg,
                int flags)
{
#if defined (ZCE_OS_WINDOWS)

    assert(msg->msg_iovlen < IOV_MAX);

    WSABUF wsa_buf[IOV_MAX];

    for (size_t i = 0; i < msg->msg_iovlen; ++i)
    {
        wsa_buf[i].buf = static_cast<CHAR*>(msg->msg_iov[i].iov_base);
        wsa_buf[i].len = static_cast<ULONG>(msg->msg_iov[i].iov_len);
    }

    DWORD bytes_received = 0;

    int result = ::WSARecvFrom((SOCKET)handle,
                               wsa_buf,
                               (DWORD)(msg->msg_iovlen),
                               &bytes_received,
                               (DWORD*)&flags,
                               static_cast<sockaddr*>(msg->msg_name),
                               &msg->msg_namelen,
                               0,
                               0);

    if (result != 0)
    {
        errno = ::WSAGetLastError();
        return -1;
    }

    return bytes_received;

#elif defined (ZCE_OS_LINUX)

    return ::recvmsg(handle, msg, flags);

#endif
}

ssize_t sendmsg(ZCE_SOCKET handle,
                const struct msghdr* msg,
                int flags)
{
#if defined (ZCE_OS_WINDOWS)

    //
    assert(msg->msg_iovlen < IOV_MAX);

    WSABUF wsa_buf[IOV_MAX];

    for (size_t i = 0; i < msg->msg_iovlen; ++i)
    {
        wsa_buf[i].buf = static_cast<CHAR*>(msg->msg_iov[i].iov_base);
        wsa_buf[i].len = static_cast<ULONG>(msg->msg_iov[i].iov_len);
    }

    DWORD bytes_sent = 0;
    int result = ::WSASendTo((SOCKET)handle,
                             wsa_buf,
                             (DWORD)msg->msg_iovlen,
                             &bytes_sent,
                             flags,
                             static_cast<const sockaddr*>(msg->msg_name),
                             msg->msg_namelen,
                             0,
                             0);
    if (result != 0)
    {
        errno = ::WSAGetLastError();
        return -1;
    }

    return (ssize_t)bytes_sent;

#elif defined (ZCE_OS_LINUX)
    //
    return ::sendmsg(handle, msg, flags);
# endif
}

//--------------------------------------------------------------------------------------------
//尽量收取len个数据，直到出现错误
ssize_t recvn(ZCE_SOCKET handle,
              void* buf,
              size_t len,
              int flags)
{
    ssize_t result = 0;
    bool error_occur = false;

    ssize_t onetime_recv = 0, bytes_recv = 0;

    //一定准备确保收到这么多字节
    for (bytes_recv = 0; static_cast<size_t>(bytes_recv) < len; bytes_recv += onetime_recv)
    {
        //使用端口进行接收
        onetime_recv = zce::recv(handle,
                                 static_cast <char*> (buf) + bytes_recv,
                                 len - bytes_recv,
                                 flags);

        if (onetime_recv > 0)
        {
            continue;
        }
        //如果出现错误,退出循环
        else
        {
            //出现错误，进行处理
            error_occur = true;
            result = onetime_recv;
            break;
        }
    }

    //如果发生错误
    if (error_occur)
    {
        return result;
    }

    return bytes_recv;
}

//尽量发送N个数据，直到出现错误
ssize_t sendn(ZCE_SOCKET handle,
              const void* buf,
              size_t len,
              int flags)
{
    bool error_occur = false;
    ssize_t result = 0, bytes_send = 0, onetime_send = 0;

    //一定准备确保收到这么多字节，但是一旦出现错误，就退出
    for (bytes_send = 0; static_cast<size_t>(bytes_send) < len; bytes_send += onetime_send)
    {
        //发送数据，，
        onetime_send = zce::send(handle,
                                 static_cast <const char*> (buf) + bytes_send,
                                 len - bytes_send,
                                 flags);

        if (onetime_send > 0)
        {
            continue;
        }
        //如果出现错误,== 0一般是是端口断开，==-1表示错误
        else
        {
            //出现错误，进行处理
            error_occur = true;
            result = onetime_send;
            break;
        }
    }

    //发送了错误，返回错误返回值
    if (error_occur)
    {
        return result;
    }

    return bytes_send;
}

//打开某些选项，WIN32目前只支持O_NONBLOCK
int sock_enable(ZCE_SOCKET handle, int flags)
{
#if defined (ZCE_OS_WINDOWS)

    switch (flags)
    {
    case O_NONBLOCK:
        // nonblocking argument (1)
        // blocking:            (0)
    {
        u_long nonblock = 1;
        int zce_result = ::ioctlsocket(handle, FIONBIO, &nonblock);

        //将错误信息设置到errno，详细请参考上面zce名字空间后面的解释
        if (SOCKET_ERROR == zce_result)
        {
            errno = ::WSAGetLastError();
        }

        return zce_result;
    }
    default:
    {
        return (-1);
    }
    }

#elif defined (ZCE_OS_LINUX)
    int val = ::fcntl(handle, F_GETFL, 0);

    if (val == -1)
    {
        return -1;
    }

    // Turn on flags.
    ZCE_SET_BITS(val, flags);

    if (::fcntl(handle, F_SETFL, val) == -1)
    {
        return -1;
    }

    return 0;
#endif
}

//关闭某些选项，WIN32目前只支持O_NONBLOCK
int sock_disable(ZCE_SOCKET handle, int flags)
{
#if defined (ZCE_OS_WINDOWS)

    switch (flags)
    {
    case O_NONBLOCK:
        // nonblocking argument (1)
        // blocking:            (0)
    {
        u_long nonblock = 0;
        int zce_result = ::ioctlsocket(handle, FIONBIO, &nonblock);

        //将错误信息设置到errno，详细请参考上面zce名字空间后面的解释
        if (SOCKET_ERROR == zce_result)
        {
            errno = ::WSAGetLastError();
        }

        return zce_result;
    }

    default:
        return (-1);
    }

#elif defined (ZCE_OS_LINUX)
    int val = ::fcntl(handle, F_GETFL, 0);

    if (val == -1)
    {
        return -1;
    }

    // Turn on flags.
    ZCE_CLR_BITS(val, flags);

    if (::fcntl(handle, F_SETFL, val) == -1)
    {
        return -1;
    }

    return 0;
#endif
}

//如果使用大量的端口,select 是不合适的，需要使用EPOLL,此时可以打开下面的注释
#define HANDLEREADY_USE_EPOLL

//FD_SET 里面的 whlie(0)会产生一个告警，这个应该是windows内部自己没有处理好。微软说VS2005SP1就修复了，见鬼。
#if defined (ZCE_OS_WINDOWS)
#pragma warning(disable : 4127)
#pragma warning(disable : 6262)
#endif

//检查在（一定时间内），某个SOCKET句柄关注的单个事件是否触发，如果触发，返回触发事件个数，如果成功，一般触发返回值都是1
int handle_ready(ZCE_SOCKET handle,
                 zce::time_value* timeout_tv,
                 HANDLE_READY ready_todo)
{
#if defined ZCE_OS_WINDOWS || (defined ZCE_OS_LINUX && !defined HANDLEREADY_USE_EPOLL)

    fd_set handle_set_read, handle_set_write, handle_set_exeception;
    fd_set* p_set_read = NULL, * p_set_write = NULL, * p_set_exception = NULL;
    FD_ZERO(&handle_set_read);
    FD_ZERO(&handle_set_write);
    FD_ZERO(&handle_set_exeception);

    if (HANDLE_READY::READ == ready_todo)
    {
        FD_SET(handle, &handle_set_read);
        p_set_read = &handle_set_read;
    }
    else if (HANDLE_READY::WRITE == ready_todo)
    {
        FD_SET(handle, &handle_set_write);
        p_set_write = &handle_set_write;
    }
    else if (HANDLE_READY::EXCEPTION == ready_todo)
    {
        FD_SET(handle, &handle_set_exeception);
        p_set_exception = &handle_set_exeception;
    }
    else if (HANDLE_READY::server_peer == ready_todo)
    {
        //accept事件是利用读取事件
        FD_SET(handle, &handle_set_read);
        p_set_read = &handle_set_read;
    }
    else if (HANDLE_READY::CONNECTED == ready_todo)
    {
        //为什么前面写的这么麻烦，其实就是因为这个CONNECTED的倒霉孩子
        //首先，CONNECT的处理，要区分成功和失败事件
        //Windows 非阻塞CONNECT, 失败调用异常，成功调用写事件
        //Windows 阻塞CONNECT, 失败调用读写事件，成功调用写事件
        //LINUX 无论阻塞，还是非阻塞，失败调用读写事件，成功调用写事件
        //所以……，你有没有感觉到蛋蛋的忧伤
        FD_SET(handle, &handle_set_read);
        p_set_read = &handle_set_read;
        FD_SET(handle, &handle_set_write);
        p_set_write = &handle_set_write;
        FD_SET(handle, &handle_set_exeception);
        p_set_exception = &handle_set_exeception;
    }
    else
    {
        ZCE_ASSERT(false);
    }

    // Wait for data or for the timeout_tv to elapse.
    int select_width = 0;

#if defined (ZCE_OS_WINDOWS)
    //如果不是0Windows下VC++会有告警
    select_width = 0;
#elif defined (ZCE_OS_LINUX)
    select_width = int(handle) + 1;
#endif

    int result = zce::select(select_width,
                             p_set_read,
                             p_set_write,
                             p_set_exception,
                             timeout_tv);

    if (0 == result)
    {
        errno = ETIMEDOUT;
        return 0;
    }

    //出现错误，
    if (0 > result)
    {
        return result;
    }

    //我们处理的是CONNECTED成功，
    if (HANDLE_READY::CONNECTED == ready_todo)
    {
        //如果是CONNECTED，读返回或者异常返回都被认为是错误
        if (FD_ISSET(handle, p_set_read)
            || FD_ISSET(handle, p_set_exception))
        {
            return -1;
        }
    }

    return result;

#else

    //用EPOLL 完成事件触发，优点是能处理的数据多，缺点是系统调用太多
    int ret = 0;
    const int MAX_EVENT_NUMBER = 64;
    int epoll_fd = ::epoll_create(MAX_EVENT_NUMBER);

    struct epoll_event ep_event;
    if (HANDLE_READY::READ == ready_todo)
    {
        ep_event.events |= EPOLLIN;
    }
    else if (HANDLE_READY::WRITE == ready_todo)
    {
        ep_event.events |= EPOLLOUT;
    }
    else if (HANDLE_READY::EXCEPTION == ready_todo)
    {
        ep_event.events |= EPOLLERR;
    }
    else if (HANDLE_READY::server_peer == ready_todo)
    {
        //accept事件是利用读取事件
        ep_event.events |= EPOLLIN;
    }
    else if (HANDLE_READY::CONNECTED == ready_todo)
    {
        //LINUX 无论阻塞，还是非阻塞，失败调用读写事件，成功调用写事件
        ep_event.events |= EPOLLOUT;
        ep_event.events |= EPOLLIN;
    }
    else
    {
        ZCE_ASSERT(false);
    }

    ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, handle, &ep_event);
    if (ret != 0)
    {
        return ret;
    }

    //默认一直阻塞
    int msec_timeout = -1;

    if (timeout_tv)
    {
        //根据bluehu 提醒修改了下面这段，（不过小伙子们，你们改代码要认真一点喔）
        //由于select的超时参数可以精确到微秒，而epoll_wait的参数只精确到毫秒
        //当超时时间小于1000微秒时，比如20微秒，将时间转换成毫秒会变成0毫秒
        //所以如果用epoll_wait的话，超时时间大于0并且小于1毫秒的情况下统一改为1毫秒

        msec_timeout = static_cast<int>(timeout_tv->total_msec_round());
    }

    int event_happen = 0;
    //EPOLL等待事件触发，
    const int ONCE_MAX_EVENTS = 10;
    struct epoll_event once_events_ary[ONCE_MAX_EVENTS];

    event_happen = ::epoll_wait(epoll_fd, once_events_ary, ONCE_MAX_EVENTS, msec_timeout);

    //完成清理工程，调用epoll_ctl EPOLL_CTL_DEL 删除注册对象,
    struct epoll_event event_del;
    event_del.events = 0;
    ::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, handle, &event_del);
    ::close(epoll_fd);

    if (0 == event_happen)
    {
        errno = ETIMEDOUT;
        return 0;
    }

    //出现错误，
    if (0 > event_happen)
    {
        return event_happen;
    }

    if (HANDLE_READY::CONNECTED == ready_todo)
    {
        if (once_events_ary[1].events & EPOLLIN)
        {
            return -1;
        }
    }
    return event_happen;
#endif
}

#if defined (ZCE_OS_WINDOWS)
#pragma warning(default : 4127)
#pragma warning(default : 6262)
#endif

//--------------------------------------------------------------------------------------------
//因为WINdows 不支持取得socket 是否是阻塞的模式，所以Windows 下我无法先取得socket的选项，然后判断是否取消阻塞模式
//所以请你务必保证你的Socket 是阻塞模式的，否则有问题

int connect_timeout(ZCE_SOCKET handle,
                    const sockaddr* addr,
                    socklen_t addr_len,
                    zce::time_value& timeout_tv)
{
    int ret = 0;
    //不能对阻塞的句柄进行超时处理
    ret = zce::sock_enable(handle, O_NONBLOCK);
    if (ret != 0)
    {
        zce::close_socket(handle);
        return -1;
    }

    ret = zce::connect(handle, addr, addr_len);
    //
    if (ret != 0)
    {
        //WINDOWS下返回EWOULDBLOCK，LINUX下返回EINPROGRESS
        int last_err = zce::last_error();

        if (EINPROGRESS != last_err && EWOULDBLOCK != last_err)
        {
            zce::close_socket(handle);
            return ret;
        }
    }

    //进行超时处理
    ret = zce::handle_ready(handle,
                            &timeout_tv,
                            HANDLE_READY::CONNECTED);

    const int HANDLE_READY_ONE = 1;

    if (ret != HANDLE_READY_ONE)
    {
        zce::close_socket(handle);
        return -1;
    }

    ret = zce::sock_disable(handle, O_NONBLOCK);
    if (ret != 0)
    {
        zce::close_socket(handle);
        return -1;
    }

    return 0;
}

int connect_timeout(ZCE_SOCKET handle,
                    const char* host_name,
                    uint16_t port,
                    sockaddr* host_addr,
                    socklen_t addr_len,
                    zce::time_value& timeout_tv)
{
    //只能是IPV4，IPV6或者两个兼而有之
    int ret = 0;

    //如果有Hostname，会进行DNS解析，得到连接的IP地址
    if (host_name)
    {
        ret = zce::getaddrinfo_to_addr(host_name,
                                       nullptr,
                                       host_addr,
                                       addr_len);
        if (ret != 0)
        {
            return ret;
        }

        //设置端口
        if (sizeof(sockaddr_in) == addr_len)
        {
            sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(host_addr);
            addr4->sin_port = htons(port);
        }
        else if (sizeof(sockaddr_in6) == addr_len)
        {
            sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(host_addr);
            addr6->sin6_port = htons(port);
        }
        else
        {
            ZCE_ASSERT(false);
        }
    }

    return zce::connect_timeout(handle,
                                host_addr,
                                addr_len,
                                timeout_tv);
}

ZCE_SOCKET accept_timeout(ZCE_SOCKET handle,
                          sockaddr* addr,
                          socklen_t* addr_len,
                          zce::time_value& timeout_tv)
{
    int ret = 0;
    ret = zce::handle_ready(handle,
                            &timeout_tv,
                            zce::HANDLE_READY::server_peer);
    const int HANDLE_READY_ONE = 1;
    if (ret != HANDLE_READY_ONE)
    {
        return ZCE_INVALID_SOCKET;
    }

    //
    ZCE_SOCKET sock_handle = zce::accept(handle,
                                         addr,
                                         addr_len);
    if (sock_handle == ZCE_INVALID_SOCKET)
    {
        return ZCE_INVALID_SOCKET;
    }
    return sock_handle;
}

ssize_t recvn_timeout(ZCE_SOCKET handle,
                      void* buf,
                      size_t len,
                      zce::time_value& timeout_tv,
                      int flags,
                      bool only_once)
{
    ssize_t result = 0;
    bool error_occur = false;

    //如果只等待有限时间
#if defined  (ZCE_OS_WINDOWS)

    //WIN32下只能简单的打开非阻塞了
    zce::sock_enable(handle, O_NONBLOCK);

#elif defined  (ZCE_OS_LINUX)
    //Linux简单很多，只需要对这一次发送做一些无阻塞限定就OK。
    //MSG_DONTWAIT选线，WIN32不支持
    flags |= MSG_DONTWAIT;
#endif

    int ret = 0;
    ssize_t onetime_recv = 0, bytes_recv = 0;

    //一定准备确保收到这么多字节
    for (bytes_recv = 0; static_cast<size_t>(bytes_recv) < len; bytes_recv += onetime_recv)
    {
        //等待端口准备好了处罚接收事件，这儿其实不严谨，理论，这儿timeout_tv 应该减去消耗的时间
        //LINUX的SELECT会做这件事情，但WINDOWS的不会
        ret = zce::handle_ready(handle,
                                &timeout_tv,
                                HANDLE_READY::READ);

        const int HANDLE_READY_ONE = 1;

        if (ret != HANDLE_READY_ONE)
        {
            //
            if (0 == ret)
            {
                errno = ETIMEDOUT;
                result = -1;
            }

            error_occur = true;
            result = ret;
            break;
        }

        //使用非阻塞端口进行接收
        onetime_recv = zce::recv(handle,
                                 static_cast <char*> (buf) + bytes_recv,
                                 len - bytes_recv,
                                 flags);

        if (onetime_recv > 0)
        {
            //如果只收取一次数据
            if (only_once)
            {
                bytes_recv += onetime_recv;
                break;
            }
            continue;
        }
        //如果出现错误,== 0一般是是端口断开，==-1表示
        else
        {
            //==-1，但是表示阻塞错误，进行循环处理
            if (onetime_recv < 0 && errno == EWOULDBLOCK)
            {
                // Did select() succeed?
                onetime_recv = 0;
                continue;
            }

            //出现错误，进行处理
            error_occur = true;
            result = onetime_recv;
            break;
        }
    }

    //如果只等待有限时间，恢复原有阻塞模式
#if defined  (ZCE_OS_WINDOWS)
    zce::sock_disable(handle, O_NONBLOCK);
#endif

    if (error_occur)
    {
        return result;
    }

    return bytes_recv;
}

//请你务必在WIN32环境保证你的Socket 是阻塞模式的，否则有问题
ssize_t sendn_timeout(ZCE_SOCKET handle,
                      const void* buf,
                      size_t len,
                      zce::time_value& timeout_tv,
                      int flags)
{
    bool error_occur = false;
    ssize_t result = 0, bytes_send = 0, onetime_send = 0;

    //如果只等待有限时间
#if defined  (ZCE_OS_WINDOWS)

    //等待一段时间要进行特殊处理
    //WIN32下只能简单的打开非阻塞了
    zce::sock_enable(handle, O_NONBLOCK);

#elif defined  (ZCE_OS_LINUX)
    //Linux简单很多，只需要对这一次发送做一些无阻塞限定就OK。
    //MSG_DONTWAIT标志，WIN32不支持
    flags |= MSG_DONTWAIT;
#endif

    int ret = 0;

    //一定准备确保收到这么多字节
    for (bytes_send = 0; static_cast<size_t>(bytes_send) < len; bytes_send += onetime_send)
    {
        //发送在处理流程上和recv不一样，因为send往往不需要等待处理，进入select浪费
        //使用非阻塞端口进行接收，
        onetime_send = zce::send(handle,
                                 static_cast <const char*> (buf) + bytes_send,
                                 len - bytes_send,
                                 flags);

        if (onetime_send > 0)
        {
            continue;
        }
        //如果出现错误,== 0一般是是端口断开，==-1表示错误
        else
        {
            //==-1，但是表示阻塞错误，进行循环处理
            if (onetime_send < 0 && errno == EWOULDBLOCK)
            {
                //准备进入select
                onetime_send = 0;

                //等待端口准备好了处理发送事件，这儿其实不严谨，这儿timeout_tv 应该减去消耗的时间
                ret = zce::handle_ready(handle,
                                        &timeout_tv,
                                        HANDLE_READY::WRITE);

                const int HANDLE_READY_ONE = 1;

                if (ret == HANDLE_READY_ONE)
                {
                    continue;
                }
                else
                {
                    //yunfeiyang:用coverity扫描发现的，发现了好几个框架的bug.棋子包括下面的这个地方，我原来写的是 result = -1
                    if (0 == ret)
                    {
                        errno = ETIMEDOUT;
                        ret = -1;
                    }

                    error_occur = true;
                    result = ret;
                    break;
                }
            }

            //出现错误，进行处理
            error_occur = true;
            result = onetime_send;
            break;
        }
    }

#if defined  ZCE_OS_WINDOWS
    //关闭非阻塞状态
    zce::sock_disable(handle, O_NONBLOCK);
#endif

    //发送了错误，返回错误返回值
    if (error_occur)
    {
        return result;
    }

    return bytes_send;
}

//收UDP的数据,也带有超时处理，但是是收到多少数据就是多少了，超时用select实现
ssize_t recvfrom_timeout(ZCE_SOCKET handle,
                         void* buf,
                         size_t len,
                         sockaddr* from,
                         socklen_t* from_len,
                         zce::time_value& timeout_tv,
                         int flags)
{
    //如果只等待有限时间
#if defined  (ZCE_OS_WINDOWS)

    //等待一段时间要进行特殊处理
    //WIN32下只能简单的打开非阻塞了
    zce::sock_enable(handle, O_NONBLOCK);

#elif defined  (ZCE_OS_LINUX)
    //Linux简单很多，只需要对这一次发送做一些无阻塞限定就OK。
    //MSG_DONTWAIT标志，WIN32不支持
    flags |= MSG_DONTWAIT;
#endif

    ssize_t recv_result = 0;
    int ret = zce::handle_ready(handle,
                                &timeout_tv,
                                HANDLE_READY::READ);

    const int HANDLE_READY_ONE = 1;

    if (ret == HANDLE_READY_ONE)
    {
        //使用非阻塞端口进行接收
        recv_result = zce::recvfrom(handle,
                                    static_cast <char*> (buf),
                                    len,
                                    flags,
                                    from,
                                    from_len);
    }
    else
    {
        //
        if (ret == 0)
        {
            errno = ETIMEDOUT;
        }

        recv_result = -1;
    }

    //如果只等待有限时间，恢复原有的状态
#if defined  ZCE_OS_WINDOWS
    zce::sock_disable(handle, O_NONBLOCK);
#endif

    return recv_result;
}

//UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
//发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理
ssize_t sendto_timeout(ZCE_SOCKET handle,
                       const void* buf,
                       size_t len,
                       const sockaddr* to,
                       socklen_t to_len,
                       zce::time_value& /*timeout_tv*/,
                       int flags)
{
    return zce::sendto(handle,
                       buf,
                       len,
                       flags,
                       to,
                       to_len);
}

//--------------------------------------------------------------------------------------------
//这组函数提供仅仅为了代码测试，暂时不对外提供
//使用SO_RCVTIMEO，SO_SNDTIMEO得到一组超时处理函数

//注意SO_RCVTIMEO,SO_SNDTIMEO,只在WIN socket 2后支持
ssize_t recvn_timeout2(ZCE_SOCKET handle,
                       void* buf,
                       size_t len,
                       zce::time_value& timeout_tv,
                       int flags)
{
    int ret = 0;

    //虽然你做了一样的外层封装，但是由于内部实现不一样，你还是要吐血。
#if defined  ZCE_OS_WINDOWS
    //超时的毫秒
    DWORD  msec_timeout = static_cast<DWORD>(timeout_tv.total_msec());
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO,
                          (const void*)(&msec_timeout), sizeof(DWORD));

#elif defined  ZCE_OS_LINUX
    timeval sockopt_tv = timeout_tv;
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO,
                          (const void*)(&sockopt_tv), sizeof(timeval));
#endif

    if (0 != ret)
    {
        return -1;
    }

    ssize_t result = 0, bytes_recv = 0, onetime_recv = 0;
    bool error_occur = false;

    //一定要收到len长度的字节
    for (bytes_recv = 0; static_cast<size_t>(bytes_recv) < len; bytes_recv += onetime_recv)
    {
        //使用非阻塞端口进行接收
        onetime_recv = zce::recv(handle,
                                 static_cast <char*> (buf) + bytes_recv,
                                 len - bytes_recv,
                                 flags);

        if (onetime_recv > 0)
        {
            continue;
        }
        //如果出现错误,== 0一般是是端口断开，==-1表示
        else
        {
            //出现错误，进行处理
            error_occur = true;
            result = onetime_recv;
            break;
        }
    }

    //
    if (error_occur)
    {
        return result;
    }

    //要不要还原原来的SO_RCVTIMEO?算了，用阻塞超时调用地方应该会一直使用

    return bytes_recv;
}

//
ssize_t sendn_timeout2(ZCE_SOCKET handle,
                       void* buf,
                       size_t len,
                       zce::time_value& timeout_tv,
                       int flags)
{
    int ret = 0;

    //虽然你做了一样的外层封装，但是由于内部实现不一样，你还是要吐血。
#if defined  ZCE_OS_WINDOWS

    DWORD  msec_timeout = static_cast<DWORD>(timeout_tv.total_msec());
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, (const void*)(&msec_timeout), sizeof(DWORD));

#elif defined  ZCE_OS_LINUX
    timeval sockopt_tv = timeout_tv;
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, (const void*)(&sockopt_tv), sizeof(timeval));

#endif

    if (0 != ret)
    {
        return -1;
    }

    ssize_t result = 0, bytes_send = 0, onetime_send = 0;
    bool error_occur = false;

    //一定要发送到len长度的字节
    for (bytes_send = 0; static_cast<size_t>(bytes_send) < len; bytes_send += onetime_send)
    {
        //使用非阻塞端口进行接收
        onetime_send = zce::send(handle,
                                 static_cast <char*> (buf) + bytes_send,
                                 len - bytes_send,
                                 flags);

        //其实这儿应该调整超时时间，呵呵，偷懒了
        if (onetime_send > 0)
        {
            continue;
        }
        //如果出现错误,== 0一般是是端口断开，==-1表示
        else
        {
            //出现错误，进行处理
            error_occur = true;
            result = onetime_send;
            break;
        }
    }

    //
    if (error_occur)
    {
        return result;
    }

    return bytes_send;
}

//收UDP的数据,也带有超时处理，但是是收到多少数据就是多少了，超时用SO_RCVTIMEO实现
ssize_t recvfrom_timeout2(ZCE_SOCKET handle,
                          void* buf,
                          size_t len,
                          sockaddr* addr,
                          socklen_t* addr_len,
                          zce::time_value& timeout_tv,
                          int flags)
{
    int ret = 0;
    //虽然你做了一样的外层封装，但是由于内部实现不一样，你还是要吐血。
#if defined (ZCE_OS_WINDOWS)

    DWORD  msec_timeout = static_cast<DWORD>(timeout_tv.total_msec());
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, (const void*)(&msec_timeout), sizeof(DWORD));

#elif defined (ZCE_OS_LINUX)
    timeval sockopt_tv = timeout_tv;
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, (const void*)(&sockopt_tv), sizeof(timeval));
#endif

    //按照socket类似函数的封装，返回-1标识失败。
    if (0 != ret)
    {
        return -1;
    }

    //使用非阻塞端口进行接收
    ssize_t recv_result = zce::recvfrom(handle,
                                        static_cast <char*> (buf),
                                        len,
                                        flags,
                                        addr,
                                        addr_len);

    return recv_result;
}

//UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
//发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理
ssize_t sendto_timeout2(ZCE_SOCKET handle,
                        const void* buf,
                        size_t len,
                        const sockaddr* addr,
                        socklen_t addr_len,
                        zce::time_value& /*timeout_tv*/,
                        int flags)
{
    return zce::sendto(handle,
                       buf,
                       len,
                       flags,
                       addr,
                       addr_len);
}

//--------------------------------------------------------------------------------------------

//转换字符串到网络地址，第一个参数af是地址族，转换后存在dst中，
//注意这个函数return 1标识成功，return 负数标识错误，return 0标识格式匹配错误
int inet_pton(int family,
              const char* strptr,
              void* addrptr)
{
#if defined (ZCE_OS_WINDOWS)

    //为什么不让我用inet_pton ,(Vista才支持),不打开下面注释的原因是，编译会通过了，但你也没法用,XP和WINSERVER2003都无法使用，
    //VISTA,WINSERVER2008的_WIN32_WINNT都是0x0600. 诅咒在2011年还在用Win2003Server的公司
#if defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008
    return ::inet_pton(family, strptr, addrptr);
#else

    //sscanf取得的域的个数
    int get_fields_num = 0;

    if (AF_INET == family)
    {
        struct in_addr* in_val = reinterpret_cast<in_addr*>(addrptr);

        //为什么不直接用in_val->S_un.S_un_b.s_b1？你猜
        const int NUM_FIELDS_AF_INET = 4;
        uint32_t u[NUM_FIELDS_AF_INET] = { 0 };
        get_fields_num = sscanf(strptr,
                                "%u%.%u%.%u%.%u",
                                &(u[0]),
                                &(u[1]),
                                &(u[2]),
                                &(u[3])
        );

        //输入的字符串不合乎标准

        if (NUM_FIELDS_AF_INET != get_fields_num || u[0] > 0xFF || u[1] > 0xFF || u[2] > 0xFF || u[3] > 0xFF)
        {
            return 0;
        }

        uint32_t u32_addr = u[0] << 24 | u[1] << 16 | u[2] << 8 | u[3];
        in_val->S_un.S_addr = htonl(u32_addr);

        //注意，返回1标识成功
        return (1);
    }
    else if (AF_INET6 == family)
    {
        //fucnking 创造RFC1884的哥们，你就考虑简化，也考虑一下写代码的人如何进行转换把。送你一只草泥马
        //RFC1884 对不起，我不支持IPV6的缩略格式和IPV4映射格式，那个太太麻烦了。
        //IPV6的0缩略格式包括 '::'在开头的::FFFF:A:B  '::'在中间的 A:B:::C(你不知道有几个0被省略了)，'::'在尾巴上的
        //IPV4映射成IPV6的格式可以标识成::FFFF:A.B.C.D

        //输入的字符串不合乎标准
        const int NUM_FIELDS_AF_INET6 = 8;

        const char INET6_STR_UP_CHAR[] = { "1234567890ABCDEF" };

        //先请0
        memset(addrptr, 0, sizeof(in_addr6));
        struct in_addr6* in_val6 = reinterpret_cast<in_addr6*>(addrptr);

        size_t in_str_len = strlen(strptr);
        //前一个字符是否是冒号
        bool pre_char_colon = false;
        //字符串是否
        bool str_abbreviate = false;
        //
        int havedot_ipv4_mapped = 0;

        //
        size_t word_start_pos = 0;

        uint16_t for_word[NUM_FIELDS_AF_INET6] = { 0 };
        uint16_t back_word[NUM_FIELDS_AF_INET6] = { 0 };

        size_t forword_num = 0, backword_num = 0;

        for (size_t i = 0; i <= in_str_len; ++i)
        {
            //
            if (':' == strptr[i])
            {
                //如果后面的字符也是：，标识是缩写格式
                if (pre_char_colon == true)
                {
                    //如果没有发生过缩写
                    if (false == str_abbreviate)
                    {
                        str_abbreviate = true;
                    }
                    //不可能发送两次缩写，
                    else
                    {
                        return 0;
                    }

                    continue;
                }

                //.出现后，不可能出现：，格式错误
                if (havedot_ipv4_mapped > 0)
                {
                    return 0;
                }

                //不可能出现8个冒号
                if (backword_num + forword_num >= NUM_FIELDS_AF_INET6)
                {
                    return 0;
                }

                //如果已经有缩写，那么记录到后向数据队列中
                if (str_abbreviate)
                {
                    get_fields_num = sscanf(strptr + word_start_pos, "%hx:", &(back_word[backword_num]));
                    ++backword_num;
                }
                else
                {
                    get_fields_num = sscanf(strptr + word_start_pos, "%hx:", &(for_word[forword_num]));
                    ++forword_num;
                }

                pre_char_colon = true;
                continue;
            }
            //IPV4映射IPV6的写法
            else if ('.' == strptr[i])
            {
                //如果前面是:,错误
                if (pre_char_colon)
                {
                    return 0;
                }

                ++havedot_ipv4_mapped;
            }
            else
            {
                //出现其他字符，认为错误，滚蛋，
                if (NULL == strchr(INET6_STR_UP_CHAR, toupper(strptr[i])))
                {
                    return 0;
                }

                //如果前面一个是:
                if (pre_char_colon)
                {
                    pre_char_colon = false;
                    word_start_pos = i;
                }

                continue;
            }
        }

        //对最后一个WORD或者2个WORD进行处理

        //出现了.，并且出现了了3次，havedot_ipv4_mapped ，
        if (havedot_ipv4_mapped > 0)
        {
            const int NUM_FIELDS_AF_INET = 4;
            uint32_t u[NUM_FIELDS_AF_INET] = { 0 };
            get_fields_num = sscanf(strptr + word_start_pos,
                                    "%u%.%u%.%u%.%u",
                                    &(u[0]),
                                    &(u[1]),
                                    &(u[2]),
                                    &(u[3])
            );

            //输入的字符串不合乎标准
            if (NUM_FIELDS_AF_INET != get_fields_num || u[0] > 0xFF || u[1] > 0xFF || u[2] > 0xFF || u[3] > 0xFF)
            {
                return 0;
            }

            back_word[backword_num] = static_cast<uint16_t>(u[0] << 8 | u[1]);
            ++backword_num;
            back_word[backword_num] = static_cast<uint16_t>(u[2] << 8 | u[3]);
            ++backword_num;
        }
        else
        {
            if (false == pre_char_colon)
            {
                sscanf(strptr + word_start_pos, "%hx", &(back_word[backword_num]));
                ++backword_num;
            }
        }

        //对每一个WORD进行赋值，前赋值前向的，再赋值后向的,中间的如果被省略就是0了。不管了
        //这个转换只能在WINDOWS下用（这些union只有WINDOWS下有定义），如果要通用，要改代码。
        for (size_t k = 0; k < forword_num; ++k)
        {
            in_val6->u.Word[k] = htons(for_word[k]);
        }

        for (size_t k = 0; k < backword_num; ++k)
        {
            in_val6->u.Word[NUM_FIELDS_AF_INET6 - backword_num + k] = htons(back_word[k]);
        }

        //返回1标识成功
        return (1);
    }
    //不支持
    else
    {
        errno = EAFNOSUPPORT;
        return 0;
    }
#endif

#elif defined (ZCE_OS_LINUX)
    //LINuX下有这个函数
    return ::inet_pton(family, strptr, addrptr);
#endif
}

//函数原型如下[将“点分十进制” －> “整数”],IPV6将，:分割16进制转换成128位数字
const char* inet_ntop(int family,
                      const void* addrptr,
                      char* strptr,
                      size_t len)
{
#if defined (ZCE_OS_WINDOWS)
#if defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008
    return ::inet_ntop(family, addrptr, strptr, len);
#else
    //根据不同的协议簇进行不同的处理
    if (AF_INET == family)
    {
        const struct in_addr* in_val = reinterpret_cast<const in_addr*>(addrptr);
        int ret_len = snprintf(strptr,
                               len,
                               "%u.%u.%u.%u",
                               in_val->S_un.S_un_b.s_b1,
                               in_val->S_un.S_un_b.s_b2,
                               in_val->S_un.S_un_b.s_b3,
                               in_val->S_un.S_un_b.s_b4);

        //格式化字符串失败
        if (ret_len > static_cast<int>(len) || ret_len <= 0)
        {
            errno = ENOSPC;
            return NULL;
        }

        return strptr;
    }
    else if (AF_INET6 == family)
    {
        //对不起，我只支持转换成IPV6的标准格式，不支持转换成IPV6的缩略格式和IPV4映射格式，

        const struct in_addr6* in_val6 = reinterpret_cast<const in_addr6*>(addrptr);

        const int NUM_FIELDS_AF_INET6 = 8;
        uint16_t u[NUM_FIELDS_AF_INET6];

        //因为是short，还是要转换成本地序列
        u[0] = ::ntohs(in_val6->u.Word[0]);
        u[1] = ::ntohs(in_val6->u.Word[1]);
        u[2] = ::ntohs(in_val6->u.Word[2]);
        u[3] = ::ntohs(in_val6->u.Word[3]);
        u[4] = ::ntohs(in_val6->u.Word[4]);
        u[5] = ::ntohs(in_val6->u.Word[5]);
        u[6] = ::ntohs(in_val6->u.Word[6]);
        u[7] = ::ntohs(in_val6->u.Word[7]);

        int ret_len = snprintf(strptr,
                               len,
                               "%hx:%hx:%hx:%hx:%hx:%hx:%hx:%hx",
                               u[0],
                               u[1],
                               u[2],
                               u[3],
                               u[4],
                               u[5],
                               u[6],
                               u[7]
        );

        if (ret_len > static_cast<int>(len) || ret_len <= 0)
        {
            errno = ENOSPC;
            return NULL;
        }

        return strptr;
    }
    else
    {
        errno = EAFNOSUPPORT;
        return NULL;
    }
#endif
#elif defined (ZCE_OS_LINUX)
    //LINuX下有这个函数
    return ::inet_ntop(family, addrptr, strptr, len);
#endif
}

//输出IP地址信息，内部是不使用静态变量，线程安全，BUF长度IPV4至少长度>15.IPV6至少长度>39
const char* sockaddr_ntop(const sockaddr* sock_addr,
                          char* str_ptr,
                          size_t str_len)
{
    //根据不同的地址协议族，进行转换
    if (sock_addr->sa_family == AF_INET)
    {
        const sockaddr_in* sockadd_ipv4 = reinterpret_cast<const sockaddr_in*>(sock_addr);
        return zce::inet_ntop(AF_INET,
                              (void*)(&(sockadd_ipv4->sin_addr)),
                              str_ptr,
                              str_len);
    }
    else if (sock_addr->sa_family == AF_INET6)
    {
        const sockaddr_in6* sockadd_ipv6 = reinterpret_cast<const sockaddr_in6*>(sock_addr);
        return zce::inet_ntop(AF_INET6,
                              (void*)(&(sockadd_ipv6->sin6_addr)),
                              str_ptr, str_len);
    }
    else
    {
        errno = EAFNOSUPPORT;
        return NULL;
    }
}

//输出IP地址信息以及端口信息，内部是不使用静态变量，线程安全，BUF长度IPV4至少长度>21.IPV6至少长度>45
const char* sockaddr_ntop_ex(const sockaddr* sock_addr,
                             char* str_ptr,
                             size_t str_len,
                             size_t& use_len,
                             bool out_port_info)
{
    uint16_t addr_port = 0;
    const char* ret_str = nullptr;
    use_len = 0;
    //根据不同的地址协议族，进行转换，不使用上面那个函数的原因是因为，我同时要进行读取port的操作
    if (sock_addr->sa_family == AF_INET)
    {
        const sockaddr_in* sockadd_ipv4 = reinterpret_cast<const sockaddr_in*>(sock_addr);
        addr_port = ntohs(sockadd_ipv4->sin_port);
        ret_str = zce::inet_ntop(AF_INET,
                                 (void*)(&(sockadd_ipv4->sin_addr)),
                                 str_ptr,
                                 str_len);
    }
    else if (sock_addr->sa_family == AF_INET6)
    {
        const sockaddr_in6* sockadd_ipv6 = reinterpret_cast<const sockaddr_in6*>(sock_addr);
        addr_port = ntohs(sockadd_ipv6->sin6_port);
        ret_str = zce::inet_ntop(AF_INET6,
                                 (void*)(&(sockadd_ipv6->sin6_addr)),
                                 str_ptr, str_len);
    }
    else
    {
        errno = EAFNOSUPPORT;
        return nullptr;
    }

    //如果返回错误
    if (nullptr == ret_str)
    {
        return nullptr;
    }

    use_len += strlen(str_ptr);
    if (out_port_info)
    {
        //前面已经检查过了，这儿不判断返回了
        int port_len = snprintf(str_ptr + use_len, str_len - use_len, "#%u", addr_port);
        if (port_len <= 0 || port_len + use_len > str_len)
        {
            return nullptr;
        }
        use_len += port_len;
    }

    return str_ptr;
}

/*
我们对企业网的IP分配一般以RFC1918中定义的非Internet连接的网络地址，
也称为私有地址。由Internet地址授权机构（IANA）控制的IP地址分配方案中，
留出了三类网络地址，给不连到Internet上的专用网使用。它分别是：
A类：10.0.0.0 ~ 10.255.255.255；
B类：172.16.0.0 ~ 172.31.255.255；
C类：192.168.0.0 ~ 192.168.255.255。
其中的一个私有地址网段是：192.168.0.0是我们在内网IP分配中最常用的网段。
IANA保证这些网络号不会分配给连到Internet上的任何网络，
因此任何人都可以自由地选择这些网络地址作为自己的私有网络地址。
在申请的合法IP不足的情况下，企业网内网可以采用私有IP地址的网络地址分配方案；
企业网外网接入、DMZ区使用合法IP地址。
原来代码有人把0也认为是内外地址，这个不严谨。
*/

#if !defined ZCE_IS_INTERNAL
#define ZCE_IS_INTERNAL(ip_addr)   ((ip_addr >= 0x0A000000 && ip_addr <= 0x0AFFFFFF ) ||  \
                                    (ip_addr >= 0xAC100000 && ip_addr <= 0xAC1FFFFF) ||  \
                                    (ip_addr >= 0xC0A80000 && ip_addr <= 0xC0A8FFFF) )
#endif

//检测一个地址是否是内网地址
bool is_internal(const sockaddr* sock_addr)
{
    if (sock_addr->sa_family == AF_INET)
    {
        const sockaddr_in* sock_addr_ipv4 = reinterpret_cast<const sockaddr_in*>(sock_addr);
        uint32_t ip_addr = zce::get_ip_address(sock_addr_ipv4);
        //检查3类地址
        if (ZCE_IS_INTERNAL(ip_addr))
        {
            return true;
        }
    }
    else if (sock_addr->sa_family == AF_INET6)
    {
        const sockaddr_in6* sockadd_ipv6 = reinterpret_cast<const sockaddr_in6*>(sock_addr);
        char* addr = (char*)(&(sockadd_ipv6->sin6_addr));
        uint16_t u1 = (addr[0]);
        uint16_t u2 = (addr[1]);
        u1 = (u1 > 8) | u2;
        u1 = ntohs(u1);
        if (u1 == 0xFD00)
        {
            return true;
        }
    }
    else
    {
        errno = EAFNOSUPPORT;
        return false;
    }

    return false;
}

bool is_internal(uint32_t ipv4_addr_val)
{
    return (ZCE_IS_INTERNAL(ipv4_addr_val));
}

//-------------------------------------------------------------------------------------
//域名解析，转换IP地址的几个函数

//通过域名得到相关的IP地址
hostent* gethostbyname(const char* hostname)
{
    return ::gethostbyname(hostname);
}

// GNU extensions
hostent* gethostbyname2(const char* hostname,
                        int af)
{
#if defined (ZCE_OS_WINDOWS)

    hostent* hostent_ptr = ::gethostbyname(hostname);
    if (hostent_ptr->h_addrtype != af)
    {
        return NULL;
    }
    return hostent_ptr;

#elif defined (ZCE_OS_LINUX)
    return ::gethostbyname2(hostname, af);
#endif
}

//非标准函数,得到某个域名的IPV4的地址信息，但是使用起来比较容易和方便
//name 域名
//uint16_t service_port 端口号，本地序
//ary_addr_num  ,输入输出参数，输入标识ary_sock_addr的个数，输出时标识返回的队列数量
//ary_sock_addr ,输出参数，返回的地址队列
int gethostbyname_inary(const char* hostname,
                        uint16_t service_port,
                        size_t* ary_addr_num,
                        sockaddr_in ary_sock_addr[])
{
    //其实这个函数没法重入
    struct hostent* hostent_ptr = ::gethostbyname(hostname);

    if (!hostent_ptr)
    {
        return -1;
    }

    if (hostent_ptr->h_addrtype != AF_INET)
    {
        errno = EINVAL;
        return -1;
    }

    //检查返回
    ZCE_ASSERT(hostent_ptr->h_length == sizeof(in_addr));

    //循环得到所有的IP地址信息
    size_t i = 0;
    char** addr_pptr = hostent_ptr->h_addr_list;

    for (; (i < *ary_addr_num) && (*addr_pptr != NULL); addr_pptr++, ++i)
    {
        ary_sock_addr[i].sin_family = AF_INET;
        //本来就是网络序
        memcpy(&(ary_sock_addr[i].sin_addr), addr_pptr, hostent_ptr->h_length);
        //端口转换成网络序
        ary_sock_addr[i].sin_port = htons(service_port);
    }

    //记录数量
    *ary_addr_num = i;

    return 0;
}

//非标准函数,得到某个域名的IPV6的地址信息，但是使用起来比较容易和方便
int gethostbyname_in6ary(const char* hostname,
                         uint16_t service_port,
                         size_t* ary_addr6_num,
                         sockaddr_in6 ary_sock_addr6[])
{
    //其实这个函数没法重入
    struct hostent* hostent_ptr = ::gethostbyname(hostname);

    if (!hostent_ptr)
    {
        return -1;
    }

    if (hostent_ptr->h_addrtype != AF_INET6)
    {
        errno = EINVAL;
        return -1;
    }

    //检查返回的地址实习是不是IPV6的
    ZCE_ASSERT(hostent_ptr->h_length == sizeof(in6_addr));

    //循环得到所有的IP地址信息
    size_t i = 0;
    char** addr_pptr = hostent_ptr->h_addr_list;

    for (; (i < *ary_addr6_num) && (*addr_pptr != NULL); addr_pptr++, ++i)
    {
        ary_sock_addr6[i].sin6_family = AF_INET6;
        //本来就是网络序
        memcpy(&(ary_sock_addr6[i].sin6_addr), addr_pptr, hostent_ptr->h_length);
        //端口转换成网络序
        ary_sock_addr6[i].sin6_port = htons(service_port);
    }

    //记录数量
    *ary_addr6_num = i;

    return 0;
}

//根据地址得到域名的函数,推荐使用替代函数getnameinfo ,
hostent* gethostbyaddr(const void* addr,
                       socklen_t len,
                       int family)
{
    return ::gethostbyaddr((const char*)addr, len, family);
};

//非标准函数，通过IPV4地址取得域名
int gethostbyaddr_in(const sockaddr_in* sock_addr,
                     char* host_name,
                     size_t name_len)
{
    struct hostent* hostent_ptr = zce::gethostbyaddr(sock_addr,
                                                     sizeof(sockaddr_in),
                                                     sock_addr->sin_family
    );

    //如果返回失败
    if (!hostent_ptr)
    {
        return -1;
    }

    ::strncpy(host_name, hostent_ptr->h_name, name_len);

    return 0;
}

//非标准函数，通过IPV6地址取得域名
int gethostbyaddr_in6(const sockaddr_in6* sock_addr6,
                      char* host_name,
                      size_t name_len)
{
    struct hostent* hostent_ptr = zce::gethostbyaddr(sock_addr6,
                                                     sizeof(sockaddr_in6),
                                                     sock_addr6->sin6_family);

    //如果返回失败
    if (!hostent_ptr)
    {
        return -1;
    }

    ::strncpy(host_name, hostent_ptr->h_name, name_len);

    return 0;
}

//通过域名得到服务器地址信息，可以同时得到IPV4，和IPV6的地址
int getaddrinfo(const char* nodename,
                const char* service,
                const addrinfo* hints,
                addrinfo** result)
{
    return ::getaddrinfo(nodename,
                         service,
                         hints,
                         result);
}

//释放getaddrinfo得到的结果
void freeaddrinfo(struct addrinfo* result)
{
    return ::freeaddrinfo(result);
}

//
int getaddrinfo_result_to_oneaddr(addrinfo* result,
                                  sockaddr* addr,
                                  socklen_t addr_len)
{
    addrinfo* prc_node = result;
    for (; (prc_node != NULL); prc_node = prc_node->ai_next)
    {
        //只取相应的地址
        if (addr_len == static_cast<socklen_t>(prc_node->ai_addrlen))
        {
            ::memcpy(addr, prc_node->ai_addr, prc_node->ai_addrlen);
            break;
        }
    }
    if (nullptr == prc_node)
    {
        return -1;
    }
    return 0;
}

//将getaddrinfo的结果进行加工处理，处理成数组
void getaddrinfo_result_to_addrary(addrinfo* result,
                                   size_t* ary_addr_num,
                                   sockaddr_in ary_addr[],
                                   size_t* ary_addr6_num,
                                   sockaddr_in6 ary_addr6[])
{
    addrinfo* prc_node = result;
    size_t num_addr = 0, num_addr6 = 0;
    for (size_t i = 0;
         (i < *ary_addr_num) && (prc_node != NULL);
         prc_node = prc_node->ai_next, ++i)
    {
        //只取相应的地址
        if (AF_INET == prc_node->ai_family)
        {
            memcpy(&(ary_addr[i]), prc_node->ai_addr, prc_node->ai_addrlen);
            ++num_addr;
        }
    }
    //记录数量
    *ary_addr_num = num_addr;

    prc_node = result;
    for (size_t j = 0;
         (j < *ary_addr6_num) && (prc_node != NULL);
         prc_node = prc_node->ai_next, ++j)
    {
        if (AF_INET6 == prc_node->ai_family)
        {
            memcpy(&(ary_addr6[j]), prc_node->ai_addr, prc_node->ai_addrlen);
            ++num_addr6;
        }
    }
    *ary_addr6_num = num_addr6;
}

//非标准函数,得到某个域名的IPV4 和IPV6的地址数组，使用起来比较容易和方便
int getaddrinfo_to_addrary(const char* hostname,
                           const char* service,
                           size_t* ary_addr_num,
                           sockaddr_in ary_addr[],
                           size_t* ary_addr6_num,
                           sockaddr_in6 ary_addr6[])
{
    int ret = 0;
    addrinfo hints, * result = NULL;

    ::memset(&hints, 0, sizeof(addrinfo));
    //同时返回IPV4.和IPV6，如果只需要IPV4填写AF_INET，如果只需要IPV6填写AF_INET6
    hints.ai_family = AF_UNSPEC;
    //hints.ai_socktype = 0; 返回所有类型
    //hints.ai_flags = 0;
    ret = zce::getaddrinfo(hostname,
                           service,
                           &hints,
                           &result);
    if (ret != 0)
    {
        return ret;
    }
    if (!result)
    {
        errno = EINVAL;
        return -1;
    }

    //取回结果
    getaddrinfo_result_to_addrary(result,
                                  ary_addr_num,
                                  ary_addr,
                                  ary_addr6_num,
                                  ary_addr6);
    //释放空间
    zce::freeaddrinfo(result);
    return 0;
}

int zce::getaddrinfo_to_addr(const char* nodename,
                             const char* service,
                             sockaddr* addr,
                             socklen_t addr_len)
{
    int ret = 0;
    addrinfo hints, * result = NULL;

    ::memset(&hints, 0, sizeof(addrinfo));
    //同时返回IPV4.和IPV6，如果只需要IPV4填写AF_INET，如果只需要IPV6填写AF_INET6
    hints.ai_family = addr->sa_family;
    //优先分析nodename是否是数值地址
    hints.ai_flags = AI_PASSIVE;
    ret = zce::getaddrinfo(nodename,
                           service,
                           &hints,
                           &result);
    if (ret != 0)
    {
        //进行域名解析
        hints.ai_flags = 0;
        ret = zce::getaddrinfo(nodename,
                               NULL,
                               &hints,
                               &result);
        if (ret != 0)
        {
            return ret;
        }
    }

    if (!result)
    {
        errno = EINVAL;
        return -1;
    }

    getaddrinfo_result_to_oneaddr(result, addr, addr_len);
    //释放空间
    zce::freeaddrinfo(result);
    return 0;
}

//通过IP地址信息，反查域名.服务名，可以重入函数
int getnameinfo(const struct sockaddr* sa,
                socklen_t salen,
                char* host,
                size_t hostlen,
                char* serv,
                size_t servlen,
                int flags)
{
#if defined (ZCE_OS_WINDOWS)
    return ::getnameinfo(sa,
                         salen,
                         host,
                         static_cast<DWORD>(hostlen),
                         serv,
                         static_cast<DWORD>(servlen), flags);
#elif defined (ZCE_OS_LINUX)
    return ::getnameinfo(sa,
                         salen,
                         host,
                         hostlen,
                         serv,
                         servlen,
                         flags);
#endif
}

//非标准函数，通过IPV4地址取得域名
int getnameinfo_sockaddr(const sockaddr* sock_addr,
                         char* host_name,
                         size_t name_len)
{
    if (sock_addr->sa_family == AF_INET)
    {
        return zce::getnameinfo(sock_addr,
                                sizeof(sockaddr_in),
                                host_name,
                                name_len,
                                NULL,
                                0,
                                NI_NAMEREQD);
    }
    else if (sock_addr->sa_family == AF_INET)
    {
        return zce::getnameinfo(sock_addr,
                                sizeof(sockaddr_in6),
                                host_name,
                                name_len,
                                NULL,
                                0,
                                NI_NAMEREQD);
    }
    else
    {
        ZCE_ASSERT(false);
        return -1;
    }
}

//-------------------------------------------------------------------------------------
//IPV4和IPV6之间相互转换的函数，都是非标准函数，

//将一个IPV4的地址映射为IPV6的地址
int inaddr_map_inaddr6(const in_addr* src, in6_addr* dst)
{
    //清0
    ::memset(dst, 0, sizeof(in6_addr));

    //
    ::memcpy(reinterpret_cast<char*>(dst) + sizeof(in6_addr) - sizeof(in_addr),
             reinterpret_cast<const char*>(src),
             sizeof(in_addr));

    //映射地址的倒数第3个WORD为0xFFFF
    dst->s6_addr[10] = 0xFF;
    dst->s6_addr[11] = 0xFF;

    return 0;
}

//将一个IPV4的Sock地址映射为IPV6的地址
int zce::sockin_map_sockin6(const sockaddr_in* src, sockaddr_in6* dst)
{
    return zce::inaddr_map_inaddr6(&(src->sin_addr),
                                   &(dst->sin6_addr));
}

//判断一个地址是否是IPV4映射的地址
bool zce::is_in6_addr_v4mapped(const in6_addr* in6)
{
    //这样把映射地址和兼容地址都判断了。据说兼容地址以后会被淘汰
    if (in6->s6_addr[0] == 0
        && in6->s6_addr[1] == 0
        && in6->s6_addr[2] == 0
        && in6->s6_addr[3] == 0
        && in6->s6_addr[4] == 0
        && in6->s6_addr[5] == 0
        && in6->s6_addr[6] == 0
        && in6->s6_addr[7] == 0
        && in6->s6_addr[8] == 0
        && in6->s6_addr[9] == 0
        && ((in6->s6_addr[10] == 0xFF && in6->s6_addr[11] == 0xFF) || (in6->s6_addr[10] == 0 && in6->s6_addr[11] == 0))
        )
    {
        return true;
    }

    return false;
}

//如果一个IPV6的地址从IPV4映射过来的，转换回IPV4的地址
int mapped_in6_to_in(const in6_addr* src, in_addr* dst)
{
    //先检查是否是映射的地址
    if (false == zce::is_in6_addr_v4mapped(src))
    {
        errno = EINVAL;
        return -1;
    }

    memcpy(reinterpret_cast<char*>(dst),
           reinterpret_cast<const char*>(src) + sizeof(in6_addr) - sizeof(in_addr),
           sizeof(in_addr));
    return 0;
}
//如果一个IPV6的socketaddr_in6地址从IPV4映射过来的，转换回IPV4的socketaddr_in地址
int mapped_sockin6_to_sockin(const sockaddr_in6* src, sockaddr_in* dst)
{
    return zce::mapped_in6_to_in(&(src->sin6_addr),
                                 &(dst->sin_addr));
}

//对端口进行检查，一些端口是黑客重点扫描的端口，
bool check_safeport(uint16_t check_port)
{
    //高危端口检查常量
    const unsigned short UNSAFE_PORT1 = 1024;
    const unsigned short UNSAFE_PORT2 = 3306;
    const unsigned short UNSAFE_PORT3 = 36000;
    const unsigned short UNSAFE_PORT4 = 56000;
    const unsigned short SAFE_PORT1 = 80;

    //如果打开了保险检查,检查配置的端口
    if ((check_port <= UNSAFE_PORT1 && check_port != SAFE_PORT1) ||
        check_port == UNSAFE_PORT2 ||
        check_port == UNSAFE_PORT3 ||
        check_port == UNSAFE_PORT4)
    {
        return false;
    }
    //
    return true;
}

//--------------------------------------------------------------------------------------------
//SELECT函数，为了和LINUX平台对齐，返回时间为剩余时间
int zce::select(
    int nfds,
    fd_set* readfds,
    fd_set* writefds,
    fd_set* exceptfds,
    zce::time_value* timeout_tv
)
{
#if defined (ZCE_OS_WINDOWS)

    //检查是否没有句柄在等待
    bool no_handle_to_wait = false;

    if ((NULL == readfds) || (NULL != readfds && 0 == readfds->fd_count))
    {
        if ((NULL == writefds) || (NULL != writefds && 0 == writefds->fd_count))
        {
            if ((NULL == exceptfds) || (NULL != exceptfds && 0 == exceptfds->fd_count))
            {
                no_handle_to_wait = true;
            }
        }
    }

    //如果不需要等待句柄，直接使用sleep替代select，
    //这是因为WINDOWS的select 必须等待一个句柄，否则就会return -1，你觉得麻烦不
    if (no_handle_to_wait)
    {
        zce::sleep(*timeout_tv);
        timeout_tv->set(0, 0);
        return 0;
    }

    clock_t start_clock = 0;
    timeval* select_tv = NULL;

    if (timeout_tv)
    {
        start_clock = std::clock();
        //有函数讲内部的timv_val取出来
        select_tv = (*timeout_tv);
    }

    //WINDOWS下，timeval是一个const 参数
    int ret = ::select(
        nfds,
        readfds,
        writefds,
        exceptfds,
        select_tv
    );

    if (ret < 0)
    {
        errno = ::WSAGetLastError();
        return ret;
    }

    if (timeout_tv)
    {
        clock_t end_clock = std::clock();
        zce::time_value consume_timeval;
        consume_timeval.set_by_clock_t(end_clock - start_clock);
        *timeout_tv -= consume_timeval;
    }

    return ret;

#elif defined (ZCE_OS_LINUX)

    timeval* select_tv = NULL;

    if (timeout_tv)
    {
        //有函数将内部的timv_val取出来
        select_tv = (*timeout_tv);
    }

    return ::select(
        nfds,
        readfds,
        writefds,
        exceptfds,
        select_tv
    );
#endif
}

//这个函数是为了方便平台代码编写写的一个函数，会利用不同平台的fd_set实现加快速度，
//当然这样么有任何通用性，sorry，追求性能了。
bool zce::is_ready_fds(int no_fds,
                       const fd_set* out_fds,
                       ZCE_SOCKET* ready_fd)
{
#if defined (ZCE_OS_WINDOWS)

    if (out_fds->fd_array[no_fds] != INVALID_SOCKET)
    {
        *ready_fd = out_fds->fd_array[no_fds];
        return true;
    }

    return false;
#elif defined (ZCE_OS_LINUX)

    if (FD_ISSET(no_fds, out_fds))
    {
        *ready_fd = no_fds;
        return true;
    }

    return false;
#endif
}

//设置一个IPV4的地址,
int zce::set_sockaddr_in(sockaddr_in* sock_addr_ipv4,
                         const char* ipv4_addr_str,
                         uint16_t ipv4_port)
{
    ::memset(sock_addr_ipv4, 0, sizeof(sockaddr_in));
    sock_addr_ipv4->sin_family = AF_INET;
    //htons在某些情况下是一个宏，
    //sock_addr_ipv4->sin_port = ::htons(ipv4_port);
    sock_addr_ipv4->sin_port = htons(ipv4_port);

    //在某些平台的sockaddr_in有一个长度标识，比如BSD体系？
#if 0
    sock_addr_ipv4->sa_len = sizeof(sockaddr_in);
#endif

    sock_addr_ipv4->sin_addr.s_addr = ::inet_addr(ipv4_addr_str);

    //注意inet_pton的返回值
    if (sock_addr_ipv4->sin_addr.s_addr == INADDR_NONE)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

//设置一个IPV4的地址,如果字符串里面有#，会认为后面有端口号，会同时提取端口号，否则端口号设置0
int zce::set_sockaddr_in(sockaddr_in* sock_addr_ipv4,
                         const char* ipv4_addr_str)
{
    int ret = zce::set_sockaddr_in(sock_addr_ipv4, ipv4_addr_str, 0);
    if (ret != 0)
    {
        return ret;
    }
    //如果有#，而且后面还有空间，
    const char* port_pos = strchr(ipv4_addr_str, '#');
    if (port_pos != NULL && *(++port_pos) != '\0')
    {
        //注意到这儿pos已经++了。
        uint16_t read_port = 0;
        int fields = sscanf(port_pos, "%hu", &read_port);
        if (fields != 1)
        {
            ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
        }
        sock_addr_ipv4->sin_port = htons(read_port);
    }

    return 0;
}

//设置一个IPV4的地址,错误返回NULL，正确返回设置的地址的变换
int zce::set_sockaddr_in(sockaddr_in* sock_addr_ipv4,
                         uint32_t ipv4_addr_val,
                         uint16_t ipv4_port
)
{
    ::memset(sock_addr_ipv4, 0, sizeof(sockaddr_in));
    sock_addr_ipv4->sin_family = AF_INET;
    sock_addr_ipv4->sin_port = htons(ipv4_port);
    sock_addr_ipv4->sin_addr.s_addr = htonl(ipv4_addr_val);

    //在某些平台的sockaddr_in有一个长度标识，比如BSD体系？
#if 0
    sock_addr_ipv4->sa_len = sizeof(sockaddr_in);
#endif

    return 0;
}

//设置一个IPV6的地址,
int zce::set_sockaddr_in6(sockaddr_in6* sock_addr_ipv6,
                          const char* ipv6_addr_str,
                          uint16_t ipv6_port)
{
    ::memset(sock_addr_ipv6, 0, sizeof(sockaddr_in6));
    sock_addr_ipv6->sin6_family = AF_INET6;
    sock_addr_ipv6->sin6_port = htons(ipv6_port);

    int ret = zce::inet_pton(AF_INET6,
                             ipv6_addr_str,
                             static_cast<void*>(&(sock_addr_ipv6->sin6_addr.s6_addr)));

    //注意inet_pton的返回值,我转换我熟悉的返回
    if (ret == 1)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

//设置一个IPV6的地址,如果有端口号信息，也会
int zce::set_sockaddr_in6(sockaddr_in6* sock_addr_ipv6,
                          const char* ipv6_addr_str)
{
    int ret = set_sockaddr_in6(sock_addr_ipv6, ipv6_addr_str, 0);
    if (ret != 0)
    {
        return ret;
    }
    //如果有#，而且后面还有空间，
    const char* port_pos = strchr(ipv6_addr_str, '#');
    if (port_pos != NULL && *(++port_pos) != '\0')
    {
        //注意到这儿pos已经++了。
        uint16_t read_port = 0;
        int fields = sscanf(port_pos, "%hu", &read_port);
        if (fields != 1)
        {
            ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
        }
        sock_addr_ipv6->sin6_port = htons(read_port);
    }
    return 0;
}

//设置一个IPV6的地址,错误返回NULL，正确返回设置的地址的变换
int zce::set_sockaddr_in6(sockaddr_in6* sock_addr_ipv6,
                          uint16_t ipv6_port,
                          const char ipv6_addr_val[16])
{
    ::memset(sock_addr_ipv6, 0, sizeof(sockaddr_in6));
    //16bytes，128bit的IPV6的地址信息
    const size_t IPV6_INET6_LEN = 16;

    sock_addr_ipv6->sin6_family = AF_INET6;
    sock_addr_ipv6->sin6_port = htons(ipv6_port);
    ::memcpy(sock_addr_ipv6->sin6_addr.s6_addr, ipv6_addr_val, IPV6_INET6_LEN);
    return 0;
}
}