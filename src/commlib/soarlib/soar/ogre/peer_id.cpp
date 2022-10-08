#include "soar/predefine.h"
#include "soar/ogre/peer_id.h"

namespace soar
{
OGRE_PEER_ID::OGRE_PEER_ID(unsigned int peer_ip_address, unsigned short peer_port) :
    peer_ip_address_(peer_ip_address),
    peer_port_(peer_port)
{
}

OGRE_PEER_ID::OGRE_PEER_ID(const zce::skt::addr_in& inet_addr) :
    peer_ip_address_(inet_addr.get_ip_address()),
    peer_port_(inet_addr.get_port())
{
}

OGRE_PEER_ID::~OGRE_PEER_ID()
{
}

//
void OGRE_PEER_ID::set(unsigned int peer_ip_address, unsigned short peer_port)
{
    peer_ip_address_ = peer_ip_address;
    peer_port_ = peer_port;
}

//根据ACE的IP地址设置
void OGRE_PEER_ID::set(const zce::skt::addr_in& inet_addr)
{
    peer_ip_address_ = inet_addr.get_ip_address();
    peer_port_ = inet_addr.get_port();
}

//比较函数,
bool OGRE_PEER_ID::operator ==(const OGRE_PEER_ID& others) const
{
    if (others.peer_ip_address_ == peer_ip_address_ && others.peer_port_ == peer_port_)
    {
        return true;
    }

    return false;
}
//比较不同函数,
bool OGRE_PEER_ID::operator !=(const OGRE_PEER_ID& others) const
{
    return !(*this == others);
}
//有个排序需求,
bool OGRE_PEER_ID::operator <(const OGRE_PEER_ID& others) const
{
    if (peer_ip_address_ + peer_port_ < others.peer_ip_address_ + others.peer_port_)
    {
        return true;
    }

    return false;
}
}