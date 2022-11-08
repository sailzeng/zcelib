#pragma once

#include "zerg/ip_restrict.h"

namespace zerg
{
/*!
* @brief      TCP Accept 处理的EventHandler,
*
* @note
*/
class svc_accept : public zce::event_handler
{
public:

    /*!
    * @brief      构造函数
    * @param      svcid  相应的svc id
    * @param      addr   相应的IP地址
    */
    svc_accept(const soar::SERVICES_ID& svcid,
               const zce::skt::addr_in& addr);

protected:
    ///监听端口
    ~svc_accept();

public:

    ///创建监听的端口
    int create_listen();

    /*!
    * @brief      事件触发的毁掉处理，表示有一个accept 的数据
    * @return     int
    */
    void accept_event() override;

    ///关闭处理
    void close_handle() override;

    ///得到Acceptor的句柄
    ZCE_HANDLE get_handle(void) const override;

protected:

    ///ACCPET PEER
    zce::skt::acceptor      peer_acceptor_;

    ///自己的 SVC INFO
    soar::SERVICES_ID       my_svc_info_;

    ///邦定的地址
    zce::skt::addr_in       accept_bind_addr_;

    ///IP限制管理器
    zerg::IPRestrict_Mgr*   ip_restrict_;
};
}
