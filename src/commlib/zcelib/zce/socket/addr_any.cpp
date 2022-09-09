#include "zce/predefine.h"
#include "zce/socket/addr_any.h"

namespace zce
{
//默认初始化位IPV4的地址
Sockaddr_Any::Sockaddr_Any() :
    Sockaddr_Base(reinterpret_cast<sockaddr*>(&in4_addr_), sizeof(sockaddr_in))
{
    //把最长的清0
    ::memset(&in6_addr_, 0, sizeof(in6_addr_));
    in4_addr_.sin_family = AF_INET;
    sockaddr_size_ = sizeof(sockaddr_in);
}

//构造函数，根据family确定初始化的类型
Sockaddr_Any::Sockaddr_Any(int family)
{
    //把最长的清0
    ::memset(&in6_addr_, 0, sizeof(in6_addr_));
    if (AF_INET == family)
    {
        sockaddr_size_ = sizeof(sockaddr_in);
        in4_addr_.sin_family = AF_INET;
        sockaddr_ptr_ = reinterpret_cast<sockaddr*>(&in4_addr_);
    }
    else if (AF_INET == family)
    {
        sockaddr_size_ = sizeof(sockaddr_in6);
        in6_addr_.sin6_family = AF_INET6;
        sockaddr_ptr_ = reinterpret_cast<sockaddr*>(&in6_addr_);
    }
    else
    {
        assert(false);
    }
}

//，和析构函数
Sockaddr_Any::~Sockaddr_Any()
{
}

//设置sockaddr地址信息,设置成纯虚函数的原因不想让你使用Sockaddr_Any
void Sockaddr_Any::set_sockaddr(sockaddr* addr,
                                socklen_t len)
{
    if (len == sizeof(sockaddr_in))
    {
        in4_addr_.sin_family = AF_INET;
        in4_addr_ = *(reinterpret_cast<sockaddr_in*>(addr));
        sockaddr_ptr_ = reinterpret_cast<sockaddr*>(&in4_addr_);
        sockaddr_size_ = len;
    }
    else if (len == sizeof(sockaddr_in6))
    {
        in6_addr_.sin6_family = AF_INET6;
        in6_addr_ = *(reinterpret_cast<sockaddr_in6*>(addr));
        sockaddr_ptr_ = reinterpret_cast<sockaddr*>(&in6_addr_);
        sockaddr_size_ = len;
    }
    else
    {
        assert(false);
    }
}

//设置地址的family
void Sockaddr_Any::set_family(int family)
{
    if (family == AF_INET)
    {
        in4_addr_.sin_family = AF_INET;
        sockaddr_ptr_ = reinterpret_cast<sockaddr*>(&in4_addr_);
        sockaddr_size_ = sizeof(sockaddr_in);
    }
    else if (family == AF_INET6)
    {
        in6_addr_.sin6_family = AF_INET6;
        sockaddr_ptr_ = reinterpret_cast<sockaddr*>(&in6_addr_);
        sockaddr_size_ = sizeof(sockaddr_in6);
    }
    else
    {
        assert(false);
    }
}

//通过域名得到IP地址
int Sockaddr_Any::getaddrinfo(const char* node_name,
                              uint16_t port_number)
{
    int ret = 0;
    if (sockaddr_ptr_->sa_family == AF_INET)
    {
        sockaddr_ptr_ = reinterpret_cast<sockaddr*>(&in4_addr_);
        ret = zce::getaddrinfo_to_addr(node_name,
                                       nullptr,
                                       sockaddr_ptr_,
                                       sizeof(sockaddr_in));
        if (ret != 0)
        {
            return ret;
        }
        in4_addr_.sin_family = AF_INET;
        in4_addr_.sin_port = ntohs(port_number);
        return 0;
    }
    else if (sockaddr_ptr_->sa_family == AF_INET6)
    {
        sockaddr_ptr_ = reinterpret_cast<sockaddr*>(&in6_addr_);
        ret = zce::getaddrinfo_to_addr(node_name,
                                       nullptr,
                                       sockaddr_ptr_,
                                       sizeof(sockaddr_in6));
        if (ret != 0)
        {
            return ret;
        }
        in6_addr_.sin6_family = AF_INET6;
        in6_addr_.sin6_port = ntohs(port_number);
        return 0;
    }
    else
    {
        assert(false);
        return -1;
    }
}

int Sockaddr_Any::getaddrinfo(int family,
                              const char* node_name,
                              uint16_t port_number)
{
    int ret = 0;
    if (family == AF_INET)
    {
        sockaddr_ptr_ = reinterpret_cast<sockaddr*>(&in4_addr_);
        sockaddr_size_ = sizeof(sockaddr_in);
        ret = zce::getaddrinfo_to_addr(node_name,
                                       nullptr,
                                       sockaddr_ptr_,
                                       sizeof(sockaddr_in));
        in4_addr_.sin_family = AF_INET;
        in4_addr_.sin_port = ntohs(port_number);
        if (ret != 0)
        {
            return ret;
        }
        return 0;
    }
    else if (family == AF_INET6)
    {
        sockaddr_ptr_ = reinterpret_cast<sockaddr*>(&in6_addr_);
        sockaddr_size_ = sizeof(sockaddr_in6);
        ret = zce::getaddrinfo_to_addr(node_name,
                                       nullptr,
                                       sockaddr_ptr_,
                                       sizeof(sockaddr_in6));
        if (ret != 0)
        {
            return ret;
        }
        in6_addr_.sin6_port = ntohs(port_number);
        in6_addr_.sin6_family = AF_INET6;
        return 0;
    }
    else
    {
        assert(false);
        return -1;
    }
}

//通过IP取得域名
int Sockaddr_Any::getnameinfo(char* host_name,
                              size_t name_len) const
{
    if (sockaddr_ptr_->sa_family == AF_INET)
    {
        return zce::getnameinfo(sockaddr_ptr_,
                                sizeof(sockaddr_in),
                                host_name,
                                name_len,
                                NULL,
                                0,
                                NI_NAMEREQD);
    }
    else if (sockaddr_ptr_->sa_family == AF_INET6)
    {
        return zce::getnameinfo(sockaddr_ptr_,
                                sizeof(sockaddr_in6),
                                host_name,
                                name_len,
                                NULL,
                                0,
                                NI_NAMEREQD);
    }
    else
    {
        assert(false);
        return -1;
    }
}

//设置端口号，
void Sockaddr_Any::set_port(uint16_t port)
{
    if (sockaddr_ptr_->sa_family == AF_INET)
    {
        in4_addr_.sin_port = port;
    }
    else if (sockaddr_ptr_->sa_family == AF_INET6)
    {
        in6_addr_.sin6_port = port;
    }
    else
    {
        assert(false);
    }
}

//取得端口号
uint16_t Sockaddr_Any::get_port(void) const
{
    if (sockaddr_ptr_->sa_family == AF_INET)
    {
        return in4_addr_.sin_port;
    }
    else if (sockaddr_ptr_->sa_family == AF_INET6)
    {
        return in6_addr_.sin6_port;
    }
    else
    {
        assert(false);
        return 0;
    }
}

// 检查地址是否相等
bool Sockaddr_Any::operator == (const Sockaddr_Any& others) const
{
    //比较地址协议簇，地址
    if (sockaddr_ptr_->sa_family == AF_INET &&
        others.in4_addr_.sin_family == this->in4_addr_.sin_family &&
        others.in4_addr_.sin_addr.s_addr == this->in4_addr_.sin_addr.s_addr)
    {
        return true;
    }
    else if (sockaddr_ptr_->sa_family == AF_INET6 &&
             others.in6_addr_.sin6_family == this->in6_addr_.sin6_family &&
             0 == memcmp(&(others.in6_addr_.sin6_addr),
                         &(this->in6_addr_.sin6_addr),
                         sizeof(in6_addr)) &&
             others.in6_addr_.sin6_port == this->in6_addr_.sin6_port)
    {
        return true;
    }

    return false;
}

// 检查地址是否不相等
bool Sockaddr_Any::operator != (const Sockaddr_Any& others_sockaddr) const
{
    return !(*this == others_sockaddr);
}
}