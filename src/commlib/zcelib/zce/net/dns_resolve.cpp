#include "zce/predefine.h"

#include "zce/logger/logging.h"
#include "zce/net/dns_resolve.h"

#pragma pack (1)

struct _DNS_FLAG
{
public:

#if ZCE_BYTES_ORDER == ZCE_LITTLE_ENDIAN

    //返回码字段，表示响应的差错状态。当值为 0 时，表示查询成功;
    //当值为 1 时，表示报文格式错误(Format error)，服务器不能理解请求的报文;
    //当值为 2 时，表示域名服务器失败(Server failure)，因为服务器的原因导致没办法处理这个请求;
    //当值为 3 时，表示名字错误(Name Error)，只有对授权域名解析服务器有意义，指出解析的域名不存在;
    //当值为 4 时，表示查询类型不支持(Not Implemented)，即域名服务器不支持查询类型;
    //当值为 5 时，表示拒绝(Refused)，一般是服务器由于设置的策略拒绝给出应答，如服务器不希望对某些请求者给出应答
    uint16_t     rcode_ : 4;
    //个字段表示是否禁用安全检查。
    uint16_t     cd_ : 1;
    //这个字段表示信息是否是已授权。
    uint16_t     ad_ : 1;
    //保留字段，在所有的请求和应答报文中，它的值必须为 0。
    uint16_t     zero_ : 1;
    //可用递归字段，这个字段只出现在响应报文中。当值为 1 时，表示服务器支持递归查询
    uint16_t     ra_ : 1;
    //这个字段是期望递归字段，该字段在查询中设置，并在响应中返回。该标志告诉名称服务器必须处理这个查询，这种方式被
    //称为一个递归查询。如果该位为 0，且被请求的名称服务器没有一个授权回答，它将返回一个能解答该查询的其他名称服务
    //器列表。这种方式被称为迭代查询。
    uint16_t     rd_ : 1;
    //截断标志位，值为 1 时，表示响应已超过 512 字节并且已经被截断，只返回前 512 个字节
    uint16_t     tc_ : 1;
    //AA 代表授权应答，这个 AA 只在响应报文中有效，值为 1 时，表示名称服务器是权威服务器;值为 0 时，
    //表示不是权威服务器
    uint16_t     aa_ : 1;
    //0标准查询，1 表示反向查询，2 表示服务器状态请求
    uint16_t     opcode_ : 4;
    //QR 标识报文是查询报文还是响应报文，查询报文时 QR = 0，响应报文时 QR = 1
    uint16_t     qr_ : 1;
#else
    uint16_t     qr_ : 1;
    uint16_t     opcode_ : 4;
    uint16_t     aa_ : 1;
    uint16_t     tc_ : 1;
    uint16_t     rd_ : 1;
    uint16_t     ra_ : 1;
    uint16_t     zero_ : 1;
    uint16_t     ad_ : 1;
    uint16_t     cd_ : 1;
    uint16_t     rcode_ : 4;
#endif
};

//DNS协议的头部信息
struct _DNS_Head
{
    //ID，查询标识
    uint16_t  tid_;
    //FLAG，各种标志位
    union
    {
        uint16_t  u16_flags_;
        _DNS_FLAG flags_;
    };
    //!问题的数量
    uint16_t  qd_count_;
    //!回复答案的数量
    uint16_t  an_count_;
    //!返回的权威名称服务器数量
    uint16_t  ns_count_;
    //!额外服务器的数量
    uint16_t  ar_count_;
};

#pragma pack ()

namespace zce
{
//================================================================================================

DNS_Resolve::DNS_Resolve()
{
    send_packet_ = new char[DNS_PACKET_MAX_LEN];
    recv_packet_ = new char[DNS_PACKET_MAX_LEN];
}
DNS_Resolve::~DNS_Resolve()
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

int DNS_Resolve::initialize(sockaddr *dns_svr_addr,
                            socklen_t addr_len)
{
    int ret = 0;
    dns_server_addr_.set_sockaddr(dns_svr_addr, addr_len);
    ret = dns_socket_.open(dns_server_addr_.get_family());
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

int DNS_Resolve::initialize(int dns_svr_family,
                            const char *dns_svr,
                            uint16_t dns_svr_port)
{
    int ret = 0;
    ret = dns_server_addr_.getaddrinfo(dns_svr_family,
                                       dns_svr,
                                       dns_svr_port);
    if (ret != 0)
    {
        return ret;
    }
    ret = dns_socket_.open(dns_svr_family);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

ZCE_SOCKET DNS_Resolve::get_handle()
{
    return dns_socket_.get_handle();
}

int DNS_Resolve::query(const char *query_name,
                       uint16_t query_type,
                       uint16_t *tid)
{
    size_t len = DNS_PACKET_MAX_LEN;
    int ret = DNS_Resolve::pack_request(send_packet_,
                                        &len,
                                        query_name,
                                        query_type,
                                        tid);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "pack_request error ,error code = %d", ret);
        return ret;
    }
    ssize_t snd_len = dns_socket_.sendto(send_packet_,
                                         len,
                                         0,
                                         &dns_server_addr_);
    if ((size_t)snd_len != len)
    {
        return -1;
    }
    return 0;
}

int DNS_Resolve::answer(uint16_t *tid,
                        int family,
                        struct sockaddr addrs[],
                        size_t *addrs_num,
                        zce::time_value* timeout_tv)
{
    size_t len = DNS_PACKET_MAX_LEN;
    Sockaddr_Any recv_addr;
    ssize_t rcv_len = 0;
    int ret = 0;
    if (timeout_tv)
    {
        rcv_len = dns_socket_.recvfrom_timeout(recv_packet_,
                                               len,
                                               &recv_addr,
                                               *timeout_tv);
    }
    else
    {
        rcv_len = dns_socket_.recvfrom(recv_packet_,
                                       len,
                                       0,
                                       &recv_addr);
    }

    if (rcv_len <= 0)
    {
        return -1;
    }
    if (recv_addr != dns_server_addr_)
    {
        return -1;
    }
    ret = parse_response(recv_packet_,
                         rcv_len,
                         tid,
                         family,
                         addrs,
                         addrs_num);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

int DNS_Resolve::pack_request(char *buf,
                              size_t *size,
                              const char *query_name,
                              uint16_t query_type,
                              uint16_t *tid)
{
    static uint16_t TID = 0x1212;

    size_t max_size = *size;
    size_t name_len = ::strlen(query_name);
    //名字长度必须小于255
    if (name_len > DNS_Resolve::MAX_NAME_LEN)
    {
        return -1;
    }
    // + 2 因为有多一个长度表示字节(其他长度字节数量比.多1)，同时会有一个'\0',
    // + 4 因为某个查询问题后面都有2个字节的查询类型和2个字节的查询类
    if (max_size < sizeof(_DNS_Head) + (name_len + 2 + 4))
    {
        return -1;
    }

    size_t pos = 0;
    _DNS_Head *header = (_DNS_Head *)buf;
    memset(header, 0, sizeof(_DNS_Head));
    header->tid_ = htons(++TID);
    *tid = TID;
    header->flags_.opcode_ = 0;
    //要求DNS服务器进行递归查询
    header->flags_.rd_ = 1;
    header->u16_flags_ = htons(header->u16_flags_);
    //一般来说。一次只能干一件事，见Stack Overflow
    header->qd_count_ = htons(0x1);
    pos += sizeof(_DNS_Head);

    const char *pname = query_name;
    const char *ppos = nullptr;
    size_t len = 0;
    for (;;)
    {
        ppos = strchr(pname, '.');
        len = ppos ? ppos - pname : strlen(pname);
        //长度错误，写不下，每个段不能大于64
        if (len > MAX_LABEL_LEN)
        {
            return -1;
        }
        buf[pos++] = (uint8_t)len;
        memcpy(buf + pos, pname, len);
        pos += len;
        if (ppos)
        {
            pname = ppos + 1;
        }
        else
        {
            break;
        }
    }
    if (max_size < pos + 1)
    {
        return -1;
    }
    buf[pos++] = '\0';

    // A or AAAA 等
    uint16_t qtype = ::htons(query_type);
    if (max_size < pos + 2)
    {
        return -1;
    }
    memcpy(buf + pos, &qtype, 2);
    pos += 2;
    //查询类 IN
    uint16_t qclass = ::htons(QCLASS_IN);
    if (max_size < pos + 2)
    {
        return -1;
    }
    memcpy(buf + pos, &qclass, 2);
    pos += 2;

    *size = pos;
    return 0;
}

int DNS_Resolve::parse_response(char *buf,
                                size_t size,
                                uint16_t *tid,
                                int family,
                                struct sockaddr addrs[],
                                size_t *addrs_num)
{
    if (size < sizeof(_DNS_Head) || *addrs_num <= 0)
    {
        return -1;
    }
    size_t pos = 0;
    struct _DNS_Head *header = (struct _DNS_Head *)(buf + pos);
    pos += sizeof(_DNS_Head);
    header->u16_flags_ = ntohs(header->u16_flags_);
    //检查 rcode 是否正确
    if (header->flags_.rcode_ != 0)
    {
        return header->flags_.rcode_;
    }
    *tid = ntohs(header->tid_);
    //处理 query 节
    uint16_t qdcount = ntohs(header->qd_count_);
    if (qdcount != 1)
    {
        return -1;
    }
    size_t len = 0;
    //处理query节的名字部分
    while (buf[pos] != '\0')
    {
        if (size < pos + 1)
        {
            return -1;
        }

        len = *((uint8_t *)(buf + pos));
        if ((len & 0xC0) == 0xC0)
        {
            //要处理名字指针的情况，如果是高位是0xC0，表示一种指向跳跃，（省略空间写法）
            pos++;
            break;
        }
        else
        {
            pos++;
            if (size < pos + len)
            {
                return -1;
            }
            pos += len;
        }
    }
    if (size < pos + 1)
    {
        return -1;
    }
    ++pos;
    if (size < pos + 4)
    {
        return -1;
    }
    pos += 4;

    //处理 answer 节,
    uint16_t ancount = ntohs(header->an_count_);
    if (ancount < 1)
    {
        return -1;
    }
    //所有的RR(Resource Recode/资源记录)都有如下所示的相同的顶层格式
    size_t num = 0;
    for (size_t i = 0; i < ancount; ++i)
    {
        //处理域名部分
        while (buf[pos] != '\0')
        {
            if (size < pos + 1)
            {
                return -1;
            }
            len = *((uint8_t *)(buf + pos));
            //要处理名字指针的情况
            if ((len & 0xC0) == 0xC0)
            {
                pos++;
                break;
            }
            else
            {
                pos++;
                if (size < pos + len)
                {
                    return -1;
                }
                pos += len;
            }
        }
        //跳过 '/0'
        if (size < pos + (1))
        {
            return -1;
        }
        ++pos;
        // atype
        if (size < pos + (2))
        {
            return -1;
        }
        //检查查询的类型，根据类型判定写入的地址是否正确
        uint16_t atype = ntohs(*((uint16_t *)(buf + pos)));
        pos += 2;

        // skip class, ttl
        if (size < pos + (6))
        {
            return -1;
        }
        pos += 6;
        // rdlen
        if (size < pos + (2))
        {
            return -1;
        }
        uint16_t rdlen = ntohs(*((uint16_t *)(buf + pos)));
        pos += 2;
        // rddata
        if (size < pos + (rdlen))
        {
            return -1;
        }
        if (atype == QTYPE_A && family == AF_INET && num < *addrs_num)
        {
            sockaddr_in *addrs_in = (sockaddr_in *)addrs;
            memset(&addrs_in[num], 0, sizeof(sockaddr_in));
            addrs_in[num].sin_family = AF_INET;
            memcpy(&addrs_in[num].sin_addr, buf + pos, 4);
            num++;
        }
        else if (atype == QTYPE_AAAA && family == AF_INET6 && num < *addrs_num)
        {
            sockaddr_in6 *addrs_in6 = (sockaddr_in6 *)addrs;
            memset(&addrs_in6[num], 0, sizeof(sockaddr_in6));
            addrs_in6[num].sin6_family = AF_INET6;
            memcpy(&addrs_in6[num].sin6_addr, buf + pos, 16);
            num++;
        }
        else
        {
            //其他类型暂时不处理，有的类型SOA rddata内部还有数据，比较复杂
        }
        pos += rdlen;
    }

    *addrs_num = num;
    //后面还有NS权威服务器信息的节，暂时不处理，目测也是RR格式
    //后面还有NS附加信息的节，暂时不处理
    return 0;
}
}