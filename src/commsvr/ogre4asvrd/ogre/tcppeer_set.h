#pragma once
namespace ogre
{
class svc_tcp;
/****************************************************************************************************
class  tcppeer_set
****************************************************************************************************/
class tcppeer_set
{
public:
    ///构造函数,
    tcppeer_set();
    ~tcppeer_set();

    //初始化
    void init_services_peerinfo(size_t szpeer);

    ///查询配置信息
    int find_services_peerinfo(const soar::OGRE_PEER_ID& svrinfo,
                               svc_tcp*&);
    ///设置配置信息
    int add_services_peerinfo(const soar::OGRE_PEER_ID& svrinfo,
                              svc_tcp*);
    ///删除配置信息
    size_t del_services_peerinfo(const soar::OGRE_PEER_ID& svrinfo);
    //
    void clear_and_close();

protected:
    //
    typedef std::unordered_map < soar::OGRE_PEER_ID,
        svc_tcp*,
        soar::HASH_OF_OGREPEERID > MAP_OF_SOCKETPEER_ID;
    //
    MAP_OF_SOCKETPEER_ID  peer_info_set_;
};
}
