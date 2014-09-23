

#ifndef OGRE_TCP_SVR_PEER_INFO_SET_H_
#define OGRE_TCP_SVR_PEER_INFO_SET_H_

class Ogre_TCP_Svc_Handler;
/****************************************************************************************************
class  PeerInfoSetToTCPHdlMap
****************************************************************************************************/
class PeerInfoSetToTCPHdlMap
{
protected:
    typedef hash_map<Socket_Peer_Info, Ogre_TCP_Svc_Handler *, HashofSocketPeerInfo > MapOfSocketPeerInfo;
    //
    MapOfSocketPeerInfo  peer_info_set_;

public:
    //构造函数,
    PeerInfoSetToTCPHdlMap();
    ~PeerInfoSetToTCPHdlMap();

    //初始化
    void init_services_peerinfo(size_t szpeer);

    //查询配置信息
    int find_services_peerinfo(const Socket_Peer_Info &svrinfo, Ogre_TCP_Svc_Handler*&);
    //设置配置信息
    int add_services_peerinfo(const Socket_Peer_Info &svrinfo, Ogre_TCP_Svc_Handler *);
    //删除配置信息
    size_t del_services_peerinfo(const Socket_Peer_Info &svrinfo);
    //
    void clear_and_close();
};

#endif //_OGRE_TCP_SVR_PEER_INFO_SET_H_

