/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   zce/net/dns_resolve.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       nim1z
* @brief
* @details
*
*
*
* @note
*
*/

#pragma once

#include "zce/os_adapt/socket.h"
#include "zce/socket/addr_any.h"
#include "zce/socket/datagram.h"

namespace zce
{
class DNS_Resolve
{
public:

    DNS_Resolve();
    ~DNS_Resolve();

    int initialize(sockaddr *dns_svr_addr,
                   socklen_t addr_len);

    int initialize(int dns_svr_family,
                   const char *dns_svr_ip,
                   uint16_t dns_svr_port);

    int add_dns_server(int dns_svr_family,
                       const char *dns_svr_ip,
                       uint16_t dns_svr_port);

    ZCE_SOCKET get_handle();

    //发起域名查询
    int query(const char *query_name,
              uint16_t query_type,
              uint16_t *tid);

    //
    int answer(uint16_t *tid,
               int family,
               struct sockaddr addrs[],
               size_t *addrs_num,
               zce::Time_Value* timeout_tv);

protected:

    static int pack_request(char *buf,
                            size_t *size,
                            const char *query_name,
                            uint16_t query_type,
                            uint16_t *tid);

    static int parse_response(char *buf,
                              size_t size,
                              uint16_t *tid,
                              int family,
                              struct sockaddr addrs[],
                              size_t *addrs_num);

public:

    //查询类型，目前只支持这两种A 和AAAA
    static constexpr uint16_t QTYPE_A = 1;
    static constexpr uint16_t QTYPE_AAAA = 28;
    //查询类，一般都是1
    static constexpr uint16_t QCLASS_IN = 1;

protected:
    //DNS 包的最大长度
    static constexpr size_t MAX_PACKET_LEN = 512;
    //DNS 头部长度
    static constexpr size_t DNS_HEADER_LEN = 12;
    //每个标签最大的长度
    static constexpr size_t MAX_LABEL_LEN = 63;
    //查询名称的最大长度
    static constexpr size_t MAX_NAME_LEN = 255;

protected:
    //!发送缓冲区
    char              *send_packet_ = nullptr;
    //!接收缓冲区
    char              *recv_packet_ = nullptr;

    //!DNS 服务器地址
    zce::Sockaddr_Any   dns_server_addr_;

    //!UDP socket
    zce::Socket_DataGram    dns_socket_;
};
}
