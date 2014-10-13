

#ifndef OGRE_UDP_CONTROL_SERVICE_H_
#define OGRE_UDP_CONTROL_SERVICE_H_

//forward declaration
class Ogre_IPRestrict_Mgr;

class OgreUDPSvcHandler: public ZCE_Event_Handler
{
protected:
    //
public:
    OgreUDPSvcHandler(const ZCE_Sockaddr_In &upd_addr,
                      ZCE_Reactor *reactor = ZCE_Reactor::instance());
protected:
    ~OgreUDPSvcHandler();

public:
    //取得句柄
    virtual ZCE_HANDLE get_handle(void) const;
    //
    virtual int handle_input(ZCE_HANDLE);
    //
    virtual int handle_close();

public:
    //初始化UPD端口
    int InitUDPServices();
    //
    int SendDataToUDP(Ogre4a_App_Frame *send_frame);
public:
    //初始化静态的发送端口
    static int OpenUDPSendPeer();
    //发送UDP数据给
    static int SendAllDataToUDP(Ogre4a_App_Frame *send_frame);

protected:

    //从PEER读取数据
    int ReadDataFromUDP(size_t &szrevc, ZCE_Sockaddr_In &remote_addr);
    //将收到的数据放入管道
    int pushdata_to_recvpipe();

protected:

    //
    ZCE_Socket_DataGram        dgram_peer_;

    //邦定的地址
    ZCE_Sockaddr_In            udp_bind_addr_;
    //Socket_Peer_Info
    SOCKET_PERR_ID           peer_svc_info_;

    //
    Ogre4a_App_Frame           *dgram_databuf_;

    //IP限制管理器
    Ogre_IPRestrict_Mgr       *ip_restrict_;

protected:
    //
    static  std::vector<OgreUDPSvcHandler *> ary_upd_peer_;

};

#endif //#ifndef _OGRE_UDP_CONTROL_SERVICE_H_

