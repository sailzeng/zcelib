


#include "zealot_predefine.h"


int test_host_name_fun(int, char * [])
{

    int ret = 0;

    ZCE_OS::socket_init();

    size_t     ary_addr_num = 16;
    sockaddr_in ary_sock_addr[16];
    sockaddr_in6 ary_sock_addr6[16];

    ary_addr_num = 16;
    ret = ZCE_OS::gethostbyname_inary("www.qq.com",
                                      80,
                                      &ary_addr_num,
                                      ary_sock_addr);

    ary_addr_num = 16;
    ret = ZCE_OS::getaddrinfo_inary("www.google.com.hk",
                                    80,
                                    &ary_addr_num,
                                    ary_sock_addr);

    ary_addr_num = 16;
    ret = ZCE_OS::getaddrinfo_in6ary("www.google.com.hk",
                                     80,
                                     &ary_addr_num,
                                     ary_sock_addr6);

    char domain_name[256 + 1];
    domain_name[256] = '\0';
    ret = ZCE_OS::getnameinfo_in(&ary_sock_addr[0],
                                 domain_name,
                                 256);

    ret = ZCE_OS::getnameinfo_in6(&ary_sock_addr6[0],
                                  domain_name,
                                  256);

    ZCE_OS::socket_fini();

    return 0;
}



