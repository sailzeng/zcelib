#include "zce/predefine.h"
#include "zce/os_adapt/netdb.h"
#include "zce/socket/addr_base.h"
#include "zce/socket/addr_in.h"
#include "zce/socket/addr_in6.h"

namespace zce::skt
{
addr_base::addr_base(sockaddr* sockaddr_ptr, int sa_size) :
    sockaddr_ptr_(sockaddr_ptr),
    sockaddr_size_(sa_size)
{
}

//，和析构函数
addr_base::~addr_base()
{
}

// 检查地址是否相等
bool addr_base::operator == (const addr_base& others_sockaddr) const
{
    if (sockaddr_ptr_ == others_sockaddr.sockaddr_ptr_ &&
        sockaddr_size_ == others_sockaddr.sockaddr_size_)
    {
        return true;
    }
    return false;
}

// 检查地址是否不相等
bool addr_base::operator != (const addr_base& others_sockaddr) const
{
    return !(*this == others_sockaddr);
}

//检查地址是否是一个内网地址
bool addr_base::is_internal()
{
    return zce::is_internal(sockaddr_ptr_);
}

//检查地址是否是一个外网地址，其实我就简单认为不是外网地址就是内网地址
bool addr_base::is_internet()
{
    return !(zce::is_internal(sockaddr_ptr_));
}
}