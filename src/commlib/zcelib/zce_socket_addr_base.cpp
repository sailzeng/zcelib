#include "zce_predefine.h"
#include "zce_socket_addr_base.h"

ZCE_Sockaddr::ZCE_Sockaddr(sockaddr *sockaddr_ptr, int sa_size ):
    sockaddr_ptr_(sockaddr_ptr),
    sockaddr_size_(sa_size)
{

}

//������������
ZCE_Sockaddr::~ZCE_Sockaddr()
{

}

// ����ַ�Ƿ����
bool ZCE_Sockaddr::operator == (const ZCE_Sockaddr &others_sockaddr) const
{
    if (sockaddr_ptr_ == others_sockaddr.sockaddr_ptr_ &&
        sockaddr_size_ == others_sockaddr.sockaddr_size_ )
    {
        return true;
    }

    return false;
}
// ����ַ�Ƿ����
bool ZCE_Sockaddr::operator != (const ZCE_Sockaddr &others_sockaddr) const
{
    return !(*this == others_sockaddr);
}

