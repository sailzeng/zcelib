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
//! DNS解析
class dns_resolve
{
public:

    dns_resolve();
    ~dns_resolve();

    /**
     * @brief 初始化DNS服务器的地址信息
     * @param dns_svr_addr DNS服务器的socket地址
     * @param addr_len     DNS服务器的socket地址的长度,可以是sockadd_in or sockadd_in6的长度
     * @return
    */
    int initialize(sockaddr *dns_svr_addr,
                   socklen_t addr_len);

    /**
     * @brief 初始化DNS服务器的地址信息
     * @param dns_svr_family DNS服务器的family,是AF_INET or AF_INET6
     * @param dns_svr_ip 服务器的地址描述信息,可以是.,也可以是域名
     * @param dns_svr_port DNS服务器的端口号
     * @return
    */
    int initialize(int dns_svr_family,
                   const char *dns_svr_ip,
                   uint16_t dns_svr_port);

    ZCE_SOCKET get_handle();

    //发起域名查询
    int query(const char *query_name,
              uint16_t query_type,
              uint16_t *tid);

    /**
     * @brief 处理DNS的应答消息
     * @param tid 返回的TID，可以检查是否相等，也可以用于内部事务管理
     * @param family 期待的地址的family
     * @param addrs  输出参数，地址数值，
     * @param addrs_num  输入输出参数，地址数组的数量，
     * @param timeout_tv 超时时间，如果不需要超时等待，传递null，
     * @return
    */
    int answer(uint16_t *tid,
               int family,
               struct sockaddr *addrs,
               size_t *addrs_num,
               zce::time_value* timeout_tv);

protected:

    /*!
    * @brief      对请求进行打包处理
    * @return     int  返回0表示成果
    * @param      buf  BUFFER
    * @param      size
    * @param      query_name
    * @param      query_type
    * @param      tid
    * @note
    */
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
    static constexpr size_t DNS_PACKET_MAX_LEN = 512;
    //DNS 头部长度
    static constexpr size_t DNS_HEADER_LEN = 12;
    //每个标签最大的长度
    static constexpr size_t MAX_LABEL_LEN = 63;
    //查询名称的最大长度
    static constexpr size_t MAX_NAME_LEN = 255;

protected:
    //!发送缓冲区
    char               *send_packet_ = nullptr;
    //!接收缓冲区
    char               *recv_packet_ = nullptr;

    //!DNS 服务器地址
    zce::skt::addr_any  dns_server_addr_;

    //!UDP socket
    zce::skt::datagram  dns_socket_;
};
}
