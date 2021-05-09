#pragma once

#include "zerg/ip_restrict.h"

/*!
* @brief      TCP Accept 处理的EventHandler,
*
* @note
*/
class TCP_Accept_Handler: public zce::Event_Handler
{
public:

    /*!
    * @brief      构造函数
    * @param      svcid  相应的svc id
    * @param      addr   相应的IP地址
    */
    TCP_Accept_Handler(const soar::SERVICES_ID& svcid,
                       const zce::Sockaddr_In& addr);

protected:
    ///监听端口
    ~TCP_Accept_Handler();

public:

    ///创建监听的端口
    int create_listen();

    /*!
    * @brief      事件触发的毁掉处理，表示有一个accept 的数据
    * @return     int
    */
    virtual int handle_input();

    ///关闭处理
    virtual int handle_close();

    ///得到Acceptor的句柄
    virtual ZCE_HANDLE get_handle(void) const;

protected:

    ///ACCPET PEER
    zce::Socket_Acceptor      peer_acceptor_;

    ///自己的 SVC INFO
    soar::SERVICES_ID         my_svc_info_;

    ///邦定的地址
    zce::Sockaddr_In          accept_bind_addr_;

    ///IP限制管理器
    zerg::IPRestrict_Mgr* ip_restrict_;
};
