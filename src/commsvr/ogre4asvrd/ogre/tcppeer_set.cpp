#include "ogre/predefine.h"
#include "ogre/tcppeer_set.h"
#include "ogre/auto_connect.h"
#include "ogre/svc_tcp.h"
#include "ogre/tcppeer_set.h"

namespace ogre
{
tcppeer_set::tcppeer_set()
{
}

tcppeer_set::~tcppeer_set()
{
}

void tcppeer_set::init_services_peerinfo(size_t szpeer)
{
    peer_info_set_.rehash(szpeer);
}

//根据SERVICEINFO查询PEER信息
int tcppeer_set::find_services_peerinfo(const soar::OGRE_PEER_ID& svrinfo,
                                        svc_tcp*& svrhandle)
{
    MAP_OF_SOCKETPEER_ID::iterator iter = peer_info_set_.find(svrinfo);

    const size_t BUFFER_SIZE = 32;
    char buffer[BUFFER_SIZE];

    if (iter == peer_info_set_.end())
    {
        ZCE_LOG(RS_INFO, "Can't find svchanle info. svrinfo IP|Port:[%s|%u] .\n",
                zce::inet_ntoa(svrinfo.peer_ip_address_, buffer, BUFFER_SIZE),
                svrinfo.peer_port_);
        return SOAR_RET::ERR_OGRE_NO_FIND_EVENT_HANDLE;
    }

    svrhandle = (*(iter)).second;
    return 0;
}

//设置PEER信息
int tcppeer_set::add_services_peerinfo(const soar::OGRE_PEER_ID& peer_info, svc_tcp* svrhandle)
{
    MAP_OF_SOCKETPEER_ID::iterator iter = peer_info_set_.find(peer_info);

    const size_t BUFFER_SIZE = 32;
    char buffer[BUFFER_SIZE];

    //已经有相关的记录了
    if (iter != peer_info_set_.end())
    {
        ZCE_LOG(RS_INFO, "Can't add_services_peerinfo peer_info:[%s|%u] ",
                zce::inet_ntoa(peer_info.peer_ip_address_, buffer, BUFFER_SIZE),
                peer_info.peer_port_);
        return SOAR_RET::ERR_OGRE_SERVER_ALREADY_LONGIN;
    }

    peer_info_set_[peer_info] = svrhandle;

    return 0;
}

//根据Socket_Peer_Info,删除PEER信息,
size_t tcppeer_set::del_services_peerinfo(const soar::OGRE_PEER_ID& peer_info)
{
    MAP_OF_SOCKETPEER_ID::iterator iter = peer_info_set_.find(peer_info);

    size_t szdel = peer_info_set_.erase(peer_info);

    //如果没有找到,99.99%理论上应该是代码写的有问题,除非插入没有成功的情况.调用了event_close.
    if (szdel <= 0)
    {
        ZCE_LOG(RS_ERROR, "Can't PeerInfoSetToTCPHdlMap::del_services_peerinfo size "
                "peer_info_set_ %u: szdel:%u peer_info:%u|%u .\n",
                peer_info_set_.size(),
                szdel,
                peer_info.peer_ip_address_,
                peer_info.peer_port_);
    }

    //ZCE_ASSERT(szdel >0 );
    return szdel;
}

void tcppeer_set::clear_and_close()
{
    const size_t SHOWINFO_NUMBER = 500;

    ZCE_LOG(RS_INFO, "Has %u peer want to close. Please wait. ACE that is accursed.\n",
            peer_info_set_.size());

    //这个函数可能是绝对的慢
    while (peer_info_set_.size() > 0)
    {
        if (peer_info_set_.size() % SHOWINFO_NUMBER == 0)
        {
            ZCE_LOG(RS_INFO, "Now remain %u peer want to close. Please wait. ACE that is accursed.\n",
                    peer_info_set_.size());
        }

        MAP_OF_SOCKETPEER_ID::iterator iter = peer_info_set_.begin();
        svc_tcp* svrhandle = (*(iter)).second;

        //svc_tcp::close_handle调用了del_services_peerinfo
        svrhandle->close_handle();
    }
}
}