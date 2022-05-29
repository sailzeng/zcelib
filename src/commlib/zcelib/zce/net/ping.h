/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   zce/net/ping.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date
* @brief      完成IMCP里面的ping功能
* @details
*
*
*
* @note
*
*/

#pragma once

#include "zce/os_adapt/socket.h"
#include "zce/time/progress_timer.h"

//-------------------------------------------------------------------------------------
//socks 5 代理部分

namespace zce
{
class Ping
{
public:

    Ping() = default;
    ~Ping() = default;

    int initialize(::sockaddr *ping_addr,
                   socklen_t addr_len);

    int initialize(int svr_family,
                   const char *ping_svr);

    ZCE_SOCKET get_handle();

    int send_echo(uint32_t ident, uint32_t seq);

    int recv_echo(uint32_t *ident,
                  uint32_t *seq,
                  uint64_t *take_msec,
                  uint8_t *ttl,
                  zce::Time_Value* timeout_tv);

    int ping(size_t test_num);

    //计算check sum
    static uint16_t calculate_checksum(char* buffer, size_t bytes);

protected:
    //内部初始化
    int initialize();

protected:
    //DNS 包的最大长度
    static constexpr size_t PING_PACKET_MAX_LEN = 256;
    //DNS 头部长度
    static constexpr size_t DNS_HEADER_LEN = 12;

protected:

    //! ping的地址的family
    int  addr_family_ = 0;

    //!ping 服务器地址
    zce::sockaddr_any   ping_addr_;
    //!本地地址
    zce::sockaddr_any local_addr_;

    socklen_t addr_len_ = 0;

    //!UDP socket
    ZCE_SOCKET    ping_socket_ = ZCE_INVALID_SOCKET;
};
}
