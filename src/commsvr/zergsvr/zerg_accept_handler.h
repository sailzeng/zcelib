
#ifndef ZERG_TCP_ACCEPT_HANDLER_H_
#define ZERG_TCP_ACCEPT_HANDLER_H_

class TcpHandlerImpl;
class Zerg_IPRestrict_Mgr;
/****************************************************************************************************
class  TCP_Accept_Handler TCP Accept 处理的EventHandler
****************************************************************************************************/
class TCP_Accept_Handler : public ZCE_Event_Handler
{

    typedef unordered_set<unsigned int> SetOfIPAddress;

protected:

    ~TCP_Accept_Handler();
public:
    //构造函数
    TCP_Accept_Handler(const SERVICES_ID &svcid,
                       const ZCE_Sockaddr_In &addr,
                       bool sessionkey_verify = true);

    //创建监听的端口
    int create_listen(unsigned int backlog);

    //处理有端口
    virtual int handle_input();

    //关闭处理
    virtual int handle_close ();

    //得到Acceptor的句柄
    virtual ZCE_HANDLE get_handle(void) const;



protected:

    //ACCPET PEER
    ZCE_Socket_Acceptor      peer_acceptor_;

    //自己的 SVC INFO
    SERVICES_ID              my_svc_info_;

    //邦定的地址
    ZCE_Sockaddr_In          accept_bind_addr_;

    //是否验证用户的sessionkey_verify
    bool                     sessionkey_verify_;

    //IP限制管理器
    Zerg_IPRestrict_Mgr      *ip_restrict_;


};

#endif //_ZERG_TCP_ACCEPT_HANDLER_H_

