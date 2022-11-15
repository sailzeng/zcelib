#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/netdb.h"

namespace zce
{
//-------------------------------------------------------------------------------------
//域名解析，转换IP地址的几个函数

//通过域名得到相关的IP地址
hostent* gethostbyname(const char* hostname)
{
    return ::gethostbyname(hostname);
}

// GNU extensions
hostent* gethostbyname2(const char* hostname,
                        int af)
{
#if defined (ZCE_OS_WINDOWS)

    hostent* hostent_ptr = ::gethostbyname(hostname);
    if (hostent_ptr->h_addrtype != af)
    {
        return nullptr;
    }
    return hostent_ptr;

#elif defined (ZCE_OS_LINUX)
    return ::gethostbyname2(hostname, af);
#endif
}

//非标准函数,得到某个域名的IPV4的地址信息，但是使用起来比较容易和方便
//name 域名
//uint16_t service_port 端口号，本地序
//ary_addr_num  ,输入输出参数，输入标识ary_sock_addr的个数，输出时标识返回的队列数量
//ary_sock_addr ,输出参数，返回的地址队列
int gethostbyname_inary(const char* hostname,
                        uint16_t service_port,
                        size_t* ary_addr_num,
                        sockaddr_in ary_sock_addr[])
{
    //其实这个函数没法重入
    struct hostent* hostent_ptr = ::gethostbyname(hostname);

    if (!hostent_ptr)
    {
        return -1;
    }

    if (hostent_ptr->h_addrtype != AF_INET)
    {
        errno = EINVAL;
        return -1;
    }

    //检查返回
    ZCE_ASSERT(hostent_ptr->h_length == sizeof(in_addr));

    //循环得到所有的IP地址信息
    size_t i = 0;
    char** addr_pptr = hostent_ptr->h_addr_list;

    for (; (i < *ary_addr_num) && (*addr_pptr != nullptr); addr_pptr++, ++i)
    {
        ary_sock_addr[i].sin_family = AF_INET;
        //本来就是网络序
        memcpy(&(ary_sock_addr[i].sin_addr), addr_pptr, hostent_ptr->h_length);
        //端口转换成网络序
        ary_sock_addr[i].sin_port = htons(service_port);
    }

    //记录数量
    *ary_addr_num = i;

    return 0;
}

//非标准函数,得到某个域名的IPV6的地址信息，但是使用起来比较容易和方便
int gethostbyname_in6ary(const char* hostname,
                         uint16_t service_port,
                         size_t* ary_addr6_num,
                         sockaddr_in6 ary_sock_addr6[])
{
    //其实这个函数没法重入
    struct hostent* hostent_ptr = ::gethostbyname(hostname);

    if (!hostent_ptr)
    {
        return -1;
    }

    if (hostent_ptr->h_addrtype != AF_INET6)
    {
        errno = EINVAL;
        return -1;
    }

    //检查返回的地址实习是不是IPV6的
    ZCE_ASSERT(hostent_ptr->h_length == sizeof(in6_addr));

    //循环得到所有的IP地址信息
    size_t i = 0;
    char** addr_pptr = hostent_ptr->h_addr_list;

    for (; (i < *ary_addr6_num) && (*addr_pptr != nullptr); addr_pptr++, ++i)
    {
        ary_sock_addr6[i].sin6_family = AF_INET6;
        //本来就是网络序
        memcpy(&(ary_sock_addr6[i].sin6_addr), addr_pptr, hostent_ptr->h_length);
        //端口转换成网络序
        ary_sock_addr6[i].sin6_port = htons(service_port);
    }

    //记录数量
    *ary_addr6_num = i;

    return 0;
}

//根据地址得到域名的函数,推荐使用替代函数getnameinfo ,
hostent* gethostbyaddr(const void* addr,
                       socklen_t len,
                       int family)
{
    return ::gethostbyaddr((const char*)addr, len, family);
};

//非标准函数，通过IPV4地址取得域名
int gethostbyaddr_in(const sockaddr_in* sock_addr,
                     char* host_name,
                     size_t name_len)
{
    struct hostent* hostent_ptr = zce::gethostbyaddr(sock_addr,
                                                     sizeof(sockaddr_in),
                                                     sock_addr->sin_family
    );

    //如果返回失败
    if (!hostent_ptr)
    {
        return -1;
    }

    ::strncpy(host_name, hostent_ptr->h_name, name_len);

    return 0;
}

//非标准函数，通过IPV6地址取得域名
int gethostbyaddr_in6(const sockaddr_in6* sock_addr6,
                      char* host_name,
                      size_t name_len)
{
    struct hostent* hostent_ptr = zce::gethostbyaddr(sock_addr6,
                                                     sizeof(sockaddr_in6),
                                                     sock_addr6->sin6_family);

    //如果返回失败
    if (!hostent_ptr)
    {
        return -1;
    }

    ::strncpy(host_name, hostent_ptr->h_name, name_len);

    return 0;
}

//通过域名得到服务器地址信息，可以同时得到IPV4，和IPV6的地址
int getaddrinfo(const char* nodename,
                const char* service,
                const addrinfo* hints,
                addrinfo** result)
{
    return ::getaddrinfo(nodename,
                         service,
                         hints,
                         result);
}

//释放getaddrinfo得到的结果
void freeaddrinfo(struct addrinfo* result)
{
    return ::freeaddrinfo(result);
}

//
int getaddrinfo_result_to_oneaddr(addrinfo* result,
                                  sockaddr* addr,
                                  socklen_t addr_len)
{
    addrinfo* prc_node = result;
    for (; (prc_node != nullptr); prc_node = prc_node->ai_next)
    {
        //只取相应的地址
        if (addr_len == static_cast<socklen_t>(prc_node->ai_addrlen))
        {
            ::memcpy(addr, prc_node->ai_addr, prc_node->ai_addrlen);
            break;
        }
    }
    if (nullptr == prc_node)
    {
        return -1;
    }
    return 0;
}

//将getaddrinfo的结果进行加工处理，处理成数组
void getaddrinfo_result_to_addrary(addrinfo* result,
                                   size_t* ary_addr_num,
                                   sockaddr_in ary_addr[],
                                   size_t* ary_addr6_num,
                                   sockaddr_in6 ary_addr6[])
{
    addrinfo* prc_node = result;
    size_t num_addr = 0, num_addr6 = 0;
    for (size_t i = 0;
         (i < *ary_addr_num) && (prc_node != nullptr);
         prc_node = prc_node->ai_next, ++i)
    {
        //只取相应的地址
        if (AF_INET == prc_node->ai_family)
        {
            memcpy(&(ary_addr[i]), prc_node->ai_addr, prc_node->ai_addrlen);
            ++num_addr;
        }
    }
    //记录数量
    *ary_addr_num = num_addr;

    prc_node = result;
    for (size_t j = 0;
         (j < *ary_addr6_num) && (prc_node != nullptr);
         prc_node = prc_node->ai_next, ++j)
    {
        if (AF_INET6 == prc_node->ai_family)
        {
            memcpy(&(ary_addr6[j]), prc_node->ai_addr, prc_node->ai_addrlen);
            ++num_addr6;
        }
    }
    *ary_addr6_num = num_addr6;
}

//非标准函数,得到某个域名的IPV4 和IPV6的地址数组，使用起来比较容易和方便
int getaddrinfo_addrary(const char* hostname,
                        const char* service,
                        size_t* ary_addr_num,
                        sockaddr_in ary_addr[],
                        size_t* ary_addr6_num,
                        sockaddr_in6 ary_addr6[])
{
    int ret = 0;
    addrinfo hints, * result = nullptr;

    ::memset(&hints, 0, sizeof(addrinfo));
    //同时返回IPV4.和IPV6，如果只需要IPV4填写AF_INET，如果只需要IPV6填写AF_INET6
    hints.ai_family = AF_UNSPEC;
    //hints.ai_socktype = 0; 返回所有类型
    //hints.ai_flags = 0;
    ret = zce::getaddrinfo(hostname,
                           service,
                           &hints,
                           &result);
    if (ret != 0)
    {
        return ret;
    }
    if (!result)
    {
        errno = EINVAL;
        return -1;
    }

    //取回结果
    getaddrinfo_result_to_addrary(result,
                                  ary_addr_num,
                                  ary_addr,
                                  ary_addr6_num,
                                  ary_addr6);
    //释放空间
    zce::freeaddrinfo(result);
    return 0;
}

int getaddrinfo_to_addr(const char* nodename,
                        const char* service,
                        sockaddr* addr,
                        socklen_t addr_len)
{
    int ret = 0;
    addrinfo hints, * result = nullptr;

    ::memset(&hints, 0, sizeof(addrinfo));
    //同时返回IPV4.和IPV6，如果只需要IPV4填写AF_INET，如果只需要IPV6填写AF_INET6
    hints.ai_family = addr->sa_family;
    //优先分析nodename是否是数值地址
    hints.ai_flags = AI_PASSIVE;
    ret = zce::getaddrinfo(nodename,
                           service,
                           &hints,
                           &result);
    if (ret != 0)
    {
        //进行域名解析
        hints.ai_flags = 0;
        ret = zce::getaddrinfo(nodename,
                               nullptr,
                               &hints,
                               &result);
        if (ret != 0)
        {
            return ret;
        }
    }

    if (!result)
    {
        errno = EINVAL;
        return -1;
    }

    getaddrinfo_result_to_oneaddr(result, addr, addr_len);
    //释放空间
    zce::freeaddrinfo(result);
    return 0;
}

//通过IP地址信息，反查域名.服务名，可以重入函数
int getnameinfo(const struct sockaddr* sa,
                socklen_t salen,
                char* host,
                size_t hostlen,
                char* serv,
                size_t servlen,
                int flags)
{
#if defined (ZCE_OS_WINDOWS)
    return ::getnameinfo(sa,
                         salen,
                         host,
                         static_cast<DWORD>(hostlen),
                         serv,
                         static_cast<DWORD>(servlen), flags);
#elif defined (ZCE_OS_LINUX)
    return ::getnameinfo(sa,
                         salen,
                         host,
                         hostlen,
                         serv,
                         servlen,
                         flags);
#endif
}

//非标准函数，通过IPV4地址取得域名
int getnameinfo_sockaddr(const sockaddr* sock_addr,
                         char* host_name,
                         size_t name_len)
{
    if (sock_addr->sa_family == AF_INET)
    {
        return zce::getnameinfo(sock_addr,
                                sizeof(sockaddr_in),
                                host_name,
                                name_len,
                                nullptr,
                                0,
                                NI_NAMEREQD);
    }
    else if (sock_addr->sa_family == AF_INET)
    {
        return zce::getnameinfo(sock_addr,
                                sizeof(sockaddr_in6),
                                host_name,
                                name_len,
                                nullptr,
                                0,
                                NI_NAMEREQD);
    }
    else
    {
        ZCE_ASSERT(false);
        return -1;
    }
}

//--------------------------------------------------------------------------------------------------------------------
//返回当前机器的主机名称,2个平台应该都支持这个函数
inline int zce::gethostname(char* name, size_t name_len)
{
#if defined (ZCE_OS_WINDOWS)
    return ::gethostname(name, static_cast<int>(name_len));
#elif defined (ZCE_OS_LINUX)
    return ::gethostname(name, name_len);
#endif
}
}