#ifndef ZCE_LIB_SOCKET_STREAM_H_
#define ZCE_LIB_SOCKET_STREAM_H_

class ZCE_Time_Value;
class ZCE_Sockaddr;
class ZCE_Socket_Base;

class ZCE_Socket_Stream : public ZCE_Socket_Base
{

public:

    //���캯��
    ZCE_Socket_Stream();
    explicit ZCE_Socket_Stream(const ZCE_SOCKET &socket_hanle);
    ~ZCE_Socket_Stream();

public:

    //Open SOCK�������BIND���ص�ַ�ķ�ʽ
    int open(int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = true);

    //Open SOCK�����BIND���ص�ַ�ķ�ʽ,һ������²�������ʹ�ã����ǡ���
    //protocol_family ����������AF_INET,����AF_INET6��
    int open(const ZCE_Sockaddr *local_addr,
             int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = false);

    //shutdown,����Ĺرշ�ʽ������how������LINUX�¿���,SHUT_RD,SHUT_WR,SHUT_RDWR
    int shutdown (int how) const;

    //����ʱ����ķ��ͺ���������N���ֽں󷵻�,�ڲ��õ���select��¼ʱ�䣬��ʱ��ȷһ��
    ssize_t recv_n (void *buf,
                    size_t len,
                    ZCE_Time_Value  *timeout_tv,
                    int flags = 0)  const;

    //����ʱ����ķ��ͺ���������N���ֽں󷵻�,�ڲ��õ���select��¼ʱ�䣬��ʱ��ȷһ��
    ssize_t send_n (const void *buf,
                    size_t len,
                    ZCE_Time_Value  *timeout_tv,
                    int flags = 0)  const;

    //����ʱ����Ľ��ܺ���������N���ֽں󷵻�,�ڲ��õ���SO_RCVTIMEO����ʱ�����ر�׼ȷ��
    ssize_t recvn_timeout (void *buf,
                           size_t len,
                           ZCE_Time_Value &timeout_tv,
                           int flags = 0)  const;

    //����ʱ����ķ��ͺ���������N���ֽں󷵻�,�ڲ��õ���SO_SNDTIMEO����ʱ�����ر�׼ȷ��
    ssize_t sendn_timeout (void *buf,
                           size_t len,
                           ZCE_Time_Value &timeout_tv,
                           int flags = 0);

};

#endif //#ifndef ZCE_LIB_SOCKET_STREAM_H_

