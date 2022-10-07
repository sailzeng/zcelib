#pragma once

//SOCKET地址的基类
#include "zce/socket/addr_base.h"
#include "zce/time/time_value.h"

namespace zce::skt
{
//SOCKET的基类
class socket_base
{
    //socket_base不会提供给外部用，所以不准用
protected:

    /*!
    * @brief      构造函数
    */
    socket_base();

    /*!
    * @brief      拷贝构造函数
    * @param      socket_hanle
    */
    explicit socket_base(const ZCE_SOCKET& socket_hanle);

    /*!
    * @brief      析构函数，会调用close，
    * @note       这儿必须说明一下，ACE在设计socket封装的时候，在析构函数里面
    *             没有调用close，他的出发点是避免在参数(value)传递的时候被析构
    *             了，但我觉得良好的参数设计应该更好，而让普罗大众知道这个析构没
    *             有释放资源，这个反而更难。
    */
    ~socket_base();

public:

    //设置句柄
    void set_handle(const ZCE_SOCKET& socket_hanle);
    //获取句柄
    ZCE_SOCKET get_handle() const;

    //Open SOCK句柄，不BIND本地地址的方式
    int open(int type,
             int family,
             bool nonblock = false,
             int protocol = 0,
             bool reuse_addr = true);

    //Open SOCK句柄，BIND本地地址的方式
    int open(int type,
             const zce::skt::addr_base* local_addr,
             bool nonblock = false,
             int protocol = 0,
             bool reuse_addr = false);

    ///关闭之,一般情况下，我会假设释放资源是必然成功的一件事
    int close();

    /*!
    * @brief      释放对句柄的管理，将句柄置为无效而已，在其和其的子类作为参数传递时
    *             为了避免析构函数主动关闭close了句柄时，在析构前使用这个函数
    * @note       请参考析构函数的说明
    */
    void release_noclose();

    /*!
    * @brief      绑定某个本地的IP地址，
    * @return     int
    * @param      add_name
    */
    int bind(const zce::skt::addr_base* add_name) const;

    ///打开某些选项，WIN32目前只支持O_NONBLOCK
    int sock_enable(int value) const;

    ///关闭某些选项，WIN32目前只支持O_NONBLOCK
    int sock_disable(int value) const;

    //获取Socket的选项
    int getsockopt(int level,
                   int optname,
                   void* optval,
                   socklen_t* optlen)  const;

    //设置Socket的选项
    int setsockopt(int level,
                   int optname,
                   const void* optval,
                   int optlen) const;

    //取得对端的地址信息
    int getpeername(zce::skt::addr_base* addr)  const;

    //取得本地的地址信息
    int getsockname(zce::skt::addr_base* addr)  const;

    //需要说明的是，UDP也可以用connect函数（UDP的connect并不发起握手,只是记录通信地址），
    //然后可以直接调用send or recv，而不明确要通信的地址
    //所以将connect，send，recv 3个函数放到了base里面，大家都可以使用

    //connect某个地址
    int connect(const zce::skt::addr_base* addr) const;

    //接受数据，根据阻塞状态决定行为
    ssize_t recv(void* buf,
                 size_t len,
                 int flags = 0) const;

    //发送数据，根据阻塞状态决定行为
    ssize_t send(const void* buf,
                 size_t len,
                 int flags = 0) const;

protected:

    //SOCKET句柄，LINUX下是int，文件句柄，WINDOWS是一个SOCKET类型，
    //其实就是一个WINDOWS句柄，也就是指针
    ZCE_SOCKET     socket_handle_;
};
}
