

#ifndef OGRE_UDP_CONTROL_SERVICE_H_
#define OGRE_UDP_CONTROL_SERVICE_H_

//forward declaration
class Ogre_IPRestrict_Mgr;

class Ogre_UDPSvc_Hdl: public ZCE_Event_Handler
{
protected:
    //
public:
    Ogre_UDPSvc_Hdl(const ZCE_Sockaddr_In &upd_addr,
                    ZCE_Reactor *reactor = ZCE_Reactor::instance());
protected:
    ~Ogre_UDPSvc_Hdl();

public:
    //ȡ�þ��
    virtual ZCE_HANDLE get_handle(void) const;
    //
    virtual int handle_input(ZCE_HANDLE);
    //
    virtual int handle_close();

public:
    //��ʼ��UPD�˿�
    int init_udp_peer();

public:

    //����UDP���ݸ�
    static int send_alldata_to_udp(Ogre4a_App_Frame *send_frame);

protected:

    //��PEER��ȡ����
    int read_data_fromudp(size_t &szrevc, ZCE_Sockaddr_In &remote_addr);
    //���յ������ݷ���ܵ�
    int pushdata_to_recvpipe();

protected:

    //
    ZCE_Socket_DataGram        dgram_peer_;

    //��ĵ�ַ
    ZCE_Sockaddr_In            udp_bind_addr_;
    //Socket_Peer_Info
    OGRE_PEER_ID           peer_svc_info_;

    //
    Ogre4a_App_Frame           *dgram_databuf_;

    //IP���ƹ�����
    Ogre_IPRestrict_Mgr       *ip_restrict_;

protected:
    //
    static  std::vector<Ogre_UDPSvc_Hdl *> ary_upd_peer_;

};

#endif //#ifndef OGRE_UDP_CONTROL_SERVICE_H_

