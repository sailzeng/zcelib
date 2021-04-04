#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_socket_addr_base.h"
#include "zce_socket_base.h"
#include "zce_os_adapt_socket.h"
#include "zce_socket_datagram.h"

ZCE_Socket_DataGram::ZCE_Socket_DataGram():
    ZCE_Socket_Base()
{
}

ZCE_Socket_DataGram::ZCE_Socket_DataGram(const ZCE_SOCKET &socket_hanle):
    ZCE_Socket_Base(socket_hanle)
{

}

ZCE_Socket_DataGram::~ZCE_Socket_DataGram()
{
}

//Open SOCK�������BIND���ص�ַ�ķ�ʽ
int ZCE_Socket_DataGram::open(int protocol_family,
                              int protocol,
                              bool reuse_addr)
{
    int ret = 0;
    //��
    ret = ZCE_Socket_Base::open(SOCK_DGRAM,
                                protocol_family,
                                protocol,
                                reuse_addr);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

///Open SOCK�����BIND���ص�ַ�ķ�ʽ,һ������²�������ʹ�ã����ǡ���
//protocol_family ����������AF_INET,����AF_INET6��
int ZCE_Socket_DataGram::open(const ZCE_Sockaddr *local_addr,
                              int protocol_family,
                              int protocol,
                              bool reuse_addr)
{
    int ret = 0;

    ret = ZCE_Socket_Base::open(SOCK_DGRAM,
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

//��UDP������,Ҳ���г�ʱ�����������յ��������ݾ��Ƕ����ˣ���ʱ��SO_RCVTIMEOʵ��
ssize_t ZCE_Socket_DataGram::recvfrom_timeout (void *buf,
                                               size_t len,
                                               ZCE_Sockaddr *from_addr,
                                               ZCE_Time_Value &timeout_tv,
                                               int flags)  const
{
    return zce::recvfrom_timeout (socket_handle_,
                                  buf,
                                  len,
                                  from_addr->sockaddr_ptr_,
                                  &from_addr->sockaddr_size_,
                                  timeout_tv,
                                  flags);
}

//UDP�ķ�����ʱ�ǲ��������ģ����ó�ʱ����д���������ȫ��Ϊ�˺�ǰ�����
//����UDP������,����ʱ�������������ʵ���Ͻ���û�г�ʱ����
ssize_t ZCE_Socket_DataGram::sendto_timeout (const void *buf,
                                             size_t len,
                                             const ZCE_Sockaddr *to_addr,
                                             ZCE_Time_Value &timeout_tv,
                                             int flags)  const
{
    return zce::sendto_timeout (socket_handle_,
                                buf,
                                len,
                                to_addr->sockaddr_ptr_,
                                to_addr->sockaddr_size_,
                                timeout_tv,
                                flags);
}

