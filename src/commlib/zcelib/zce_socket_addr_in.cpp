#include "zce_predefine.h"
#include "zce_os_adapt_socket.h"
#include "zce_socket_addr_base.h"
#include "zce_log_logging.h"
#include "zce_socket_addr_in.h"

//Ĭ�Ϲ��캯��
ZCE_Sockaddr_In::ZCE_Sockaddr_In (void):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in4_addr_), sizeof(sockaddr_in))
{

}

//����sockaddr_in���죬
ZCE_Sockaddr_In::ZCE_Sockaddr_In (const sockaddr_in *addr):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in4_addr_), sizeof(sockaddr_in))
{
    in4_addr_ = *addr;
}

//����IP��ַ(XXX.XXX.XXX.XXX)�ַ������˿ںų�ʼ������
ZCE_Sockaddr_In::ZCE_Sockaddr_In (const char *ip_addr_str,
                                  uint16_t port_number):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in4_addr_), sizeof(sockaddr_in))
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr_str, port_number);

    if (ret != 0)
    {
        ZCE_ASSERT(false);
    }
}

//���ݵ�ַ(����)���˿ںų�ʼ������
ZCE_Sockaddr_In::ZCE_Sockaddr_In (uint32_t ip_addr,
                                  uint16_t port_number):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in4_addr_), sizeof(sockaddr_in))
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr, port_number);

    if (ret != 0)
    {
        ZCE_ASSERT(false);
    }
}

//�������죬һ��Ҫд�������Ļ���ָ����ָ���Լ���һ����ַ�ģ�
ZCE_Sockaddr_In::ZCE_Sockaddr_In (const ZCE_Sockaddr_In &others):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in4_addr_), sizeof(sockaddr_in))
{
    in4_addr_ = others.in4_addr_;
}

ZCE_Sockaddr_In::~ZCE_Sockaddr_In()
{

}

//����ַ�Ƿ���һ��������ַ
bool ZCE_Sockaddr_In::is_internal()
{
    return zce::is_internal(&in4_addr_);
}

//����ַ�Ƿ���һ��������ַ����ʵ�Ҿͼ���Ϊ����������ַ����������ַ
bool ZCE_Sockaddr_In::is_internet()
{
    return !(zce::is_internal(&in4_addr_));
}

//���õ�ַ��Ϣ
void ZCE_Sockaddr_In::set_sockaddr (sockaddr *addr, socklen_t len)
{
    in4_addr_ = *(reinterpret_cast<sockaddr_in *>(addr));
    sockaddr_size_ = len;
}

//����IP��ַ(XXX.XXX.XXX.XXX)�ַ������˿ں�����
int ZCE_Sockaddr_In::set(const char ip_addr_str[16],
                         uint16_t port_number)
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr_str, port_number);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//���ݵ�ַIP���˿ں�����
int ZCE_Sockaddr_In::set(uint32_t ip_addr,
                         uint16_t port_number)
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr, port_number);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}


//�����ַ�������IP��ַ�������#����ȡ�˿ں�
int ZCE_Sockaddr_In::set(const char *ip_addr_str)
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr_str);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}


///���˿ں��Ƿ���һ����ȫ�˿�
bool ZCE_Sockaddr_In::check_safeport()
{
    return zce::check_safeport(this->get_port_number());
}

//�Ƚ�������ַ�Ƿ����
bool ZCE_Sockaddr_In::operator == (const ZCE_Sockaddr_In &others) const
{
    //in4_addr_.sin_zero Ҫ�Ƚ�����ʱ���ˡ�

    //�Ƚϵ�ַЭ��أ���ַ���˿�
    if (others.in4_addr_.sin_family == in4_addr_.sin_family &&
        others.in4_addr_.sin_addr.s_addr == in4_addr_.sin_addr.s_addr &&
        others.in4_addr_.sin_port == in4_addr_.sin_port )
    {
        return true;
    }

    return false;
}

//�Ƚ�������ַ�Ƿ����
bool ZCE_Sockaddr_In::operator != (const ZCE_Sockaddr_In &others) const
{
    return !(*this == others);
}

//���IP��ַ�Ƿ����
bool ZCE_Sockaddr_In::is_ip_equal (const ZCE_Sockaddr_In &others) const
{
    //�Ƚϵ�ַЭ��أ���ַ
    if (others.in4_addr_.sin_family == in4_addr_.sin_family &&
        others.in4_addr_.sin_addr.s_addr == in4_addr_.sin_addr.s_addr)
    {
        return true;
    }

    return false;
}

//ȡ��IP��ַ��ص�������Ϣ,���ú�����getnameinfo
int ZCE_Sockaddr_In::get_name_info(char *host_name, size_t name_len) const
{
    return zce::getnameinfo(reinterpret_cast<const sockaddr *>(&in4_addr_),
                            sizeof(sockaddr_in),
                            host_name,
                            name_len,
                            NULL,
                            0,
                            NI_NAMEREQD);
}

//ȡ��������ص�IP��ַ��Ϣ�����ú�����getaddrinfo_to_addr
int ZCE_Sockaddr_In::getaddrinfo_to_addr(const char *nodename)
{
    return zce::getaddrinfo_to_addr(nodename,
                                    sockaddr_ptr_,
                                    sizeof(sockaddr_in));
}

//����sockaddr_in
ZCE_Sockaddr_In::operator sockaddr_in () const
{
    return in4_addr_;
}

//�����ڲ�const sockaddr_in��ָ�룬�������Ա��޸ģ�
ZCE_Sockaddr_In::operator const sockaddr_in *() const
{
    return &in4_addr_;
}

//�����ڲ�sockaddr_in��ָ�룬�����Ա��޸ģ�
ZCE_Sockaddr_In::operator sockaddr_in *()
{
    return &in4_addr_;
}
