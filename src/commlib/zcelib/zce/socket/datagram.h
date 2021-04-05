#ifndef ZCE_LIB_SOCKET_DATAGRAM_H_
#define ZCE_LIB_SOCKET_DATAGRAM_H_

class ZCE_Time_Value;
class ZCE_Sockaddr;
class ZCE_Socket_Base;

//UDP socket
class ZCE_Socket_DataGram : public ZCE_Socket_Base
{

public:

    //���캯��������������
    ZCE_Socket_DataGram();
    explicit ZCE_Socket_DataGram(const ZCE_SOCKET &socket_hanle);
    explicit ZCE_Socket_DataGram(const ZCE_Sockaddr &sock_addr);
    ~ZCE_Socket_DataGram();

    //Open SOCK�������BIND���ص�ַ�ķ�ʽ
    int open(int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = true);

    ///Open SOCK�����BIND���ص�ַ�ķ�ʽ,һ������²�������ʹ�ã����ǡ���
    ///protocol_family ����������AF_INET,����AF_INET6��
    int open(const ZCE_Sockaddr *local_addr,
             int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = false);

    //UDP����
    inline ssize_t recvfrom (void *buf,
                             size_t len,
                             int flags,
                             ZCE_Sockaddr *addr) const;

    //UDP����
    inline ssize_t sendto (const void *buf,
                           size_t len,
                           int flags,
                           const ZCE_Sockaddr *to_addr) const;

    //��UDP������,Ҳ���г�ʱ�����������յ��������ݾ��Ƕ����ˣ���ʱ��selectʵ��
    inline ssize_t recvfrom (void *buf,
                             size_t len,
                             ZCE_Sockaddr *addr,
                             ZCE_Time_Value  *timeout_tv,
                             int flags = 0) const;

    //UDP�ķ�����ʱ�ǲ��������ģ����ó�ʱ����д���������ȫ��Ϊ�˺�ǰ�����
    //����UDP������,����ʱ�������������ʵ���Ͻ���û�г�ʱ����
    inline ssize_t sendto (const void *buf,
                           size_t len,
                           const ZCE_Sockaddr *addr,
                           ZCE_Time_Value *   /*timeout_tv*/,
                           int flags = 0) const;

    //��UDP������,Ҳ���г�ʱ�����������յ��������ݾ��Ƕ����ˣ���ʱ��SO_RCVTIMEOʵ��
    ssize_t recvfrom_timeout (void *buf,
                              size_t len,
                              ZCE_Sockaddr *addr,
                              ZCE_Time_Value  &timeout_tv,
                              int flags = 0) const;

    //UDP�ķ�����ʱ�ǲ��������ģ����ó�ʱ����д���������ȫ��Ϊ�˺�ǰ�����
    //����UDP������,����ʱ�������������ʵ���Ͻ���û�г�ʱ����
    ssize_t sendto_timeout (const void *buf,
                            size_t len,
                            const ZCE_Sockaddr *addr,
                            ZCE_Time_Value &   /*timeout_tv*/,
                            int flags = 0) const;
};

//UDP����
inline ssize_t ZCE_Socket_DataGram::recvfrom (void *buf,
                                              size_t len,
                                              int flags,
                                              ZCE_Sockaddr *addr)  const
{
    return zce::recvfrom (socket_handle_,
                          buf,
                          len,
                          flags,
                          addr->sockaddr_ptr_,
                          &addr->sockaddr_size_
                         );
}

//UDP����
inline ssize_t ZCE_Socket_DataGram::sendto (const void *buf,
                                            size_t len,
                                            int flags,
                                            const ZCE_Sockaddr *to_addr)  const
{
    return zce::sendto (socket_handle_,
                        buf,
                        len,
                        flags,
                        to_addr->sockaddr_ptr_,
                        to_addr->sockaddr_size_);
}

//��UDP������,Ҳ���г�ʱ�����������յ��������ݾ��Ƕ����ˣ���ʱ��selectʵ��
inline ssize_t ZCE_Socket_DataGram::recvfrom (void *buf,
                                              size_t len,
                                              ZCE_Sockaddr *from_addr,
                                              ZCE_Time_Value *timeout_tv,
                                              int flags)  const
{
    return zce::recvfrom (socket_handle_,
                          buf,
                          len,
                          from_addr->sockaddr_ptr_,
                          &from_addr->sockaddr_size_,
                          timeout_tv,
                          flags);
}

//UDP�ķ�����ʱ�ǲ��������ģ����ó�ʱ����д���������ȫ��Ϊ�˺�ǰ�����
//����UDP������,����ʱ�������������ʵ���Ͻ���û�г�ʱ����
inline ssize_t ZCE_Socket_DataGram::sendto (const void *buf,
                                            size_t len,
                                            const ZCE_Sockaddr *to_addr,
                                            ZCE_Time_Value *timeout_tv,
                                            int flags)  const
{
    return zce::sendto (socket_handle_,
                        buf,
                        len,
                        to_addr->sockaddr_ptr_,
                        to_addr->sockaddr_size_,
                        timeout_tv,
                        flags);
}

#endif //#ifndef ZCE_LIB_SOCKET_DATAGRAM_H_

