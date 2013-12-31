#ifndef ZCE_LIB_SOCKET_ADDR_BASE_
#define ZCE_LIB_SOCKET_ADDR_BASE_

#include "zce_os_adapt_socket.h"

//在改写发生3个月左右，我都一致使用sockaddr sockaddr_in作为地址参数，为什么要变化呢，
//因为在有一天改写原来代码时，突然觉得为什么为什么彻底OO一点点？
//整体参考ACE_INET_Addr ACE INET Addr实现的，当然也有一些变化，ACE没有直接使用sockaddr，不知道为啥

//Socket地址的基类。
class ZCELIB_EXPORT ZCE_Sockaddr
{
public:

    //构造函数，
    ZCE_Sockaddr (sockaddr *sockaddr_ptr = NULL, int sockaddr_size = -1);
    //析构函数,内部有virtual函数
    virtual ~ZCE_Sockaddr (void);

    //设置sockaddr地址信息,设置成纯虚函数的原因不想让你使用ZCE_Sockaddr
    virtual  void set_sockaddr (sockaddr *sockaddr_ptr, socklen_t sockaddr_size) = 0;

    //Get/set the size of the address.
    inline socklen_t get_size (void) const;
    //
    inline void  set_size (int sockaddr_size);

    //设置地址信息
    inline void set_addr (sockaddr *sockaddr_ptr);
    //取得地址信息
    inline sockaddr *get_addr (void) const;

    // 检查地址是否相等
    bool operator == (const ZCE_Sockaddr &others_sockaddr) const;
    // 检查地址是否不相等
    bool operator != (const ZCE_Sockaddr &others_sockaddr) const;

    //转换成字符串,
    inline const char *to_string(char *buffer, size_t buf_len) const
    {
        return ZCE_OS::socketaddr_ntop_ex(sockaddr_ptr_, buffer, buf_len);
    }

    //转换成字符串,同时输出字符串的长度
    inline const char *to_string(char *buffer, size_t buf_len, size_t &use_buf) const
    {

        const char *ret_str = ZCE_OS::socketaddr_ntop_ex(sockaddr_ptr_, buffer, buf_len);
        if (ret_str)
        {
            use_buf = 0;
        }
        else
        {
            use_buf = strlen(ret_str);
        }
        return ret_str;
    }

public:

    // 地址类型的指针,
    sockaddr *sockaddr_ptr_;

    // 地址结构的长度 Number of bytes in the address.
    socklen_t sockaddr_size_;
};

//Get/set the size of the address.
inline socklen_t ZCE_Sockaddr::get_size (void) const
{
    return sockaddr_size_;
}
//
inline void ZCE_Sockaddr::set_size (int sockaddr_size)
{
    sockaddr_size_ = sockaddr_size;
}

//设置地址信息
inline void ZCE_Sockaddr::set_addr (sockaddr *sockaddr_ptr)
{
    sockaddr_ptr_ = sockaddr_ptr;
}
//取得地址信息
inline sockaddr *ZCE_Sockaddr::get_addr (void) const
{
    return sockaddr_ptr_;
}

#endif  //ZCE_LIB_SOCKET_ADDR_BASE_
