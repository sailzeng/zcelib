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
* @note       Socks5的文档，
*             https://www.rfc-editor.org/rfc/rfc1928
*             https://www.rfc-editor.org/rfc/rfc1929
*             中文翻译，能看看吧。
*             https://www.quarkay.com/code/383/socks5-protocol-rfc-chinese-traslation
*             danted 服务器的配置
*             https://www.keepnight.com/archives/657/
*             https://lixingcong.github.io/2018/05/25/dante-socks5/
*/

#pragma once

#include "zce/util/scope_guard.h"

//=================================================================================

//socks 5 代理部分
namespace zce::socks5
{
const char CMD_CONNECT = 0x1;
const char CMD_BIND = 0x2;              //不支持
const char CMD_UDP = 0x3;

class base
{
public:

    /*!
     * @brief 把host地址转换到buf中
     * @param host_name  域名
     * @param host_port  域名端口
     * @param host_addr  地址（包括地址和端口）
     * @param addr_len   地址长度，表面是sockaddr_in,sockaddr_in6
     * @param buf        buf
     * @param buf_len    buf的长度
     * @param use_len    buf的使用的长度
     * @return 0 成功，
    */
    static int hostaddr_to_buf(const char* host_name,
                               uint16_t host_port,
                               const sockaddr* host_addr,
                               socklen_t addr_len,
                               char *buf,
                               size_t buf_len,
                               size_t &use_len);

    //!把buf你们的信息填写到host中
    static int buf_to_hostaddr(const char *buf,
                               size_t buf_len,
                               size_t &use_len,
                               char* host_name,
                               uint16_t *host_port,
                               sockaddr* host_addr,
                               socklen_t addr_len);
};

//!socks的客户端
class client :public base
{
protected:
    //!
    client() = default;
    ~client() = default;
public:
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

protected:
    /*!
    * @brief      SOCKS5代理初始化，进行用户验证等
    * @return     int  返回0标识成功
    * @param      cmd        CMD_CONNECT or CMD_UDP
    * @param      host_name  目标（通过代理跳转）的域名，域名和地址只选一个，优先域名，为nullptr
    * @param      host_port  目标端口
    * @param      host_addr  目标的地址,和host_name互斥
    * @param      addr_len   目标地址的长度，
    * @param      bind_addr  socks5服务器提供的绑定地址，通过这个地址和目标同学
    * @param      timeout_tv 超时时间、
    * @note       对于UDP 穿透，模板地址其实是自己的地址0+端口
    *             对于TCP，Connect，之后使用bind_addr进行通信
    */
    int socks5_cmd(char cmd,
                   const char* host_name,
                   uint16_t host_port,
                   const sockaddr* host_addr,
                   socklen_t addr_len,
                   sockaddr* bind_addr,
                   zce::time_value& timeout_tv);

protected:
    //！
    static const size_t CMD_BUF_LEN = 1024;
protected:

    //连接SOCKS5服务器的Socket句柄,必须先连接 connect
    zce::auto_socket socks5_hdl_ = INVALID_SOCKET;
    //!socks5代理的地址
    zce::sockaddr_any socks5_addr_;
    //!命令传送的buf
    std::unique_ptr<char[]>cmd_buffer_{ new char[CMD_BUF_LEN] };
};

//=================================================================================
//
class tcp_connect :public client
{
public:
    //!
    tcp_connect() = default;
    ~tcp_connect() = default;

    /*!
     * @brief socks5代理，TCP Connected
     * @param timeout_tv
     * @return
     * @note      在initialize后调用
     *            https://codeantenna.com/a/jTZOi7GrY2
    */
    int tcp_connect_cmd(const char* host_name,
                        uint16_t host_port,
                        const sockaddr* host_addr,
                        socklen_t addr_len,
                        zce::time_value& timeout_tv);

    //!取的TCP Connect的句柄
    ZCE_SOCKET get_handle();
    //将handle交换出去，自己不析构处理
    ZCE_SOCKET exchange();

protected:
    //是否成功链接
    bool connected_ = false;
    //! socks5 代理服务链接目标服务器后绑定的IP地址，对TCP其实没啥用。
    zce::sockaddr_any bind_addr_;
};

//=================================================================================
//!UDP 穿透，Socks5
class udp_associate :public client
{
public:
    //!
    udp_associate() = default;
    ~udp_associate() = default;

    /*!
     * @brief socks5代理，UDP穿透的初始化
     * @param timeout_tv
     * @return
     * @note      在initialize后调用
     *            https://codeantenna.com/a/jTZOi7GrY2
    */
    int udp_associate_cmd(zce::time_value& timeout_tv);

    /**
     * @brief Socks5，的UDP穿透的发送数据，会附带一个头部，还可能分包
     * @param dst_name   目的地域名
     * @param dst_port   域名对应的端口
     * @param dst_addr   目的地址
     * @param addr_len   目的地址长度
     * @param snd_buf    发送的buf
     * @param buf_len    发送的buf 长度
     * @param fragments  是否进行分片，因为穿透会有一些前缀，可能一个包无法包装，
     * @return
    */
    int send(const char* dst_name,
             uint16_t dst_port,
             const sockaddr* dst_addr,
             socklen_t addr_len,
             const char *snd_buf,
             size_t buf_len,
             bool fragments);

    //! Socks5，的UDP穿透的接收数据
    int recv(sockaddr* dst_addr,
             socklen_t addr_len,
             char *rcv_buf,
             size_t buf_len,
             zce::time_value& timeout_tv,
             bool fragments);

    //!取得UDP associate的句柄
    ZCE_SOCKET get_handle();
    //将handle交换出去，自己不析构处理
    ZCE_SOCKET exchange();
protected:

    //! UDP 包的最大长度
    static const size_t UDP_MAX_DGRAM = 64 * 1024;
    //! 发送接受的BUFFER
    static const size_t SR_BUF_LEN = 64 * 1024;
    //! 一个包的容量
    static const size_t ONE_DGRAM_CAPACITY = SR_BUF_LEN - 262;

protected:
    //! 穿透的UDP句柄
    zce::auto_socket associate_hdl_ = INVALID_SOCKET;
    //! socks5 代理服务器返回的绑定地址，穿透请求通过这个端口服务
    zce::sockaddr_any bind_addr_;

    //是否一句建立了穿透
    bool associate_ = false;

    //! 发送接收的数据缓存
    std::unique_ptr<char[]> snd_rcv_buf_{ new char[SR_BUF_LEN] };
};
}
