
#ifndef ZERG_TCP_ACCEPT_HANDLER_H_
#define ZERG_TCP_ACCEPT_HANDLER_H_


class Zerg_IPRestrict_Mgr;


/*!
* @brief      TCP Accept �����EventHandler,
*
* @note
*/
class TCP_Accept_Handler : public ZCE_Event_Handler
{
public:

    /*!
    * @brief      ���캯��
    * @param      svcid  ��Ӧ��svc id
    * @param      addr   ��Ӧ��IP��ַ
    */
    TCP_Accept_Handler(const SERVICES_ID &svcid,
                       const ZCE_Sockaddr_In &addr);

protected:
    ///�����˿�
    ~TCP_Accept_Handler();

public:

    ///���������Ķ˿�
    int create_listen();

    /*!
    * @brief      �¼������Ļٵ�������ʾ��һ��accept ������
    * @return     int
    */
    virtual int handle_input();

    ///�رմ���
    virtual int handle_close ();

    ///�õ�Acceptor�ľ��
    virtual ZCE_HANDLE get_handle(void) const;



protected:

    ///ACCPET PEER
    ZCE_Socket_Acceptor      peer_acceptor_;

    ///�Լ��� SVC INFO
    SERVICES_ID              my_svc_info_;

    ///��ĵ�ַ
    ZCE_Sockaddr_In          accept_bind_addr_;


    ///IP���ƹ�����
    Zerg_IPRestrict_Mgr      *ip_restrict_;


};

#endif //_ZERG_TCP_ACCEPT_HANDLER_H_

