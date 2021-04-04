#include "soar_predefine.h"
#include "soar_ogre_peer_id.h"

/****************************************************************************************************
class  Socket_Peer_Info ��������Ϣ,Ϊ��ͬʱ��ʾIP+�˿ڵ�ģʽ������UNION��ʽ
****************************************************************************************************/

OGRE_PEER_ID::OGRE_PEER_ID(unsigned int peer_ip_address, unsigned short peer_port):
    peer_ip_address_(peer_ip_address),
    peer_port_(peer_port)
{
}

OGRE_PEER_ID::OGRE_PEER_ID(const ZCE_Sockaddr_In &inet_addr):
    peer_ip_address_(inet_addr.get_ip_address()),
    peer_port_(inet_addr.get_port_number())
{
}

OGRE_PEER_ID::~OGRE_PEER_ID()
{
}

//
void OGRE_PEER_ID::set(unsigned int peer_ip_address, unsigned short peer_port )
{
    peer_ip_address_ = peer_ip_address;
    peer_port_ = peer_port;
}

//����ACE��IP��ַ����
void OGRE_PEER_ID::set(const ZCE_Sockaddr_In &inet_addr)
{
    peer_ip_address_ = inet_addr.get_ip_address();
    peer_port_ = inet_addr.get_port_number();
}

//�ȽϺ���,
bool OGRE_PEER_ID::operator ==(const OGRE_PEER_ID &others) const
{
    if (others.peer_ip_address_ == peer_ip_address_ && others.peer_port_ == peer_port_)
    {
        return true;
    }

    return false;
}
//�Ƚϲ�ͬ����,
bool OGRE_PEER_ID::operator !=(const OGRE_PEER_ID &others) const
{
    return !(*this == others);
}
//�и���������,
bool OGRE_PEER_ID::operator <(const OGRE_PEER_ID &others) const
{
    if (peer_ip_address_ + peer_port_ < others.peer_ip_address_ + others.peer_port_ )
    {
        return true;
    }

    return false;
}

