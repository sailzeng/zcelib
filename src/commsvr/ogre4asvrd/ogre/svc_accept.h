#pragma once

#include "ogre/configure.h"

namespace ogre
{
class ip_restrict;

/*!
* @brief      TCP Accept 处理的EventHandler
*
* @note
*/
class svc_accept : public zce::event_handler
{
public:

    //构造函数
    svc_accept(const TCP_PEER_CONFIG_INFO& config_info,
               zce::reactor* reactor = zce::reactor::instance());
protected:
    ~svc_accept();
public:

    //创建监听的端口
    int create_listenpeer();

    //处理有端口
    virtual int accept_event(ZCE_HANDLE);

    //关闭处理
    virtual void close_event();

    //得到Acceptor的句柄
    ZCE_HANDLE get_handle(void) const;

protected:

    //ACCPET PEER
    zce::skt::acceptor    peer_acceptor_;

    TCP_PEER_MODULE_INFO   peer_module_info_;

    //IP限制管理器
    ip_restrict* ip_restrict_;
};
}//namespace ogre
