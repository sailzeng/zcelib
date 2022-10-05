#ifndef OGRE_UDP_CONTROL_SERVICE_H_
#define OGRE_UDP_CONTROL_SERVICE_H_

//forward declaration
class Ogre_IPRestrict_Mgr;

class Ogre_UDPSvc_Hdl : public zce::event_handler
{
protected:
    //
public:
    Ogre_UDPSvc_Hdl(const zce::skt::addr_in& upd_addr,
                    zce::reactor* reactor = zce::reactor::instance());
protected:
    ~Ogre_UDPSvc_Hdl();

public:
    //取得句柄
    virtual ZCE_HANDLE get_handle(void) const;
    //
    virtual int read_event(ZCE_HANDLE);
    //
    virtual int event_close();

public:
    //初始化UPD端口
    int init_udp_peer();

public:

    //发送UDP数据给
    static int send_alldata_to_udp(ogre4a_frame* send_frame);

protected:

    //从PEER读取数据
    int read_data_fromudp(size_t& szrevc, zce::skt::addr_in& remote_addr);
    //将收到的数据放入管道
    int pushdata_to_recvpipe();

protected:

    //
    zce::skt::datagram     dgram_peer_;

    //邦定的地址
    zce::skt::addr_in      udp_bind_addr_;
    //Socket_Peer_Info
    OGRE_PEER_ID           peer_svc_info_;

    //
    ogre4a_frame*      dgram_databuf_;

    //IP限制管理器
    Ogre_IPRestrict_Mgr* ip_restrict_;

protected:
    //
    static  std::vector<Ogre_UDPSvc_Hdl*> ary_upd_peer_;
};

#endif //#ifndef OGRE_UDP_CONTROL_SERVICE_H_
