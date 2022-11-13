#pragma once

namespace ogre
{
//forward declaration
class ip_restrict;

class svc_udp : public zce::event_handler
{
protected:
    //
public:
    svc_udp(const zce::skt::addr_in& upd_addr,
            zce::reactor* reactor = zce::reactor::instance());
protected:
    ~svc_udp();

public:
    //取得句柄
    virtual ZCE_HANDLE get_handle(void) const;
    //
    void read_event() override;
    //
    void close_handle() override;

public:
    //初始化UPD端口
    int init_udp_peer();

public:

    //发送UDP数据给
    static int send_alldata_to_udp(soar::ogre4a_frame* send_frame);

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
    soar::OGRE_PEER_ID           peer_svc_info_;

    //
    soar::ogre4a_frame*      dgram_databuf_;

    //IP限制管理器
    ip_restrict* ip_restrict_;

protected:
    //
    static  std::vector<svc_udp*> ary_upd_peer_;
};
} //namespace ogre
