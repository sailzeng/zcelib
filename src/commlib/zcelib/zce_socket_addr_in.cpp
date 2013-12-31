#include "zce_predefine.h"
#include "zce_os_adapt_socket.h"
#include "zce_socket_addr_base.h"
#include "zce_trace_log_debug.h"
#include "zce_socket_addr_in.h"

//默认构造函数
ZCE_Sockaddr_In::ZCE_Sockaddr_In (void):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in4_addr_), sizeof(sockaddr_in))
{

}

//根据sockaddr_in构造，
ZCE_Sockaddr_In::ZCE_Sockaddr_In (const sockaddr_in *addr):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in4_addr_), sizeof(sockaddr_in))
{
    in4_addr_ = *addr;
}

//根据IP地址(XXX.XXX.XXX.XXX)字符串，端口号初始化构造
ZCE_Sockaddr_In::ZCE_Sockaddr_In (const char *ip_addr_str,
                                  uint16_t port_number):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in4_addr_), sizeof(sockaddr_in))
{
    int ret = ZCE_OS::set_sockaddr_in(&in4_addr_, ip_addr_str, port_number);

    if (ret != 0)
    {
        ZCE_ASSERT(false);
    }
}

//根据地址(整数)，端口号初始化构造
ZCE_Sockaddr_In::ZCE_Sockaddr_In (uint32_t ip_addr,
                                  uint16_t port_number):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in4_addr_), sizeof(sockaddr_in))
{
    int ret = ZCE_OS::set_sockaddr_in(&in4_addr_, ip_addr, port_number);

    if (ret != 0)
    {
        ZCE_ASSERT(false);
    }
}

//拷贝构造，一定要写，这个类的基类指针是指向自己的一个地址的，
ZCE_Sockaddr_In::ZCE_Sockaddr_In (const ZCE_Sockaddr_In &others):
    ZCE_Sockaddr(reinterpret_cast<sockaddr *>(&in4_addr_), sizeof(sockaddr_in))
{
    in4_addr_ = others.in4_addr_;
}

ZCE_Sockaddr_In::~ZCE_Sockaddr_In()
{

}

//检查地址是否是一个内网地址
bool ZCE_Sockaddr_In::is_internal()
{
    return ZCE_OS::is_internal(&in4_addr_);
}

//检查地址是否是一个外网地址，其实我就简单认为不是外网地址就是内网地址
bool ZCE_Sockaddr_In::is_internet()
{
    return !(ZCE_OS::is_internal(&in4_addr_));
}

//设置地址信息
void ZCE_Sockaddr_In::set_sockaddr (sockaddr *addr, socklen_t len)
{
    in4_addr_ = *(reinterpret_cast<sockaddr_in *>(addr));
    sockaddr_size_ = len;
}

//根据IP地址(XXX.XXX.XXX.XXX)字符串，端口号设置
int ZCE_Sockaddr_In::set(const char ip_addr_str[16],
                         uint16_t port_number)
{
    int ret = ZCE_OS::set_sockaddr_in(&in4_addr_, ip_addr_str, port_number);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//根据地址IP，端口号设置
int ZCE_Sockaddr_In::set(uint32_t ip_addr,
                         uint16_t port_number)
{
    int ret = ZCE_OS::set_sockaddr_in(&in4_addr_, ip_addr, port_number);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//比较两个地址是否相等
bool ZCE_Sockaddr_In::operator == (const ZCE_Sockaddr_In &others) const
{
    //in4_addr_.sin_zero 要比较吗？暂时算了。

    //比较地址协议簇，地址，端口
    if (others.in4_addr_.sin_family == in4_addr_.sin_family &&
        others.in4_addr_.sin_addr.s_addr == in4_addr_.sin_addr.s_addr &&
        others.in4_addr_.sin_port == in4_addr_.sin_port )
    {
        return true;
    }

    return false;
}

//比较两个地址是否不想等
bool ZCE_Sockaddr_In::operator != (const ZCE_Sockaddr_In &others) const
{
    return !(*this == others);
}

//检查IP地址是否相等
bool ZCE_Sockaddr_In::is_ip_equal (const ZCE_Sockaddr_In &others) const
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
int ZCE_Sockaddr_In::get_name_info(char *host_name, size_t name_len) const
{
    return ZCE_OS::getnameinfo(reinterpret_cast<const sockaddr *>(&in4_addr_),
                               sizeof(sockaddr_in),
                               host_name,
                               name_len,
                               NULL,
                               0,
                               NI_NAMEREQD);
}

//取得域名相关的IP地址信息，调用函数是getaddrinfo
int ZCE_Sockaddr_In::get_addr_info(const char *hostname, uint16_t service_port)
{
    size_t only_one_addr = 1;
    return ZCE_OS::getaddrinfo_inary(hostname,
                                     service_port,
                                     &only_one_addr,
                                     &in4_addr_);
}

//返回sockaddr_in
ZCE_Sockaddr_In::operator sockaddr_in () const
{
    return in4_addr_;
}

//返回内部const sockaddr_in的指针，（不可以被修改）
ZCE_Sockaddr_In::operator const sockaddr_in *() const
{
    return &in4_addr_;
}

//返回内部sockaddr_in的指针，（可以被修改）
ZCE_Sockaddr_In::operator sockaddr_in *()
{
    return &in4_addr_;
}
