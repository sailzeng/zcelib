#include "zce/predefine.h"
#include "zce/socket/addr_base.h"

namespace zce
{
Sockaddr_Base::Sockaddr_Base(sockaddr* sockaddr_ptr, int sa_size) :
    sockaddr_ptr_(sockaddr_ptr),
    sockaddr_size_(sa_size)
{
}

//，和析构函数
Sockaddr_Base::~Sockaddr_Base()
{
}

// 检查地址是否相等
bool Sockaddr_Base::operator == (const Sockaddr_Base& others_sockaddr) const
{
    if (sockaddr_ptr_ == others_sockaddr.sockaddr_ptr_ &&
        sockaddr_size_ == others_sockaddr.sockaddr_size_)
    {
        return true;
    }

    return false;
}
// 检查地址是否不相等
bool Sockaddr_Base::operator != (const Sockaddr_Base& others_sockaddr) const
{
    return !(*this == others_sockaddr);
}

//检查地址是否是一个内网地址
bool Sockaddr_Base::is_internal()
{
    return zce::is_internal(sockaddr_ptr_);
}

//检查地址是否是一个外网地址，其实我就简单认为不是外网地址就是内网地址
bool Sockaddr_Base::is_internet()
{
    return !(zce::is_internal(sockaddr_ptr_));
}
}