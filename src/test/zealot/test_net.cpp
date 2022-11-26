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

//const char *SOCKS_SERVER_IP = "192.111.130.2";
//const uint16_t SOCKS_SERVER_PORT = 4145;

const char *SOCKS_SERVER_IP = "192.168.56.103";
const uint16_t SOCKS_SERVER_PORT = 1080;

int test_socks5_connect([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    int ret = 0;
    zce::socks5::tcp_connect tc;
    sockaddr_in socks5_in;
    zce::set_sockaddr_in(&socks5_in,
                         SOCKS_SERVER_IP,
                         SOCKS_SERVER_PORT);
    zce::time_value tv(5, 0);
    ret = tc.initialize((sockaddr *)&socks5_in,
                        sizeof(sockaddr_in),
                        "socks5",
                        "123456",
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
    ret = tc.tcp_connect_cmd(nullptr,
                             0,
                             (sockaddr *)&want_to,
                             sizeof(sockaddr_in),
                             tv);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

//https://lixingcong.github.io/2018/05/25/dante-socks5/
int test_socks5_udp_associate([[maybe_unused]] int argc,
                              [[maybe_unused]] char* argv[])
{
    int ret = 0;
    zce::socks5::udp_associate ua;
    sockaddr_in socks5_in;
    zce::set_sockaddr_in(&socks5_in,
                         SOCKS_SERVER_IP,
                         SOCKS_SERVER_PORT);
    zce::time_value tv(5, 0);
    ret = ua.initialize((sockaddr *)&socks5_in,
                        sizeof(sockaddr_in),
                        nullptr,
                        nullptr,
                        tv);
    if (ret != 0)
    {
        return ret;
    }

    tv.set(5, 0);
    ret = ua.udp_associate_cmd(tv);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}