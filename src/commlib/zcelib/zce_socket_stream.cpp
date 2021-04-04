#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_os_adapt_socket.h"
#include "zce_socket_base.h"
#include "zce_socket_stream.h"

//���캯��
ZCE_Socket_Stream::ZCE_Socket_Stream():
    ZCE_Socket_Base()
{
}

ZCE_Socket_Stream::ZCE_Socket_Stream(const ZCE_SOCKET &socket_handle):
    ZCE_Socket_Base(socket_handle)
{
}

ZCE_Socket_Stream::~ZCE_Socket_Stream()
{
}

//Open SOCK�������BIND���ص�ַ�ķ�ʽ
int ZCE_Socket_Stream::open(int protocol_family,
                            int protocol,
                            bool reuse_addr)
{
    int ret = 0;
    //��
    ret = ZCE_Socket_Base::open(SOCK_STREAM, protocol_family, protocol, reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

///Open SOCK�����BIND���ص�ַ�ķ�ʽ,һ������²�������ʹ�ã����ǡ���
//protocol_family ����������AF_INET,����AF_INET6��
int ZCE_Socket_Stream::open(const ZCE_Sockaddr *local_addr,
                            int protocol_family,
                            int protocol,
                            bool reuse_addr)
{
    int ret = 0;

    ret = ZCE_Socket_Base::open(SOCK_STREAM,
                                local_addr,
                                protocol_family,
                                protocol,
                                reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//shutdown,����Ĺرշ�ʽ������how������LINUX�¿���,SHUT_RD,SHUT_WR,SHUT_RDWR
int ZCE_Socket_Stream::shutdown (int how) const
{
    return zce::shutdown(socket_handle_, how);
}

//����ʱ����ķ��ͺ���������N���ֽں󷵻�,�ڲ��õ���select��¼ʱ�䣬��ʱ�����ر�׼ȷ��
ssize_t ZCE_Socket_Stream::recv_n (void *buf,
                                   size_t len,
                                   ZCE_Time_Value  *timeout_tv,
                                   int flags)  const
{
    return zce::recv_n(socket_handle_,
                       buf,
                       len,
                       timeout_tv,
                       flags);
}

//����ʱ����ķ��ͺ���������N���ֽں󷵻�,�ڲ��õ���select��¼ʱ�䣬��ʱ�����ر�׼ȷ��
ssize_t ZCE_Socket_Stream::send_n (const void *buf,
                                   size_t len,
                                   ZCE_Time_Value  *timeout_tv,
                                   int flags)  const
{
    return zce::send_n(socket_handle_,
                       buf,
                       len,
                       timeout_tv,
                       flags);
}

//����ʱ����Ľ��ܺ���������N���ֽں󷵻�,�ڲ��õ���SO_RCVTIMEO����ʱ�����ر�׼ȷ��
ssize_t ZCE_Socket_Stream::recvn_timeout (void *buf,
                                          size_t len,
                                          ZCE_Time_Value  &timeout_tv,
                                          int flags)  const
{
    return zce::recvn_timeout(socket_handle_,
                              buf,
                              len,
                              timeout_tv,
                              flags);
}

//����ʱ����ķ��ͺ���������N���ֽں󷵻�,�ڲ��õ���SO_SNDTIMEO����ʱ�����ر�׼ȷ��
ssize_t ZCE_Socket_Stream::sendn_timeout (void *buf,
                                          size_t len,
                                          ZCE_Time_Value  &timeout_tv,
                                          int flags)
{
    return zce::sendn_timeout(socket_handle_,
                              buf,
                              len,
                              timeout_tv,
                              flags);
}

