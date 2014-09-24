#include "ogre_predefine.h"
#include "ogre_svrpeer_info_set.h"
#include "ogre_auto_connect.h"
#include "ogre_tcp_ctrl_handler.h"
#include "ogre_svrpeer_info_set.h"

/****************************************************************************************************
class  PeerInfoSetToTCPHdlMap
****************************************************************************************************/
PeerInfoSetToTCPHdlMap::PeerInfoSetToTCPHdlMap()
{
}

PeerInfoSetToTCPHdlMap::~PeerInfoSetToTCPHdlMap()
{
}

void PeerInfoSetToTCPHdlMap::init_services_peerinfo(size_t szpeer)
{
    peer_info_set_.resize(szpeer);
}

//根据SERVICEINFO查询PEER信息
int PeerInfoSetToTCPHdlMap::find_services_peerinfo(const Socket_Peer_Info &svrinfo, Ogre_TCP_Svc_Handler *&svrhandle)
{
    MapOfSocketPeerInfo::iterator iter = peer_info_set_.find(svrinfo);

    const size_t BUFFER_SIZE = 32;
    char buffer[BUFFER_SIZE];

    if (iter == peer_info_set_.end() )
    {
        ZLOG_INFO( "Can't find svchanle info. svrinfo IP|Port:[%s|%u] .\n",
                   ZEN_OS::inet_ntoa(svrinfo.peer_ip_address_, buffer, BUFFER_SIZE),
                   svrinfo.peer_port_);
        return SOAR_RET::ERR_OGRE_NO_FIND_EVENT_HANDLE;
    }

    svrhandle = (*(iter)).second;
    return 0;
}

//设置PEER信息
int PeerInfoSetToTCPHdlMap::add_services_peerinfo(const Socket_Peer_Info &peer_info, Ogre_TCP_Svc_Handler *svrhandle)
{
    MapOfSocketPeerInfo::iterator iter = peer_info_set_.find(peer_info);

    const size_t BUFFER_SIZE = 32;
    char buffer[BUFFER_SIZE];

    //已经有相关的记录了
    if (iter != peer_info_set_.end() )
    {
        ZLOG_INFO( "Can't add_services_peerinfo peer_info:[%s|%u] ",
                   ZEN_OS::inet_ntoa(peer_info.peer_ip_address_, buffer, BUFFER_SIZE),
                   peer_info.peer_port_);
        return SOAR_RET::ERR_OGRE_SERVER_ALREADY_LONGIN;
    }

    peer_info_set_[peer_info] = svrhandle;

    return 0;
}

/******************************************************************************************
Author          : Sail ZENGXING  Date Of Creation: 2006年3月22日
Function        : PeerInfoSetToTCPHdlMap::del_services_peerinfo
Return          : size_t
Parameter List  :
Param1: const Socket_Peer_Info& svrinfo
Description     : 根据Socket_Peer_Info,删除PEER信息,
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
size_t PeerInfoSetToTCPHdlMap::del_services_peerinfo(const Socket_Peer_Info &peer_info)
{
    MapOfSocketPeerInfo::iterator iter = peer_info_set_.find(peer_info);

    size_t szdel = peer_info_set_.erase(peer_info);

    //如果没有找到,99.99%理论上应该是代码写的有问题,除非插入没有成功的情况.调用了handle_close.
    if (szdel <= 0)
    {
        ZLOG_ERROR( "Can't PeerInfoSetToTCPHdlMap::del_services_peerinfo Size peer_info_set_ %u: szdel:%u peer_info:%u|%u .\n", peer_info_set_.size(), szdel, peer_info.peer_ip_address_, peer_info.peer_port_);
    }

    //ZCE_ASSERT(szdel >0 );
    return szdel;
}

void PeerInfoSetToTCPHdlMap::clear_and_close()
{
    const size_t SHOWINFO_NUMBER = 500;

    ZLOG_INFO( "Has %u peer want to close. Please wait. ACE that is accursed.\n", peer_info_set_.size());

    //这个函数可能是绝对的慢
    while (peer_info_set_.size() > 0)
    {
        if (peer_info_set_.size() % SHOWINFO_NUMBER == 0 )
        {
            ZLOG_INFO( "Now remain %u peer want to close. Please wait. ACE that is accursed.\n", peer_info_set_.size());
        }

        MapOfSocketPeerInfo::iterator iter = peer_info_set_.begin();
        Ogre_TCP_Svc_Handler *svrhandle = (*(iter)).second;

        //Ogre_TCP_Svc_Handler::handle_close调用了del_services_peerinfo
        svrhandle->handle_close();
    }
}

