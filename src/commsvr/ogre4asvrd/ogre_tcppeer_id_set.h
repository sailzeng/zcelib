

#ifndef OGRE_SOCKETPEER_ID_SET_H_
#define OGRE_SOCKETPEER_ID_SET_H_

class Ogre_TCP_Svc_Handler;
/****************************************************************************************************
class  PeerID_To_TCPHdl_Map
****************************************************************************************************/
class PeerID_To_TCPHdl_Map
{


public:
    ///构造函数,
    PeerID_To_TCPHdl_Map();
    ~PeerID_To_TCPHdl_Map();

    //初始化
    void init_services_peerinfo(size_t szpeer);

    ///查询配置信息
    int find_services_peerinfo(const OGRE_PEER_ID &svrinfo, Ogre_TCP_Svc_Handler* &);
    ///设置配置信息
    int add_services_peerinfo(const OGRE_PEER_ID &svrinfo, Ogre_TCP_Svc_Handler *);
    ///删除配置信息
    size_t del_services_peerinfo(const OGRE_PEER_ID &svrinfo);
    //
    void clear_and_close();

protected:
    //
    typedef std::unordered_map < OGRE_PEER_ID,
            Ogre_TCP_Svc_Handler *,
            HASH_OF_OGREPEERID > MAP_OF_SOCKETPEER_ID;
    //
    MAP_OF_SOCKETPEER_ID  peer_info_set_;

};

#endif //OGRE_SOCKETPEER_ID_SET_H_

