/******************************************************************************************
Copyright           : 2000-2004, FXL Technology (Shenzhen) Company Limited.
FileName            : ogre_accepthandler.h
Author              : Jovi(HuangHao),Sail(ZENGXING)
Version             :
Date Of Creation    : 2006年11月7日
Description         : ACCPET的Handler的类

Others              :
Function List       :
    1.  ......
Modification History:
    1.Date  :
      Author  :
      Modification  :
******************************************************************************************/

#ifndef OGRE_TCP_ACCEPT_HANDLER_H_
#define OGRE_TCP_ACCEPT_HANDLER_H_

/****************************************************************************************************
class  OgreTCPAcceptHandler TCP Accept 处理的EventHandler
****************************************************************************************************/
class Ogre4aIPRestrictMgr;
class OgreTCPAcceptHandler : public ZCE_Event_Handler
{

public:
    //构造函数
    OgreTCPAcceptHandler(const ZCE_Sockaddr_In &listen_addr,
                         const char *recv_mod_file,
                         ZCE_Reactor *reactor = ZCE_Reactor::instance());
protected:
    ~OgreTCPAcceptHandler();
public:
    //设置Bind的IP地址
    void set_peer_bindaddr(ZCE_Sockaddr_In &addr);

    //创建监听的端口
    int create_listenpeer();

    //处理有端口
    virtual int handle_input(ZCE_HANDLE );

    //关闭处理
    virtual int handle_close ();

    //得到Acceptor的句柄
    ZCE_SOCKET get_handle(void) const;

protected:

    //ACCPET PEER
    ZCE_Socket_Acceptor        peer_acceptor_;

    //邦定的地址
    ZCE_Sockaddr_In            accept_bind_addr_;

    //收取数据的模块的文件名称
    std::string                recv_mod_file_;
    //TCP收取数据的模块HANDLER
    ZCE_SHLIB_HANDLE           recv_mod_handler_;

    //模块总结的函数名称
    FPJudgeRecvWholeFrame      fp_judge_whole_frame_;

    //IP限制管理器
    Ogre4aIPRestrictMgr       *ip_restrict_;

};

#endif //_OGRE_TCP_ACCEPT_HANDLER_H_

