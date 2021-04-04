
#ifndef OGRE_TCP_ACCEPT_HANDLER_H_
#define OGRE_TCP_ACCEPT_HANDLER_H_

#include "ogre_configure.h"

class Ogre_IPRestrict_Mgr;


/*!
* @brief      TCP Accept �����EventHandler
*
* @note
*/
class Ogre_TCPAccept_Hdl : public ZCE_Event_Handler
{

public:

    //���캯��
    Ogre_TCPAccept_Hdl(const TCP_PEER_CONFIG_INFO &config_info,
                       ZCE_Reactor *reactor = ZCE_Reactor::instance());
protected:
    ~Ogre_TCPAccept_Hdl();
public:


    //���������Ķ˿�
    int create_listenpeer();

    //�����ж˿�
    virtual int handle_input(ZCE_HANDLE );

    //�رմ���
    virtual int handle_close ();

    //�õ�Acceptor�ľ��
    ZCE_HANDLE get_handle(void) const;

protected:

    //ACCPET PEER
    ZCE_Socket_Acceptor    peer_acceptor_;


    TCP_PEER_MODULE_INFO   peer_module_info_;

    //IP���ƹ�����
    Ogre_IPRestrict_Mgr   *ip_restrict_;

};

#endif //OGRE_TCP_ACCEPT_HANDLER_H_

