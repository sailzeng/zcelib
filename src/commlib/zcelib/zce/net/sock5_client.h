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

namespace zce
{
/*!
* @brief      SOCKS5代理初始化，进行用户验证等
* @return     int 返回0标识成功
* @param      handle      已经连接SOCKS5服务器的句柄，必须先连接 connect,可以使用connect_timeout函数
* @param      username    验证模式下的用户名称，如果不需要验证用填写NULL
* @param      password    验证模式下的密码，如果不需要验证用填写NULL
* @param      timeout_tv  超时时间
* @note       handle 必须先连接
*/
int socks5_initialize(ZCE_SOCKET handle,
                      const char* username,
                      const char* password,
                      zce::Time_Value& timeout_tv);

/*!
* @brief      SOCKS5代理初始化，进行用户验证等
* @return     int  返回0标识成功
* @param      handle     已经连接SOCKS5服务器的句柄，必须先连接 connect
* @param      host_name  跳转的域名，域名和地址只选一个，优先域名，为NULL
* @param      port       跳转的端口
* @param      addr       跳转的地址
* @param      addr_len   跳转的地址的长度
* @param      timeout_tv 超时时间
*/
int socks5_connect_host(ZCE_SOCKET handle,
                        const char* host_name,
                        const sockaddr* host_addr,
                        int addr_len,
                        uint16_t host_port,
                        zce::Time_Value& timeout_tv);

//socks5代理，UDP穿透
int socks5_udp_associate(ZCE_SOCKET handle,
                         const sockaddr* bind_addr,
                         int addr_len,
                         sockaddr* udp_addr,
                         zce::Time_Value& timeout_tv);
}
