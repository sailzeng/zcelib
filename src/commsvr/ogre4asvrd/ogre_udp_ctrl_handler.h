

#ifndef OGRE_UDP_CONTROL_SERVICE_H_
#define OGRE_UDP_CONTROL_SERVICE_H_

//forward declaration
class Ogre_IPRestrict_Mgr;

class Ogre_UDPSvc_Hdl: public ZCE_Event_Handler
{
protected:
    //
public:
    Ogre_UDPSvc_Hdl(const zce::Sockaddr_In &upd_addr,
                    ZCE_Reactor *reactor = ZCE_Reactor::instance());
protected:
    ~Ogre_UDPSvc_Hdl();

public:
    //取得句柄
    virtual ZCE_HANDLE get_handle(void) const;
    //
    virtual int handle_input(ZCE_HANDLE);
    //
    virtual int handle_close();

public:
    //初始化UPD端口
    int init_udp_peer();

public:

    //发送UDP数据给
    static int send_alldata_to_udp(Ogre4a_App_Frame *send_frame);

protected:

    //从PEER读取数据
    int read_data_fromudp(size_t &szrevc, zce::Sockaddr_In &remote_addr);
    //将收到的数据放入管道
    int pushdata_to_recvpipe();

protected:

    //
    zce::Socket_DataGram        dgram_peer_;

    //邦定的地址
    zce::Sockaddr_In            udp_bind_addr_;
    //Socket_Peer_Info
    OGRE_PEER_ID           peer_svc_info_;

    //
    Ogre4a_App_Frame           *dgram_databuf_;

    //IP限制管理器
    Ogre_IPRestrict_Mgr       *ip_restrict_;

protected:
    //
    static  std::vector<Ogre_UDPSvc_Hdl *> ary_upd_peer_;

};

#endif //#ifndef OGRE_UDP_CONTROL_SERVICE_H_

