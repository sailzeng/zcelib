/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   zce/net/dns_resolve.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       20221123
* @brief
* @details
*
*
*
* @note
*
*/

#pragma once
//=================================================================================
//socks 5 代理部分

namespace zce::socks5
{
class udp_associate;

class base
{
public:
    static int hostaddr_to_buf(const char* host_name,
                               uint16_t host_port,
                               const sockaddr* host_addr,
                               socklen_t addr_len,
                               char *buf,
                               size_t &buf_len);

    static int buf_to_hostaddr(const char *buf,
                               size_t buf_len,
                               char* host_name,
                               uint16_t *host_port,
                               sockaddr* host_addr,
                               socklen_t addr_len);
};

class client :public base
{
public:
    //!
    client() = default;
    ~client() = default;
    /*!
    * @brief      SOCKS5代理初始化，进行用户验证等
    * @return     int 返回0标识成功
    * @param      username    验证模式下的用户名称，如果不需要验证用填写nullptr
    * @param      password    验证模式下的密码，如果不需要验证用填写nullptr
    * @param      timeout_tv  超时时间
    * @note       先连接socks5_addr的服务器，然后认证初始化
    */
    int initialize(const sockaddr* socks5_addr,
                   socklen_t addr_len,
                   const char* username,
                   const char* password,
                   zce::time_value& timeout_tv);

    /*!
    * @brief      SOCKS5代理初始化，进行用户验证等
    * @return     int  返回0标识成功
    * @param      host_name  目标（通过代理跳转）的域名，域名和地址只选一个，优先域名，为nullptr
    * @param      host_port  目标端口
    * @param      host_addr  目标的地址,和host_name互斥
    * @param      addr_len   目标地址的长度，
    * @param      bind_addr  socks5服务器提供的绑定地址，通过这个地址和目标同学
    * @param      timeout_tv 超时时间
    */
    int socks5_cmd(char cmd,
                   const char* host_name,
                   uint16_t host_port,
                   const sockaddr* host_addr,
                   socklen_t addr_len,
                   sockaddr* bind_addr,
                   zce::time_value& timeout_tv);

    /*!
     * @brief socks5代理，UDP穿透的初始化
     * @param ua
     * @param timeout_tv
     * @return
     * @note  https://codeantenna.com/a/jTZOi7GrY2
    */
    int init_udp_associate(udp_associate &ua,
                           zce::time_value& timeout_tv);

protected:
    //！
    static const size_t CMD_BUF_LEN = 1024;
protected:

    //连接SOCKS5服务器的Socket句柄,必须先连接 connect
    ZCE_SOCKET socks5_hdl_ = INVALID_SOCKET;
    //
    std::unique_ptr<char[]>cmd_buffer_{ new char[CMD_BUF_LEN] };
};

//=================================================================================
//!UDP 穿透，Socks5
class udp_associate :public base
{
    friend class zce::socks5::client;
public:
    //!
    udp_associate() = default;
    ~udp_associate() = default;
    //!
    int open(int family);
    //!
    int send(const char* host_name,
             uint16_t host_port,
             const sockaddr* host_addr,
             socklen_t addr_len,
             const char *snd_buf,
             size_t buf_len);
    //!
    int recv(sockaddr* host_addr,
             char *rcv_buf,
             size_t buf_len);
protected:

    static const size_t UDP_MAX_DGRAM = 64 * 1024;

    static const size_t SR_BUF_LEN = 64 * 1024;

protected:
    //!
    ZCE_SOCKET associate_hdl_ = INVALID_SOCKET;
    //!
    int family_ = 0;
    //
    zce::sockaddr_any local_addr_;
    //
    zce::sockaddr_any bind_addr_;
    //
    std::unique_ptr<char[]> snd_rcv_buf_{ new char[SR_BUF_LEN] };
};
}
