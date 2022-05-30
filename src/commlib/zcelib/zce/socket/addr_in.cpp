#include "zce/predefine.h"
#include "zce/os_adapt/socket.h"
#include "zce/socket/addr_base.h"
#include "zce/logger/logging.h"
#include "zce/socket/addr_in.h"

namespace zce
{
//默认构造函数
Sockaddr_In::Sockaddr_In(void) :
    Sockaddr_Base(reinterpret_cast<sockaddr*>(&in4_addr_), sizeof(sockaddr_in))
{
    ::memset(&in4_addr_, 0, sizeof(in4_addr_));
    in4_addr_.sin_family = AF_INET;
}

//根据sockaddr_in构造，
Sockaddr_In::Sockaddr_In(const sockaddr_in* addr) :
    Sockaddr_Base(reinterpret_cast<sockaddr*>(&in4_addr_), sizeof(sockaddr_in))
{
    in4_addr_ = *addr;
}

//根据IP地址(XXX.XXX.XXX.XXX)字符串，端口号初始化构造
Sockaddr_In::Sockaddr_In(const char* ip_addr_str,
                         uint16_t port_number) :
    Sockaddr_Base(reinterpret_cast<sockaddr*>(&in4_addr_), sizeof(sockaddr_in))
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr_str, port_number);

    if (ret != 0)
    {
        ZCE_ASSERT(false);
    }
}

//根据地址(整数)，端口号初始化构造
Sockaddr_In::Sockaddr_In(uint32_t ip_addr,
                         uint16_t port_number) :
    Sockaddr_Base(reinterpret_cast<sockaddr*>(&in4_addr_), sizeof(sockaddr_in))
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr, port_number);

    if (ret != 0)
    {
        ZCE_ASSERT(false);
    }
}

//拷贝构造，一定要写，这个类的基类指针是指向自己的一个地址的，
Sockaddr_In::Sockaddr_In(const Sockaddr_In& others) :
    Sockaddr_Base(reinterpret_cast<sockaddr*>(&in4_addr_), sizeof(sockaddr_in))
{
    in4_addr_ = others.in4_addr_;
}

Sockaddr_In::~Sockaddr_In()
{
}

//设置地址信息
void Sockaddr_In::set_sockaddr(sockaddr* addr, socklen_t len)
{
    in4_addr_ = *(reinterpret_cast<sockaddr_in*>(addr));
    sockaddr_size_ = len;
}

//根据IP地址(XXX.XXX.XXX.XXX)字符串，端口号设置
int Sockaddr_In::set(const char ip_addr_str[16],
                     uint16_t port_number)
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr_str, port_number);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//根据地址IP，端口号设置
int Sockaddr_In::set(uint32_t ip_addr,
                     uint16_t port_number)
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr, port_number);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//根据字符串设置IP地址，如果有#会提取端口号
int Sockaddr_In::set(const char* ip_addr_str)
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr_str);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

///检查端口号是否是一个安全端口
bool Sockaddr_In::check_safeport()
{
    return zce::check_safeport(this->get_port());
}

//比较两个地址是否相等
bool Sockaddr_In::operator == (const Sockaddr_In& others) const
{
    //in4_addr_.sin_zero 要比较吗？暂时算了。有一些时候没有memset，sin_zero会不一样

    //比较地址协议簇，地址，端口
    if (others.in4_addr_.sin_family == in4_addr_.sin_family &&
        others.in4_addr_.sin_addr.s_addr == in4_addr_.sin_addr.s_addr &&
        others.in4_addr_.sin_port == in4_addr_.sin_port)
    {
        return true;
    }

    return false;
}

//比较两个地址是否不想等
bool Sockaddr_In::operator != (const Sockaddr_In& others) const
{
    return !(*this == others);
}

//检查IP地址是否相等
bool Sockaddr_In::is_ip_equal(const Sockaddr_In& others) const
{
    //比较地址协议簇，地址
    if (others.in4_addr_.sin_family == in4_addr_.sin_family &&
        others.in4_addr_.sin_addr.s_addr == in4_addr_.sin_addr.s_addr)
    {
        return true;
    }

    return false;
}

//取得IP地址相关的域名信息,调用函数是getnameinfo
int Sockaddr_In::getnameinfo(char* host_name, size_t name_len) const
{
    return zce::getnameinfo(reinterpret_cast<const sockaddr*>(&in4_addr_),
                            sizeof(sockaddr_in),
                            host_name,
                            name_len,
                            NULL,
                            0,
                            NI_NAMEREQD);
}

//取得域名相关的IP地址信息，调用函数是getaddrinfo_to_addr
int Sockaddr_In::getaddrinfo(const char* nodename,
                             uint16_t port_number)
{
    in4_addr_.sin_port = ntohs(port_number);
    return zce::getaddrinfo_to_addr(nodename,
                                    sockaddr_ptr_,
                                    sizeof(sockaddr_in));
}

//返回sockaddr_in
Sockaddr_In::operator sockaddr_in () const
{
    return in4_addr_;
}

//返回内部const sockaddr_in的指针，（不可以被修改）
Sockaddr_In::operator const sockaddr_in* () const
{
    return &in4_addr_;
}

//返回内部sockaddr_in的指针，（可以被修改）
Sockaddr_In::operator sockaddr_in* ()
{
    return &in4_addr_;
}
}