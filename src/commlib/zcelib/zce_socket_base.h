#ifndef ZCE_LIB_SOCKET_BASE_H_
#define ZCE_LIB_SOCKET_BASE_H_

//SOCKET地址的基类
class ZCE_Sockaddr;
class ZCE_Time_Value;

//SOCKET的基类
class ZCELIB_EXPORT ZCE_Socket_Base
{
protected:

    //SOCKET句柄，LINUX下是int，文件句柄，WINDOWS是一个SOCKET类型，其实就是一个WINDOWS句柄，也就是指针
    ZCE_SOCKET     socket_handle_;

    //ZCE_Socket_Base不会提供给外部用，所以不准用
protected:

    //构造函数，析构函数
    ZCE_Socket_Base();
    explicit ZCE_Socket_Base(const ZCE_SOCKET &socket_hanle);
    ~ZCE_Socket_Base();

public:

    //设置句柄
    void set_handle(const ZCE_SOCKET &socket_hanle);
    //获取句柄
    ZCE_SOCKET get_handle() const;

    //Open SOCK句柄，不BIND本地地址的方式
    int open(int type,
             int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = true);

    //Open SOCK句柄，BIND本地地址的方式
    int open(int type,
             const ZCE_Sockaddr *local_addr,
             int protocol_family = AF_INET,
             int protocol = 0,
             bool reuse_addr = false);

    //关闭之
    int close();

    int bind(const ZCE_Sockaddr *add_name) const;

    //打开某些选项，WIN32目前只支持O_NONBLOCK
    int sock_enable (int value) const;

    //关闭某些选项，WIN32目前只支持O_NONBLOCK
    int sock_disable(int value) const;

    //获取Socket的选项
    int getsockopt (int level,
                    int optname,
                    void *optval,
                    socklen_t *optlen)  const;

    //设置Socket的选项
    int setsockopt (int level,
                    int optname,
                    const void *optval,
                    int optlen) const;

    //取得对端的地址信息
    int getpeername (ZCE_Sockaddr *addr)  const;

    //取得本地的地址信息
    int getsockname (ZCE_Sockaddr *addr)  const;

    //需要说明的是，UDP也可以用connect函数（UDP的connect并不发起握手,只是记录通信地址），然后可以直接调用send or recv，而不明确要通信的地址
    //所以将connect，send，recv 3个函数放到了base里面，大家都可以使用

    //connect某个地址
    int connect(const ZCE_Sockaddr *addr) const;

    //接受数据，根据阻塞状态决定行为
    ssize_t recv (void *buf,
                  size_t len,
                  int flags = 0) const;

    //发送数据，根据阻塞状态决定行为
    ssize_t send (const void *buf,
                  size_t len,
                  int flags = 0) const;

};

#endif //#ifndef ZCE_LIB_SOCKET_BASE_H_

