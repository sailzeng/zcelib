/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_socket.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年5月1日
* @brief      SOCKET操作的适配器层，主要还是向LINUX下靠拢
*
* @details    ZCE_SOCKET 在LINUX下就是int，文件描述符，
*             WINDOWS下是SOCKET，你可以认为他就是一个HANDLE，
*
* @note       2011 年 10月31日，做了体检回来改代码
*             将内部所有的很多timeval 换成ZCE_Time_Value;因为如果不换，其实上层写起来反而难看
*             2013 年 1月13日 深圳冬天的太阳暖意十足。
*             将所有的注释doxygen化，
*/

#ifndef ZCE_LIB_OS_ADAPT_SOCKET_H_
#define ZCE_LIB_OS_ADAPT_SOCKET_H_

#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_error.h"
#include "zce_time_value.h"

class ZCE_Time_Value;

namespace ZCE_OS
{

/*!
* @brief      创建一个SOCKET对象
* @return     ZCE_SOCKET 返回SOCKET句柄
* @param[in]  family   (地址)协议族，为AF_INET，AF_INET6，AF_UNSPEC等,协议族，按道理是和地址族一一对应的，所以PF_XXX,约等于AF_XXX
* @param[in]  type     是SOCK_DGRAM或者SOCK_STREAM，SOCK_RAW
* @param[in]  proto    协议类型，主要在原生SOCKET中使用，比如ICMP等，一般填写为0
*/
inline ZCE_SOCKET socket (int family,
                          int type,
                          int proto = 0);

/*!
* @brief      接收一个accept请求的socket
* @return     inline ZCE_SOCKET  返回接受的SOCKET句柄，返回句柄等于，ZCE_INVALID_SOCKET标识失败
* @param      handle             监听端口的句柄
* @param      addr               accept的SOCKET的地址信息，sockaddr的其实可以是各种协议族
* @param      addrlen            accept的SOCKET的地址长度
*/
inline ZCE_SOCKET accept (ZCE_SOCKET handle,
                          sockaddr *addr,
                          socklen_t *addrlen);

/*!
* @brief      绑定一个地址，主要用于监听端口，当然其他操作也可以用，本地端口（CONNECT出去的端口），一般不指定本地地址
* @return     int      0标识成功
* @param      handle   绑定操作的地址
* @param      addr     句柄要求绑定的的地址信息，sockaddr的其实可以是各种协议族
* @param      addrlen  句柄要求绑定的的地址长度
*/
int bind (ZCE_SOCKET handle,
          const struct sockaddr *addr,
          socklen_t addrlen);

/*!
* @brief      关闭句柄，为什么不定义close函数？为了在WIN下兼容，其没用用close这个函数，
* @return     int 0成功，-1失败
* @param      要关闭的句柄
*/
inline int closesocket (ZCE_SOCKET handle);

/*!
* @brief      shutdown，how参数向LINUX下靠齐SHUT_RD
* @return     int     关闭某单边的通道
* @param      handle  关闭单边通道的句柄
* @param      how     如何关闭，SHUT_RD,SHUT_WR,SHUT_RDWR
*/
inline int shutdown (ZCE_SOCKET handle,
                     int how);

/*!
* @brief      非标准函数，语法糖，打开某些选项，WIN32目前只支持O_NONBLOCK,
* @return     int     0成功，-1失败
* @param      handle  要设置的句柄，
* @param      value   选项值，WIN32目前只支持O_NONBLOCK，
*/
int sock_enable (ZCE_SOCKET handle, int value);

/*!
* @brief      关闭某些选项
* @return     int    0成功，-1失败
* @param      handle 要设置的句柄，
* @param      value  选项值，WIN32目前只支持O_NONBLOCK
*/
int sock_disable(ZCE_SOCKET handle, int value);

/*!
* @brief      WINDOWS 的SOCKET必须调用一下初始化WSAStartup
* @return     int 0成功，
* @param[in]  version_high  版本的高位
* @param[in]  version_low   版本的低位
* @note       在NT SP4以后，就支持2.2的版本了，我实在想不出理由，你要用更低的版本。
*/
int socket_init (int version_high = 2,
                 int version_low = 2);

/*!
* @brief      程序退出，DLL鞋砸时，关闭SOCKET（WinSock）的使用，Finalize WinSock after last use (e.g., when a DLL is unloaded).
* @return     int 0成功，-1失败
*/
int socket_fini (void);

/*!
* @brief      连接某个地址
* @return     int     0成功，-1失败
* @param[in]  handle  链接的的句柄
* @param[in]  addr    链接目标的地址信息
* @param[in]  addrlen 链接目标的地址长度
* @note
*/
inline int connect (ZCE_SOCKET handle,
                    const sockaddr *addr,
                    socklen_t addrlen);

/*!
* @brief      (TCP)取得对端的地址信息
* @return     int     0成功，-1失败
* @param      handle  取对端的地址的句柄
* @param[out] addr    返回的地址信息
* @param[out] addrlen 返回的地址长度
*/
inline int getpeername (ZCE_SOCKET handle,
                        sockaddr *addr,
                        socklen_t *addrlen);

/*!
* @brief      取得本地的地址信息，BIND的地址，或者是OS分配的本地地址
* @return     int     0成功，-1失败
* @param      handle  句柄ID
* @param[out] addr    返回的地址地址信息
* @param[out] addrlen 返回的地址长度
*/
inline int getsockname (ZCE_SOCKET handle,
                        sockaddr *addr,
                        socklen_t *addrlen);

/*!
* @brief      取得句柄的某个选项值
* @return     int     0成功，-1失败
* @param[in]  handle  要操作的句柄
* @param[in]  level   选项的级别
* @param[in]  optname 选项的名字，枚举定义
* @param[out] optval  取得选项数据
* @param[out] optlen  取得选项数据的长度
*/
inline int getsockopt (ZCE_SOCKET handle,
                       int level,
                       int optname,
                       void *optval,
                       socklen_t *optlen);

/*!
* @brief      设置句柄的某个选项值
* @return     int     0成功，-1失败
* @param[in]  handle  要操作的句柄
* @param[in]  level   选项的级别
* @param[in]  optname 选项的名字，枚举定义
* @param[in]  optval  设置选项数据
* @param[in]  optlen  设置选项数据的长度
* @note
*/
inline int setsockopt (ZCE_SOCKET handle,
                       int level,
                       int optname,
                       const void *optval,
                       socklen_t optlen);

/*!
* @brief      监听某个端口
* @return     int 0成功，-1失败
* @param      handle 监听端口的句柄
* @param      backlog 内核为次套接字排队的最大连接数量，同学们反馈此数值在大规模链接的服务器还是
*             挺有用的，如果规模较大，建议调整这个数值
*             同时请调整  /proc/sys/net/core/somaxconn 
*             /proc/sys/net/ipv4/tcp_max_syn_backlog
*/
inline int listen (ZCE_SOCKET handle,
                   int backlog = ZCE_DEFAULT_BACKLOG);

/*!
* @brief      接收数据，
* @return     ssize_t -1标识失败（失败原因从errno获得），0表示端口被断开，>0表示接收到的数据 数据长度
* @param[in]  handle  操作的句柄
* @param[out] buf     接收数据的buffer
* @param[out] len     接收数据的buffer的长度
* @param[in]  flags   接收操作的flag 默认为0，WIN和LINUX并不通用，下面列出常用的一些
*                     MSG_WAITALL WIN，LINUX都有，会等待接受指定长度的数据，才返回
*                     MSG_DONTWAIT LINUX 特有。相当于非阻塞接收
*                     MSG_OOB WIN，LINUX都有，处理带外数据的，但估计也没人用的着
*/
inline ssize_t recv (ZCE_SOCKET handle,
                     void *buf,
                     size_t len,
                     int flags = 0);

/*!
* @brief      发送数据，
* @return     ssize_t -1表示失败（失败原因从errno获得），0表示端口被断开，>0表示发送的的数据长度
* @param[in]  handle  操作的句柄
* @param[in]  buf     发送数据的buffer
* @param[in]  len     发送数据的buffer的长度
* @param[in]  flags   发送操作的flag,各平台不太通用，
*                     MSG_DONTWAIT,LINUX特有，相当于非阻塞发送
*/
inline ssize_t send (ZCE_SOCKET handle,
                     const void *buf,
                     size_t len,
                     int flags = 0);

/*!
* @brief      UDP接收
* @return     ssize_t  -1表示失败（失败原因从errno获得），0表示端口被断开，>0表示接收到的数据 数据长度
* @param[in]  handle   操作的句柄
* @param[out] buf      接收数据的buffer
* @param[out] len      接收数据的buffer的长度
* @param[in]  flags    接受操作的flag 默认为0，MSG_DONTWAI LINUX支持，相当于非阻塞接收
* @param[out] from     数据来源的地址，from可以和from_len一起为NULL,表示不关心从哪儿来的数据
* @param[out] from_len 数据来源地址的长度
*/
inline ssize_t recvfrom (ZCE_SOCKET handle,
                         void *buf,
                         size_t len,
                         int flags,
                         sockaddr *from,
                         socklen_t *from_len);

/*!
* @brief      UDP发送
* @return     ssize_t  -1表示失败（失败原因从errno获得），0表示端口被断开，>0表示接收到的数据 数据长度
* @param[in]  handle  操作的句柄
* @param[in]  buf     发送数据的buffer
* @param[in]  len     发送数据的buffer的长度
* @param[in]  flags   发送操作的flag 默认为0，MSG_DONTWAI LINUX支持，相当于非阻塞发送
* @param[in]  addr    目标的地址
* @param[in]  addrlen 目标的地址的长度
*/
inline ssize_t sendto (ZCE_SOCKET handle,
                       const void *buf,
                       size_t len,
                       int flags,
                       const struct sockaddr *addr,
                       socklen_t addrlen);

//--------------------------------------------------------------------------------------------
//下面两个函数是非标准函数，并且并不会刻意阻塞
//两个函数并不在意句柄是阻塞还是非阻塞，但两种情况下表现会大相径庭，一个阻塞，一个可能立即返回错误
//如果要进行超时处理，请使用recv_n,send_n

/*!
* @brief      尽量收取len长的数据，直到出现错误,
* @return     ssize_t -1表示失败（失败原因从errno获得），0表示端口被断开，>0表示接收到的数据 数据长度
* @param[in]  handle  操作的句柄，
* @param[out] buf     接收数据的buffer
* @param[out] len     接收数据的buffer的长度
* @param[in]  flags   接收操作的flag
* @note               如果句柄本来就是阻塞的,会阻塞
*                     如果句柄非阻塞操作，可能会立即错误，errno被置为EWOULDBLOCK
*/
ssize_t recvn (ZCE_SOCKET handle,
               void *buf,
               size_t len,
               int flags = 0);

/*!
* @brief      尽量发送N个数据，直到出现错误
* @return     ssize_t -1表示失败（失败原因从errno获得），0表示端口被断开，>0表示接收到的数据 数据长度
* @param[in]  handle  操作的句柄
* @param[in]  buf     发送数据的buffer
* @param[in]  len     发送数据的buffer的长度
* @param[in]  flags   发送操作的flag,各平台不太通用，
*/
ssize_t sendn (ZCE_SOCKET handle,
               const void *buf,
               size_t len,
               int flags = 0);

//--------------------------------------------------------------------------------------------

/*!
* @brief      is_ready_fds用于帮助你逃避该死的select的代码编写麻烦。我认为select是一个设计的相当失败的API
*             此函数用于帮助检查一个fd_set中间的句柄是否ready了，如果这个fd ready了，然后true，否则是false
*             你可以用一个for循环调用is_ready_fds函数，轮流检查各个句柄是否ready
* @return     bool     如果检查的no_fds下标的句柄已经ready，就返回true，否则返回false
* @param[in]  no_fds   no_fds是检查的序号，小于nfds，
* @param[in]  out_fds  就是select的返回fd_set参数
* @param[out] ready_fd 如果已经ready了，ready_fd返回ready的句柄
*/
inline bool is_ready_fds(int no_fds,
                         const fd_set *out_fds,
                         ZCE_SOCKET *ready_fd);

/*!
* @brief      跨平台的select函数，在函数功能上尽量想LINUX下靠齐，
*             LINUX如果没有句柄检查的时候，相当于sleep，但WINDOWS却会返回错误，我们的封装统一向LINUX靠齐，
* @return        int         和select的返回一样，返回触发的句柄数量，-1表示出现问题
* @param[in]     nfds        最大的句柄ID，WINDOWS下可以填写为0（不为0好像还有告警），LINUX下为最大的句柄ID+1
* @param[in,out] readfds     进行读取触发检查的句柄集合
* @param[in,out] writefds    进行写入触发检查的句柄集合
* @param[in,out] exceptfds   进行异常触发检查的句柄集合，
* @param[in,out] timeout_tv  超时时间，注意这个参数是非const的，完成后返回剩余时间
* @note       UNIX 网络编程卷1中叙述timeout_tv参数是const的，而且举例很多，我也就相信了
*             但今天刘侃反馈说LINUX下的select 的timeout_tv不是const的，看了一下，真不是，
*             于是我面临两个选择，向谁看起的问题，最后考虑还是想LINUX看齐吧。
*/
inline int select(
    int nfds,
    fd_set *readfds,
    fd_set *writefds,
    fd_set *exceptfds,
    ZCE_Time_Value *timeout_tv
);

///用于handle_ready函数,handle_multi_ready函数
enum HANDLE_READY_TODO
{
    ///读事件
    HANDLE_READY_READ            = 0x1,

    ///写事件
    HANDLE_READY_WRITE           = 0x2,

    ///异常
    HANDLE_READY_EXCEPTION       = 0x3,

    ///ACCPET 事件，也是放在读的SET
    HANDLE_READY_ACCEPT          = 0x4,

    ///连接成功，我为连接成功做了多少努力，你看不见
    HANDLE_READY_CONNECTED       = 0x5,

};

/*!
* @brief         非标准函数，(在一个时间段内)看handle准备好用于干嘛没有，内部用select触发，用于某些对单个端口的单个事件处理，
* @return        int          返回值和select 一致，如果成功返回值是触发事件个数，如果事件触发一般触发返回值都是1
* @param[in]     handle       等待触发的句柄
* @param[in,out] timeout_tv   超时的时间
* @param[in]     ready_todo   等待什么事件（只能单个），@ref HANDLE_READY_TODO
* @note          特别提醒，只能处理单个事件喔，不能同时处理读写,
*/
int handle_ready (ZCE_SOCKET handle,
                  ZCE_Time_Value *timeout_tv,
                  HANDLE_READY_TODO ready_todo);



//--------------------------------------------------------------------------------------------
//带有超时处理的一组函数，根据timeout_tv确定是否进行超时处理


/*!
* @brief         接收数据，接收len长的数据或者超时后返回，(也或者一致等待，或者立即返回)
* @return        ssize_t    -1表示失败（失败原因从errno获得），0表示端口被断开，>0表示接收到的数据 数据长度
* @param[in]     handle     操作的句柄，WINDOWS下要求句柄是阻塞状态的
* @param[out]    buf        接收数据的buffer
* @param[out]    len        指定接收数据的长度
* @param[in,out] timeout_tv 超时时长，相对时长，timeout_tv !=NULL标识超时处理，超时是用SELECT函数进行的处理，
*                           timeout_tv ==NULL 标识不进行超时处理,根据Socket状态自己决定，返回剩余的时间
* @param[in]     flags      接收的flag,参考@ref recv
* @note
*/
inline ssize_t recv_n (ZCE_SOCKET handle,
                       void *buf,
                       size_t len,
                       ZCE_Time_Value *timeout_tv,
                       int flags = 0);

/*!
* @brief         发送数据，发送len长的数据或者超时后返回，(也或者一致等待，或者立即返回)
* @return        ssize_t    -1表示失败（失败原因从errno获得），0表示端口被断开，>0表示发送的数据 数据长度
* @param[in]     handle     操作的句柄，WINDOWS下要求句柄是阻塞状态的
* @param[in]     buf        发送数据的buffer
* @param[in]     len        指定发送数据的长度
* @param[in,out] timeout_tv 超时时长，相对时长，timeout_tv !=NULL标识超时处理,超时是用SELECT函数进行的处理，
*                           timeout_tv ==NULL 标识不进行超时处理,根据Socket状态自己决定,返回剩余的时间
* @param[in]     flags      发送的flag，参考@ref send
*/
inline ssize_t send_n (ZCE_SOCKET handle,
                       const void *buf,
                       size_t len,
                       ZCE_Time_Value *timeout_tv,
                       int flags = 0);

/*!
* @brief      接收UDP数据，接收到一个的数据包或者超时后返回，(也或者一致等待，或者立即返回)
* @return     ssize_t  -1表示失败（失败原因从errno获得），0表示端口被断开，>0表示接收的数据 数据长度
* @param[in]     handle     操作的句柄，WINDOWS下要求句柄是阻塞状态的
* @param[out]    buf        接收数据的buffer
* @param[out]    len        接收数据的buffer的长度
* @param[out]    from       数据来源的地址，from可以和from_len一起为NULL,表示不关心从哪儿来的数据
* @param[out]    from_len   数据来源地址的长度
* @param[in,out] timeout_tv 超时时长，相对时长，timeout_tv !=NULL标识超时处理,超时是用SELECT函数进行的处理，
*                           timeout_tv ==NULL 标识不进行超时处理,根据Socket状态自己决定,返回剩余的时间
* @param[in]     flags      接收操作的flag 参考@ref recv
* @note
*/
inline ssize_t recvfrom (ZCE_SOCKET handle,
                         void *buf,
                         size_t len,
                         sockaddr *from,
                         socklen_t *from_len,
                         ZCE_Time_Value *timeout_tv,
                         int flags = 0);

/*!
* @brief      UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
*             发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理，
* @return     ssize_t    -1表示失败（失败原因从errno获得），0表示端口被断开，>0表示发送的数据 数据长度
* @param[in]  handle     操作的句柄
* @param[in]  buf        发送数据的buffer
* @param[in]  len        发送数据的buffer的长度
* @param[in]  addr       目标的地址
* @param[in]  addrlen    目标的地址的长度
* @param[in]  timeout_tv 此参数没有意义
* @param[in]  flags      接收操作的flag 参考@ref send
* @note
*/
inline ssize_t sendto (ZCE_SOCKET handle,
                       const void *buf,
                       size_t len,
                       const sockaddr *addr,
                       int addrlen,
                       ZCE_Time_Value *timeout_tv = NULL,
                       int flags = 0);

//==================================================================================================
///因为WINdows 不支持取得socket 是否是阻塞的模式，所以Windows 下我无法先取得socket的选项，
///然后判断是否取消阻塞模式所以请你务必保证你的Socket 是阻塞模式的，否则有问题
///一组带有超时的SOCKET处理函数，内部使用使用select作为超时函数处理
//超时的时间参数必须填写



int connect_timeout(ZCE_SOCKET handle,
                    const sockaddr *addr,
                    socklen_t addrlen,
                    ZCE_Time_Value &timeout_tv);


/*!
* @brief      接收数据，接收len长的数据或者超时后返回，除了timeout_tv参数，清参考@ref recv_n
*             recvn_timeout 和 recvn_n 的区别是recvn_n 如果超时参数为NULL，可能立即返回或者一致阻塞等待
*             内部超时用select 实现
* @param      timeout_tv 等待的时间参数，引用值，你必须填写一个数值
*/
ssize_t recvn_timeout (ZCE_SOCKET handle,
                       void *buf,
                       size_t len,
                       ZCE_Time_Value &timeout_tv,
                       int flags = 0);

/*!
* @brief      发送数据，发送len长的数据或者超时后返回，除了timeout_tv参数，清参考@ref sendv_n
*             sendn_timeout 和 sendv_n 的区别是recvn_n 如果超时参数为NULL，可能立即返回或者一致阻塞等待
*             而sendn_timeout 的超时参数必须填写
*             内部超时用select 实现
* @param      timeout_tv 等待的时间参数，引用值，你必须填写一个数值
*/
ssize_t sendn_timeout(ZCE_SOCKET handle,
                      const void *buf,
                      size_t len,
                      ZCE_Time_Value &timeout_tv,
                      int flags = 0);

/*!
* @brief      接收UDP数据，接收到一个的数据包或者超时后返回，请参考@ref recvfrom
* @param      timeout_tv 等待的时间参数，引用值，你必须填写一个数值
*/
ssize_t recvfrom_timeout (ZCE_SOCKET handle,
                          void *buf,
                          size_t len,
                          sockaddr *from,
                          socklen_t *from_rlen,
                          ZCE_Time_Value &timeout_tv,
                          int flags = 0);

/*!
* @brief      UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
*             发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理
*/
ssize_t sendto_timeout (ZCE_SOCKET handle,
                        const void *buf,
                        size_t len,
                        const sockaddr *addr,
                        int addrlen,
                        ZCE_Time_Value & /*timeout_tv*/,
                        int flags = 0);

//==================================================================================================
//这组函数提供仅仅为了代码测试，暂时不对外提供
//这组函数是使用SO_RCVTIMEO，SO_SNDTIMEO得到一组超时处理函数

/*!
* @brief      接收数据，接收len长的数据或者超时后返回，除了timeout_tv参数，清参考@ref recv_n
*             recvn_timeout2 和 recvn_timeout 的区别是带recvn_timeout2的名字函数是使用是SO_RCVTIMEO选项进行超时处理，
*             记时不是特别准确，
* @param      timeout_tv 等待的时间参数，引用值，你必须填写一个数值
*/
ssize_t recvn_timeout2 (ZCE_SOCKET handle,
                        void *buf,
                        size_t len,
                        ZCE_Time_Value &timeout_tv,
                        int flags = 0);

/*!
* @brief      发送数据，发送len长的数据或者超时后返回，除了timeout_tv参数，请参考@ref sendv_n
*             sendn_timeout2 和 sendn_timeout 的区别是带sendn_timeout2的名字函数是使用是SO_SNDTIMEO选项进行超时处理，
*             记时不是特别准确，
* @param      timeout_tv 等待的时间参数，引用值，你必须填写一个数值
*/
ssize_t sendn_timeout2 (ZCE_SOCKET handle,
                        void *buf,
                        size_t len,
                        ZCE_Time_Value &timeout_tv,
                        int flags = 0);

/*!
* @brief      收UDP的数据,也带有超时处理，但是是收到多少数据就是多少了，超时用SO_RCVTIMEO实现
*             其他参数返回值参考recvfrom，
*/
ssize_t recvfrom_timeout2 (ZCE_SOCKET handle,
                           void *buf,
                           size_t len,
                           sockaddr *from,
                           socklen_t *from_rlen,
                           ZCE_Time_Value &timeout_tv,
                           int flags = 0);

/*!
* @brief      UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
*             发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理
*             其他参数返回值参考sendto，
*/
ssize_t sendto_timeout2 (ZCE_SOCKET handle,
                         const void *buf,
                         size_t len,
                         const sockaddr *addr,
                         int addrlen,
                         ZCE_Time_Value & /*timeout_tv*/,
                         int flags = 0);

//--------------------------------------------------------------------------------------------
//一次操作一组数据的的readv，writev函数

/*!
* @brief      一次读取一组数据，WINDOWS下用，WSARecv模拟，
*             readv这种函数表面看起来可能比较一些数据的分节处理，但由于没有超时等辅助，也未必真好用
* @return     ssize_t 读取数据的总长度
* @param[in]     handle  操作的句柄
* @param[in,out] iov     读取的数据数组，iovec里面有 buf，和len，
* @param[in]     iovlen  数组长度
*/
ssize_t readv (ZCE_SOCKET handle,
               iovec *iov,
               int iovlen);

/*!
* @brief      一次写入一组数据，WINDOWS下用，WSASend模拟，
* @return     ssize_t 发送数据的总长度
* @param[in]  handle  操作的句柄
* @param[in]  iov     写的数据数组，iovec里面有 buf，和len，
* @param[in]  iovcnt  数组长度
* @note
*/
ssize_t writev (ZCE_SOCKET handle,
                const iovec *iov,
                int iovcnt);

///sendmsg
ssize_t sendmsg (ZCE_SOCKET handle,
                 const struct msghdr *msg,
                 int flags);
///recvmsg
ssize_t recvmsg (ZCE_SOCKET handle,
                 msghdr *msg,
                 int flags);

//--------------------------------------------------------------------------------------------

//IP地址的几种长度,注意注意注意注意注意 这些长度没有计算结尾的'\0'
//           12345678901234567890123456789012345678901234567890
//IPV4       202.202.202.202                                      15
//IPV4+Port  202.202.202.202#36010                                21
//IPV6       102A:F02B:A1F2:745B:112D:1241:1834:124A              39
//IPV4+Port  102A:F02B:A1F2:745B:112D:1241:1834:124A#40012        45

/*!
* @brief      函数将addrptr的地址信息转换为地址字符串，
*             inet_ntop,inet_pton 这两个函数在LINUX下有，
*             在VISTA的SOCKET函数里面得到了支持，但是我们只能自己模拟了,
* @return     const char* 得到的地址字符串
* @param[in]  family      协议族
* @param[in]  addrptr     地址
* @param[out] strptr      返回的的地址字符串
* @param[in]  len         字符串长度
*/
const char *inet_ntop (int family,
                       const void *addrptr,
                       char *strptr,
                       size_t len);

/*!
* @brief      转换字符串到网络地址，第一个参数af是地址族，转换后存在addrptr中，
* @return     int  注意这个函数返回1标识成功，返回0表示失败，尊重原来的设计（但这个设计也太）
* @param      family 协议族
* @param      strptr 地址的字符传标识
* @param      addrptr 转换得到的地址
* @note       注意返回值
*/
int inet_pton (int family,
               const char *strptr,
               void *addrptr);

/*!
* @brief      将IPV4的整数的地址信息打印出来，第一个参数是网络序，非标准函数，但是重入安全
* @return     const char* 得到的地址字符串
* @param[in]  in          IPV4网络地址，网络序
* @param[out] addr_buf    返回的的地址字符串
* @param[in]  addr_size   字符串长度
*/
inline const char *inet_ntoa(uint32_t in,
                             char *addr_buf,
                             size_t addr_size);

/*!
* @brief      通过IPV4的地址，得到对应的字符串表示，不可重入函数，不太建议使用
*             这两个函数WIN32和LINUX都有
* @return     inline const char*
* @param      in  IPV4的地址，
*/
inline const char *inet_ntoa(struct in_addr in);

/*!
* @brief      通过一个IP地址的字符串表示得到一个IPV4的的整数
* @return     inline uint32_t 返回的地址数值，网络线
* @param      cp 地址字符串
*/
inline uint32_t inet_addr(const char *cp);

/*!
* @brief      输出IP地址信息，内部是不使用静态变量，线程安全，BUF长度IPV4至少长度>15.IPV6至少长度>39
* @return     const char*  得到的地址字符串
* @param      sock_addr    sockaddr 地址描述
* @param      str_ptr      返回的的地址字符串
* @param      str_len      字符串长度
*/
const char *socketaddr_ntop(const sockaddr *sock_addr,
                            char *str_ptr,
                            size_t str_len);

/*!
* @brief      socketaddr_ntop_ex 输出IP地址信息和端口号,端口号
*             输出IP地址信息以及端口信息，内部是不使用静态变量，线程安全，BUF长度IPV4至少长度>21.IPV6至少长度>45
*             和socketaddr_ntop的区别就在于socketaddr_ntop_ex同时输出了端口号
*             参数参考 @ref socketaddr_ntop
*/
const char *socketaddr_ntop_ex(const sockaddr *sock_addr,
                               char *str_ptr,
                               size_t str_len);

//======================================================================================================
//域名解析，转换IP地址的几个函数
//基础函数部分，第一部分函数详细间《UNIX网络编程》卷1的第9章

/*!
* @brief      根据域名解析得到对应的IP地址，这个函数已经不是特别推荐，因为种种不如意，
*             比如不能重入，如果一个域名同时有IPV4和IPV6，那么就只有返回IPV4，
*             推荐使用getaddrinfo， gethostbyname的使用可以参考gethostbyname_inary函数
* @return     hostent*  返回的IP地址信息
* @param      hostname  查询的域名
*/
hostent *gethostbyname(const char *hostname);

/*!
* @brief      GNU extensions ,根据协议簇，得到解析的地址
* @return     hostent*
* @param      hostname
* @param      af
*/
hostent *gethostbyname2(const char *hostname,
                        int af);

//相对而言，下面这2个函数更加使用更加简单一点，

/*!
* @brief      非标准函数,得到某个域名的IPV4的地址数组，使用起来比较容易和方便，内部使用gethostbyname
* @return        int           0成功，-1失败
* @param[in]     hostname      域名
* @param[in]     service_port  端口号，会和查询到的地址信息一起设置到sockaddr_in
* @param[in,out] ary_addr_num  输入时数组的长度，输出返回实际获得的数组长度
* @param[out]    ary_sock_addr 域名+端口对应的sockaddr_in 数组信息
*/
int gethostbyname_inary(const char *hostname,
                        uint16_t service_port,
                        size_t *ary_addr_num,
                        sockaddr_in ary_sock_addr[]);

/*!
* @brief      非标准函数,得到某个域名的IPV6的地址数组，使用起来比较容易和方便
* @return        int            0成功，-1失败
* @param[in]     hostname       域名
* @param[in]     service_port   端口号，会和查询到的地址信息一起设置到sockaddr_in
* @param[in,out] ary_addr6_num  输入时数组的长度，输出返回实际获得的地址个数
* @param[out]    ary_sock_addr6 域名+端口对应的sockaddr_in6 数组
* @note
*/
int gethostbyname_in6ary(const char *hostname,
                         uint16_t service_port,
                         size_t *ary_addr6_num,
                         sockaddr_in6 ary_sock_addr6[]);

/*!
* @brief      通过IP地址信息，反查域名，不可重入，不推荐使用，推荐使用getnameinfo
* @return     hostent* 查询到的域名
* @param[in]  addr     指向地址的指针，注意地址是in_addr，in6_addr等等
* @param[in]  len      地址的长度
* @param[in]  family   地址协议族
*/
hostent *gethostbyaddr(const void *addr,
                       socklen_t len,
                       int family);

/*!
* @brief      非标准函数，通过IPV4地址取得域名
* @return     int        0成功，-1失败
* @param[in]  sock_addr  IPV4的sockaddr
* @param[out] host_name  返回的域名信息
* @param[int] name_len   域名buffer的长度
*/
int gethostbyaddr_in(const sockaddr_in *sock_addr,
                     char *host_name,
                     size_t name_len);

/*!
* @brief      非标准函数，通过IPV6地址取得域名
* @return     int        0成功，-1失败
* @param[in]  sock_addr6 IPV6的sockaddr
* @param[out] host_name  返回的域名信息
* @param[int] name_len   域名buffer的长度
*/
int gethostbyaddr_in6(const sockaddr_in6 *sock_addr6,
                      char *host_name,
                      size_t name_len);

//几个更加高级一些的的函数，Posix 1g的函数，推荐使用他们，他们可能可以重入（要看底层实现，如果底层就是gethostbyname，那么……）
//高级部分函数详细间《UNIX网络编程》卷1的第11章

/*!
* @brief      通过域名得到服务器地址信息，可以同时得到IPV4，和IPV6的地址
* @return     int        0成功，非0表示失败，以及错误ID
* @param[in]  hostname   域名信息
* @param[in]  service    服务的名称，比如"http","ftp"等，用于决定服务的端口号
* @param[in]  hints      调用者所需要的地址的信息
* @param[out] result     返回的结果数组
*/
int getaddrinfo( const char *hostname,
                 const char *service,
                 const addrinfo *hints,
                 addrinfo **result );

/*!
* @brief      释放getaddrinfo得到的结果
* @param      result  getaddrinfo返回的结果
*/
void freeaddrinfo(struct addrinfo *result);

/*!
* @brief         非标准函数,得到某个域名的IPV4的地址数组，使用起来比较容易和方便,底层使用getaddrinfo
* @return        int           0成功，-1失败
* @param[in]     hostname      域名
* @param[in]     service_port  端口号，会和查询到的地址信息一起设置到sockaddr_in
* @param[in,out] ary_addr_num  输入时数组的长度，输出返回实际获得的数组长度
* @param[out]    ary_sock_addr 域名+端口对应的sockaddr_in 数组信息
*/
int getaddrinfo_inary(const char *hostname,
                      uint16_t service_port,
                      size_t *ary_addr_num,
                      sockaddr_in ary_sock_addr[]);

/*!
* @brief      非标准函数,得到某个域名的IPV6的地址数组，使用起来比较容易和方便,底层使用getaddrinfo
* @return        int            0成功，-1失败
* @param[in]     hostname       域名
* @param[in]     service_port   端口号，会和查询到的地址信息一起设置到sockaddr_in
* @param[in,out] ary_addr6_num  输入时数组的长度，输出返回实际获得的地址个数
* @param[out]    ary_sock_addr6 域名+端口对应的sockaddr_in6 数组
*/
int getaddrinfo_in6ary(const char *hostname,
                       uint16_t service_port,
                       size_t *ary_addr6_num,
                       sockaddr_in6 ary_sock_addr6[]);

/*!
* @brief      通过IP地址信息，反查域名.服务名，（可能）可以重入函数（要看底层实现），
* @return     int   0成功，非0表示失败，以及错误ID
* @param[in]  sa      socket地址的指针，通过这个查询域名
* @param[in]  salen   scket地址的长度
* @param[out] host    返回的域名
* @param[in]  hostlen 域名的buffer的长度
* @param[out] serv    返回的服务名称buffer
* @param[in]  servlen 服务名称buffer的长度
* @param[in]  flags   flags 参数，可以使用的值包括
*                     NI_NOFQDN        对于本地主机, 仅返回完全限定域名的节点名部分.比如bear.qq.com，返回点bear，而不返回.qq.com
*                     NI_NUMERICHOST  host参数返回数字的IP地址信息信息
*                     NI_NAMEREQD     如果IP地址不能解析为域名，返回一个错误
*                     NI_NUMERICSERV  serv参数返回数值的字符串信息
*                     NI_DGRAM        服务基于数据报而非基于流，（比如同一个端口号，UDP和TCP的服务类型是不一样的）
*/
int getnameinfo(const struct sockaddr *sa,
                socklen_t salen,
                char *host,
                size_t hostlen,
                char *serv,
                size_t servlen,
                int flags);

/*!
* @brief      非标准函数，通过IPV4地址取得域名
* @return     int  0成功，非0表示失败，以及错误ID
* @param[in]  sock_addr  IPV4的sockaddr
* @param[out] host_name  返回的域名信息
* @param[int] name_len   域名buffer的长度
*/
int getnameinfo_in(const sockaddr_in *sock_addr,
                   char *host_name,
                   size_t name_len);

/*!
* @brief      非标准函数，通过IPV6地址取得域名
* @return     int  0成功，非0表示失败，以及错误ID
* @param[in]  sock_addr6 IPV6的sockaddr
* @param[out] host_name  返回的域名信息
* @param[int] name_len   域名buffer的长度
*/
int getnameinfo_in6(const sockaddr_in6 *sock_addr6,
                    char *host_name,
                    size_t name_len);

/*!
* @brief      返回当前机器的主机名称,2个平台应该都支持这个函数
* @return     int        0成功
* @param[out] name       返回的主机名称，
* @param[in]  name_len   主机名称的buffer长度
*/
inline int gethostname(char *name, size_t name_len);

//-------------------------------------------------------------------------------------
//为sockaddr_in增加的一组函数，方便使用,

//本来设置函数为什么要返回sockaddr *，方便直接作为参数在大量函数使用，但如果错误呢，，，，


/*!
* @brief
* @return     int 错误返回-1，正确返回0
* @param[out] sock_addr_ipv4 被设置的IPV4地址
* @param[in]  ipv4_addr_str  地址信息字符串
* @param[in]  ipv4_port      端口，本地序
*/
inline int set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                           const char *ipv4_addr_str,
                           uint16_t ipv4_port);

/*!
* @brief      设置一个IPV4的地址，如果字符串里面有端口信息，也会同时设置端口
* @return     int 错误返回-1，正确返回0
* @param      sock_addr_ipv4 被设置的IPV4地址
* @param      ipv4_addr_str  地址字符串,如果里面有字符'#'，会认为有端口号，
*/
inline int set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                           const char *ipv4_addr_str);

/*!
* @brief      设置一个IPV4的地址
* @return     int 错误返回-1，正确返回0
* @param[out] sock_addr_ipv4 被设置的IPV4地址
* @param[in]  ipv4_addr_val  表示IPV4的32位整数，本地序
* @param[in]  ipv4_port      端口号，本地序
* @note
*/
inline int set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                           uint32_t ipv4_addr_val,
                           uint16_t ipv4_port );



/*!
* @brief      设置一个IPV6的地址,
* @return     int 错误返回-1， 正确返回0
* @param[out] sock_addr_ipv6 被设置的IPV6地址
* @param[in]  ipv6_addr_str  IPV6地址信息字符串，这种格式"0:0:0:0:0:0:0:0"
* @param[in]  ipv6_port      端口号，本地序
*/
inline int set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                            const char *ipv6_addr_str ,
                            uint16_t ipv6_port);


/*!
* @brief      设置一个IPV4的地址，如果字符串里面有端口信息，也会同时设置端口
* @return     int 错误返回-1，正确返回0
* @param      sock_addr_ipv6 被设置的IPV6地址
* @param      ipv6_addr_str  地址字符串,如果里面有字符'#'，会认为有端口号，
*/
inline int set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                            const char *ipv6_addr_str);

/*!
* @brief      设置一个IPV6的地址,和上一个函数的区别主要在参数顺序上，注意
* @return     int   错误返回-1，正确返回0
* @param[out] sock_addr_ipv6 被设置的IPV6地址
* @param[in]  ipv6_port      端口号，本地序
* @param[in]  ipv6_addr_val  16个字节的地址信息
*/
inline int set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                            uint16_t ipv6_port,
                            const char ipv6_addr_val[16]);

//下面一些是针对IPV4的函数

//返回端口号,用指针作为参数主要是希望统一
inline uint16_t get_port_number(const sockaddr_in *sock_addr_ipv4);

//返回地址信息
inline const char *get_host_addr(const sockaddr_in *sock_addr_ipv4,
                                 char *addr_buf,
                                 size_t addr_size);

//得到IP地址和端口信息的字符串描述
inline const char *get_host_addr_port(const sockaddr_in *sock_addr_ipv4,
                                      char *addr_buf,
                                      size_t addr_size);

/*!
* @brief      返回IP地址的整数,本地序
* @return     inline uint32_t
* @param      sock_addr_ipv4
* @note
*/
inline uint32_t get_ip_address(const sockaddr_in *sock_addr_ipv4);

/*!
* @brief      检查一个地址是否是内网地址,这个还真得扫盲一下，IPV6没有局域网这个概念。
* @return     bool true是内网地址，false不是，
* @param      sock_addr_ipv4 判断的sockaddr_in
*/
bool is_internal(const sockaddr_in *sock_addr_ipv4);



/*!
* @brief      检查一个IPV4地址（整数）是否是内网地址,
* @return     bool ture是内网地址，false不是，
* @param[in]  ipv4_addr_val 整数表示IPV4地址，本地序
*/
bool is_internal(uint32_t ipv4_addr_val);

//uint32_t inet_lnaof(struct in_addr in); uint32_t inet_netof(struct in_addr in);
//这两个函数用于得到各类地址的网络ID和主机ID，我觉得基本没人会用这个函数吧。算了。

//-------------------------------------------------------------------------------------
//IPV4和IPV6之间相互转换的函数，都是非标准函数，

/*!
* @brief      将一个IPV4的地址映射为IPV6的地址
* @return     int  0成功，-1失败
* @param      src  要进行转换的in_addr
* @param      dst  转换得到IPV6的地址in6_addr
*/
int inaddr_map_inaddr6(const in_addr *src, in6_addr *dst);

/*!
* @brief      将一个IPV4的Sock地址映射为IPV6的地址
* @return     int  0成功，-1失败
* @param      src
* @param      dst
*/
int sockin_map_sockin6(const sockaddr_in *src, sockaddr_in6 *dst);

/*!
* @brief      判断一个地址是否是IPV4映射的地址
* @return     bool TRUE是映射过滤的，FALSE不是
* @param      in6
*/
bool is_in6_addr_v4mapped(const in6_addr *in6);

/*!
* @brief      如果一个IPV6的地址从IPV4映射过来的，转换回IPV4的地址
* @return     int  0成功，-1失败
* @param      src
* @param      dst
*/
int mapped_in6_to_in(const in6_addr *src, in_addr *dst);

/*!
* @brief      如果一个IPV6的socketaddr_in6地址从IPV4映射过来的，转换回IPV4的socketaddr_in地址
* @return     int  0成功，-1失败
* @param[in]  src  IPV6的socketaddr_in6
* @param[out] dst  转换成的IPV4的socketaddr_in
*/
int mapped_sockin6_to_sockin(const sockaddr_in6 *src, sockaddr_in *dst);

}

//-----------------------------------------------------------------------------------------

//详细解释一下为什么所有的Windows下的调用API都会有一段代码
//errno = ::WSAGetLastError ();
//因为Windows下其实有两个error记录，一个是C的errno，一个是::GetLastError,或者::WSAGetLastError ()
//返回的错误，而这两个error可能同时存在，以至于我们的代码很难写，而我们整体的上倾向用errno，因为Windows的::GetLastError
//和我们定义差别很大，所以我不得不这样处理一下，

//
inline ZCE_SOCKET ZCE_OS::socket (int family,
                                  int type,
                                  int proto)
{
    ZCE_SOCKET socket_hdl =::socket (family,
                                     type,
                                     proto);

#if defined (ZCE_OS_WINDOWS)

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if (ZCE_INVALID_SOCKET  == socket_hdl)
    {

        errno = ::WSAGetLastError ();
    }
    else
    {
        //关闭这个地方实验一下
        //已经初始化成功，并且是UDP ，去掉这一段
        //SIO_UDP_CONNRESET，是让你在向一个错误UDP peer发送一个数据后，你后收到一个RST错误
        if ( type == SOCK_DGRAM )
        {
            DWORD bytes_returned = 0;
            BOOL new_behavior = FALSE;
            DWORD status;
            status = WSAIoctl(socket_hdl, SIO_UDP_CONNRESET,
                              &new_behavior, sizeof(bytes_returned),
                              NULL, 0, &bytes_returned,
                              NULL, NULL);

            if (SOCKET_ERROR == status)
            {
                errno = ::WSAGetLastError ();
                //干点啥呢，不干啥，可能更好？
            }
        }
    }

#endif //

    return socket_hdl;
}

// 接收一个accept请求的socket
inline ZCE_SOCKET ZCE_OS::accept (ZCE_SOCKET handle,
                                  sockaddr *addr,
                                  socklen_t *addrlen)
{

#if defined (ZCE_OS_WINDOWS)

    ZCE_SOCKET accept_hdl = ::accept (handle,
                                      addr,
                                      addrlen);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if (ZCE_INVALID_SOCKET  == accept_hdl)
    {
        errno = ::WSAGetLastError ();
    }

    return accept_hdl;

#elif defined (ZCE_OS_LINUX)
    return ::accept (handle,
                     addr,
                     addrlen);
#endif
}

//绑定IP地址协议，，
inline int ZCE_OS::bind (ZCE_SOCKET handle,
                         const sockaddr *addr,
                         socklen_t addrlen)
{
#if defined (ZCE_OS_WINDOWS)

    int zce_result = ::bind (handle,
                             addr,
                             addrlen);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;
#elif defined (ZCE_OS_LINUX)
    return ::bind (handle,
                   addr,
                   addrlen);
#endif
}

//这个只能向Windows靠齐，因为我自己还有一个close函数，但windows下两个函数完全是两回事
inline int ZCE_OS::closesocket (ZCE_SOCKET handle)
{
#if defined ZCE_OS_WINDOWS
    int zce_result = ::closesocket (handle);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;
#elif defined (ZCE_OS_LINUX)
    return ::close(handle);
#endif
}

//连接服务器
inline int ZCE_OS::connect (ZCE_SOCKET handle,
                            const sockaddr *addr,
                            socklen_t addrlen)
{

#if defined ZCE_OS_WINDOWS
    int zce_result = ::connect (handle,
                                addr,
                                addrlen);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;
#elif defined (ZCE_OS_LINUX)
    return ::connect (handle,
                      addr,
                      addrlen);
#endif
}

//取得对端的地址信息
inline int ZCE_OS::getpeername (ZCE_SOCKET handle,
                                sockaddr *addr,
                                socklen_t *addrlen)
{

#if defined ZCE_OS_WINDOWS
    int zce_result = ::getpeername (handle,
                                    addr,
                                    addrlen);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;
#elif defined (ZCE_OS_LINUX)
    return ::getpeername (handle,
                          addr,
                          addrlen);
#endif
}

//取得对端的地址信息
inline int ZCE_OS::getsockname (ZCE_SOCKET handle,
                                sockaddr *addr,
                                socklen_t *addrlen)
{
#if defined ZCE_OS_WINDOWS

    int zce_result = ::getsockname (handle,
                                    addr,
                                    addrlen);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;
#elif defined (ZCE_OS_LINUX)
    return ::getsockname (handle,
                          addr,
                          addrlen);
#endif

}

//取得某个选项，注意WINDOWS和LINUX还是有很多的代码差异的，请注意
inline int ZCE_OS::getsockopt (ZCE_SOCKET handle,
                               int level,
                               int optname,
                               void *optval,
                               socklen_t *optlen)
{

#if defined (ZCE_OS_WINDOWS)

    int zce_result =  ::getsockopt (handle,
                                    level,
                                    optname,
                                    static_cast<char *>(optval),
                                    optlen);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;

#elif defined (ZCE_OS_LINUX)

    return ::getsockopt (handle,
                         level,
                         optname,
                         optval,
                         optlen);

#endif
}

inline int ZCE_OS::setsockopt (ZCE_SOCKET handle,
                               int level,
                               int optname,
                               const void *optval,
                               socklen_t optlen)
{

#if defined (ZCE_OS_WINDOWS)

    //Windows平台下，SO_REUSEPORT 和SO_REUSEADDR 效果一致，SO_REUSEADDR 是默认效果
    if (optname == SO_REUSEPORT)
    {
        optname = SO_REUSEADDR;
    }

    int zce_result = ::setsockopt ( handle,
                                    level,
                                    optname,
                                    static_cast<const char *>( optval),
                                    optlen);

#elif defined (ZCE_OS_LINUX)
    int zce_result = ::setsockopt ( handle,
                                    level,
                                    optname,
                                    optval,
                                    optlen);
#endif

    //统一错误返回
#if defined (ZCE_OS_WINDOWS)

    if (zce_result == SOCKET_ERROR )
    {
        if (errno == WSAEOPNOTSUPP || errno == WSAENOPROTOOPT)
        {
            errno = EOPNOTSUPP;
        }
        else
        {
            errno = ::WSAGetLastError ();
        }
    }

#endif

    return zce_result;

}

//打开一个监听端口
inline int ZCE_OS::listen (ZCE_SOCKET handle, int backlog)
{
#if defined (ZCE_OS_WINDOWS)

    int zce_result =  ::listen (handle, backlog);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;

#elif defined (ZCE_OS_LINUX)

    return ::listen (handle, backlog);
#endif

}

//shutdown，how参数向LINUX下靠齐,SHUT_RD,SHUT_WR,SHUT_RDWR
inline int ZCE_OS::shutdown (ZCE_SOCKET handle, int how)
{
#if defined (ZCE_OS_WINDOWS)

    int zce_result = ::shutdown (handle, how);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;

#elif defined (ZCE_OS_LINUX)

    return ::shutdown (handle, how);
#endif

}

//接收数据
//参数flags通用性非常差，LINUX，和WINDOWS下几乎不通用，LINUX下有一个比较有用的参数MSG_DONTWAIT，但是WINDOWS下不提供。
//阻塞错误请使用EWOULDBLOCK
inline ssize_t ZCE_OS::recv (ZCE_SOCKET handle, void *buf, size_t len, int flags)
{

#if defined (ZCE_OS_WINDOWS)
    int zce_result = ::recv (handle,
                             static_cast<char *>(buf),
                             static_cast<int> (len),
                             flags);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;

#elif defined (ZCE_OS_LINUX)

    ssize_t zce_result  = 0;
    zce_result = ::recv (handle, buf, len, flags);

    //统一错误成EWOULDBLOCK
# if (EAGAIN != EWOULDBLOCK)

    if (zce_result_ == -1 && errno == EAGAIN)
    {
        errno = EWOULDBLOCK;
    }

# endif //# if (EAGAIN != EWOULDBLOCK)

    return zce_result;
#endif
}

//发送数据
inline ssize_t ZCE_OS::send (ZCE_SOCKET handle,
                             const void *buf,
                             size_t len,
                             int flags)
{

#if defined (ZCE_OS_WINDOWS)

    int zce_result = ::send (handle,
                             static_cast<const char *>(buf),
                             static_cast<int> (len),
                             flags);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;

#elif defined (ZCE_OS_LINUX)

    ssize_t const zce_result = ::send (handle, buf, len, flags);

    //统一错误成EWOULDBLOCK,ACE曾经说过，在某些时候这个语句会失效，但是就我所知，在LINUX平台，EAGAIN == EWOULDBLOCK
    //这段代码仅仅作为一种防御
# if (EAGAIN != EWOULDBLOCK)

    if (zce_result == -1 && errno == EAGAIN)
    {
        errno = EWOULDBLOCK;
    }

# endif //# if (EAGAIN != EWOULDBLOCK)

    return zce_result;

#endif
}

inline ssize_t ZCE_OS::recvfrom (ZCE_SOCKET handle,
                                 void *buf,
                                 size_t len,
                                 int flags,
                                 sockaddr *from,
                                 socklen_t *from_len)
{

#if defined (ZCE_OS_WINDOWS)

    int const shortened_len = static_cast<int> (len);
    int const result = ::recvfrom ( handle,
                                    static_cast<char *>(buf),
                                    shortened_len,
                                    flags,
                                    from,
                                    from_len);

    if (result == SOCKET_ERROR)
    {
        errno = ::WSAGetLastError ();
        //MSG_PEEK的解释
        //Peek at the incoming data. The data is copied into the buffer, but is not removed from the input queue. This flag is valid only for non-overlapped sockets.

        //也就是锁如果有这个标志flags，发现消息过长的错误，返回消息接受缓冲长度给调用者
        if (errno == WSAEMSGSIZE && (flags & MSG_PEEK))
        {
            return shortened_len;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return result;
    }

#elif defined (ZCE_OS_LINUX)

    return ::recvfrom (handle,
                       buf,
                       len,
                       flags,
                       from,
                       from_len);

#endif
}

//
inline ssize_t ZCE_OS::sendto (ZCE_SOCKET handle,
                               const void *buf,
                               size_t len,
                               int flags,
                               const struct sockaddr *addr,
                               socklen_t addrlen)
{

#if defined (ZCE_OS_WINDOWS)

    int const zce_result = ::sendto (handle,
                                     static_cast<const char *>(buf),
                                     static_cast<int> (len),
                                     flags,
                                     addr,
                                     addrlen);

    //将错误信息设置到errno，详细请参考上面ZCE_OS名字空间后面的解释
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;
#elif defined (ZCE_OS_LINUX)

    return ::sendto (handle,
                     buf,
                     len,
                     flags,
                     addr,
                     addrlen);
#endif
}

//--------------------------------------------------------------------------------------------
//根据timeout_tv决定是否超时处理的TCP接收函数，
//timeout_tv !=NULL标识超时处理，接收N个字节后返回,超时用的是select那组超时函数
//timeout_tv ==NULL 标识不进行超时处理,根据Socket状态自己决定
inline ssize_t ZCE_OS::recv_n (ZCE_SOCKET handle,
                               void *buf,
                               size_t len,
                               ZCE_Time_Value *timeout_tv,
                               int flags)
{
    if (timeout_tv)
    {
        return ZCE_OS::recvn_timeout(handle,
                                     buf,
                                     len,
                                     *timeout_tv,
                                     flags);
    }
    else
    {
        return ZCE_OS::recvn(handle,
                             buf,
                             len,
                             flags);
    }
}

//根据timeout_tv决定是否超时处理的TCP发送函数
//timeout_tv !=NULL标识超时处理，发送N个字节后返回,超时用的是select那组超时函数 ,
//timeout_tv ==NULL 标识不进行超时处理,根据Socket状态自己决定,可能阻塞或者非阻塞,
inline ssize_t ZCE_OS::send_n (ZCE_SOCKET handle,
                               const void *buf,
                               size_t len,
                               ZCE_Time_Value *timeout_tv,
                               int flags)
{
    if (timeout_tv)
    {
        return ZCE_OS::sendn_timeout(handle,
                                     buf,
                                     len,
                                     *timeout_tv,
                                     flags);
    }
    else
    {
        return ZCE_OS::sendn(handle,
                             buf,
                             len,
                             flags);

    }
}

//根据timeout_tv决定是否超时处理的UDP接收函数，
//timeout_tv !=NULL标识超时处理，发送N个字节后返回,超时用的是select那组超时函数 ,
//timeout_tv ==NULL 标识不进行超时处理,根据Socket状态自己决定,可能阻塞或者非阻塞,
inline ssize_t ZCE_OS::recvfrom (ZCE_SOCKET handle,
                                 void *buf,
                                 size_t len,
                                 sockaddr *from,
                                 socklen_t *from_len,
                                 ZCE_Time_Value *timeout_tv,
                                 int flags)
{
    if (timeout_tv)
    {
        return ZCE_OS::recvfrom_timeout(handle,
                                        buf,
                                        len,
                                        from,
                                        from_len,
                                        *timeout_tv,
                                        flags);
    }
    else
    {
        return ZCE_OS::recvfrom(handle,
                                buf,
                                len,
                                flags,
                                from,
                                from_len);
    }
}

//UDP的发送暂时是不会阻塞的，不用超时处理，写这个函数完全是为了和前面对齐
//发送UDP的数据,带超时处理参数，但是实际上进行没有超时处理，
inline ssize_t ZCE_OS::sendto (ZCE_SOCKET handle,
                               const void *buf,
                               size_t len,
                               const sockaddr *addr,
                               int addrlen,
                               ZCE_Time_Value *timeout_tv,
                               int flags)
{
    if (timeout_tv)
    {
        return ZCE_OS::sendto_timeout(handle,
                                      buf,
                                      len,
                                      addr,
                                      addrlen,
                                      *timeout_tv,
                                      flags);
    }
    else
    {
        return ZCE_OS::sendto(handle,
                              buf,
                              len,
                              flags,
                              addr,
                              addrlen);
    }
}

//--------------------------------------------------------------------------------------------
//SELECT函数，为了和LINUX平台对齐，返回时间为剩余时间
inline int ZCE_OS::select(
    int nfds,
    fd_set *readfds,
    fd_set *writefds,
    fd_set *exceptfds,
    ZCE_Time_Value *timeout_tv
)
{
#if defined (ZCE_OS_WINDOWS)

    //检查是否没有句柄在等待
    bool no_handle_to_wait = false;

    if ( (NULL == readfds) ||  (NULL != readfds && 0 == readfds->fd_count ) )
    {
        if ( (NULL == writefds) || ( NULL != writefds && 0 == writefds->fd_count) )
        {
            if ( (NULL == exceptfds) || ( NULL != exceptfds && 0 == exceptfds->fd_count) )
            {
                no_handle_to_wait = true;
            }
        }
    }

    //如果不需要等待句柄，直接使用sleep替代select，
    //这是因为WINDOWS的select 必须等待一个句柄，否则就会return -1，看了
    if (no_handle_to_wait)
    {
        ZCE_OS::sleep(*timeout_tv);
        timeout_tv->set(0, 0);
        return 0;
    }

    clock_t start_clock = 0;
    timeval *select_tv = NULL;

    if (timeout_tv)
    {
        start_clock = std::clock();
        //有函数讲内部的timv_val取出来
        select_tv = (*timeout_tv);
    }

    //WINDOWS下，timeval是一个const 参数
    int ret =::select(
                 nfds,
                 readfds,
                 writefds,
                 exceptfds,
                 select_tv
             );

    if (ret < 0)
    {
        errno = ::WSAGetLastError ();
        return ret;
    }

    if (timeout_tv)
    {
        clock_t end_clock = std::clock();
        ZCE_Time_Value consume_timeval;
        consume_timeval.set_by_clock_t( end_clock - start_clock );
        *timeout_tv -= consume_timeval;
    }

    return ret;

#elif defined (ZCE_OS_LINUX)

    timeval *select_tv = NULL;

    if (timeout_tv)
    {
        //有函数讲内部的timv_val取出来
        select_tv = (*timeout_tv);
    }

    return ::select(
               nfds,
               readfds,
               writefds,
               exceptfds,
               select_tv
           );
#endif
}

//这个函数是为了方便平台代码编写写的一个函数，会利用不同平台的fd_set实现加快速度，
//当然这样么有任何通用性，sorry，追求性能了。
inline bool ZCE_OS::is_ready_fds(int no_fds,
                                 const fd_set *out_fds,
                                 ZCE_SOCKET *ready_fd)
{
#if defined (ZCE_OS_WINDOWS)

    if (out_fds->fd_array[no_fds] != INVALID_SOCKET)
    {
        *ready_fd = out_fds->fd_array[no_fds];
        return true;
    }

    return false;
#elif defined (ZCE_OS_LINUX)

    if (FD_ISSET(no_fds, out_fds))
    {
        *ready_fd = no_fds;
        return true;
    }

    return false;
#endif
}

//返回端口号
inline uint16_t ZCE_OS::get_port_number(const sockaddr_in *sock_addr_ipv4)
{
    return ntohs(sock_addr_ipv4->sin_port);
}

//返回地址信息
inline const char *ZCE_OS::get_host_addr(const sockaddr_in *sock_addr_ipv4,
                                         char *addr_buf,
                                         size_t addr_size)
{
    return ZCE_OS::inet_ntop(AF_INET,
                             (void *)(sock_addr_ipv4),
                             addr_buf,
                             addr_size);
}

//非标准函数，但是重入安全
//将地址信息打印出来，地址不使用网络序
inline const char *ZCE_OS::inet_ntoa(uint32_t in, char *addr_buf, size_t addr_size)
{
    sockaddr_in sock_addr_ipv4;
    sock_addr_ipv4.sin_addr.s_addr = ntohl(in);
    return ZCE_OS::inet_ntop(AF_INET,
                             (void *)(&sock_addr_ipv4),
                             addr_buf,
                             addr_size);
};

//这两个函数WIN32和LINUX都有
//通过IPV4的地址，得到对应的字符串表示，不可重入函数，不太建议使用
inline const char *ZCE_OS::inet_ntoa(struct in_addr in)
{
    return ::inet_ntoa(in);
}

//通过一个IP地址的字符串表示得到一个IPV4的的整数
inline uint32_t ZCE_OS::inet_addr(const char *cp)
{
    return ::inet_addr(cp);
};

//返回IP地址的整数
inline uint32_t ZCE_OS::get_ip_address(const sockaddr_in *sock_addr_ipv4)
{
    return ntohl(sock_addr_ipv4->sin_addr.s_addr);
}

//--------------------------------------------------------------------------------------------------------------------

//设置一个IPV4的地址,
inline int ZCE_OS::set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                                   const char *ipv4_addr_str,
                                   uint16_t ipv4_port)
{
    sock_addr_ipv4->sin_family = AF_INET;
    //htons在某些情况下是一个宏，
    //sock_addr_ipv4->sin_port = ::htons(ipv4_port);
    sock_addr_ipv4->sin_port = htons(ipv4_port);

    //在某些平台的sockaddr_in有一个长度标识，比如BSD体系？
#if 0
    sock_addr_ipv4->sa_len = sizeof(sockaddr_in);
#endif

    sock_addr_ipv4->sin_addr.s_addr = ::inet_addr(ipv4_addr_str);

    //注意inet_pton的返回值
    if (sock_addr_ipv4->sin_addr.s_addr == INADDR_NONE)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

//设置一个IPV4的地址,如果字符串里面有#，会认为后面有端口号，会同时提取端口号，否则端口号设置0
inline int ZCE_OS::set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                                   const char *ipv4_addr_str)
{
    int ret = ZCE_OS::set_sockaddr_in(sock_addr_ipv4, ipv4_addr_str, 0);
    if (ret != 0)
    {
        return ret;
    }
    //如果有#，而且后面还有空间，
    const char *port_pos = strchr(ipv4_addr_str, '#');
    if ( port_pos != NULL && *(++port_pos) != '\0' )
    {
        //注意到这儿pos已经++了。
        uint16_t read_port = 0;
        sscanf(port_pos, "%hu", &read_port);
        sock_addr_ipv4->sin_port = htons(read_port);
    }

    return 0;
}


//设置一个IPV4的地址,错误返回NULL，正确返回设置的地址的变换
inline int ZCE_OS::set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                                   uint32_t ipv4_addr_val,
                                   uint16_t ipv4_port
                                  )
{
    //memset(sock_addr_ipv4,0,sizeof(sockaddr_in));
    sock_addr_ipv4->sin_family = AF_INET;
    sock_addr_ipv4->sin_port = htons(ipv4_port);
    sock_addr_ipv4->sin_addr.s_addr = htonl(ipv4_addr_val);

    //在某些平台的sockaddr_in有一个长度标识，比如BSD体系？
#if 0
    sock_addr_ipv4->sa_len = sizeof(sockaddr_in);
#endif

    return 0;
}

//设置一个IPV6的地址,
inline int ZCE_OS::set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                                    const char *ipv6_addr_str ,
                                    uint16_t ipv6_port)
{
    sock_addr_ipv6->sin6_family = AF_INET6;
    sock_addr_ipv6->sin6_port = htons(ipv6_port);

    int ret = ZCE_OS::inet_pton(AF_INET6,
                                ipv6_addr_str,
                                static_cast<void *>(&(sock_addr_ipv6->sin6_addr.s6_addr)));

    //注意inet_pton的返回值,我转换我熟悉的返回
    if (ret == 1)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

//设置一个IPV6的地址,如果有端口号信息，也会
inline int ZCE_OS::set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                                    const char *ipv6_addr_str)
{
    int ret = set_sockaddr_in6(sock_addr_ipv6, ipv6_addr_str, 0);
    if (ret != 0)
    {
        return ret;
    }
    //如果有#，而且后面还有空间，
    const char *port_pos = strchr(ipv6_addr_str, '#');
    if (port_pos != NULL && *(++port_pos) != '\0')
    {
        //注意到这儿pos已经++了。
        uint16_t read_port = 0;
        sscanf(port_pos, "%hu", &read_port);
        sock_addr_ipv6->sin6_port = htons(read_port);
    }
    return 0;
}


//设置一个IPV6的地址,错误返回NULL，正确返回设置的地址的变换
inline int ZCE_OS::set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                                    uint16_t ipv6_port,
                                    const char ipv6_addr_val[16])
{
    //16bytes，128bit的IPV6的地址信息
    const size_t IPV6_INET6_LEN = 16;

    sock_addr_ipv6->sin6_family = AF_INET6;
    sock_addr_ipv6->sin6_port = htons(ipv6_port);
    memcpy(sock_addr_ipv6->sin6_addr.s6_addr, ipv6_addr_val, IPV6_INET6_LEN);
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------
//返回当前机器的主机名称,2个平台应该都支持这个函数
inline int ZCE_OS::gethostname(char *name, size_t name_len)
{
#if defined (ZCE_OS_WINDOWS)
    return ::gethostname(name, static_cast<int>(name_len) );
#elif defined (ZCE_OS_LINUX)
    return ::gethostname(name, name_len);
#endif

}

#endif //ZCE_LIB_OS_ADAPT_SOCKET_H_

