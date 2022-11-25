#include "predefine.h"

int test_dns_resolve([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    zce::socket_init();

    int ret = 0;
    zce::dns_resolve dns;
    ret = dns.initialize(AF_INET, "114.114.114.114", 53);
    printf("initialize ret=%d\n", ret);
    if (ret != 0)
    {
        return ret;
    }
    uint16_t tid = 0;
    ret = dns.query("www.sina.com.cn", zce::dns_resolve::QTYPE_A, &tid);
    printf("query ret=%d tid = %x \n", ret, tid);
    if (ret != 0)
    {
        return ret;
    }
    const size_t ARRAYS_SIZE = 64;
    size_t arrays_size = ARRAYS_SIZE;
    ::sockaddr_in addrs_ary[ARRAYS_SIZE];
    zce::time_value tv(5);
    ret = dns.answer(&tid, AF_INET, (sockaddr*)addrs_ary, &arrays_size, &tv);
    printf("query ret=%d tid = %x arrays_size = %" PRId64 "\n",
           ret,
           tid,
           arrays_size);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

int test_ping([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    int ret = 0;
    zce::ping ping;
    ret = ping.initialize(AF_INET, "114.114.114.114");
    printf("initialize ret=%d\n", ret);
    if (ret != 0)
    {
        return ret;
    }
    ping.ping_test(10);
    zce::ping ping6;
    ret = ping6.initialize(AF_INET6, "::1");
    printf("initialize ret=%d\n", ret);
    if (ret != 0)
    {
        return ret;
    }
    ping6.ping_test(10);
    return 0;
}

int test_socks5([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    int ret = 0;
    zce::socks5::client sc;
    sockaddr_in socks5_in;
    zce::set_sockaddr_in(&socks5_in,
                         "184.178.172.28",
                         15294);
    zce::time_value tv(5, 0);
    ret = sc.initialize((sockaddr *)&socks5_in,
                        sizeof(sockaddr_in),
                        nullptr,
                        nullptr,
                        tv);
    if (ret != 0)
    {
        return ret;
    }
    tv.set(5, 0);
    sockaddr_in want_to;
    zce::set_sockaddr_in(&want_to,
                         "39.156.66.10",
                         443);
    sockaddr_in bind_addr;
    ret = sc.socks5_cmd(zce::socks5::CMD_CONNECT,
                        nullptr,
                        0,
                        (sockaddr *)&want_to,
                        sizeof(sockaddr_in),
                        (sockaddr *)&bind_addr,
                        tv);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}