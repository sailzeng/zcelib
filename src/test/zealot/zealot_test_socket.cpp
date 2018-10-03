


#include "zealot_predefine.h"


int test_host_name_fun(int, char * [])
{

    int ret = 0;

    ZCE_LIB::socket_init();

    size_t     ary_addr_num = 16, ary_addr6_num = 16 ;
    sockaddr_in ary_sock_addr[16];
    sockaddr_in6 ary_sock_addr6[16];

    ary_addr_num = 16;
    ary_addr6_num = 16;
    ret = ZCE_LIB::gethostbyname_inary("www.qq.com",
                                      80,
                                      &ary_addr_num,
                                      ary_sock_addr);

    ary_addr_num = 16;
    ary_addr6_num = 16;
    ret = ZCE_LIB::getaddrinfo_to_addrary("www.google.com.hk",
                                          &ary_addr_num,
                                          ary_sock_addr,
                                          &ary_addr6_num,
                                          ary_sock_addr6);

    ary_addr_num = 16;
    ary_addr6_num = 16;
    ret = ZCE_LIB::getaddrinfo_to_addrary("www.google.com.hk",
                                          &ary_addr_num,
                                          ary_sock_addr,
                                          &ary_addr6_num,
                                          ary_sock_addr6);

    char domain_name[256 + 1];
    domain_name[256] = '\0';
    ret = ZCE_LIB::getnameinfo_sockaddr((sockaddr *)&ary_sock_addr[0],
                                        domain_name,
                                        256);

    ret = ZCE_LIB::getnameinfo_sockaddr((sockaddr *)&ary_sock_addr6[0],
                                        domain_name,
                                        256);

    ZCE_LIB::socket_fini();

    ZCE_UNUSED_ARG(ret);

    return 0;
}


int test_net_getaddrinfo(int /*argc*/, char * /*argv*/[])
{
    int ret = 0;

    ZCE_LIB::socket_init();

    const size_t ARRAY_NUM = 50;
    size_t ary_addr_num = ARRAY_NUM;
    sockaddr_in ary_addr[ARRAY_NUM];
    size_t ary_addr6_num = ARRAY_NUM;
    sockaddr_in6 ary_addr6[ARRAY_NUM];

    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM;

    addrinfo *result = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_flags = 0;
    ret = ZCE_LIB::getaddrinfo("ipv6.google.com",
                               NULL,
                               &hints,
                               &result);
    ary_addr_num = ARRAY_NUM;
    ary_addr6_num = ARRAY_NUM;
    ZCE_LIB::getaddrinfo_result_to_addrary(result,
                                           &ary_addr_num,
                                           ary_addr,
                                           &ary_addr6_num,
                                           ary_addr6);
    ZCE_LIB::freeaddrinfo(result);
    std::cout << "ary_addr_num=" << ary_addr_num << " ary_addr6_num=" << ary_addr6_num << std::endl;

    hints.ai_family = AF_INET6;
    hints.ai_flags = AI_ALL;
    ret = ZCE_LIB::getaddrinfo("ipv6.google.com",
                               NULL,
                               &hints,
                               &result);
    ary_addr_num = ARRAY_NUM;
    ary_addr6_num = ARRAY_NUM;
    ZCE_LIB::getaddrinfo_result_to_addrary(result,
                                           &ary_addr_num,
                                           ary_addr,
                                           &ary_addr6_num,
                                           ary_addr6);
    ZCE_LIB::freeaddrinfo(result);
    std::cout << "ary_addr_num=" << ary_addr_num << " ary_addr6_num=" << ary_addr6_num << std::endl;

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = 0;
    ret = ZCE_LIB::getaddrinfo("ipv6.google.com",
                               NULL,
                               &hints,
                               &result);
    ary_addr_num = ARRAY_NUM;
    ary_addr6_num = ARRAY_NUM;
    ZCE_LIB::getaddrinfo_result_to_addrary(result,
                                           &ary_addr_num,
                                           ary_addr,
                                           &ary_addr6_num,
                                           ary_addr6);
    ZCE_LIB::freeaddrinfo(result);
    std::cout << "ary_addr_num=" << ary_addr_num << " ary_addr6_num=" << ary_addr6_num << std::endl;

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_ALL;
    ret = ZCE_LIB::getaddrinfo("ipv6.google.com",
                               NULL,
                               &hints,
                               &result);
    ary_addr_num = ARRAY_NUM;
    ary_addr6_num = ARRAY_NUM;
    ZCE_LIB::getaddrinfo_result_to_addrary(result,
                                           &ary_addr_num,
                                           ary_addr,
                                           &ary_addr6_num,
                                           ary_addr6);
    ZCE_LIB::freeaddrinfo(result);
    std::cout << "ary_addr_num=" << ary_addr_num << " ary_addr6_num=" << ary_addr6_num << std::endl;


    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_ALL;
    ret = ZCE_LIB::getaddrinfo("ipv6.google.com",
                               NULL,
                               &hints,
                               &result);
    ary_addr_num = ARRAY_NUM;
    ary_addr6_num = ARRAY_NUM;
    ZCE_LIB::getaddrinfo_result_to_addrary(result,
                                           &ary_addr_num,
                                           ary_addr,
                                           &ary_addr6_num,
                                           ary_addr6);
    ZCE_LIB::freeaddrinfo(result);
    std::cout << "ary_addr_num=" << ary_addr_num << " ary_addr6_num=" << ary_addr6_num << std::endl;

    return 0;
}


 