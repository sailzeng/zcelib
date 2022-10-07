#ifndef OGRE_TCP_ACCEPT_HANDLER_H_
#define OGRE_TCP_ACCEPT_HANDLER_H_

#include "ogre_configure.h"

class Ogre_IPRestrict_Mgr;

/*!
* @brief      TCP Accept 处理的EventHandler
*
* @note
*/
class Ogre_TCPAccept_Hdl : public zce::event_handler
{
public:

    //构造函数
    Ogre_TCPAccept_Hdl(const TCP_PEER_CONFIG_INFO& config_info,
                       zce::reactor* reactor = zce::reactor::instance());
protected:
    ~Ogre_TCPAccept_Hdl();
public:

    //创建监听的端口
    int create_listenpeer();

    //处理有端口
    virtual int accept_event(ZCE_HANDLE);

    //关闭处理
    virtual int close_event();

    //得到Acceptor的句柄
    ZCE_HANDLE get_handle(void) const;

protected:

    //ACCPET PEER
    zce::skt::acceptor    peer_acceptor_;

    TCP_PEER_MODULE_INFO   peer_module_info_;

    //IP限制管理器
    Ogre_IPRestrict_Mgr* ip_restrict_;
};

#endif //OGRE_TCP_ACCEPT_HANDLER_H_
