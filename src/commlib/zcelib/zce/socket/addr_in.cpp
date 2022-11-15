#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/socket.h"
#include "zce/os_adapt/netdb.h"
#include "zce/socket/addr_base.h"
#include "zce/socket/addr_in.h"

namespace zce::skt
{
//默认构造函数
addr_in::addr_in(void) :
    zce::skt::addr_base(reinterpret_cast<sockaddr*>(&in4_addr_),
                        sizeof(::sockaddr_in))
{
    ::memset(&in4_addr_, 0, sizeof(in4_addr_));
    in4_addr_.sin_family = AF_INET;
}

//根据addr_in构造，
addr_in::addr_in(const sockaddr_in* addr) :
    zce::skt::addr_base(reinterpret_cast<sockaddr*>(&in4_addr_),
                        sizeof(::sockaddr_in))
{
    in4_addr_ = *addr;
}

//根据IP地址(XXX.XXX.XXX.XXX)字符串，端口号初始化构造
addr_in::addr_in(const char* ip_addr_str,
                 uint16_t port_number) :
    zce::skt::addr_base(reinterpret_cast<sockaddr*>(&in4_addr_),
                        sizeof(::sockaddr_in))
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr_str, port_number);
    if (ret != 0)
    {
        ZCE_ASSERT(false);
    }
}

//根据地址(整数)，端口号初始化构造
addr_in::addr_in(uint32_t ip_addr,
                 uint16_t port_number) :
    zce::skt::addr_base(reinterpret_cast<sockaddr*>(&in4_addr_),
                        sizeof(::sockaddr_in))
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr, port_number);
    if (ret != 0)
    {
        ZCE_ASSERT(false);
    }
}

//拷贝构造，一定要写，这个类的基类指针是指向自己的一个地址的，
addr_in::addr_in(const addr_in& others) :
    zce::skt::addr_base(reinterpret_cast<sockaddr*>(&in4_addr_),
                        sizeof(::sockaddr_in))
{
    in4_addr_ = others.in4_addr_;
}

//设置地址信息
void addr_in::set_sockaddr(sockaddr* addr, socklen_t len)
{
    in4_addr_ = *(reinterpret_cast<addr_in*>(addr));
    sockaddr_size_ = len;
}

//根据IP地址(XXX.XXX.XXX.XXX)字符串，端口号设置
int addr_in::set(const char ip_addr_str[16],
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
int addr_in::set(uint32_t ip_addr,
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
int addr_in::set(const char* ip_addr_str)
{
    int ret = zce::set_sockaddr_in(&in4_addr_, ip_addr_str);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

///检查端口号是否是一个安全端口
bool addr_in::check_safeport()
{
    return zce::check_safeport(this->get_port());
}

//比较两个地址是否相等
bool addr_in::operator == (const addr_in& others) const
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
bool addr_in::operator != (const addr_in& others) const
{
    return !(*this == others);
}

//检查IP地址是否相等
bool addr_in::is_ip_equal(const addr_in& others) const
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
int addr_in::getnameinfo(char* host_name, size_t name_len) const
{
    return zce::getnameinfo(reinterpret_cast<const sockaddr*>(&in4_addr_),
                            sizeof(addr_in),
                            host_name,
                            name_len,
                            nullptr,
                            0,
                            NI_NAMEREQD);
}

//取得域名相关的IP地址信息，调用函数是getaddrinfo_to_addr
int addr_in::getaddrinfo(const char* nodename,
                         uint16_t port_number)
{
    in4_addr_.sin_port = ntohs(port_number);
    return zce::getaddrinfo_addr(nodename,
                                 nullptr,
                                 sockaddr_ptr_,
                                 sizeof(addr_in));
}

//返回addr_in
addr_in::operator sockaddr_in () const
{
    return in4_addr_;
}

//返回内部const addr_in的指针，（不可以被修改）
addr_in::operator const sockaddr_in* () const
{
    return &in4_addr_;
}

//返回内部addr_in的指针，（可以被修改）
addr_in::operator sockaddr_in* ()
{
    return &in4_addr_;
}
}