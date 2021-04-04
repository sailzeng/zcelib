#ifndef ZCE_LIB_SOCKET_ADDR_BASE_
#define ZCE_LIB_SOCKET_ADDR_BASE_

#include "zce_os_adapt_socket.h"

//�ڸ�д����3�������ң��Ҷ�һ��ʹ��sockaddr sockaddr_in��Ϊ��ַ������ΪʲôҪ�仯�أ�
//��Ϊ����һ���дԭ������ʱ��ͻȻ����ΪʲôΪʲô����OOһ��㣿
//����ο�ACE_INET_Addr ACE INET Addrʵ�ֵģ���ȻҲ��һЩ�仯��ACEû��ֱ��ʹ��sockaddr����֪��Ϊɶ

//Socket��ַ�Ļ��ࡣ
class ZCE_Sockaddr
{
public:

    //���캯����
    ZCE_Sockaddr (sockaddr *sockaddr_ptr = NULL, int sa_size = -1);
    //��������,�ڲ���virtual����
    virtual ~ZCE_Sockaddr (void);

    //����sockaddr��ַ��Ϣ,���óɴ��麯����ԭ��������ʹ��ZCE_Sockaddr
    virtual  void set_sockaddr (sockaddr *sockaddr_ptr, socklen_t sockaddr_size) = 0;

    //Get/set the size of the address.
    inline socklen_t get_size (void) const;
    //
    inline void  set_size (int sa_size);

    //���õ�ַ��Ϣ
    inline void set_addr (sockaddr *sockaddr_ptr);
    //ȡ�õ�ַ��Ϣ
    inline sockaddr *get_addr (void) const;

    // ����ַ�Ƿ����
    bool operator == (const ZCE_Sockaddr &others_sockaddr) const;
    // ����ַ�Ƿ����
    bool operator != (const ZCE_Sockaddr &others_sockaddr) const;


    //ת�����ַ���,ͬʱ����ַ����ĳ���
    inline const char *to_string(char *buffer,
                                 size_t buf_len,
                                 size_t &use_buf,
                                 bool out_port_info = true) const
    {

        return zce::socketaddr_ntop_ex(sockaddr_ptr_, buffer, buf_len, use_buf, out_port_info);
    }

public:

    // ��ַ���͵�ָ��,
    sockaddr *sockaddr_ptr_;

    // ��ַ�ṹ�ĳ��� Number of bytes in the address.
    socklen_t sockaddr_size_;
};

//Get/set the size of the address.
inline socklen_t ZCE_Sockaddr::get_size (void) const
{
    return sockaddr_size_;
}
//
inline void ZCE_Sockaddr::set_size (int sa_size)
{
    sockaddr_size_ = sa_size;
}

//���õ�ַ��Ϣ
inline void ZCE_Sockaddr::set_addr (sockaddr *sockaddr_ptr)
{
    sockaddr_ptr_ = sockaddr_ptr;
}
//ȡ�õ�ַ��Ϣ
inline sockaddr *ZCE_Sockaddr::get_addr (void) const
{
    return sockaddr_ptr_;
}

#endif  //ZCE_LIB_SOCKET_ADDR_BASE_
