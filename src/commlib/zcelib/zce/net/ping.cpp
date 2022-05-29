#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/socket.h"
#include "zce/net/ping.h"

//==============================================================================================
//https://www.jianshu.com/p/8e9903c7f9e8
//https://network.fasionchan.com/zh_CN/latest/practices/ping-by-icmp-c.html
//https://blog.csdn.net/qq_33690566/article/details/103653440

namespace zce
{
#define MAGIC "12345678901"
#define MAGIC_LEN 12
#define MTU 1500

#pragma pack (1)

//IPV6的IMCPV6的Checksum计算需要一个伪头部
struct _ICMPV6_PSEUDO
{
    //
    in6_addr src_addr_;
    in6_addr dst_addr_;
    //
    uint32_t upper_layer_len_;
    uint8_t zero_[3];
    uint8_t upper_proto_;
};

//ICMP的ECHO信息，前3个字节是头部
struct _ICMP_ECHO
{
    //前3个字段是 头部

    uint8_t type_;
    //
    uint8_t code;
    //
    uint16_t check_sum_;

    //
    uint32_t ident_;
    //
    uint32_t seq_;

    // data
    uint64_t sending_ts_;
    //
    char magic_[MAGIC_LEN];
};

#pragma pack ()

int Ping::initialize(::sockaddr *ping_addr,
                     socklen_t addr_len)
{
    addr_len_ = addr_len;
    if (addr_len == sizeof(::sockaddr_in))
    {
        addr_family_ = AF_INET;
        ping_addr_.in_.sin_family = AF_INET;
    }
    else if (addr_len == sizeof(::sockaddr_in6))
    {
        addr_family_ = AF_INET6;
        ping_addr_.in6_.sin6_family = AF_INET6;
    }
    else
    {
        assert(false);
    }
    ping_addr_.set(ping_addr, addr_family_);
    int ret = initialize();
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

int Ping::initialize(int svr_family,
                     const char *ping_svr)
{
    int ret = 0;
    addr_family_ = svr_family;
    if (addr_family_ == AF_INET)
    {
        addr_len_ = sizeof(sockaddr_in);
        ping_addr_.in_.sin_family = AF_INET;
        local_addr_.in_.sin_family = AF_INET;
    }
    else if (addr_family_ == AF_INET6)
    {
        addr_len_ = sizeof(sockaddr_in6);
        ping_addr_.in6_.sin6_family = AF_INET6;
        local_addr_.in6_.sin6_family = AF_INET6;
    }
    else
    {
        assert(false);
    }
    ret = getaddrinfo_to_addr(ping_svr,
                              (sockaddr *)&ping_addr_,
                              addr_len_);
    if (ret != 0)
    {
        return ret;
    }
    ret = initialize();
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

int Ping::initialize()
{
    int ret = 0;
    int proto = IPPROTO_ICMP;
    if (addr_family_ == AF_INET6)
    {
        proto = IPPROTO_ICMPV6;
    }

    ping_socket_ = zce::socket(addr_family_,
                               SOCK_RAW,
                               proto);
    if (ping_socket_ == ZCE_INVALID_SOCKET)
    {
        return -1;
    }

    socklen_t opval = 8 * 1024;
    ret = ::setsockopt(ping_socket_,
                       SOL_SOCKET,
                       SO_RCVBUF,
                       (const char*)&opval,
                       sizeof(socklen_t));
    if (ret != 0)
    {
        return ret;
    }
    //必须先connect，否则喔不不知道本地IP。
    ret = zce::connect(ping_socket_,
                       (struct sockaddr*)&ping_addr_,
                       addr_len_);
    if (ret != 0)
    {
        return ret;
    }
    ret = zce::getsockname(ping_socket_,
                           (sockaddr *)&local_addr_,
                           &addr_len_);
    return 0;
}

ZCE_SOCKET Ping::get_handle()
{
    return ping_socket_;
}

uint16_t Ping::calculate_checksum(char* buffer, size_t bytes)
{
    uint32_t checksum = 0;
    char* end = buffer + bytes;

    // odd bytes add last byte and reset end
    if (bytes % 2 == 1)
    {
        end = buffer + bytes - 1;
        checksum += (*end) << 8;
    }

    // add words of two bytes, one by one
    while (buffer < end)
    {
        checksum += buffer[0] << 8;
        checksum += buffer[1];
        buffer += 2;
    }

    // add carry if any
    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);

    // negate it
    return (~checksum) & 0xffff;
}

int Ping::send_echo(uint32_t ident,
                    uint32_t seq)
{
    // allocate memory for icmp packet
    char buffer[MTU];
    struct _ICMP_ECHO *icmp = (struct _ICMP_ECHO *)(buffer + sizeof(_ICMP_ECHO));
    struct _ICMPV6_PSEUDO *v6_pseudo = (struct _ICMPV6_PSEUDO *)(buffer);

    // fill header files
    if (addr_family_ == AF_INET)
    {
        ::memset(icmp, 0, sizeof(_ICMP_ECHO));
        icmp->type_ = 8;
        icmp->code = 0;
        icmp->ident_ = htonl(ident);
        icmp->seq_ = htonl(seq);
        // fill magic_ string
        strncpy(icmp->magic_, MAGIC, MAGIC_LEN);
        // fill sending timestamp
        icmp->sending_ts_ = htonll(zce::clock_ms());
        // calculate and fill checksum
        icmp->check_sum_ = htons(
            calculate_checksum((char*)icmp, sizeof(_ICMP_ECHO)));
    }
    else if (addr_family_ == AF_INET6)
    {
        ::memset(v6_pseudo, 0, sizeof(_ICMP_ECHO) + sizeof(_ICMPV6_PSEUDO));
        memcpy(&v6_pseudo->src_addr_, &local_addr_.in6_.sin6_addr, 16);
        memcpy(&v6_pseudo->dst_addr_, &ping_addr_.in6_.sin6_addr, 16);
        v6_pseudo->upper_layer_len_ = htonl(sizeof(_ICMP_ECHO));
        v6_pseudo->upper_proto_ = IPPROTO_ICMPV6;

        icmp->type_ = 128;
        icmp->code = 0;
        icmp->ident_ = htonl(ident);
        icmp->seq_ = htonl(seq);
        // fill magic_ string
        strncpy(icmp->magic_, MAGIC, MAGIC_LEN);
        // fill sending timestamp
        icmp->sending_ts_ = htonll(zce::clock_ms());

        icmp->check_sum_ = htons(
            calculate_checksum((char *)v6_pseudo, sizeof(_ICMPV6_PSEUDO) + sizeof(_ICMP_ECHO)));
    }
    else
    {
        assert(false);
    }

    // send it
    ssize_t bytes = zce::sendto(ping_socket_,
                                icmp,
                                sizeof(_ICMP_ECHO),
                                0,
                                (struct sockaddr*)&ping_addr_,
                                addr_len_);
    if (bytes < 0)
    {
        return -1;
    }
    return 0;
}

int Ping::recv_echo(uint32_t *ident,
                    uint32_t *seq,
                    uint64_t *take_msec,
                    uint8_t *ttl,
                    zce::Time_Value* timeout_tv)
{
    // allocate buffer
    char buffer[MTU];
    // receive another packet
    zce::sockaddr_any recv_addr;
    ssize_t bytes = zce::recvfrom(ping_socket_,
                                  buffer,
                                  sizeof(buffer),
                                  (struct sockaddr*)&recv_addr,
                                  &addr_len_,
                                  timeout_tv);
    if (bytes < 0)
    {
        // normal return when timeout
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return -1;
        }
        return -1;
    }

    // find icmp packet in ip packet
    struct _ICMP_ECHO* icmp = nullptr;
    if (addr_family_ == AF_INET)
    {
        icmp = (struct _ICMP_ECHO*)(buffer + 20);
        *ttl = buffer[8];
        // check type
        if (icmp->type_ != 0 || icmp->code != 0)
        {
            return -1;
        }
    }
    else if (addr_family_ == AF_INET6)
    {
        //这儿有个缺陷，无法收取IPV6头部，所以TTL获取不了，Linux下可以用recvmsg改进。
        //windows可能就要用IPV6_HDRINCL ，但这个需要admin的权限才能跑
        //对我来说没有必要，放弃
        icmp = (struct _ICMP_ECHO*)(buffer);
        *ttl = 0;
        if (icmp->type_ != 129 || icmp->code != 0)
        {
            return -1;
        }
    }
    else
    {
        assert(false);
    }

    *ident = ntohl(icmp->ident_);
    *seq = ntohl(icmp->seq_);
    uint64_t sending_ts = ntohll(icmp->sending_ts_);
    *take_msec = zce::clock_ms() - sending_ts;
    return 0;
}

int Ping::ping(size_t test_num)
{
    int ret = 0;
    uint32_t send_ident = getpid(), recv_ident = 0;
    uint32_t send_seq = 1, recv_seq = 0;

    for (size_t i = 0; i < test_num; ++i)
    {
        // send it
        ret = send_echo(send_ident, send_seq);
        if (ret == -1)
        {
            fprintf(stderr, "%04u.Send request fail.\n", send_seq);
        }
        do
        {
            // try to receive and print reply
            uint64_t ms = 0;
            uint8_t ttl = 0;
            zce::Time_Value timeout_tv(1, 0);
            ret = recv_echo(&recv_ident,
                            &recv_seq,
                            &ms,
                            &ttl,
                            &timeout_tv);
            if (ret == -1)
            {
                if (zce::last_error() == ETIMEDOUT)
                {
                    fprintf(stderr, "%04u.Receive timeout.\n", send_seq);
                }
                else
                {
                    fprintf(stderr, "%04u.Receive failed.\n", send_seq);
                }
                break;
            }
            //收到一个正确的书籍，但序列号对不上，可能是超时过来的
            if (ret == 0 && (recv_ident != send_ident || recv_seq != send_seq))
            {
                fprintf(stderr, "%04u.Receive failed.\n", recv_seq);
                continue;
            }
            char addr_buf[64];
            fprintf(stdout,
                    "%04u. %s Rely size=%zu,time=%llums,TTL=%u.\n",
                    send_seq,
                    zce::sockaddr_ntop((sockaddr *)&ping_addr_, addr_buf, 63),
                    sizeof(_ICMP_ECHO),
                    ms,
                    ttl);
        } while (false);
        // increase sequence number
        send_seq += 1;
    }

    return 0;
}
}