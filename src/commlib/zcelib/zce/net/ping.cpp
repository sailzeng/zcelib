#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/socket.h"
#include "zce/net/ping.h"

//==============================================================================================
//https://www.jianshu.com/p/8e9903c7f9e8
//https://network.fasionchan.com/zh_CN/latest/practices/ping-by-icmp-c.html

namespace zce
{
#define MAGIC "12345678901"
#define MAGIC_LEN 12
#define MTU 1500
#define RECV_TIMEOUT_USEC 100000

#pragma pack (1)

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

Ping::Ping()
{
    send_packet_ = new char[PING_PACKET_MAX_LEN];
    recv_packet_ = new char[PING_PACKET_MAX_LEN];
}
Ping::~Ping()
{
    if (send_packet_)
    {
        delete send_packet_;
        send_packet_ = nullptr;
    }
    if (recv_packet_)
    {
        delete recv_packet_;
        recv_packet_ = nullptr;
    }
}
int Ping::initialize(::sockaddr *ping_addr,
                     socklen_t addr_len)
{
    addr_len_ = addr_len;
    if (addr_len == sizeof(::sockaddr_in))
    {
        addr_family_ = AF_INET;
    }
    else if (addr_len == sizeof(::sockaddr_in6))
    {
        addr_family_ = AF_INET6;
    }
    else
    {
        assert(false);
    }
    ping_addr_.set(ping_addr, addr_family_);
    ping_socket_ = zce::socket(addr_family_,
                               SOCK_RAW,
                               IPPROTO_ICMP);
    if (ping_socket_ != ZCE_INVALID_SOCKET)
    {
        return -1;
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
    }
    else if (addr_family_ == AF_INET6)
    {
        addr_len_ = sizeof(sockaddr_in6);
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
    ping_socket_ = zce::socket(addr_family_,
                               SOCK_RAW,
                               IPPROTO_ICMP);
    if (ping_socket_ != ZCE_INVALID_SOCKET)
    {
        return -1;
    }
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
    uint32_t carray = checksum >> 16;
    while (carray)
    {
        checksum = (checksum & 0xffff) + carray;
        carray = checksum >> 16;
    }

    // negate it
    checksum = ~checksum;

    return checksum & 0xffff;
}

int Ping::send_echo(uint32_t ident,
                    uint32_t seq)
{
    // allocate memory for icmp packet
    struct _ICMP_ECHO icmp;
    ::memset(&icmp, 0, sizeof(icmp));

    // fill header files
    if (addr_family_ == AF_INET)
    {
        icmp.type_ = 8;
        icmp.code = 0;
    }
    else if (addr_family_ == AF_INET6)
    {
        icmp.type_ = 128;
        icmp.code = 0;
    }
    else
    {
        assert(false);
    }

    icmp.ident_ = htonl(ident);
    icmp.seq_ = htonl(seq);
    // fill magic_ string
    strncpy(icmp.magic_, MAGIC, MAGIC_LEN);

    // fill sending timestamp
    icmp.sending_ts_ = htonll(zce::clock_ms());

    // calculate and fill checksum
    icmp.check_sum_ = htons(
        calculate_checksum((char*)&icmp, sizeof(icmp))
    );

    // send it
    ssize_t bytes = zce::sendto(ping_socket_,
                                &icmp,
                                sizeof(icmp),
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
    ssize_t bytes = zce::recvfrom(ping_socket_,
                                  buffer,
                                  sizeof(buffer),
                                  (struct sockaddr*)&ping_addr_,
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
        if (icmp->type_ != 8 || icmp->code != 0)
        {
            return -1;
        }
    }
    else if (addr_family_ == AF_INET6)
    {
        icmp = (struct _ICMP_ECHO*)(buffer + 40);
        *ttl = buffer[7];
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
        // increase sequence number
        send_seq += 1;

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
            if (ret == 0 && (recv_ident != send_ident || recv_seq != send_seq))
            {
                fprintf(stderr, "%04u.Receive failed.\n", recv_seq);
                continue;
            }
            fprintf(stdout,
                    "%04u. rely size=%llu,time=%llums,TTL=%u.\n",
                    send_seq,
                    sizeof(_ICMP_ECHO),
                    ms,
                    ttl);
        } while (false);
    }

    return 0;
}
}