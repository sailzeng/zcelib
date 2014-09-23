

#ifndef OGRE_UDP_CONTROL_SERVICE_H_
#define OGRE_UDP_CONTROL_SERVICE_H_

//forward declaration
class Ogre4aIPRestrictMgr;

class OgreUDPSvcHandler: public ZEN_Event_Handler
{
protected:
    //
public:
    OgreUDPSvcHandler(const ZEN_Sockaddr_In &upd_addr,
                      ZEN_Reactor *reactor = ZEN_Reactor::instance());
protected:
    ~OgreUDPSvcHandler();

public:
    //取得句柄
    virtual ZEN_SOCKET get_handle(void) const;
    //
    virtual int handle_input(ZEN_HANDLE);
    //
    virtual int handle_close();

public:
    //初始化UPD端口
    int InitUDPServices();
    //
    int SendDataToUDP(Ogre4a_AppFrame *send_frame);
public:
    //初始化静态的发送端口
    static int OpenUDPSendPeer();
    //发送UDP数据给
    static int SendAllDataToUDP(Ogre4a_AppFrame *send_frame);

protected:

    //从PEER读取数据
    int ReadDataFromUDP(size_t &szrevc, ZEN_Sockaddr_In &remote_addr);
    //将收到的数据放入管道
    int pushdata_to_recvpipe();

protected:

    //
    Zen_Socket_DataGram        dgram_peer_;

    //邦定的地址
    ZEN_Sockaddr_In            udp_bind_addr_;
    //Socket_Peer_Info
    Socket_Peer_Info             peer_svc_info_;

    //
    Ogre4a_AppFrame           *dgram_databuf_;

    //IP限制管理器
    Ogre4aIPRestrictMgr       *ip_restrict_;

protected:
    //
    static  std::vector<OgreUDPSvcHandler *> ary_upd_peer_;

};

#endif //#ifndef _OGRE_UDP_CONTROL_SERVICE_H_

