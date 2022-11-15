#pragma once

#include "zce/socket/addr_base.h"

namespace zce::skt
{
/*!
* @brief      IPv4的物理结构
*
* @note
*/
class addr_in : public zce::skt::addr_base
{
public:

    ///默认构造函数
    addr_in(void);

    ///根据addr_in构造，
    explicit addr_in(const sockaddr_in* addr);

    ///根据IP地址(XXX.XXX.XXX.XXX)字符串，端口号初始化构造
    addr_in(const char* ip_addr_str,
            uint16_t port_number);

    ///根据地址(整数)，端口号初始化构造
    addr_in(uint32_t ip_addr,
            uint16_t port_number);

    ///拷贝构造，一定要写，这个类的基类指针是指向自己的一个地址的，
    addr_in(const addr_in& others);

    ///析构函数
    virtual ~addr_in() = default;

public:

    ///设置地址信息
    virtual void set_sockaddr(sockaddr* addr, socklen_t len) override;

    ///取得域名相关的IP地址信息，调用函数是getaddrinfo，notename可以是数值地址，或者域名
    virtual int getaddrinfo(const char* nodename,
                            uint16_t port_number = 0) override;

    ///DNS相关函数，
    ///取得IP地址相关的域名信息,调用函数是getnameinfo
    virtual int getnameinfo(char* host_name, size_t name_len) const override;

    ///设置端口号，
    virtual inline void set_port(uint16_t) override;
    ///取得端口号
    virtual inline uint16_t get_port(void) const override;

    /*!
    * @brief      根据IP地址(XXX.XXX.XXX.XXX)字符串，端口号根据参数设置
    * @return     int  返回0表示转换成功
    * @param      ip_addr_str
    * @param      port_number
    * @note
    */
    int set(const char ip_addr_str[16],
            uint16_t port_number);

    /*!
    * @brief      根据地址IP整数，端口号设置
    * @return     int 返回0表示转换成功
    * @param      ip_addr
    * @param      port_number
    */
    int set(uint32_t ip_addr,
            uint16_t port_number);

    /*!
    * @brief      根据字符串取得IP地址信息，以及端口号信息,
    * @return     int  返回0表示转换成功
    * @param      ip_addr_str 地址字符串
    * @note       如果字符串里面有#,会被认为有端口号，如果没有，端口号为0
    */
    int set(const char* ip_addr_str);

    ///检查端口号是否是一个安全端口
    bool check_safeport();

    //端口的打印输出使用to_string函数

    //取得IP地址，本地序列
    inline uint32_t get_ip_address(void) const;

    //内外地址判断在基类

    //比较两个地址是否相等
    bool operator == (const addr_in& others) const;
    //比较两个地址是否不想等
    bool operator != (const addr_in& others) const;

    //检查IP地址是否相等,忽视端口
    bool is_ip_equal(const addr_in& others) const;

    ///各种操作符号转换函数，方便各种使用，让addr_in的行为和addr_in基本一致
    ///返回addr_in
    operator sockaddr_in () const;
    ///返回内部const addr_in的指针，（不可以被修改）
    operator const sockaddr_in* () const;
    ///返回内部addr_in的指针，（可以被修改）
    operator sockaddr_in* ();

protected:

    ///IPV4的地址
    ::sockaddr_in           in4_addr_;
};

//取得IP地址，本地序列
inline uint32_t addr_in::get_ip_address(void) const
{
    return ntohl(in4_addr_.sin_addr.s_addr);
}

//设置端口好，
inline void addr_in::set_port(uint16_t port_number)
{
    in4_addr_.sin_port = ntohs(port_number);
}
//取得端口号
inline uint16_t addr_in::get_port(void) const
{
    return ntohs(in4_addr_.sin_port);
}
}
