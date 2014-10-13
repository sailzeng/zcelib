

#ifndef OGRE_TCP_SVR_PEER_INFO_SET_H_
#define OGRE_TCP_SVR_PEER_INFO_SET_H_

class Ogre_TCP_Svc_Handler;
/****************************************************************************************************
class  PeerInfoSetToTCPHdlMap
****************************************************************************************************/
class PeerInfoSetToTCPHdlMap
{


public:
    //构造函数,
    PeerInfoSetToTCPHdlMap();
    ~PeerInfoSetToTCPHdlMap();

    //初始化
    void init_services_peerinfo(size_t szpeer);

    //查询配置信息
    int find_services_peerinfo(const SOCKET_PERR_ID &svrinfo, Ogre_TCP_Svc_Handler*&);
    //设置配置信息
    int add_services_peerinfo(const SOCKET_PERR_ID &svrinfo, Ogre_TCP_Svc_Handler *);
    //删除配置信息
    size_t del_services_peerinfo(const SOCKET_PERR_ID &svrinfo);
    //
    void clear_and_close();

protected:
    //
    typedef std::unordered_map<SOCKET_PERR_ID, Ogre_TCP_Svc_Handler *, HashofSocketPeerInfo > MapOfSocketPeerInfo;
    //
    MapOfSocketPeerInfo  peer_info_set_;

};

#endif //OGRE_TCP_SVR_PEER_INFO_SET_H_

