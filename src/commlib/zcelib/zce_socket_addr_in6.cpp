#include "zce_predefine.h"
#include "zce_os_adapt_socket.h"
#include "zce_socket_addr_base.h"
#include "zce_socket_addr_in.h"
#include "zce_socket_addr_in6.h"

//默认构造函数
ZCE_Sockaddr_In6::ZCE_Sockaddr_In6 (void):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in6_addr_), sizeof(sockaddr_in6))
{

}

//根据sockaddr_in构造，
ZCE_Sockaddr_In6::ZCE_Sockaddr_In6 (const sockaddr_in6 *addr):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in6_addr_), sizeof(sockaddr_in6))
{
    in6_addr_ = *addr;
}

//根据地址名字，端口号初始化构造
ZCE_Sockaddr_In6::ZCE_Sockaddr_In6 (const char ip_addr_str[],
                                    uint16_t port_number):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in6_addr_), sizeof(sockaddr_in))
{
    int ret = zce::set_sockaddr_in6(&in6_addr_, ip_addr_str, port_number);

    if (ret != 0)
    {

    }
}

//根据端口号，和IP地址信息构造
ZCE_Sockaddr_In6::ZCE_Sockaddr_In6 (uint16_t port_number,
                                    const char ipv6_addr_val[16]):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in6_addr_), sizeof(sockaddr_in))
{
    int ret = zce::set_sockaddr_in6(&in6_addr_, port_number, ipv6_addr_val);

    if (ret != 0)
    {

    }
}

//拷贝构造，一定要写，这个类的基类指针是指向自己的一个地址的，
ZCE_Sockaddr_In6::ZCE_Sockaddr_In6 (const ZCE_Sockaddr_In6 &others):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in6_addr_), sizeof(sockaddr_in6))
{
    in6_addr_ = others.in6_addr_;
}

ZCE_Sockaddr_In6::~ZCE_Sockaddr_In6()
{

}

//设置地址信息
void ZCE_Sockaddr_In6::set_sockaddr (sockaddr *addr, socklen_t len)
{
    in6_addr_ = *(reinterpret_cast<sockaddr_in6 *>(addr));
    sockaddr_size_ = len;
}

//根据地址名字，端口号设置
int ZCE_Sockaddr_In6::set(const char ip_addr_str[],
                          uint16_t port_number)
{
    int ret = zce::set_sockaddr_in6(&in6_addr_, ip_addr_str, port_number);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}


//根据字符串取得IP地址信息，以及端口号信息,如果字符串里面有#,会被认为有端口号，如果没有，端口号为0
int ZCE_Sockaddr_In6::set(const char *ip_addr_str)
{
    int ret = zce::set_sockaddr_in6(&in6_addr_, ip_addr_str);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}


///检查端口号是否是一个安全端口
bool ZCE_Sockaddr_In6::check_safeport()
{
    return zce::check_safeport(this->get_port_number());
}


//比较两个地址是否相等
bool ZCE_Sockaddr_In6::operator == (const ZCE_Sockaddr_In6 &others) const
{
    //in6_addr_.sin_zero 要比较吗？暂时算了。

    //比较地址协议簇，地址，端口
    if (others.in6_addr_.sin6_family == this->in6_addr_.sin6_family &&
        0 == memcmp(&(others.in6_addr_.sin6_addr), &(this->in6_addr_.sin6_addr), sizeof(in6_addr))  &&
        others.in6_addr_.sin6_port == this->in6_addr_.sin6_port )
    {
        return true;
    }

    return false;
}

//比较两个地址是否不想等
bool ZCE_Sockaddr_In6::operator != (const ZCE_Sockaddr_In6 &others) const
{
    return !(*this == others);
}

//检查IP地址是否相等,忽视端口
bool ZCE_Sockaddr_In6::is_ip_equal (const ZCE_Sockaddr_In6 &others) const
{
    //比较地址协议簇，地址
    if (others.in6_addr_.sin6_family == this->in6_addr_.sin6_family &&
        0 == memcmp(&(others.in6_addr_.sin6_addr), &(this->in6_addr_.sin6_addr), sizeof(in6_addr)) )
    {
        return true;
    }

    return false;
}

//取得IP地址相关的域名信息，调用的是getnameinfo
int ZCE_Sockaddr_In6::get_name_info(char *host_name, size_t name_len) const
{
    return zce::getnameinfo(reinterpret_cast<const sockaddr *>(&in6_addr_),
                            sizeof(sockaddr_in6),
                            host_name,
                            name_len,
                            NULL,
                            0,
                            NI_NAMEREQD);
}

//取得域名相关的IP地址信息，调用的是getaddrinfo_to_addr
int ZCE_Sockaddr_In6::getaddrinfo_to_addr(const char *nodename)
{
    return zce::getaddrinfo_to_addr(nodename,
                                    sockaddr_ptr_,
                                    sizeof(sockaddr_in6));
}

//这个IPV6的地址是否是IPV4的地址映射的
bool ZCE_Sockaddr_In6::is_v4mapped() const
{
    return zce::is_in6_addr_v4mapped(&(in6_addr_.sin6_addr));
}

//从一个IPV4的地址得到对应映射的IPV6的地址，
int ZCE_Sockaddr_In6::map_from_inaddr(const ZCE_Sockaddr_In &from)
{
    return zce::sockin_map_sockin6((from), &(in6_addr_));
}

//如果这个IPV6的地址是IPV4映射过来的，将其还原为IPV4的地址
int ZCE_Sockaddr_In6::mapped_to_inaddr(ZCE_Sockaddr_In &to) const
{
    return zce::mapped_sockin6_to_sockin(&(in6_addr_), (to));
}

//返回sockaddr_in
ZCE_Sockaddr_In6::operator sockaddr_in6 () const
{
    return in6_addr_;
}

//返回内部const sockaddr_in的指针，（不可以被修改）
ZCE_Sockaddr_In6::operator const sockaddr_in6 *() const
{
    return &in6_addr_;
}

//返回内部sockaddr_in的指针，（可以被修改）
ZCE_Sockaddr_In6::operator sockaddr_in6 *()
{
    return &in6_addr_;
}

