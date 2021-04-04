#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_log_logging.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_socket.h"

//��ʼ��Socket��
int zce::socket_init(int version_high, int version_low)
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

    SOCKET no_use = INVALID_SOCKET;
    GUID guid_acceptex = WSAID_ACCEPTEX;
    DWORD dw_bytes = 0;

    int result = 0;
    result = ::WSAIoctl(no_use,
                        SIO_GET_EXTENSION_FUNCTION_POINTER,
                        &guid_acceptex,
                        sizeof(guid_acceptex),
                        &WSAAcceptEx,
                        sizeof(WSAAcceptEx),
                        &dw_bytes,
                        NULL,
                        NULL);

    if (result == SOCKET_ERROR)
    {
        ::fprintf(stderr, "WSAIoctl get WSAAcceptEx function failed with error: %u\n",
                  WSAGetLastError());
        WSACleanup();
        return -1;
    }

    GUID guid_connectex = WSAID_CONNECTEX;
    result = ::WSAIoctl(no_use,
                        SIO_GET_EXTENSION_FUNCTION_POINTER,
                        &guid_connectex,
                        sizeof(guid_connectex),
                        &WSAConnectEx,
                        sizeof(WSAConnectEx),
                        &dw_bytes,
                        NULL,
                        NULL);

    if (result == SOCKET_ERROR)
    {
        ::fprintf(stderr, "WSAIoctl get WSAConnectEx function failed with error: %u\n",
                  WSAGetLastError());
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

int zce::socket_finish(void)
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

# endif
    return 0;
}

//
ssize_t zce::writev (ZCE_SOCKET handle,
                     const iovec *buffers,
                     int iovcnt)
{
#if defined (ZCE_OS_WINDOWS)

    assert(iovcnt <= IOV_MAX);

    WSABUF wsa_buf[IOV_MAX];

    for (int i = 0; i < iovcnt; ++i)
    {
        wsa_buf[i].buf = static_cast<CHAR *>(buffers[i].iov_base);
        wsa_buf[i].len = static_cast<ULONG>(buffers[i].iov_len);
    }

    DWORD bytes_sent = 0;
    int zce_result = ::WSASend ((SOCKET) handle,
                                wsa_buf,
                                iovcnt,
                                &bytes_sent,
                                0,
                                0,
                                0);

    if (zce_result == SOCKET_ERROR)
    {
        errno = ::WSAGetLastError ();
        return -1;
    }

    return (ssize_t) bytes_sent;

#elif defined (ZCE_OS_LINUX)

    return ::writev (handle, buffers, iovcnt);
#endif
}

//
ssize_t zce::readv (ZCE_SOCKET handle,
                    iovec *buffers,
                    int iovcnt)
{
#if defined (ZCE_OS_WINDOWS)

    DWORD bytes_received = 0;
    int result = 1;

    // Winsock 2 has WSARecv and can do this directly, but Winsock 1 needs
    // to do the recvs piece-by-piece.

    //IOV_MAX���ݸ���ƽ̨��̫һ��
    assert(iovcnt <= IOV_MAX);

    WSABUF wsa_buf[IOV_MAX];

    for (int i = 0; i < iovcnt; ++i)
    {
        wsa_buf[i].buf = static_cast<CHAR *>(buffers[i].iov_base);
        wsa_buf[i].len = static_cast<ULONG>(buffers[i].iov_len);
    }

    DWORD flags = 0;
    result = ::WSARecv ( handle,
                         wsa_buf,
                         iovcnt,
                         &bytes_received,
                         &flags,
                         0,
                         0);

    if (result == SOCKET_ERROR)
    {
        errno = ::WSAGetLastError ();
        return -1;
    }

    return (ssize_t) bytes_received;

#elif defined (ZCE_OS_LINUX)

    return ::readv (handle, buffers, iovcnt);

#endif
}

//
ssize_t zce::recvmsg (ZCE_SOCKET handle,
                      msghdr *msg,
                      int flags)
{

#if defined (ZCE_OS_WINDOWS)

    assert(msg->msg_iovlen < IOV_MAX);

    WSABUF wsa_buf[IOV_MAX];

    for (size_t i = 0; i < msg->msg_iovlen; ++i)
    {
        wsa_buf[i].buf = static_cast<CHAR *>(msg->msg_iov[i].iov_base);
        wsa_buf[i].len = static_cast<ULONG>(msg->msg_iov[i].iov_len);
    }

    DWORD bytes_received = 0;

    int result = ::WSARecvFrom ((SOCKET) handle,
                                wsa_buf,
                                (DWORD)(msg->msg_iovlen),
                                &bytes_received,
                                (DWORD *) &flags,
                                static_cast<sockaddr *>(msg->msg_name),
                                &msg->msg_namelen,
                                0,
                                0);

    if (result != 0)
    {
        errno = ::WSAGetLastError ();
        return -1;
    }

    return bytes_received;

#elif defined (ZCE_OS_LINUX)

    return ::recvmsg (handle, msg, flags);

#endif
}

ssize_t zce::sendmsg (ZCE_SOCKET handle,
                      const struct msghdr *msg,
                      int flags)
{
#if defined (ZCE_OS_WINDOWS)

    //
    assert(msg->msg_iovlen < IOV_MAX);

    WSABUF wsa_buf[IOV_MAX];

    for (size_t i = 0; i < msg->msg_iovlen; ++i)
    {
        wsa_buf[i].buf = static_cast<CHAR *>(msg->msg_iov[i].iov_base);
        wsa_buf[i].len = static_cast<ULONG>(msg->msg_iov[i].iov_len);
    }

    DWORD bytes_sent = 0;
    int result = ::WSASendTo ((SOCKET) handle,
                              wsa_buf,
                              (DWORD)msg->msg_iovlen,
                              &bytes_sent,
                              flags,
                              static_cast<const sockaddr *>(msg->msg_name),
                              msg->msg_namelen,
                              0,
                              0);

    if (result != 0)
    {
        errno = ::WSAGetLastError ();
        return -1;
    }

    return (ssize_t) bytes_sent;

#elif defined (ZCE_OS_LINUX)
    //
    return ::sendmsg (handle, msg, flags);
# endif

}



//--------------------------------------------------------------------------------------------
//������ȡlen�����ݣ�ֱ�����ִ���
ssize_t zce::recvn (ZCE_SOCKET handle,
                    void *buf,
                    size_t len,
                    int flags )
{
    ssize_t result = 0;
    bool error_occur = false;

    ssize_t onetime_recv = 0, bytes_recv = 0;

    //һ��׼��ȷ���յ���ô���ֽ�
    for (bytes_recv = 0; static_cast<size_t>(bytes_recv) < len; bytes_recv += onetime_recv)
    {

        //ʹ�ö˿ڽ��н���
        onetime_recv = zce::recv (handle,
                                  static_cast <char *> (buf) + bytes_recv,
                                  len - bytes_recv,
                                  flags);

        if (onetime_recv > 0)
        {
            continue;
        }
        //������ִ���,�˳�ѭ��
        else
        {
            //���ִ��󣬽��д���
            error_occur = true;
            result = onetime_recv;
            break;
        }
    }

    //�����������
    if (error_occur)
    {
        return result;
    }

    return bytes_recv;
}

//��������N�����ݣ�ֱ�����ִ���
ssize_t zce::sendn (ZCE_SOCKET handle,
                    const void *buf,
                    size_t len,
                    int flags)
{
    bool error_occur = false;
    ssize_t result = 0, bytes_send = 0, onetime_send = 0;

    //һ��׼��ȷ���յ���ô���ֽڣ�����һ�����ִ��󣬾��˳�
    for (bytes_send = 0; static_cast<size_t>(bytes_send) < len; bytes_send += onetime_send)
    {
        //�������ݣ���
        onetime_send = zce::send (handle,
                                  static_cast <const char *> (buf) + bytes_send,
                                  len - bytes_send,
                                  flags);

        if (onetime_send > 0)
        {
            continue;
        }
        //������ִ���,== 0һ�����Ƕ˿ڶϿ���==-1��ʾ����
        else
        {
            //���ִ��󣬽��д���
            error_occur = true;
            result = onetime_send;
            break;
        }
    }

    //�����˴��󣬷��ش��󷵻�ֵ
    if (error_occur)
    {
        return result;
    }

    return bytes_send;
}

//��ĳЩѡ�WIN32Ŀǰֻ֧��O_NONBLOCK
int zce::sock_enable (ZCE_SOCKET handle, int flags)
{

#if defined (ZCE_OS_WINDOWS)

    switch (flags)
    {
        case O_NONBLOCK:
            // nonblocking argument (1)
            // blocking:            (0)
        {
            u_long nonblock = 1;
            int zce_result = ::ioctlsocket (handle, FIONBIO, &nonblock);

            //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
            if ( SOCKET_ERROR == zce_result)
            {
                errno = ::WSAGetLastError ();
            }

            return zce_result;
        }

        default:
        {
            return (-1);
        }

    }

#elif defined (ZCE_OS_LINUX)
    int val = ::fcntl (handle, F_GETFL, 0);

    if (val == -1)
    {
        return -1;
    }

    // Turn on flags.
    ZCE_SET_BITS (val, flags);

    if (::fcntl (handle, F_SETFL, val) == -1)
    {
        return -1;
    }

    return 0;
#endif
}

//�ر�ĳЩѡ�WIN32Ŀǰֻ֧��O_NONBLOCK
int zce::sock_disable(ZCE_SOCKET handle, int flags)
{
#if defined (ZCE_OS_WINDOWS)

    switch (flags)
    {
        case O_NONBLOCK:
            // nonblocking argument (1)
            // blocking:            (0)
        {
            u_long nonblock = 0;
            int zce_result =  ::ioctlsocket (handle, FIONBIO, &nonblock);

            //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
            if ( SOCKET_ERROR == zce_result)
            {
                errno = ::WSAGetLastError ();
            }

            return zce_result;
        }

        default:
            return (-1);
    }

#elif defined (ZCE_OS_LINUX)
    int val = ::fcntl (handle, F_GETFL, 0);

    if (val == -1)
    {
        return -1;
    }

    // Turn on flags.
    ZCE_CLR_BITS (val, flags);

    if (::fcntl (handle, F_SETFL, val) == -1)
    {
        return -1;
    }

    return 0;
#endif
}


//���ʹ�ô����Ķ˿�,select �ǲ����ʵģ���Ҫʹ��EPOLL,��ʱ���Դ������ע��
#define HANDLEREADY_USE_EPOLL

//FD_SET ����� whlie(0)�����һ���澯�����Ӧ����windows�ڲ��Լ�û�д���á�΢��˵VS2005SP1���޸��ˣ�����
#if defined (ZCE_OS_WINDOWS)
#pragma warning(disable : 4127)
#pragma warning(disable : 6262)
#endif

//����ڣ�һ��ʱ���ڣ���ĳ��SOCKET�����ע�ĵ����¼��Ƿ񴥷���������������ش����¼�����������ɹ���һ�㴥������ֵ����1
int zce::handle_ready(ZCE_SOCKET handle,
                      ZCE_Time_Value *timeout_tv,
                      HANDLE_READY_TODO ready_todo)
{
#if defined ZCE_OS_WINDOWS || (defined ZCE_OS_LINUX && !defined HANDLEREADY_USE_EPOLL)

    fd_set handle_set_read, handle_set_write, handle_set_exeception;
    fd_set *p_set_read  = NULL, *p_set_write = NULL, *p_set_exception = NULL;
    FD_ZERO(&handle_set_read);
    FD_ZERO(&handle_set_write);
    FD_ZERO(&handle_set_exeception);




    if (HANDLE_READY_READ == ready_todo)
    {
        FD_SET(handle, &handle_set_read);
        p_set_read = &handle_set_read;
    }
    else if ( HANDLE_READY_WRITE == ready_todo)
    {
        FD_SET(handle, &handle_set_write);
        p_set_write = &handle_set_write;
    }
    else if (HANDLE_READY_EXCEPTION == ready_todo)
    {
        FD_SET(handle, &handle_set_exeception);
        p_set_exception = &handle_set_exeception;
    }
    else if (HANDLE_READY_ACCEPT == ready_todo)
    {
        //accept�¼������ö�ȡ�¼�
        FD_SET(handle, &handle_set_read);
        p_set_read = &handle_set_read;
    }
    else if (HANDLE_READY_CONNECTED == ready_todo)
    {
        //Ϊʲôǰ��д����ô�鷳����ʵ������Ϊ���CONNECTED�ĵ�ù����
        //���ȣ�CONNECT�Ĵ���Ҫ���ֳɹ���ʧ���¼�
        //Windows ������CONNECT, ʧ�ܵ����쳣���ɹ�����д�¼�
        //Windows ����CONNECT, ʧ�ܵ��ö�д�¼����ɹ�����д�¼�
        //LINUX �������������Ƿ�������ʧ�ܵ��ö�д�¼����ɹ�����д�¼�
        //���ԡ���������û�ио�������������
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
    //�������0Windows��VC++���и澯
    select_width = 0;
#elif defined (ZCE_OS_LINUX)
    select_width = int (handle) + 1;
#endif

    int result = zce::select (select_width,
                              p_set_read,
                              p_set_write,
                              p_set_exception,
                              timeout_tv);

    if (0 == result )
    {
        errno = ETIMEDOUT;
        return 0;
    }

    //���ִ���
    if (0 > result )
    {
        return result;
    }

    //���Ǵ������CONNECTED�ɹ���
    if (HANDLE_READY_CONNECTED == ready_todo)
    {
        //�����CONNECTED�������ػ����쳣���ض�����Ϊ�Ǵ���
        if (FD_ISSET(handle, p_set_read)
            || FD_ISSET(handle, p_set_exception))
        {
            return -1;
        }
    }

    return result;

#else

    //��EPOLL ����¼��������ŵ����ܴ�������ݶ࣬ȱ����ϵͳ����̫��
    int ret = 0;
    const int MAX_EVENT_NUMBER = 64;
    int epoll_fd = ::epoll_create( MAX_EVENT_NUMBER);

    struct epoll_event ep_event;
    if (HANDLE_READY_READ == ready_todo)
    {
        ep_event.events |= EPOLLIN;
    }
    else if ( HANDLE_READY_WRITE == ready_todo)
    {
        ep_event.events |= EPOLLOUT;
    }
    else if (HANDLE_READY_EXCEPTION == ready_todo)
    {
        ep_event.events |= EPOLLERR;
    }
    else if (HANDLE_READY_ACCEPT == ready_todo)
    {
        //accept�¼������ö�ȡ�¼�
        ep_event.events |= EPOLLIN;
    }
    else if (HANDLE_READY_CONNECTED == ready_todo)
    {
        //LINUX �������������Ƿ�������ʧ�ܵ��ö�д�¼����ɹ�����д�¼�
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

    //Ĭ��һֱ����
    int msec_timeout = -1;

    if (timeout_tv)
    {
        //����bluehu �����޸���������Σ�������С�����ǣ����ǸĴ���Ҫ����һ��ร�
        //����select�ĳ�ʱ�������Ծ�ȷ��΢�룬��epoll_wait�Ĳ���ֻ��ȷ������
        //����ʱʱ��С��1000΢��ʱ������20΢�룬��ʱ��ת���ɺ������0����
        //���������epoll_wait�Ļ�����ʱʱ�����0����С��1����������ͳһ��Ϊ1����

        msec_timeout = static_cast<int>( timeout_tv->total_msec_round());
    }

    int event_happen = 0;
    //EPOLL�ȴ��¼�������
    const int ONCE_MAX_EVENTS = 10;
    struct epoll_event once_events_ary[ONCE_MAX_EVENTS];

    event_happen = ::epoll_wait(epoll_fd, once_events_ary, ONCE_MAX_EVENTS, msec_timeout);

    //��������̣�����epoll_ctl EPOLL_CTL_DEL ɾ��ע�����,
    struct epoll_event event_del;
    event_del.events = 0;
    ::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, handle, &event_del);
    ::close(epoll_fd);

    if (0 == event_happen )
    {
        errno = ETIMEDOUT;
        return 0;
    }

    //���ִ���
    if (0 > event_happen )
    {
        return event_happen;
    }

    if (HANDLE_READY_CONNECTED == ready_todo)
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
//��ΪWINdows ��֧��ȡ��socket �Ƿ���������ģʽ������Windows �����޷���ȡ��socket��ѡ�Ȼ���ж��Ƿ�ȡ������ģʽ
//����������ر�֤���Socket ������ģʽ�ģ�����������


int zce::connect_timeout(ZCE_SOCKET handle,
                         const sockaddr *addr,
                         socklen_t addr_len,
                         ZCE_Time_Value &timeout_tv)
{

    int ret = 0;
    //���ܶԷ������ľ�����г�ʱ����
    ret = zce::sock_enable(handle, O_NONBLOCK);
    if (ret != 0)
    {
        zce::closesocket(handle);
        return -1;
    }

    ret = zce::connect(handle, addr, addr_len);
    //
    if (ret != 0)
    {
        //WINDOWS�·���EWOULDBLOCK��LINUX�·���EINPROGRESS
        int last_err = zce::last_error();

        if (EINPROGRESS != last_err &&  EWOULDBLOCK != last_err)
        {
            zce::closesocket(handle);
            return ret;
        }
    }

    //���г�ʱ����
    ret = zce::handle_ready(handle,
                            &timeout_tv,
                            zce::HANDLE_READY_CONNECTED);

    const int HANDLE_READY_ONE = 1;

    if (ret != HANDLE_READY_ONE)
    {
        zce::closesocket(handle);
        return -1;
    }

    ret = zce::sock_disable(handle, O_NONBLOCK);
    if (ret != 0)
    {
        zce::closesocket(handle);
        return -1;
    }

    return 0;
}


int zce::connect_timeout(ZCE_SOCKET handle,
                         const char *host_name,
                         uint16_t port,
                         sockaddr *host_addr,
                         socklen_t addr_len,
                         ZCE_Time_Value &timeout_tv)
{
    //ֻ����IPV4��IPV6�������������֮
    int ret = 0;

    //�����Hostname�������DNS�������õ����ӵ�IP��ַ
    if (host_name)
    {
        ret = zce::getaddrinfo_to_addr(host_name,
                                       host_addr,
                                       addr_len);
        if (ret != 0)
        {
            return ret;
        }

        //���ö˿�
        if (sizeof(sockaddr_in) == addr_len)
        {
            sockaddr_in *addr4 = reinterpret_cast<sockaddr_in *>(host_addr);
            addr4->sin_port = htons(port);
        }
        else if (sizeof(sockaddr_in6) == addr_len)
        {
            sockaddr_in6 *addr6 = reinterpret_cast<sockaddr_in6 *>(host_addr);
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


ssize_t zce::recvn_timeout (ZCE_SOCKET handle,
                            void *buf,
                            size_t len,
                            ZCE_Time_Value &timeout_tv,
                            int flags,
                            bool only_once)
{

    ssize_t result = 0;
    bool error_occur = false;

    //���ֻ�ȴ�����ʱ��
#if defined  (ZCE_OS_WINDOWS)

    //WIN32��ֻ�ܼ򵥵Ĵ򿪷�������
    zce::sock_enable (handle, O_NONBLOCK);

#elif defined  (ZCE_OS_LINUX)
    //Linux�򵥺ֻܶ࣬��Ҫ����һ�η�����һЩ�������޶���OK��
    //MSG_DONTWAITѡ�ߣ�WIN32��֧��
    flags |= MSG_DONTWAIT;
#endif

    int ret = 0;
    ssize_t onetime_recv = 0, bytes_recv = 0;

    //һ��׼��ȷ���յ���ô���ֽ�
    for (bytes_recv = 0; static_cast<size_t>(bytes_recv) < len; bytes_recv += onetime_recv)
    {

        //�ȴ��˿�׼�����˴��������¼��������ʵ���Ͻ������ۣ����timeout_tv Ӧ�ü�ȥ���ĵ�ʱ��
        //LINUX��SELECT����������飬��WINDOWS�Ĳ���
        ret = zce::handle_ready (handle,
                                 &timeout_tv,
                                 HANDLE_READY_READ);

        const int HANDLE_READY_ONE = 1;

        if ( ret != HANDLE_READY_ONE )
        {
            //
            if ( 0 == ret)
            {
                errno = ETIMEDOUT;
                result = -1;
            }

            error_occur = true;
            result = ret;
            break;
        }

        //ʹ�÷������˿ڽ��н���
        onetime_recv = zce::recv (handle,
                                  static_cast <char *> (buf) + bytes_recv,
                                  len - bytes_recv,
                                  flags);

        if (onetime_recv > 0)
        {
            //���ֻ��ȡһ������
            if (only_once)
            {
                bytes_recv += onetime_recv;
                break;
            }
            continue;
        }
        //������ִ���,== 0һ�����Ƕ˿ڶϿ���==-1��ʾ
        else
        {
            //==-1�����Ǳ�ʾ�������󣬽���ѭ������
            if (onetime_recv < 0 && errno == EWOULDBLOCK)
            {
                // Did select() succeed?
                onetime_recv = 0;
                continue;
            }

            //���ִ��󣬽��д���
            error_occur = true;
            result = onetime_recv;
            break;
        }
    }

    //���ֻ�ȴ�����ʱ�䣬�ָ�ԭ������ģʽ
#if defined  (ZCE_OS_WINDOWS)
    zce::sock_disable (handle, O_NONBLOCK);
#endif

    if (error_occur)
    {
        return result;
    }

    return bytes_recv;
}


//���������WIN32������֤���Socket ������ģʽ�ģ�����������
ssize_t zce::sendn_timeout(ZCE_SOCKET handle,
                           const void *buf,
                           size_t len,
                           ZCE_Time_Value &timeout_tv,
                           int flags )
{

    bool error_occur = false;
    ssize_t result = 0, bytes_send = 0, onetime_send = 0;

    //���ֻ�ȴ�����ʱ��
#if defined  (ZCE_OS_WINDOWS)

    //�ȴ�һ��ʱ��Ҫ�������⴦��
    //WIN32��ֻ�ܼ򵥵Ĵ򿪷�������
    zce::sock_enable (handle, O_NONBLOCK);

#elif defined  (ZCE_OS_LINUX)
    //Linux�򵥺ֻܶ࣬��Ҫ����һ�η�����һЩ�������޶���OK��
    //MSG_DONTWAIT��־��WIN32��֧��
    flags |= MSG_DONTWAIT;
#endif

    int ret = 0;

    //һ��׼��ȷ���յ���ô���ֽ�
    for (bytes_send = 0; static_cast<size_t>(bytes_send) < len; bytes_send += onetime_send)
    {
        //�����ڴ��������Ϻ�recv��һ������Ϊsend��������Ҫ�ȴ���������select�˷�
        //ʹ�÷������˿ڽ��н��գ�
        onetime_send = zce::send (handle,
                                  static_cast <const char *> (buf) + bytes_send,
                                  len - bytes_send,
                                  flags);

        if (onetime_send > 0)
        {
            continue;
        }
        //������ִ���,== 0һ�����Ƕ˿ڶϿ���==-1��ʾ����
        else
        {
            //==-1�����Ǳ�ʾ�������󣬽���ѭ������
            if ( onetime_send < 0 && errno == EWOULDBLOCK)
            {
                //׼������select
                onetime_send = 0;

                //�ȴ��˿�׼�����˴������¼��������ʵ���Ͻ������timeout_tv Ӧ�ü�ȥ���ĵ�ʱ��
                ret = zce::handle_ready (handle,
                                         &timeout_tv,
                                         HANDLE_READY_WRITE);

                const int HANDLE_READY_ONE = 1;

                if (ret == HANDLE_READY_ONE)
                {
                    continue;
                }
                else
                {
                    //yunfeiyang:��coverityɨ�跢�ֵģ������˺ü�����ܵ�bug.���Ӱ������������ط�����ԭ��д���� result = -1
                    if ( 0 == ret)
                    {
                        errno = ETIMEDOUT;
                        ret = -1;
                    }

                    error_occur = true;
                    result = ret;
                    break;
                }
            }

            //���ִ��󣬽��д���
            error_occur = true;
            result = onetime_send;
            break;
        }
    }

#if defined  ZCE_OS_WINDOWS
    //�رշ�����״̬
    zce::sock_disable (handle, O_NONBLOCK);
#endif

    //�����˴��󣬷��ش��󷵻�ֵ
    if (error_occur)
    {
        return result;
    }

    return bytes_send;
}

//��UDP������,Ҳ���г�ʱ�����������յ��������ݾ��Ƕ����ˣ���ʱ��selectʵ��
ssize_t zce::recvfrom_timeout (ZCE_SOCKET handle,
                               void *buf,
                               size_t len,
                               sockaddr *from,
                               socklen_t *from_len,
                               ZCE_Time_Value &timeout_tv,
                               int flags)
{
    //���ֻ�ȴ�����ʱ��
#if defined  (ZCE_OS_WINDOWS)

    //�ȴ�һ��ʱ��Ҫ�������⴦��
    //WIN32��ֻ�ܼ򵥵Ĵ򿪷�������
    zce::sock_enable (handle, O_NONBLOCK);

#elif defined  (ZCE_OS_LINUX)
    //Linux�򵥺ֻܶ࣬��Ҫ����һ�η�����һЩ�������޶���OK��
    //MSG_DONTWAIT��־��WIN32��֧��
    flags |= MSG_DONTWAIT;
#endif

    ssize_t recv_result = 0;
    int ret = zce::handle_ready (handle,
                                 &timeout_tv,
                                 HANDLE_READY_READ);

    const int HANDLE_READY_ONE = 1;

    if (ret == HANDLE_READY_ONE)
    {
        //ʹ�÷������˿ڽ��н���
        recv_result = zce::recvfrom (handle,
                                     static_cast <char *> (buf),
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

    //���ֻ�ȴ�����ʱ�䣬�ָ�ԭ�е�״̬
#if defined  ZCE_OS_WINDOWS
    zce::sock_disable (handle, O_NONBLOCK);
#endif

    return recv_result;
}

//UDP�ķ�����ʱ�ǲ��������ģ����ó�ʱ����д���������ȫ��Ϊ�˺�ǰ�����
//����UDP������,����ʱ�������������ʵ���Ͻ���û�г�ʱ����
ssize_t zce::sendto_timeout (ZCE_SOCKET handle,
                             const void *buf,
                             size_t len,
                             const sockaddr *addr,
                             socklen_t addrlen,
                             ZCE_Time_Value & /*timeout_tv*/,
                             int flags)
{
    return zce::sendto(handle,
                       buf,
                       len,
                       flags,
                       addr,
                       addrlen
                      );
}

//--------------------------------------------------------------------------------------------
//���麯���ṩ����Ϊ�˴�����ԣ���ʱ�������ṩ
//ʹ��SO_RCVTIMEO��SO_SNDTIMEO�õ�һ�鳬ʱ������

//ע��SO_RCVTIMEO,SO_SNDTIMEO,ֻ��WIN socket 2��֧��
ssize_t zce::recvn_timeout2 (ZCE_SOCKET handle,
                             void *buf,
                             size_t len,
                             ZCE_Time_Value &timeout_tv,
                             int flags)
{
    int ret = 0;

    //��Ȼ������һ��������װ�����������ڲ�ʵ�ֲ�һ�����㻹��Ҫ��Ѫ��
#if defined  ZCE_OS_WINDOWS
    //��ʱ�ĺ���
    DWORD  msec_timeout = static_cast<DWORD>(timeout_tv.total_msec());
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, (const void *)(&msec_timeout), sizeof(DWORD));

#elif defined  ZCE_OS_LINUX
    timeval sockopt_tv = timeout_tv;
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, (const void *)(&sockopt_tv), sizeof(timeval));
#endif

    if (0 != ret )
    {
        return -1;
    }

    ssize_t result = 0, bytes_recv = 0, onetime_recv = 0;
    bool error_occur = false;

    //һ��Ҫ�յ�len���ȵ��ֽ�
    for (bytes_recv = 0; static_cast<size_t>(bytes_recv) < len; bytes_recv += onetime_recv)
    {
        //ʹ�÷������˿ڽ��н���
        onetime_recv = zce::recv (handle,
                                  static_cast <char *> (buf) + bytes_recv,
                                  len - bytes_recv,
                                  flags);

        if (onetime_recv > 0)
        {
            continue;
        }
        //������ִ���,== 0һ�����Ƕ˿ڶϿ���==-1��ʾ
        else
        {
            //���ִ��󣬽��д���
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

    //Ҫ��Ҫ��ԭԭ����SO_RCVTIMEO?���ˣ���������ʱ���õط�Ӧ�û�һֱʹ��

    return bytes_recv;
}

//
ssize_t zce::sendn_timeout2 (ZCE_SOCKET handle,
                             void *buf,
                             size_t len,
                             ZCE_Time_Value &timeout_tv,
                             int flags)
{

    int ret = 0;

    //��Ȼ������һ��������װ�����������ڲ�ʵ�ֲ�һ�����㻹��Ҫ��Ѫ��
#if defined  ZCE_OS_WINDOWS

    DWORD  msec_timeout = static_cast<DWORD>(timeout_tv.total_msec());
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, (const void *)(&msec_timeout), sizeof(DWORD));

#elif defined  ZCE_OS_LINUX
    timeval sockopt_tv = timeout_tv;
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, (const void *)(&sockopt_tv), sizeof(timeval));

#endif

    if (0 != ret )
    {
        return -1;
    }

    ssize_t result = 0, bytes_send = 0, onetime_send = 0;
    bool error_occur = false;

    //һ��Ҫ���͵�len���ȵ��ֽ�
    for (bytes_send = 0; static_cast<size_t>(bytes_send) < len; bytes_send += onetime_send)
    {
        //ʹ�÷������˿ڽ��н���
        onetime_send = zce::send (handle,
                                  static_cast <char *> (buf) + bytes_send,
                                  len - bytes_send,
                                  flags);

        //��ʵ���Ӧ�õ�����ʱʱ�䣬�Ǻǣ�͵����
        if (onetime_send > 0)
        {
            continue;
        }
        //������ִ���,== 0һ�����Ƕ˿ڶϿ���==-1��ʾ
        else
        {
            //���ִ��󣬽��д���
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

//��UDP������,Ҳ���г�ʱ�����������յ��������ݾ��Ƕ����ˣ���ʱ��SO_RCVTIMEOʵ��
ssize_t zce::recvfrom_timeout2 (ZCE_SOCKET handle,
                                void *buf,
                                size_t len,
                                sockaddr *addr,
                                socklen_t *addrlen,
                                ZCE_Time_Value &timeout_tv,
                                int flags)
{
    int ret = 0;
    //��Ȼ������һ��������װ�����������ڲ�ʵ�ֲ�һ�����㻹��Ҫ��Ѫ��
#if defined (ZCE_OS_WINDOWS)

    DWORD  msec_timeout = static_cast<DWORD>(timeout_tv.total_msec());
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, (const void *)(&msec_timeout), sizeof(DWORD));

#elif defined (ZCE_OS_LINUX)
    timeval sockopt_tv = timeout_tv;
    ret = zce::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, (const void *)(&sockopt_tv), sizeof(timeval));
#endif

    //����socket���ƺ����ķ�װ������-1��ʶʧ�ܡ�
    if (0 != ret)
    {
        return -1;
    }

    //ʹ�÷������˿ڽ��н���
    ssize_t recv_result = zce::recvfrom (handle,
                                         static_cast <char *> (buf),
                                         len,
                                         flags,
                                         addr,
                                         addrlen);

    return recv_result;
}

//UDP�ķ�����ʱ�ǲ��������ģ����ó�ʱ����д���������ȫ��Ϊ�˺�ǰ�����
//����UDP������,����ʱ�������������ʵ���Ͻ���û�г�ʱ����
ssize_t zce::sendto_timeout2 (ZCE_SOCKET handle,
                              const void *buf,
                              size_t len,
                              const sockaddr *addr,
                              socklen_t addrlen,
                              ZCE_Time_Value & /*timeout_tv*/,
                              int flags)
{
    return zce::sendto(handle,
                       buf,
                       len,
                       flags,
                       addr,
                       addrlen
                      );
}

//--------------------------------------------------------------------------------------------

//ת���ַ����������ַ����һ������af�ǵ�ַ�壬ת�������dst�У�
//ע���������return 1��ʶ�ɹ���return ������ʶ����return 0��ʶ��ʽƥ�����
int zce::inet_pton (int family,
                    const char *strptr,
                    void *addrptr)
{
#if defined (ZCE_OS_WINDOWS)

    //Ϊʲô��������inet_pton ,(Vista��֧��),��������ע�͵�ԭ���ǣ������ͨ���ˣ�����Ҳû����,XP��WINSERVER2003���޷�ʹ�ã�
    //VISTA,WINSERVER2008��_WIN32_WINNT����0x0600
#if defined ZCE_SUPPORT_WINSVR2008
    return ::inet_pton(family, strptr, addrptr);
#else

    //sscanfȡ�õ���ĸ���
    int get_fields_num = 0;

    if (  AF_INET == family )
    {
        struct in_addr *in_val = reinterpret_cast<in_addr *>(addrptr);

        //Ϊʲô��ֱ����in_val->S_un.S_un_b.s_b1�����
        const int NUM_FIELDS_AF_INET = 4;
        uint32_t u[NUM_FIELDS_AF_INET] = {0};
        get_fields_num = sscanf(strptr,
                                "%u%.%u%.%u%.%u",
                                &(u[0]),
                                &(u[1]),
                                &(u[2]),
                                &(u[3])
                               );

        //������ַ������Ϻ���׼

        if ( NUM_FIELDS_AF_INET != get_fields_num || u[0] > 0xFF || u[1] > 0xFF || u[2] > 0xFF || u[3] > 0xFF )
        {
            return 0;
        }

        uint32_t u32_addr = u[0] << 24 | u[1] << 16 | u[2] << 8 | u[3];
        in_val->S_un.S_addr = htonl(u32_addr);


        //ע�⣬����1��ʶ�ɹ�
        return (1);
    }
    else if ( AF_INET6 == family )
    {
        //fucnking ����RFC1884�ĸ��ǣ���Ϳ��Ǽ򻯣�Ҳ����һ��д���������ν���ת���ѡ�����һֻ������
        //RFC1884 �Բ����Ҳ�֧��IPV6�����Ը�ʽ��IPV4ӳ���ʽ���Ǹ�̫̫�鷳�ˡ�
        //IPV6��0���Ը�ʽ���� '::'�ڿ�ͷ��::FFFF:A:B  '::'���м�� A:B:::C(�㲻֪���м���0��ʡ����)��'::'��β���ϵ�
        //IPV4ӳ���IPV6�ĸ�ʽ���Ա�ʶ��::FFFF:A.B.C.D

        //������ַ������Ϻ���׼
        const int NUM_FIELDS_AF_INET6 = 8;

        const char INET6_STR_UP_CHAR[] = {"1234567890ABCDEF"};

        //����0
        memset(addrptr, 0, sizeof(in_addr6));
        struct in_addr6 *in_val6 = reinterpret_cast<in_addr6 *>(addrptr);

        size_t in_str_len = strlen(strptr);
        //ǰһ���ַ��Ƿ���ð��
        bool pre_char_colon = false;
        //�ַ����Ƿ�
        bool str_abbreviate = false;
        //
        int havedot_ipv4_mapped = 0;

        //
        size_t word_start_pos = 0;

        uint16_t for_word[NUM_FIELDS_AF_INET6] = {0};
        uint16_t back_word[NUM_FIELDS_AF_INET6] = {0};

        size_t forword_num = 0, backword_num = 0;

        for (size_t i = 0; i <= in_str_len; ++i)
        {
            //
            if (':' == strptr [i] )
            {
                //���������ַ�Ҳ�ǣ�����ʶ����д��ʽ
                if ( pre_char_colon == true  )
                {
                    //���û�з�������д
                    if (false == str_abbreviate)
                    {
                        str_abbreviate = true;
                    }
                    //�����ܷ���������д��
                    else
                    {
                        return 0;
                    }

                    continue;
                }

                //.���ֺ󣬲����ܳ��֣�����ʽ����
                if (havedot_ipv4_mapped > 0)
                {
                    return 0;
                }

                //�����ܳ���8��ð��
                if (backword_num + forword_num >= NUM_FIELDS_AF_INET6)
                {
                    return 0;
                }

                //����Ѿ�����д����ô��¼���������ݶ�����
                if (str_abbreviate)
                {
                    get_fields_num = sscanf(strptr + word_start_pos, "%hx:",  &(back_word[backword_num]));
                    ++backword_num;
                }
                else
                {
                    get_fields_num = sscanf(strptr + word_start_pos, "%hx:",  &(for_word[forword_num]));
                    ++forword_num;
                }

                pre_char_colon = true;
                continue;
            }
            //IPV4ӳ��IPV6��д��
            else if ( '.' == strptr [i] )
            {
                //���ǰ����:,����
                if (pre_char_colon)
                {
                    return 0;
                }

                ++havedot_ipv4_mapped;
            }
            else
            {
                //���������ַ�����Ϊ���󣬹�����
                if ( NULL == strchr(INET6_STR_UP_CHAR, toupper(strptr [i])) )
                {
                    return 0;
                }

                //���ǰ��һ����:
                if (pre_char_colon)
                {
                    pre_char_colon = false;
                    word_start_pos = i;
                }

                continue;
            }
        }

        //�����һ��WORD����2��WORD���д���

        //������.�����ҳ�������3�Σ�havedot_ipv4_mapped ��
        if (havedot_ipv4_mapped > 0)
        {
            const int NUM_FIELDS_AF_INET = 4;
            uint32_t u[NUM_FIELDS_AF_INET] = {0};
            get_fields_num = sscanf(strptr + word_start_pos,
                                    "%u%.%u%.%u%.%u",
                                    &(u[0]),
                                    &(u[1]),
                                    &(u[2]),
                                    &(u[3])
                                   );

            //������ַ������Ϻ���׼
            if ( NUM_FIELDS_AF_INET != get_fields_num || u[0] > 0xFF || u[1] > 0xFF || u[2] > 0xFF || u[3] > 0xFF )
            {
                return 0;
            }

            back_word[backword_num] = static_cast<uint16_t>( u[0] << 8 | u[1]);
            ++backword_num;
            back_word[backword_num] = static_cast<uint16_t>( u[2] << 8 | u[3]);
            ++backword_num;
        }
        else
        {
            if (false == pre_char_colon)
            {
                sscanf(strptr + word_start_pos, "%hx",  &(back_word[backword_num]));
                ++backword_num;
            }
        }

        //��ÿһ��WORD���и�ֵ��ǰ��ֵǰ��ģ��ٸ�ֵ�����,�м�������ʡ�Ծ���0�ˡ�������
        //���ת��ֻ����WINDOWS���ã���Щunionֻ��WINDOWS���ж��壩�����Ҫͨ�ã�Ҫ�Ĵ��롣
        for (size_t k = 0; k < forword_num; ++k)
        {
            in_val6->u.Word[k] = htons( for_word[k]);
        }

        for (size_t k = 0; k < backword_num; ++k)
        {
            in_val6->u.Word[NUM_FIELDS_AF_INET6 - backword_num + k] = htons( back_word[k]);
        }

        //����1��ʶ�ɹ�
        return (1);
    }
    //��֧��
    else
    {
        errno = EAFNOSUPPORT;
        return 0;
    }
#endif

#elif defined (ZCE_OS_LINUX)
    //LINuX�����������
    return ::inet_pton(family, strptr, addrptr);
#endif
}

//����ԭ������[�������ʮ���ơ� ��> ��������],IPV6����:�ָ�16����ת����128λ����
const char *zce::inet_ntop(int family,
                           const void *addrptr,
                           char *strptr,
                           size_t len)
{

#if defined (ZCE_OS_WINDOWS)

    //���ݲ�ͬ��Э��ؽ��в�ͬ�Ĵ���
    if (  AF_INET == family )
    {
        const struct in_addr *in_val = reinterpret_cast<const in_addr *>(addrptr);
        int ret_len = snprintf(strptr,
                               len,
                               "%u.%u.%u.%u",
                               in_val->S_un.S_un_b.s_b1,
                               in_val->S_un.S_un_b.s_b2,
                               in_val->S_un.S_un_b.s_b3,
                               in_val->S_un.S_un_b.s_b4);

        //��ʽ���ַ���ʧ��
        if (ret_len > static_cast<int>(len) || ret_len <= 0 )
        {
            errno = ENOSPC;
            return NULL;
        }

        return strptr;
    }
    else if ( AF_INET6 == family )
    {
        //�Բ�����ֻ֧��ת����IPV6�ı�׼��ʽ����֧��ת����IPV6�����Ը�ʽ��IPV4ӳ���ʽ��

        const struct in_addr6 *in_val6 = reinterpret_cast<const in_addr6 *>(addrptr);

        const int NUM_FIELDS_AF_INET6 = 8;
        uint16_t u[NUM_FIELDS_AF_INET6];

        //��Ϊ��short������Ҫת���ɱ�������
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

        if (ret_len > static_cast<int>(len) || ret_len <= 0 )
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

#elif defined (ZCE_OS_LINUX)
    //LINuX�����������
    return ::inet_ntop(family, addrptr, strptr, len);
#endif
}

//���IP��ַ��Ϣ���ڲ��ǲ�ʹ�þ�̬�������̰߳�ȫ��BUF����IPV4���ٳ���>15.IPV6���ٳ���>39
const char *zce::socketaddr_ntop(const sockaddr *sock_addr,
                                 char *str_ptr,
                                 size_t str_len)
{
    //���ݲ�ͬ�ĵ�ַЭ���壬����ת��
    if (sock_addr->sa_family == AF_INET)
    {
        const sockaddr_in *sockadd_ipv4 = reinterpret_cast<const sockaddr_in *>(sock_addr);
        return zce::inet_ntop(AF_INET,
                              (void *)(&(sockadd_ipv4->sin_addr)),
                              str_ptr,
                              str_len);
    }
    else if (sock_addr->sa_family == AF_INET6)
    {
        const sockaddr_in6 *sockadd_ipv6 = reinterpret_cast<const sockaddr_in6 *>(sock_addr);
        return zce::inet_ntop(AF_INET6,
                              (void *)(&(sockadd_ipv6->sin6_addr)),
                              str_ptr, str_len);
    }
    else
    {
        errno = EAFNOSUPPORT;
        return NULL;
    }
}

//���IP��ַ��Ϣ�Լ��˿���Ϣ���ڲ��ǲ�ʹ�þ�̬�������̰߳�ȫ��BUF����IPV4���ٳ���>21.IPV6���ٳ���>45
const char *zce::socketaddr_ntop_ex(const sockaddr *sock_addr,
                                    char *str_ptr,
                                    size_t str_len,
                                    size_t &use_len,
                                    bool out_port_info)
{
    uint16_t addr_port = 0;
    const char *ret_str = nullptr;
    use_len = 0;
    //���ݲ�ͬ�ĵ�ַЭ���壬����ת������ʹ�������Ǹ�������ԭ������Ϊ����ͬʱҪ���ж�ȡport�Ĳ���
    if (sock_addr->sa_family == AF_INET)
    {
        const sockaddr_in *sockadd_ipv4 = reinterpret_cast<const sockaddr_in *>(sock_addr);
        addr_port = ntohs(sockadd_ipv4->sin_port);
        ret_str = zce::inet_ntop(AF_INET,
                                 (void *)(&(sockadd_ipv4->sin_addr)),
                                 str_ptr,
                                 str_len);
    }
    else if (sock_addr->sa_family == AF_INET6)
    {
        const sockaddr_in6 *sockadd_ipv6 = reinterpret_cast<const sockaddr_in6 *>(sock_addr);
        addr_port = ntohs(sockadd_ipv6->sin6_port);
        ret_str = zce::inet_ntop(AF_INET6,
                                 (void *)(&(sockadd_ipv6->sin6_addr)),
                                 str_ptr, str_len);
    }
    else
    {
        errno = EAFNOSUPPORT;
        return nullptr;
    }

    //������ش���
    if (nullptr == ret_str )
    {
        return nullptr;
    }

    use_len += strlen(str_ptr);
    if (out_port_info)
    {
        //ǰ���Ѿ������ˣ�������жϷ�����
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
���Ƕ���ҵ����IP����һ����RFC1918�ж���ķ�Internet���ӵ������ַ��
Ҳ��Ϊ˽�е�ַ����Internet��ַ��Ȩ������IANA�����Ƶ�IP��ַ���䷽���У�
���������������ַ����������Internet�ϵ�ר����ʹ�á����ֱ��ǣ�
A�ࣺ10.0.0.0 ~ 10.255.255.255��
B�ࣺ172.16.0.0 ~ 172.31.255.255��
C�ࣺ192.168.0.0 ~ 192.168.255.255��
���е�һ��˽�е�ַ�����ǣ�192.168.0.0������������IP��������õ����Ρ�
IANA��֤��Щ����Ų�����������Internet�ϵ��κ����磬
����κ��˶��������ɵ�ѡ����Щ�����ַ��Ϊ�Լ���˽�������ַ��
������ĺϷ�IP���������£���ҵ���������Բ���˽��IP��ַ�������ַ���䷽����
��ҵ���������롢DMZ��ʹ�úϷ�IP��ַ��
�����ȫ0��Ҳ���Լ��ڲ�ip <==== ������Ǹ�ͬѧ�ӵģ������ĳ�̶ֳ����ǶԵģ���Ϊ0ֻ������ڱ������ж���
*/

#if !defined ZCE_IS_INTERNAL
#define ZCE_IS_INTERNAL(ip_addr)   ((ip_addr >= 0x0A000000 && ip_addr <= 0x0AFFFFFF ) ||  \
                                    (ip_addr >= 0xAC100000 && ip_addr <= 0xAC1FFFFF) ||  \
                                    (ip_addr >= 0xC0A80000 && ip_addr <= 0xC0A8FFFF) ||  \
                                    (ip_addr == INADDR_ANY))
#endif

//���һ����ַ�Ƿ���������ַ
bool zce::is_internal(const sockaddr_in *sock_addr_ipv4)
{
    uint32_t ip_addr = zce::get_ip_address(sock_addr_ipv4);

    //���3���ַ
    if (ZCE_IS_INTERNAL(ip_addr))
    {
        return true;
    }
    return false;
}

bool zce::is_internal(uint32_t ipv4_addr_val)
{
    //���3���ַ
    if (ZCE_IS_INTERNAL(ipv4_addr_val))
    {
        return true;
    }
    return false;
}



//-------------------------------------------------------------------------------------
//����������ת��IP��ַ�ļ�������

//ͨ�������õ���ص�IP��ַ
hostent *zce::gethostbyname(const char *hostname)
{
    return ::gethostbyname(hostname);
}

// GNU extensions
hostent *zce::gethostbyname2(const char *hostname,
                             int af)
{
#if defined (ZCE_OS_WINDOWS)

    hostent *hostent_ptr = ::gethostbyname(hostname);

    //
    if (hostent_ptr->h_addrtype != af)
    {
        return NULL;
    }

    return hostent_ptr;

#elif defined (ZCE_OS_LINUX)
    return ::gethostbyname2(hostname, af);
#endif
}

//�Ǳ�׼����,�õ�ĳ��������IPV4�ĵ�ַ��Ϣ������ʹ�������Ƚ����׺ͷ���
//name ����
//uint16_t service_port �˿ںţ�������
//ary_addr_num  ,������������������ʶary_sock_addr�ĸ��������ʱ��ʶ���صĶ�������
//ary_sock_addr ,������������صĵ�ַ����
int zce::gethostbyname_inary(const char *hostname,
                             uint16_t service_port,
                             size_t *ary_addr_num,
                             sockaddr_in ary_sock_addr[])
{
    //��ʵ�������û������
    struct hostent *hostent_ptr = ::gethostbyname(hostname);

    if (!hostent_ptr)
    {
        return -1;
    }

    if (hostent_ptr->h_addrtype != AF_INET)
    {
        errno = EINVAL;
        return -1;
    }

    //��鷵��
    ZCE_ASSERT(hostent_ptr->h_length == sizeof(in_addr));

    //ѭ���õ����е�IP��ַ��Ϣ
    size_t i = 0;
    char **addr_pptr = hostent_ptr->h_addr_list;

    for (; (i < *ary_addr_num) && (*addr_pptr != NULL); addr_pptr++, ++i)
    {
        ary_sock_addr[i].sin_family = AF_INET;
        //��������������
        memcpy(&(ary_sock_addr[i].sin_addr), addr_pptr, hostent_ptr->h_length);
        //�˿�ת����������
        ary_sock_addr[i].sin_port = htons(service_port);
    }

    //��¼����
    *ary_addr_num = i;

    return 0;
}

//�Ǳ�׼����,�õ�ĳ��������IPV6�ĵ�ַ��Ϣ������ʹ�������Ƚ����׺ͷ���
int zce::gethostbyname_in6ary(const char *hostname,
                              uint16_t service_port,
                              size_t *ary_addr6_num,
                              sockaddr_in6 ary_sock_addr6[])
{
    //��ʵ�������û������
    struct hostent *hostent_ptr = ::gethostbyname(hostname);

    if (!hostent_ptr)
    {
        return -1;
    }

    if (hostent_ptr->h_addrtype != AF_INET6)
    {
        errno = EINVAL;
        return -1;
    }

    //��鷵�صĵ�ַʵϰ�ǲ���IPV6��
    ZCE_ASSERT(hostent_ptr->h_length == sizeof(in6_addr));

    //ѭ���õ����е�IP��ַ��Ϣ
    size_t i = 0;
    char **addr_pptr = hostent_ptr->h_addr_list;

    for (; (i < *ary_addr6_num) && (*addr_pptr != NULL); addr_pptr++, ++i)
    {
        ary_sock_addr6[i].sin6_family = AF_INET6;
        //��������������
        memcpy(&(ary_sock_addr6[i].sin6_addr), addr_pptr, hostent_ptr->h_length);
        //�˿�ת����������
        ary_sock_addr6[i].sin6_port = htons(service_port);
    }

    //��¼����
    *ary_addr6_num = i;

    return 0;
}

//���ݵ�ַ�õ������ĺ���,�Ƽ�ʹ���������getnameinfo ,
hostent *zce::gethostbyaddr(const void *addr,
                            socklen_t len,
                            int family)
{
    return ::gethostbyaddr((const char *)addr, len, family);
};

//�Ǳ�׼������ͨ��IPV4��ַȡ������
int zce::gethostbyaddr_in(const sockaddr_in *sock_addr,
                          char *host_name,
                          size_t name_len)
{
    struct hostent *hostent_ptr = zce::gethostbyaddr(sock_addr,
                                                     sizeof(sockaddr_in),
                                                     sock_addr->sin_family
                                                    );

    //�������ʧ��
    if (!hostent_ptr )
    {
        return -1;
    }

    strncpy(host_name, hostent_ptr->h_name, name_len);

    return 0;
}

//�Ǳ�׼������ͨ��IPV6��ַȡ������
int zce::gethostbyaddr_in6(const sockaddr_in6 *sock_addr6,
                           char *host_name,
                           size_t name_len)
{

    struct hostent *hostent_ptr = zce::gethostbyaddr(sock_addr6,
                                                     sizeof(sockaddr_in6),
                                                     sock_addr6->sin6_family
                                                    );

    //�������ʧ��
    if (!hostent_ptr )
    {
        return -1;
    }

    strncpy(host_name, hostent_ptr->h_name, name_len);

    return 0;
}

//ͨ�������õ���������ַ��Ϣ������ͬʱ�õ�IPV4����IPV6�ĵ�ַ
int zce::getaddrinfo( const char *nodename,
                      const char *service,
                      const addrinfo *hints,
                      addrinfo **result )
{
    return ::getaddrinfo(nodename, service, hints, result);
}

//�ͷ�getaddrinfo�õ��Ľ��
void zce::freeaddrinfo(struct addrinfo *result)
{
    return ::freeaddrinfo(result);
}

//
int zce::getaddrinfo_result_to_addr(addrinfo *result,
                                    sockaddr *addr,
                                    socklen_t addr_len)
{
    addrinfo *prc_node = result;
    for (; (prc_node != NULL); prc_node = prc_node->ai_next)
    {
        //ֻȡ��Ӧ�ĵ�ַ
        if (addr_len == static_cast<socklen_t>( prc_node->ai_addrlen))
        {
            memcpy(addr, prc_node->ai_addr, prc_node->ai_addrlen);
        }
    }
    if (nullptr == prc_node)
    {
        return -1;
    }
    return 0;
}


//��getaddrinfo�Ľ�����мӹ��������������
void zce::getaddrinfo_result_to_addrary(addrinfo *result,
                                        size_t *ary_addr_num,
                                        sockaddr_in ary_addr[],
                                        size_t *ary_addr6_num,
                                        sockaddr_in6 ary_addr6[])
{

    addrinfo *prc_node = result;
    size_t num_addr = 0, num_addr6 = 0;
    for (size_t i = 0; (i < *ary_addr_num) && (prc_node != NULL); prc_node = prc_node->ai_next, ++i)
    {
        //ֻȡ��Ӧ�ĵ�ַ
        if (AF_INET == prc_node->ai_family)
        {
            memcpy(&(ary_addr[i]), prc_node->ai_addr, prc_node->ai_addrlen);
            ++num_addr;
        }
    }
    //��¼����
    *ary_addr_num = num_addr;

    prc_node = result;
    for (size_t j = 0; (j < *ary_addr6_num) && (prc_node != NULL); prc_node = prc_node->ai_next, ++j)
    {
        if (AF_INET6 == prc_node->ai_family)
        {
            memcpy(&(ary_addr6[j]), prc_node->ai_addr, prc_node->ai_addrlen);
            ++num_addr6;
        }
    }
    *ary_addr6_num = num_addr6;
}

//�Ǳ�׼����,�õ�ĳ��������IPV4�ĵ�ַ���飬ʹ�������Ƚ����׺ͷ���
int zce::getaddrinfo_to_addrary(const char *nodename,
                                size_t *ary_addr_num,
                                sockaddr_in ary_addr[],
                                size_t *ary_addr6_num,
                                sockaddr_in6 ary_addr6[])
{

    int ret = 0;
    addrinfo hints, *result = NULL;

    memset(&hints, 0, sizeof(addrinfo));
    //ͬʱ����IPV4.��IPV6�����ֻ��ҪIPV4��дAF_INET�����ֻ��ҪIPV6��дAF_INET6
    hints.ai_family = AF_UNSPEC;
    //hints.ai_socktype = 0; ������������
    //hints.ai_flags = 0;
    ret = zce::getaddrinfo(nodename,
                           NULL,
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

    //ȡ�ؽ��
    getaddrinfo_result_to_addrary(result,
                                  ary_addr_num,
                                  ary_addr,
                                  ary_addr6_num,
                                  ary_addr6);
    //�ͷſռ�
    zce::freeaddrinfo(result);

    return 0;
}


int zce::getaddrinfo_to_addr(const char *nodename,
                             sockaddr *addr,
                             socklen_t addr_len)
{
    int ret = 0;
    addrinfo hints, *result = NULL;

    memset(&hints, 0, sizeof(addrinfo));
    //ͬʱ����IPV4.��IPV6�����ֻ��ҪIPV4��дAF_INET�����ֻ��ҪIPV6��дAF_INET6
    hints.ai_family = addr->sa_family;
    //���ȷ���nodename�Ƿ�����ֵ��ַ
    hints.ai_flags = AI_PASSIVE;
    ret = zce::getaddrinfo(nodename,
                           NULL,
                           &hints,
                           &result);
    if (ret != 0)
    {
        //������������
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

    getaddrinfo_result_to_addr(result, addr, addr_len);

    //�ͷſռ�
    zce::freeaddrinfo(result);

    return 0;
}

//ͨ��IP��ַ��Ϣ����������.���������������뺯��
int zce::getnameinfo(const struct sockaddr *sa,
                     socklen_t salen,
                     char *host,
                     size_t hostlen,
                     char *serv,
                     size_t servlen,
                     int flags)
{
#if defined (ZCE_OS_WINDOWS)
    return ::getnameinfo(sa, salen, host, static_cast<DWORD>(hostlen), serv, static_cast<DWORD>(servlen), flags);
#elif defined (ZCE_OS_LINUX)
    return ::getnameinfo(sa, salen, host, hostlen, serv, servlen, flags);
#endif
}

//�Ǳ�׼������ͨ��IPV4��ַȡ������
int zce::getnameinfo_sockaddr(const sockaddr *sock_addr,
                              char *host_name,
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
//IPV4��IPV6֮���໥ת���ĺ��������ǷǱ�׼������

//��һ��IPV4�ĵ�ַӳ��ΪIPV6�ĵ�ַ
int zce::inaddr_map_inaddr6(const in_addr *src, in6_addr *dst)
{
    //��0
    memset(dst, 0, sizeof(in6_addr));

    //
    memcpy(reinterpret_cast<char *>(dst) + sizeof(in6_addr) - sizeof(in_addr),
           reinterpret_cast<const char *>(src),
           sizeof(in_addr));

    //ӳ���ַ�ĵ�����3��WORDΪ0xFFFF
    dst->s6_addr[10] = 0xFF;
    dst->s6_addr[11] = 0xFF;

    return 0;
}

//��һ��IPV4��Sock��ַӳ��ΪIPV6�ĵ�ַ
int zce::sockin_map_sockin6(const sockaddr_in *src, sockaddr_in6 *dst)
{
    return zce::inaddr_map_inaddr6(&(src->sin_addr),
                                   &(dst->sin6_addr));
}

//�ж�һ����ַ�Ƿ���IPV4ӳ��ĵ�ַ
bool zce::is_in6_addr_v4mapped(const in6_addr *in6)
{
    //������ӳ���ַ�ͼ��ݵ�ַ���ж��ˡ���˵���ݵ�ַ�Ժ�ᱻ��̭
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

//���һ��IPV6�ĵ�ַ��IPV4ӳ������ģ�ת����IPV4�ĵ�ַ
int zce::mapped_in6_to_in(const in6_addr *src, in_addr *dst)
{
    //�ȼ���Ƿ���ӳ��ĵ�ַ
    if ( false == zce::is_in6_addr_v4mapped(src) )
    {
        errno = EINVAL;
        return -1;
    }

    memcpy(reinterpret_cast<char *>(dst),
           reinterpret_cast<const char *>(src) + sizeof(in6_addr) - sizeof(in_addr),
           sizeof(in_addr));
    return 0;
}
//���һ��IPV6��socketaddr_in6��ַ��IPV4ӳ������ģ�ת����IPV4��socketaddr_in��ַ
int zce::mapped_sockin6_to_sockin(const sockaddr_in6 *src, sockaddr_in *dst)
{
    return zce::mapped_in6_to_in(&(src->sin6_addr),
                                 &(dst->sin_addr));
}

//�Զ˿ڽ��м�飬һЩ�˿��Ǻڿ��ص�ɨ��Ķ˿ڣ�
bool zce::check_safeport(uint16_t check_port)
{
    //��Σ�˿ڼ�鳣��
    const unsigned short UNSAFE_PORT1 = 1024;
    const unsigned short UNSAFE_PORT2 = 3306;
    const unsigned short UNSAFE_PORT3 = 36000;
    const unsigned short UNSAFE_PORT4 = 56000;
    const unsigned short SAFE_PORT1 = 80;

    //������˱��ռ��,������õĶ˿�
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

//==============================================================================================
//SOCKS5֧��UDP��͸��TCP�����Ƚ�ȫ��

const unsigned char SOCKS5_VER = 0x5;
const unsigned char SOCKS5_SUCCESS = 0x0;
const unsigned char SOCKS5_METHODS_NOAUTH = 0x0;
const unsigned char SOCKS5_METHODS_AUTH_GSSAPI = 0x1; //��֧��
const unsigned char SOCKS5_METHODS_AUTH_PASSWORD = 0x2;
const unsigned char SOCKS5_CMD_CONNECT = 0x1;
const unsigned char SOCKS5_CMD_BIND = 0x2;
const unsigned char SOCKS5_CMD_UDP = 0x3;
const unsigned char SOCKS5_ATYP_IPV4 = 0x1;
const unsigned char SOCKS5_ATYP_HOSTNAME = 0x3;
const unsigned char SOCKS5_ATYP_IPV6 = 0x4;

//socks5�����ʼ����handle Ҫ��connect������ʹ��connect_timeout
int zce::socks5_initialize(ZCE_SOCKET handle,
                           const char *username,
                           const char *password,
                           ZCE_Time_Value &timeout_tv)
{

    const size_t BUFFER_LEN = 1024;
    unsigned char buffer[BUFFER_LEN] = { "" };
    ssize_t send_len = 0, recv_len = 0;
    ssize_t snd_ret = 0;

    buffer[0] = SOCKS5_VER;
    //֧�ֲ���֤����֤���ַ�ʽ
    buffer[1] = 2;
    buffer[2] = 0x0; //����Ҫ��֤
    buffer[3] = 0x2; //��Ҫ��֤
    send_len = 4;

    send_len = zce::sendn_timeout(handle, buffer, send_len, timeout_tv);
    if (snd_ret <= 0)
    {
        return -1;
    }
    //Э��涨���������ֽ�,��һ���ǰ汾��5,�ڶ����Ƿ�ʽ
    recv_len = zce::recvn_timeout(handle, buffer, 2, timeout_tv);
    if (recv_len != 2)
    {
        return -1;
    }
    bool need_auth = false;
    if (SOCKS5_VER != buffer[0])
    {
        return -1;
    }

    //֧�ֲ���֤����֤���ַ�ʽ
    if (SOCKS5_METHODS_NOAUTH == buffer[1] )
    {
        need_auth = false;
    }
    else if (SOCKS5_METHODS_AUTH_PASSWORD == buffer[1] )
    {
        need_auth = true;
    }
    else
    {
        return -1;
    }

    //�����Ҫ��֤
    if (need_auth)
    {
        const size_t MAX_STRING_LEN = 255;
        if (!username || !password)
        {
            return EINVAL;
        }
        size_t user_len = strlen(username);
        size_t pass_len = strlen(password);
        if (pass_len > MAX_STRING_LEN || user_len > MAX_STRING_LEN)
        {
            return EINVAL;
        }

        //��֯��֤Э��
        buffer[0] = SOCKS5_VER;
        buffer[1] = static_cast<unsigned char>(user_len);
        memcpy(buffer + 2, username, user_len);
        buffer[2 + user_len] = static_cast<unsigned char>(pass_len);
        memcpy(buffer + 3 + user_len, password, pass_len);
        send_len = 3 + user_len + pass_len;
        snd_ret = zce::sendn_timeout(handle, buffer, send_len, timeout_tv);
        if (snd_ret <= 0)
        {
            return -1;
        }
        recv_len = zce::recvn_timeout(handle, buffer, 2, timeout_tv);
        if (recv_len != 2)
        {
            return -1;
        }
        //��֤ʧ��
        if (buffer[0] != SOCKS5_VER || buffer[1] != SOCKS5_SUCCESS)
        {
            return -1;
        }
    }
    return 0;
}


//ͨ��socks5����TCP���ӷ�����
int zce::socks5_connect_host(ZCE_SOCKET handle,
                             const char *host_name,
                             const sockaddr *host_addr,
                             int addr_len,
                             uint16_t host_port,
                             ZCE_Time_Value &timeout_tv)
{
    const size_t BUFFER_LEN = 1024;
    unsigned char buffer[BUFFER_LEN] = { "" };
    ssize_t send_len = 0, recv_len = 0;
    const size_t MAX_STRING_LEN = 255;

    ZCE_ASSERT(host_name || host_addr);

    buffer[0] = SOCKS5_VER;

    //����
    buffer[1] = SOCKS5_CMD_CONNECT;
    //����
    buffer[2] = 0x00;
    if (host_name)
    {
        size_t host_len = strlen(host_name);
        if (host_len > MAX_STRING_LEN)
        {
            return EINVAL;
        }
        buffer[3] = SOCKS5_ATYP_HOSTNAME;
        buffer[4] = static_cast<unsigned char>(host_len);
        memcpy(buffer + 5, host_name, host_len);
        uint16_t n_port = htons(host_port);
        memcpy(buffer + 5 + host_len, &n_port, 2);
        send_len =  host_len + 6;
    }
    else if (host_addr)
    {
        if (sizeof(sockaddr_in) == addr_len)
        {
            buffer[3] = SOCKS5_ATYP_IPV4;
            const sockaddr_in *addr_in = reinterpret_cast<const sockaddr_in *>(host_addr);
            memcpy(buffer + 4, &(addr_in->sin_addr), 4);
            uint16_t n_port = addr_in->sin_port;
            memcpy(buffer + 8, &n_port, 2);
            send_len = 10;
        }
        else if (sizeof(sockaddr_in6) == addr_len)
        {
            buffer[3] = SOCKS5_ATYP_IPV6;
            const sockaddr_in6 *addr_in6 = reinterpret_cast<const sockaddr_in6 *>(host_addr);
            memcpy(buffer + 4, &(addr_in6->sin6_addr), 16);
            uint16_t n_port = addr_in6->sin6_port;
            memcpy(buffer + 20, &n_port, 2);
            send_len = 22;
        }
        else
        {
            return EINVAL;
        }
    }
    else
    {
        return EINVAL;
    }

    ssize_t snd_ret = zce::sendn_timeout(handle, buffer, send_len, timeout_tv);
    if (snd_ret <= 0)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 proxy send to socks5 proxy fail, snd_ret =%zd!", snd_ret);
        return -1;
    }
    recv_len = zce::recvn_timeout(handle, buffer, 2, timeout_tv);
    if (recv_len != 2)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 recv send to socks5 proxy fail, recv_len =%zd!", recv_len);
        return -1;
    }
    //��֤���
    int reponse_code = buffer[1];
    if (buffer[0] != SOCKS5_VER || reponse_code != SOCKS5_SUCCESS)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 proxy connect host fail, reponse code [%d] !", reponse_code);
        return reponse_code;
    }
    ZCE_LOG(RS_DEBUG, "Socks 5 Connected!");
    return 0;
}

//socks5����UDP��͸
int zce::socks5_udp_associate(ZCE_SOCKET handle,
                              const sockaddr *bind_addr,
                              int addr_len,
                              sockaddr *udp_addr,
                              ZCE_Time_Value &timeout_tv)
{
    const size_t BUFFER_LEN = 1024;
    unsigned char buffer[BUFFER_LEN] = { "" };
    ssize_t send_len = 0, recv_len = 0;

    ZCE_ASSERT(bind_addr && udp_addr);

    buffer[0] = SOCKS5_VER;

    //����
    buffer[1] = SOCKS5_CMD_UDP;
    //����
    buffer[2] = 0x00;

    if (sizeof(sockaddr_in) == addr_len)
    {
        buffer[3] = SOCKS5_ATYP_IPV4;
        const sockaddr_in *addr_in = reinterpret_cast<const sockaddr_in *>(bind_addr);
        //��Ч����д0
        memset(buffer + 4, 0, 4);
        //��ת�룬���߶�Ҫ������
        uint16_t n_port = addr_in->sin_port;
        memcpy(buffer + 8, &n_port, 2);
        send_len = 10;
    }
    else if (sizeof(sockaddr_in6) == addr_len)
    {
        buffer[3] = SOCKS5_ATYP_IPV6;
        const sockaddr_in6 *addr_in6 = reinterpret_cast<const sockaddr_in6 *>(bind_addr);
        memset(buffer + 4, 0, 16);
        uint16_t n_port = addr_in6->sin6_port;
        memcpy(buffer + 20, &n_port, 2);
        send_len = 22;
    }
    else
    {
        return EINVAL;
    }

    ssize_t snd_ret = zce::sendn_timeout(handle, buffer, send_len, timeout_tv);
    if (snd_ret <= 0)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 proxy send to socks5 proxy fail, snd_ret =%zd!", snd_ret);
        return -1;
    }
    //ֻ��ȡһ�����ݣ��������ȡ
    recv_len = zce::recvn_timeout(handle, buffer, BUFFER_LEN, timeout_tv, 0, true);
    //���ٻ����5���ֽ�
    if (recv_len <= 4)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 recv send to socks5 proxy fail, recv_len =%zd!", recv_len);
        return -1;
    }
    //��֤���
    int reponse_code = buffer[1];
    if (buffer[0] != SOCKS5_VER || reponse_code != SOCKS5_SUCCESS)
    {
        ZCE_LOG(RS_ERROR, "Socks 5 proxy connect host fail, reponse code [%d] !", reponse_code);
        return reponse_code;
    }

    if ( SOCKS5_ATYP_IPV4 == buffer[3])
    {
        //ţͷ�������죬�յ������ݳ��Ⱥ��ڴ��Ĳ�һ��
        if (sizeof(sockaddr_in) != addr_len || recv_len < 10)
        {
            return -1;
        }
        sockaddr_in *addr_in = reinterpret_cast< sockaddr_in *>(udp_addr);
        memcpy(&(addr_in->sin_addr), buffer + 4, 4);
        //��ת�룬���߶�Ҫ������
        memcpy(&(addr_in->sin_port), buffer + 8, 2);
    }
    else if ( SOCKS5_ATYP_IPV6 == buffer[3])
    {
        if (sizeof(sockaddr_in6) != addr_len || recv_len < 22)
        {
            return -1;
        }
        sockaddr_in6 *addr_in6 = reinterpret_cast<sockaddr_in6 *>(udp_addr);
        memcpy(&(addr_in6->sin6_addr), buffer + 4,  16);
        memcpy(&(addr_in6->sin6_port), buffer + 20, 2);
    }
    else
    {
        return EINVAL;
    }

    return 0;
}



