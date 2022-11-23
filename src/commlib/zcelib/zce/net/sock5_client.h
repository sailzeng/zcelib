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
//-------------------------------------------------------------------------------------
//socks 5 代理部分

namespace zce::socks5
{
/*!
* @brief      SOCKS5代理初始化，进行用户验证等
* @return     int 返回0标识成功
* @param      socks5_hdl  已经连接SOCKS5服务器的Socket句柄，必须先连接 connect
* @param      username    验证模式下的用户名称，如果不需要验证用填写nullptr
* @param      password    验证模式下的密码，如果不需要验证用填写nullptr
* @param      timeout_tv  超时时间
* @note       handle 必须先连接
*/
int initialize(ZCE_SOCKET socks5_hdl,
               const char* username,
               const char* password,
               zce::time_value& timeout_tv);

/*!
* @brief      SOCKS5代理初始化，进行用户验证等
* @return     int  返回0标识成功
* @param      socks5_hdl 已经连接SOCKS5服务器的Socket句柄，必须先连接 connect
* @param      host_name  跳转的域名，域名和地址只选一个，优先域名，为nullptr
* @param      port       跳转的端口
* @param      addr       跳转的地址
* @param      addr_len   跳转的地址的长度
* @param      timeout_tv 超时时间
*/
int tcp_connect(ZCE_SOCKET socks5_hdl,
                const char* host_name,
                const sockaddr* host_addr,
                int addr_len,
                uint16_t host_port,
                zce::time_value& timeout_tv);

/*!
 * @brief socks5代理，UDP穿透的初始化
 * @param socks5_hdl   已经连接SOCKS5服务器的Socket句柄，必须先连接 connect
 * @param local_addr   客户端发送UDP数据的本机地址端口(绑定)
 * @param addr_len     地址的长度
 * @param bind_addr    socks5代理返回的绑定地址，穿透UDP就向这个地址端口发送
 * @param timeout_tv
 * @return
 * @note  https://codeantenna.com/a/jTZOi7GrY2
*/
int udp_associate_init(ZCE_SOCKET socks5_hdl,
                       const sockaddr* local_addr,
                       int addr_len,
                       sockaddr* bind_addr,
                       zce::time_value& timeout_tv);

//
int udp_associate_send(ZCE_SOCKET udp,
                       sockaddr* bind_addr,
                       const char* host_name,
                       const sockaddr* host_addr,
                       const char *snd_buf,
                       size_t buf_len);

int udp_associate_recv(ZCE_SOCKET udp,
                       sockaddr* bind_addr,
                       const char* host_name,
                       const sockaddr* host_addr,
                       char *rcv_buf,
                       size_t buf_len);
}
