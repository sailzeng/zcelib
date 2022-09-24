/*!
* @copyright  2004-2014  Apache License, Version 2.0 FULLSAIL
* @filename   zce/socket/connector.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年6月19日
* @brief      connect的工厂
*
*
* @details
*
*
*
* @note
*
*/
#ifndef ZCE_LIB_SOCKET_CONNECTOR_H_
#define ZCE_LIB_SOCKET_CONNECTOR_H_

#include "zce/time/time_value.h"
#include "zce/socket/addr_base.h"

namespace zce
{
/*!
* @brief      connect的工厂，成功连接后产生,用于产生zce::Socket_Stream
*
* @note
*/
class Socket_Connector
{
public:
    //构造函数和析构函数
    Socket_Connector(void);
    ~Socket_Connector(void);

    /*!
    * @brief      带超时的连接
    * @return     int
    * @param[out] new_stream  返回的连接的SOCKET 句柄类
    * @param      remote_addr 远端的地址，实际结构是sockadd_in，sockadd_in6等
    * @param      timeout     超时的时间长度
    * @param      reuse_addr  是否置为reuse_addr选项
    * @param      protocol    协议，用于RAW Socket，基本没用
    * @param      local_addr  本地的BIND地址信息，CONNECT基本没用
    */
    int connect(zce::Socket_Stream& new_stream,
                const zce::Sockaddr_Base* remote_addr,
                zce::time_value& timeout,
                bool reuse_addr = false,
                int protocol = 0,
                const Sockaddr_Base* local_addr = NULL);

    /*!
    * @brief      阻塞或者非阻塞的连接
    * @return     int
    * @param      new_stream  返回的连接的SOCKET 句柄类
    * @param      remote_addr 远端的地址，实际结构是sockadd_in，sockadd_in6等
    * @param      non_blocing 是否阻塞进行连接。
    * @param      reuse_addr  是否置为reuse_addr选项
    * @param      protocol    协议，用于RAW Socket，基本没用
    * @param      local_addr  本地的BIND地址信息，CONNECT基本没用
    */
    int connect(zce::Socket_Stream& new_stream,
                const zce::Sockaddr_Base* remote_addr,
                bool k,
                bool reuse_addr = false,
                int protocol = 0,
                const Sockaddr_Base* local_addr = NULL);
};
}

#endif //#ifndef ZCE_LIB_SOCKET_CONNECTOR_H_
