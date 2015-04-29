#include "zce_predefine.h"
#include "zce_socket_addr_base.h"

ZCE_Sockaddr::ZCE_Sockaddr(sockaddr *sockaddr_ptr , int sockaddr_size ):
    sockaddr_ptr_(sockaddr_ptr),
    sockaddr_size_(sockaddr_size)
{

}

//，和析构函数
ZCE_Sockaddr::~ZCE_Sockaddr()
{

}

// 检查地址是否相等
bool ZCE_Sockaddr::operator == (const ZCE_Sockaddr &others_sockaddr) const
{
    if (sockaddr_ptr_ == others_sockaddr.sockaddr_ptr_ &&
        sockaddr_size_ == others_sockaddr.sockaddr_size_ )
    {
        return true;
    }

    return false;
}
// 检查地址是否不相等
bool ZCE_Sockaddr::operator != (const ZCE_Sockaddr &others_sockaddr) const
{
    return !(*this == others_sockaddr);
}

