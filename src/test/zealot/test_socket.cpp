#include "predefine.h"

int test_host_name_fun(int, char*[])
{
    int ret = 0;

    zce::socket_init();

    size_t     ary_addr_num = 16, ary_addr6_num = 16;
    sockaddr_in ary_sock_addr[16];
    sockaddr_in6 ary_sock_addr6[16];

    ary_addr_num = 16;
    ary_addr6_num = 16;
    ret = zce::gethostbyname_inary("www.qq.com",
                                   80,
                                   &ary_addr_num,
                                   ary_sock_addr);

    ary_addr_num = 16;
    ary_addr6_num = 16;
    ret = zce::getaddrinfo_to_addrary("www.google.com.hk",
                                      &ary_addr_num,
                                      ary_sock_addr,
                                      &ary_addr6_num,
                                      ary_sock_addr6);

    ary_addr_num = 16;
    ary_addr6_num = 16;
    ret = zce::getaddrinfo_to_addrary("www.google.com.hk",
                                      &ary_addr_num,
                                      ary_sock_addr,
                                      &ary_addr6_num,
                                      ary_sock_addr6);

    char domain_name[256 + 1];
    domain_name[256] = '\0';
    ret = zce::getnameinfo_sockaddr((sockaddr*)&ary_sock_addr[0],
                                    domain_name,
                                    256);

    ret = zce::getnameinfo_sockaddr((sockaddr*)&ary_sock_addr6[0],
                                    domain_name,
                                    256);

    zce::socket_finish();

    ZCE_UNUSED_ARG(ret);

    return 0;
}

int test_net_getaddrinfo(int /*argc*/, char* /*argv*/[])
{
    int ret = 0;

    zce::socket_init();

    const char* TEST_HOST_NAME = "www.qq.com";

    const size_t ARRAY_NUM = 50;
    size_t ary_addr_num = ARRAY_NUM;
    sockaddr_in ary_addr[ARRAY_NUM];
    size_t ary_addr6_num = ARRAY_NUM;
    sockaddr_in6 ary_addr6[ARRAY_NUM];

    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_CANONNAME;
    ret = zce::getaddrinfo(TEST_HOST_NAME,
                           NULL,
                           &hints,
                           &result);
    ary_addr_num = ARRAY_NUM;
    ary_addr6_num = ARRAY_NUM;
    zce::getaddrinfo_result_to_addrary(result,
                                       &ary_addr_num,
                                       ary_addr,
                                       &ary_addr6_num,
                                       ary_addr6);
    zce::freeaddrinfo(result);
    std::cout << "ary_addr_num=" << ary_addr_num << " ary_addr6_num=" << ary_addr6_num << std::endl;

    hints.ai_family = AF_INET6;
    hints.ai_flags = AI_CANONNAME;
    ret = zce::getaddrinfo(TEST_HOST_NAME,
                           NULL,
                           &hints,
                           &result);
    ary_addr_num = ARRAY_NUM;
    ary_addr6_num = ARRAY_NUM;
    zce::getaddrinfo_result_to_addrary(result,
                                       &ary_addr_num,
                                       ary_addr,
                                       &ary_addr6_num,
                                       ary_addr6);
    zce::freeaddrinfo(result);
    std::cout << "ary_addr_num=" << ary_addr_num << " ary_addr6_num=" << ary_addr6_num << std::endl;

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = 0;
    ret = zce::getaddrinfo(TEST_HOST_NAME,
                           NULL,
                           &hints,
                           &result);
    ary_addr_num = ARRAY_NUM;
    ary_addr6_num = ARRAY_NUM;
    zce::getaddrinfo_result_to_addrary(result,
                                       &ary_addr_num,
                                       ary_addr,
                                       &ary_addr6_num,
                                       ary_addr6);
    zce::freeaddrinfo(result);
    std::cout << "ary_addr_num=" << ary_addr_num << " ary_addr6_num=" << ary_addr6_num << std::endl;

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_ALL;
    ret = zce::getaddrinfo("ipv6.google.com",
                           NULL,
                           &hints,
                           &result);
    ary_addr_num = ARRAY_NUM;
    ary_addr6_num = ARRAY_NUM;
    zce::getaddrinfo_result_to_addrary(result,
                                       &ary_addr_num,
                                       ary_addr,
                                       &ary_addr6_num,
                                       ary_addr6);
    zce::freeaddrinfo(result);
    std::cout << "ary_addr_num=" << ary_addr_num << " ary_addr6_num=" << ary_addr6_num << std::endl;

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_ALL;
    ret = zce::getaddrinfo(TEST_HOST_NAME,
                           NULL,
                           &hints,
                           &result);
    ary_addr_num = ARRAY_NUM;
    ary_addr6_num = ARRAY_NUM;
    zce::getaddrinfo_result_to_addrary(result,
                                       &ary_addr_num,
                                       ary_addr,
                                       &ary_addr6_num,
                                       ary_addr6);
    zce::freeaddrinfo(result);
    std::cout << "ary_addr_num=" << ary_addr_num << " ary_addr6_num=" << ary_addr6_num << std::endl;

    const char* TEST_HOST_NAME1 = "203.98.7.65";
    const char* TEST_HOST_NAME2 = "2404:6800:4008:800::200e";
    const char* TEST_HOST_NAME3 = "www.qq.com";

    sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;
    sockaddr_in6 addr_in6;
    addr_in6.sin6_family = AF_INET6;
    char ip_str[256];
    ret = zce::getaddrinfo_to_addr(TEST_HOST_NAME1,
                                   (sockaddr*)(&addr_in),
                                   sizeof(addr_in));
    inet_ntop(addr_in.sin_family, (void*)&(addr_in.sin_addr), ip_str, 256);
    std::cout << TEST_HOST_NAME1 << "to sockaddr_in ret = " << ret << " ip str:" << ip_str << std::endl;
    ret = zce::getaddrinfo_to_addr(TEST_HOST_NAME1,
                                   (sockaddr*)(&addr_in6),
                                   sizeof(addr_in6));
    inet_ntop(addr_in6.sin6_family, (void*)&(addr_in6.sin6_addr), ip_str, 256);
    std::cout << TEST_HOST_NAME1 << "to sockaddr_in6 ret = " << ret << " ip str:" << ip_str << std::endl;

    ret = zce::getaddrinfo_to_addr(TEST_HOST_NAME2,
                                   (sockaddr*)(&addr_in6),
                                   sizeof(addr_in6));
    inet_ntop(addr_in6.sin6_family, (void*)&(addr_in6.sin6_addr), ip_str, 256);
    std::cout << TEST_HOST_NAME2 << "to sockaddr_in6 ret = " << ret << " ip str:" << ip_str << std::endl;
    ret = zce::getaddrinfo_to_addr(TEST_HOST_NAME2,
                                   (sockaddr*)(&addr_in),
                                   sizeof(addr_in));
    inet_ntop(addr_in.sin_family, (void*)&(addr_in.sin_addr), ip_str, 256);
    std::cout << TEST_HOST_NAME2 << "to sockaddr_in ret = " << ret << " ip str:" << ip_str << std::endl;

    ret = zce::getaddrinfo_to_addr(TEST_HOST_NAME3,
                                   (sockaddr*)(&addr_in),
                                   sizeof(addr_in));
    inet_ntop(addr_in.sin_family, (void*)&(addr_in.sin_addr), ip_str, 256);
    std::cout << TEST_HOST_NAME3 << "to sockaddr_in ret = " << ret << " ip str:" << ip_str << std::endl;
    ret = zce::getaddrinfo_to_addr(TEST_HOST_NAME3,
                                   (sockaddr*)(&addr_in6),
                                   sizeof(addr_in6));
    inet_ntop(addr_in6.sin6_family, (void*)&(addr_in6.sin6_addr), ip_str, 256);
    std::cout << TEST_HOST_NAME3 << "to sockaddr_in6 ret = " << ret << " ip str:" << ip_str << std::endl;

    return 0;
}