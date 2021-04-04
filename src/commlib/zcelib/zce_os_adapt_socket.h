/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_socket.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��5��1��
* @brief      SOCKET�������������㣬��Ҫ������LINUX�¿�£
*
* @details    ZCE_SOCKET ��LINUX�¾���int���ļ���������
*             WINDOWS����SOCKET���������Ϊ������һ��HANDLE��
*
* @note       2011 �� 10��31�գ������������Ĵ���
*             ���ڲ����еĺܶ�timeval ����ZCE_Time_Value;��Ϊ�����������ʵ�ϲ�д���������ѿ�
*             2013 �� 1��13�� ���ڶ����̫��ů��ʮ�㡣
*             �����е�ע��doxygen����
*/

#ifndef ZCE_LIB_OS_ADAPT_SOCKET_H_
#define ZCE_LIB_OS_ADAPT_SOCKET_H_

#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_error.h"
#include "zce_log_logging.h"
#include "zce_time_value.h"


class ZCE_Time_Value;

namespace zce
{

#ifdef ZCE_OS_WINDOWS
//WINSocketû�б�¶����������
static LPFN_CONNECTEX WSAConnectEx = NULL;
static LPFN_ACCEPTEX  WSAAcceptEx = NULL;
#endif


/*!
* @brief      WINDOWS ��SOCKET�������һ�³�ʼ��WSAStartup
* @return     int 0�ɹ���
* @param[in]  version_high  �汾�ĸ�λ
* @param[in]  version_low   �汾�ĵ�λ
* @note       ��NT SP4�Ժ󣬾�֧��2.2�İ汾�ˣ���ʵ���벻�����ɣ���Ҫ�ø��͵İ汾��
*/
int socket_init(int version_high = 2,
                int version_low = 2);

/*!
* @brief      �����˳���DLLЬ��ʱ���ر�SOCKET��WinSock����ʹ�ã�Finalize WinSock after last use (e.g., when a DLL is unloaded).
* @return     int 0�ɹ���-1ʧ��
*/
int socket_finish(void);

/*!
* @brief      ����һ��SOCKET����
* @return     ZCE_SOCKET ����SOCKET���
* @param[in]  family   (��ַ)Э���壬ΪAF_INET��AF_INET6��AF_UNSPEC��,Э���壬�������Ǻ͵�ַ��һһ��Ӧ�ģ�����PF_XXX,Լ����AF_XXX
* @param[in]  type     ��SOCK_DGRAM����SOCK_STREAM��SOCK_RAW
* @param[in]  proto    Э�����ͣ���Ҫ��ԭ��SOCKET��ʹ�ã�����ICMP�ȣ�һ����дΪ0
*/
inline ZCE_SOCKET socket (int family,
                          int type,
                          int proto = 0);

/*!
* @brief      ����һ��accept�����socket
* @return     inline ZCE_SOCKET  ���ؽ��ܵ�SOCKET��������ؾ�����ڣ�ZCE_INVALID_SOCKET��ʶʧ��
* @param      handle             �����˿ڵľ��
* @param      addr               accept��SOCKET�ĵ�ַ��Ϣ��sockaddr����ʵ�����Ǹ���Э����
* @param      addrlen            accept��SOCKET�ĵ�ַ����
*/
inline ZCE_SOCKET accept (ZCE_SOCKET handle,
                          sockaddr *addr,
                          socklen_t *addrlen);

/*!
* @brief      ��һ����ַ����Ҫ���ڼ����˿ڣ���Ȼ��������Ҳ�����ã����ض˿ڣ�CONNECT��ȥ�Ķ˿ڣ���һ�㲻ָ�����ص�ַ
* @return     int      0��ʶ�ɹ�
* @param      handle   �󶨲����ĵ�ַ
* @param      addr     ���Ҫ��󶨵ĵĵ�ַ��Ϣ��sockaddr����ʵ�����Ǹ���Э����
* @param      addrlen  ���Ҫ��󶨵ĵĵ�ַ����
*/
int bind (ZCE_SOCKET handle,
          const struct sockaddr *addr,
          socklen_t addrlen);

/*!
* @brief      �رվ����Ϊʲô������close������Ϊ����WIN�¼��ݣ���û����close���������
* @return     int 0�ɹ���-1ʧ��
* @param      Ҫ�رյľ��
*/
inline int closesocket (ZCE_SOCKET handle);

/*!
* @brief      shutdown��how������LINUX�¿���SHUT_RD
* @return     int     �ر�ĳ���ߵ�ͨ��
* @param      handle  �رյ���ͨ���ľ��
* @param      how     ��ιرգ�SHUT_RD,SHUT_WR,SHUT_RDWR
*/
inline int shutdown (ZCE_SOCKET handle,
                     int how);

/*!
* @brief      �Ǳ�׼�������﷨�ǣ���ĳЩѡ�WIN32Ŀǰֻ֧��O_NONBLOCK,
* @return     int     0�ɹ���-1ʧ��
* @param      handle  Ҫ���õľ����
* @param      value   ѡ��ֵ��WIN32Ŀǰֻ֧��O_NONBLOCK��
*/
int sock_enable (ZCE_SOCKET handle, int value);

/*!
* @brief      �ر�ĳЩѡ��
* @return     int    0�ɹ���-1ʧ��
* @param      handle Ҫ���õľ����
* @param      value  ѡ��ֵ��WIN32Ŀǰֻ֧��O_NONBLOCK
*/
int sock_disable(ZCE_SOCKET handle, int value);





/*!
* @brief      ����ĳ����ַ
* @return     int     0�ɹ���-1ʧ��
* @param[in]  handle  ���ӵĵľ��
* @param[in]  addr    ����Ŀ��ĵ�ַ��Ϣ
* @param[in]  addrlen ����Ŀ��ĵ�ַ����
* @note
*/
inline int connect (ZCE_SOCKET handle,
                    const sockaddr *addr,
                    socklen_t addrlen);


/*!
* @brief      (TCP)ȡ�öԶ˵ĵ�ַ��Ϣ
* @return     int     0�ɹ���-1ʧ��
* @param      handle  ȡ�Զ˵ĵ�ַ�ľ��
* @param[out] addr    ���صĵ�ַ��Ϣ
* @param[out] addrlen ���صĵ�ַ����
*/
inline int getpeername (ZCE_SOCKET handle,
                        sockaddr *addr,
                        socklen_t *addrlen);

/*!
* @brief      ȡ�ñ��صĵ�ַ��Ϣ��BIND�ĵ�ַ��������OS����ı��ص�ַ
* @return     int     0�ɹ���-1ʧ��
* @param      handle  ���ID
* @param[out] addr    ���صĵ�ַ��ַ��Ϣ
* @param[out] addrlen ���صĵ�ַ����
*/
inline int getsockname (ZCE_SOCKET handle,
                        sockaddr *addr,
                        socklen_t *addrlen);

/*!
* @brief      ȡ�þ����ĳ��ѡ��ֵ
* @return     int     0�ɹ���-1ʧ��
* @param[in]  handle  Ҫ�����ľ��
* @param[in]  level   ѡ��ļ���
* @param[in]  optname ѡ������֣�ö�ٶ���
* @param[out] optval  ȡ��ѡ������
* @param[out] optlen  ȡ��ѡ�����ݵĳ���
*/
inline int getsockopt (ZCE_SOCKET handle,
                       int level,
                       int optname,
                       void *optval,
                       socklen_t *optlen);

/*!
* @brief      ���þ����ĳ��ѡ��ֵ
* @return     int     0�ɹ���-1ʧ��
* @param[in]  handle  Ҫ�����ľ��
* @param[in]  level   ѡ��ļ���
* @param[in]  optname ѡ������֣�ö�ٶ���
* @param[in]  optval  ����ѡ������
* @param[in]  optlen  ����ѡ�����ݵĳ���
* @note
*/
inline int setsockopt (ZCE_SOCKET handle,
                       int level,
                       int optname,
                       const void *optval,
                       socklen_t optlen);

/*!
* @brief      ����ĳ���˿�
* @return     int 0�ɹ���-1ʧ��
* @param      handle �����˿ڵľ��
* @param      backlog �ں�Ϊ���׽����Ŷӵ��������������ͬѧ�Ƿ�������ֵ�ڴ��ģ���ӵķ���������
*             ͦ���õģ������ģ�ϴ󣬽�����������ֵ
*             ͬʱ�����  /proc/sys/net/core/somaxconn
*             /proc/sys/net/ipv4/tcp_max_syn_backlog
*             ��ο� http://linux.die.net/man/2/listen
*/
inline int listen (ZCE_SOCKET handle,
                   int backlog = ZCE_DEFAULT_BACKLOG);

/*!
* @brief      �������ݣ�
* @return     ssize_t -1��ʶʧ�ܣ�ʧ��ԭ���errno��ã���0��ʾ�˿ڱ��Ͽ���>0��ʾ���յ������� ���ݳ���
* @param[in]  handle  �����ľ��
* @param[out] buf     �������ݵ�buffer
* @param[out] len     �������ݵ�buffer�ĳ���
* @param[in]  flags   ���ղ�����flag Ĭ��Ϊ0��WIN��LINUX����ͨ�ã������г����õ�һЩ
*                     MSG_WAITALL WIN��LINUX���У���ȴ�����ָ�����ȵ����ݣ��ŷ���
*                     MSG_DONTWAIT LINUX ���С��൱�ڷ���������
*                     MSG_OOB WIN��LINUX���У�����������ݵģ�������Ҳû���õ���
*/
inline ssize_t recv (ZCE_SOCKET handle,
                     void *buf,
                     size_t len,
                     int flags = 0);

/*!
* @brief      �������ݣ�
* @return     ssize_t -1��ʾʧ�ܣ�ʧ��ԭ���errno��ã���0��ʾ�˿ڱ��Ͽ���>0��ʾ���͵ĵ����ݳ���
* @param[in]  handle  �����ľ��
* @param[in]  buf     �������ݵ�buffer
* @param[in]  len     �������ݵ�buffer�ĳ���
* @param[in]  flags   ���Ͳ�����flag,��ƽ̨��̫ͨ�ã�
*                     MSG_DONTWAIT,LINUX���У��൱�ڷ���������
*/
inline ssize_t send (ZCE_SOCKET handle,
                     const void *buf,
                     size_t len,
                     int flags = 0);

/*!
* @brief      UDP����
* @return     ssize_t  -1��ʾʧ�ܣ�ʧ��ԭ���errno��ã���0��ʾ�˿ڱ��Ͽ���>0��ʾ���յ������� ���ݳ���
* @param[in]  handle   �����ľ��
* @param[out] buf      �������ݵ�buffer
* @param[out] len      �������ݵ�buffer�ĳ���
* @param[in]  flags    ���ܲ�����flag Ĭ��Ϊ0��MSG_DONTWAI LINUX֧�֣��൱�ڷ���������
* @param[out] from     ������Դ�ĵ�ַ��from���Ժ�from_lenһ��ΪNULL,��ʾ�����Ĵ��Ķ���������
* @param[out] from_len ������Դ��ַ�ĳ���
*/
inline ssize_t recvfrom (ZCE_SOCKET handle,
                         void *buf,
                         size_t len,
                         int flags,
                         sockaddr *from,
                         socklen_t *from_len);

/*!
* @brief      UDP����
* @return     ssize_t  -1��ʾʧ�ܣ�ʧ��ԭ���errno��ã���0��ʾ�˿ڱ��Ͽ���>0��ʾ���յ������� ���ݳ���
* @param[in]  handle  �����ľ��
* @param[in]  buf     �������ݵ�buffer
* @param[in]  len     �������ݵ�buffer�ĳ���
* @param[in]  flags   ���Ͳ�����flag Ĭ��Ϊ0��MSG_DONTWAI LINUX֧�֣��൱�ڷ���������
* @param[in]  addr    Ŀ��ĵ�ַ
* @param[in]  addrlen Ŀ��ĵ�ַ�ĳ���
*/
inline ssize_t sendto (ZCE_SOCKET handle,
                       const void *buf,
                       size_t len,
                       int flags,
                       const struct sockaddr *addr,
                       socklen_t addrlen);

//--------------------------------------------------------------------------------------------
//�������������ǷǱ�׼���������Ҳ������������
//���������������������������Ƿ�����������������±��ֻ���ྶͥ��һ��������һ�������������ش���
//���Ҫ���г�ʱ������ʹ��recv_n,send_n

/*!
* @brief      ������ȡlen�������ݣ�ֱ�����ִ���,
* @return     ssize_t -1��ʾʧ�ܣ�ʧ��ԭ���errno��ã���0��ʾ�˿ڱ��Ͽ���>0��ʾ���յ������� ���ݳ���
* @param[in]  handle  �����ľ����
* @param[out] buf     �������ݵ�buffer
* @param[out] len     �������ݵ�buffer�ĳ���
* @param[in]  flags   ���ղ�����flag
* @note               ��������������������,������
*                     ���������������������ܻ���������errno����ΪEWOULDBLOCK
*/
ssize_t recvn (ZCE_SOCKET handle,
               void *buf,
               size_t len,
               int flags = 0);

/*!
* @brief      ��������N�����ݣ�ֱ�����ִ���
* @return     ssize_t -1��ʾʧ�ܣ�ʧ��ԭ���errno��ã���0��ʾ�˿ڱ��Ͽ���>0��ʾ���յ������� ���ݳ���
* @param[in]  handle  �����ľ��
* @param[in]  buf     �������ݵ�buffer
* @param[in]  len     �������ݵ�buffer�ĳ���
* @param[in]  flags   ���Ͳ�����flag,��ƽ̨��̫ͨ�ã�
*/
ssize_t sendn (ZCE_SOCKET handle,
               const void *buf,
               size_t len,
               int flags = 0);

//--------------------------------------------------------------------------------------------

/*!
* @brief      is_ready_fds���ڰ������ӱܸ�����select�Ĵ����д�鷳������Ϊselect��һ����Ƶ��൱ʧ�ܵ�API
*             �˺������ڰ������һ��fd_set�м�ľ���Ƿ�ready�ˣ�������fd ready�ˣ�Ȼ��true��������false
*             �������һ��forѭ������is_ready_fds��������������������Ƿ�ready
* @return     bool     �������no_fds�±�ľ���Ѿ�ready���ͷ���true�����򷵻�false
* @param[in]  no_fds   no_fds�Ǽ�����ţ�С��nfds��
* @param[in]  out_fds  ����select�ķ���fd_set����
* @param[out] ready_fd ����Ѿ�ready�ˣ�ready_fd����ready�ľ��
*/
inline bool is_ready_fds(int no_fds,
                         const fd_set *out_fds,
                         ZCE_SOCKET *ready_fd);

/*!
* @brief      ��ƽ̨��select�������ں��������Ͼ�����LINUX�¿��룬
*             LINUX���û�о������ʱ���൱��sleep����WINDOWSȴ�᷵�ش������ǵķ�װͳһ��LINUX���룬
* @return        int         ��select�ķ���һ�������ش����ľ��������-1��ʾ��������
* @param[in]     nfds        ���ľ��ID��WINDOWS�¿�����дΪ0����Ϊ0�����и澯����LINUX��Ϊ���ľ��ID+1
* @param[in,out] readfds     ���ж�ȡ�������ľ������
* @param[in,out] writefds    ����д�봥�����ľ������
* @param[in,out] exceptfds   �����쳣�������ľ�����ϣ�
* @param[in,out] timeout_tv  ��ʱʱ�䣬ע����������Ƿ�const�ģ���ɺ󷵻�ʣ��ʱ��
* @note       UNIX �����̾�1������timeout_tv������const�ģ����Ҿ����ܶ࣬��Ҳ��������
*             ��������٩����˵LINUX�µ�select ��timeout_tv����const�ģ�����һ�£��治�ǣ�
*             ��������������ѡ����˭��������⣬����ǻ�����LINUX����ɡ�
*/
inline int select(
    int nfds,
    fd_set *readfds,
    fd_set *writefds,
    fd_set *exceptfds,
    ZCE_Time_Value *timeout_tv
);

///����handle_ready����,handle_multi_ready����
enum HANDLE_READY_TODO
{
    ///���¼�
    HANDLE_READY_READ            = 0x1,

    ///д�¼�
    HANDLE_READY_WRITE           = 0x2,

    ///�쳣
    HANDLE_READY_EXCEPTION       = 0x3,

    ///ACCPET �¼���Ҳ�Ƿ��ڶ���SET
    HANDLE_READY_ACCEPT          = 0x4,

    ///���ӳɹ�����Ϊ���ӳɹ����˶���Ŭ�����㿴����
    HANDLE_READY_CONNECTED       = 0x5,

};

/*!
* @brief         �Ǳ�׼������(��һ��ʱ�����)��handle׼�������ڸ���û�У��ڲ���select����������ĳЩ�Ե����˿ڵĵ����¼�����
* @return        int          ����ֵ��select һ�£�����ɹ�����ֵ�Ǵ����¼�����������¼�����һ�㴥������ֵ����1
* @param[in]     handle       �ȴ������ľ��
* @param[in,out] timeout_tv   ��ʱ��ʱ��
* @param[in]     ready_todo   �ȴ�ʲô�¼���ֻ�ܵ�������@ref HANDLE_READY_TODO
* @note          �ر����ѣ�ֻ�ܴ������¼�ร�����ͬʱ�����д,
*/
int handle_ready (ZCE_SOCKET handle,
                  ZCE_Time_Value *timeout_tv,
                  HANDLE_READY_TODO ready_todo);



//--------------------------------------------------------------------------------------------
//���г�ʱ�����һ�麯��������timeout_tvȷ���Ƿ���г�ʱ����


/*!
* @brief         �������ݣ�����len�������ݻ��߳�ʱ�󷵻أ�(Ҳ����һֱ�ȴ���������������)
* @return        ssize_t    -1��ʾʧ�ܣ�ʧ��ԭ���errno��ã���0��ʾ�˿ڱ��Ͽ���>0��ʾ���յ������� ���ݳ���
* @param[in]     handle     �����ľ����WINDOWS��Ҫ����������״̬��
* @param[out]    buf        �������ݵ�buffer
* @param[out]    len        ָ���������ݵĳ���
* @param[in,out] timeout_tv ��ʱʱ�������ʱ����timeout_tv !=NULL��ʶ��ʱ������ʱ����SELECT�������еĴ���
*                           timeout_tv ==NULL ��ʶ�����г�ʱ����,����Socket״̬�Լ�����������ʣ���ʱ��
* @param[in]     flags      ���յ�flag,�ο�@ref recv
* @note
*/
inline ssize_t recv_n (ZCE_SOCKET handle,
                       void *buf,
                       size_t len,
                       ZCE_Time_Value *timeout_tv,
                       int flags = 0);

/*!
* @brief         �������ݣ�����len�������ݻ��߳�ʱ�󷵻أ�(Ҳ����һ�µȴ���������������)
* @return        ssize_t    -1��ʾʧ�ܣ�ʧ��ԭ���errno��ã���0��ʾ�˿ڱ��Ͽ���>0��ʾ���͵����� ���ݳ���
* @param[in]     handle     �����ľ����WINDOWS��Ҫ����������״̬��
* @param[in]     buf        �������ݵ�buffer
* @param[in]     len        ָ���������ݵĳ���
* @param[in,out] timeout_tv ��ʱʱ�������ʱ����timeout_tv !=NULL��ʶ��ʱ����,��ʱ����SELECT�������еĴ���
*                           timeout_tv ==NULL ��ʶ�����г�ʱ����,����Socket״̬�Լ�����,����ʣ���ʱ��
* @param[in]     flags      ���͵�flag���ο�@ref send
*/
inline ssize_t send_n (ZCE_SOCKET handle,
                       const void *buf,
                       size_t len,
                       ZCE_Time_Value *timeout_tv,
                       int flags = 0);

/*!
* @brief      ����UDP���ݣ����յ�һ�������ݰ����߳�ʱ�󷵻أ�(Ҳ����һ�µȴ���������������)
* @return     ssize_t  -1��ʾʧ�ܣ�ʧ��ԭ���errno��ã���0��ʾ�˿ڱ��Ͽ���>0��ʾ���յ����� ���ݳ���
* @param[in]     handle     �����ľ����WINDOWS��Ҫ����������״̬��
* @param[out]    buf        �������ݵ�buffer
* @param[out]    len        �������ݵ�buffer�ĳ���
* @param[out]    from       ������Դ�ĵ�ַ��from���Ժ�from_lenһ��ΪNULL,��ʾ�����Ĵ��Ķ���������
* @param[out]    from_len   ������Դ��ַ�ĳ���
* @param[in,out] timeout_tv ��ʱʱ�������ʱ����timeout_tv !=NULL��ʶ��ʱ����,��ʱ����SELECT�������еĴ���
*                           timeout_tv ==NULL ��ʶ�����г�ʱ����,����Socket״̬�Լ�����,����ʣ���ʱ��
* @param[in]     flags      ���ղ�����flag �ο�@ref recv
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
* @brief      UDP�ķ�����ʱ�ǲ��������ģ����ó�ʱ����д���������ȫ��Ϊ�˺�ǰ�����
*             ����UDP������,����ʱ�������������ʵ���Ͻ���û�г�ʱ����
* @return     ssize_t    -1��ʾʧ�ܣ�ʧ��ԭ���errno��ã���0��ʾ�˿ڱ��Ͽ���>0��ʾ���͵����� ���ݳ���
* @param[in]  handle     �����ľ��
* @param[in]  buf        �������ݵ�buffer
* @param[in]  len        �������ݵ�buffer�ĳ���
* @param[in]  addr       Ŀ��ĵ�ַ
* @param[in]  addrlen    Ŀ��ĵ�ַ�ĳ���
* @param[in]  timeout_tv �˲���û������
* @param[in]  flags      ���ղ�����flag �ο�@ref send
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
///��ΪWINdows ��֧��ȡ��socket �Ƿ���������ģʽ������Windows �����޷���ȡ��socket��ѡ�
///Ȼ���ж��Ƿ�ȡ������ģʽ����������ر�֤���Socket ������ģʽ�ģ�����������
///һ����г�ʱ��SOCKET���������ڲ�ʹ��ʹ��select��Ϊ��ʱ��������
///��ʱ��ʱ�����������д



/*!
* @brief      ��һ���޶�ʱ��������ĳ����������
* @return     int 0�ɹ�����0��ʾʧ�ܣ��Լ�����ID
* @param      handle      �����ľ��
* @param      addr        ��ַ
* @param      addrlen     ��ַ����
* @param      timeout_tv  ��ʱʱ��
* @note
*/
int connect_timeout(ZCE_SOCKET handle,
                    const sockaddr *addr,
                    socklen_t addrlen,
                    ZCE_Time_Value &timeout_tv);



/*!
* @brief      ����ĳ��HOSTNAME��������������Ҳ��������ֵ��ַ��ʽ
* @return     int 0�ɹ�����0��ʾʧ�ܣ��Լ�����ID
* @param      handle      �����ľ��
* @param      hostname    ������������Ҳ��������ֵ��ַ��ʽ�������ȳ�������ֵ��ַ��ʽ����
* @param      port        �˿ں�
* @param      host_addr   ����������������������hostname��������Ϊֱ����ַ�����������hostname�����ؽ����ĵ�ַ
* @param      addr_len    ��ַ����
* @param      timeout_tv  ��ʱʱ��
* @note
*/
int connect_timeout(ZCE_SOCKET handle,
                    const char *host_name,
                    uint16_t port,
                    sockaddr *host_addr,
                    socklen_t addr_len,
                    ZCE_Time_Value &timeout_tv);



/*!
* @brief      TCP�������ݣ�����len�������ݻ��߳�ʱ�󷵻أ�����timeout_tv��������ο�@ref recv_n
*             recvn_timeout �� recvn_n ��������recvn_n �����ʱ����ΪNULL�������������ػ���һ�������ȴ�
*             �ڲ���ʱ��select ʵ��
* @param      timeout_tv �ȴ���ʱ�����������ֵ���������дһ����ֵ
* @param      only_once  ֻ��ȡһ�����ݣ���ȡ��ͷ��أ����ȴ�һ��Ҫ��ȡ��len������
*/
ssize_t recvn_timeout (ZCE_SOCKET handle,
                       void *buf,
                       size_t len,
                       ZCE_Time_Value &timeout_tv,
                       int flags = 0,
                       bool only_once = false);

/*!
* @brief      TCP�������ݣ�����len�������ݻ��߳�ʱ�󷵻أ�����timeout_tv��������ο�@ref sendv_n
*             sendn_timeout �� sendv_n ��������recvn_n �����ʱ����ΪNULL�������������ػ���һ�������ȴ�
*             ��sendn_timeout �ĳ�ʱ����������д
*             �ڲ���ʱ��select ʵ��
* @param      timeout_tv �ȴ���ʱ�����������ֵ���������дһ����ֵ
*/
ssize_t sendn_timeout(ZCE_SOCKET handle,
                      const void *buf,
                      size_t len,
                      ZCE_Time_Value &timeout_tv,
                      int flags = 0);


/*!
* @brief      ����UDP���ݣ����յ�һ�������ݰ����߳�ʱ�󷵻أ���ο�@ref recvfrom
* @param      timeout_tv �ȴ���ʱ�����������ֵ���������дһ����ֵ
*/
ssize_t recvfrom_timeout (ZCE_SOCKET handle,
                          void *buf,
                          size_t len,
                          sockaddr *from,
                          socklen_t *from_rlen,
                          ZCE_Time_Value &timeout_tv,
                          int flags = 0);

/*!
* @brief      UDP�ķ�����ʱ�ǲ��������ģ����ó�ʱ����д���������ȫ��Ϊ�˺�ǰ�����
*             ����UDP������,����ʱ�������������ʵ���Ͻ���û�г�ʱ����
*/
ssize_t sendto_timeout (ZCE_SOCKET handle,
                        const void *buf,
                        size_t len,
                        const sockaddr *addr,
                        socklen_t addrlen,
                        ZCE_Time_Value & /*timeout_tv*/,
                        int flags = 0);




//==================================================================================================
//���麯���ṩ����Ϊ�˴�����ԣ���ʱ�������ṩ
//���麯����ʹ��SO_RCVTIMEO��SO_SNDTIMEO�õ�һ�鳬ʱ������

/*!
* @brief      �������ݣ�����len�������ݻ��߳�ʱ�󷵻أ�����timeout_tv��������ο�@ref recv_n
*             recvn_timeout2 �� recvn_timeout �������Ǵ�recvn_timeout2�����ֺ�����ʹ����SO_RCVTIMEOѡ����г�ʱ����
*             ��ʱ�����ر�׼ȷ��
* @param      timeout_tv �ȴ���ʱ�����������ֵ���������дһ����ֵ
*/
ssize_t recvn_timeout2 (ZCE_SOCKET handle,
                        void *buf,
                        size_t len,
                        ZCE_Time_Value &timeout_tv,
                        int flags = 0);

/*!
* @brief      �������ݣ�����len�������ݻ��߳�ʱ�󷵻أ�����timeout_tv��������ο�@ref sendv_n
*             sendn_timeout2 �� sendn_timeout �������Ǵ�sendn_timeout2�����ֺ�����ʹ����SO_SNDTIMEOѡ����г�ʱ����
*             ��ʱ�����ر�׼ȷ��
* @param      timeout_tv �ȴ���ʱ�����������ֵ���������дһ����ֵ
*/
ssize_t sendn_timeout2 (ZCE_SOCKET handle,
                        void *buf,
                        size_t len,
                        ZCE_Time_Value &timeout_tv,
                        int flags = 0);

/*!
* @brief      ��UDP������,Ҳ���г�ʱ�����������յ��������ݾ��Ƕ����ˣ���ʱ��SO_RCVTIMEOʵ��
*             ������������ֵ�ο�recvfrom��
*/
ssize_t recvfrom_timeout2 (ZCE_SOCKET handle,
                           void *buf,
                           size_t len,
                           sockaddr *from,
                           socklen_t *from_rlen,
                           ZCE_Time_Value &timeout_tv,
                           int flags = 0);

/*!
* @brief      UDP�ķ�����ʱ�ǲ��������ģ����ó�ʱ����д���������ȫ��Ϊ�˺�ǰ�����
*             ����UDP������,����ʱ�������������ʵ���Ͻ���û�г�ʱ����
*             ������������ֵ�ο�sendto��
*/
ssize_t sendto_timeout2 (ZCE_SOCKET handle,
                         const void *buf,
                         size_t len,
                         const sockaddr *addr,
                         socklen_t addrlen,
                         ZCE_Time_Value & /*timeout_tv*/,
                         int flags = 0);

//--------------------------------------------------------------------------------------------
//һ�β���һ�����ݵĵ�readv��writev����

/*!
* @brief      һ�ζ�ȡһ�����ݣ�WINDOWS���ã�WSARecvģ�⣬
*             readv���ֺ������濴�������ܱȽ�һЩ���ݵķֽڴ���������û�г�ʱ�ȸ�����Ҳδ�������
* @return     ssize_t ��ȡ���ݵ��ܳ���
* @param[in]     handle  �����ľ��
* @param[in,out] iov     ��ȡ���������飬iovec������ buf����len��
* @param[in]     iovlen  ���鳤��
*/
ssize_t readv (ZCE_SOCKET handle,
               iovec *iov,
               int iovlen);

/*!
* @brief      һ��д��һ�����ݣ�WINDOWS���ã�WSASendģ�⣬
* @return     ssize_t �������ݵ��ܳ���
* @param[in]  handle  �����ľ��
* @param[in]  iov     д���������飬iovec������ buf����len��
* @param[in]  iovcnt  ���鳤��
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

//IP��ַ�ļ��ֳ���,ע��ע��ע��ע��ע�� ��Щ����û�м����β��'\0'
//           12345678901234567890123456789012345678901234567890
//IPV4       202.202.202.202                                      15
//IPV4+Port  202.202.202.202#36010                                21
//IPV6       102A:F02B:A1F2:745B:112D:1241:1834:124A              39
//IPV4+Port  102A:F02B:A1F2:745B:112D:1241:1834:124A#40012        45

/*!
* @brief      ������addrptr�ĵ�ַ��Ϣת��Ϊ��ַ�ַ�����
*             inet_ntop,inet_pton ������������LINUX���У�
*             ��VISTA��SOCKET��������õ���֧�֣���������ֻ���Լ�ģ����,
* @return     const char* �õ��ĵ�ַ�ַ���
* @param[in]  family      Э����
* @param[in]  addrptr     ��ַ
* @param[out] strptr      ���صĵĵ�ַ�ַ���
* @param[in]  len         �ַ�������
*/
const char *inet_ntop (int family,
                       const void *addrptr,
                       char *strptr,
                       size_t len);

/*!
* @brief      �����ַ���ת���õ������ַ����һ������af�ǵ�ַ�壬ת�������addrptr�У�
* @return     int  ע�������������1��ʶ�ɹ�������0��ʾʧ�ܣ�����ԭ������ƣ���������Ҳ̫������
* @param      family Э����
* @param      strptr ��ַ���ַ�����ʶ
* @param      addrptr ת���õ��ĵ�ַ
* @note       ע�ⷵ��ֵ
*/
int inet_pton (int family,
               const char *strptr,
               void *addrptr);

/*!
* @brief      ��IPV4�������ĵ�ַ��Ϣ��ӡ��������һ�������������򣬷Ǳ�׼�������������밲ȫ
* @return     const char* �õ��ĵ�ַ�ַ���
* @param[in]  in          IPV4�����ַ��������
* @param[out] addr_buf    ���صĵĵ�ַ�ַ���
* @param[in]  addr_size   �ַ�������
*/
inline const char *inet_ntoa(uint32_t in,
                             char *addr_buf,
                             size_t addr_size);

/*!
* @brief      ͨ��IPV4�ĵ�ַ���õ���Ӧ���ַ�����ʾ���������뺯������̫����ʹ��
*             ����������WIN32��LINUX����
* @return     inline const char*
* @param      in  IPV4�ĵ�ַ��
*/
inline const char *inet_ntoa(struct in_addr in);

/*!
* @brief      ͨ��һ��IP��ַ���ַ�����ʾ�õ�һ��IPV4�ĵ�����
* @return     inline uint32_t ���صĵ�ַ��ֵ��������
* @param      cp ��ַ�ַ���
*/
inline uint32_t inet_addr(const char *cp);



static const size_t MAX_SOCKETADDR_STRING_LEN = 45;
/*!
* @brief      ���IP��ַ��Ϣ���ڲ��ǲ�ʹ�þ�̬�������̰߳�ȫ��BUF����IPV4���ٳ���>15.IPV6���ٳ���>39
* @return     const char*  �õ��ĵ�ַ�ַ���
* @param      sock_addr    sockaddr ��ַ����
* @param      str_ptr      ���صĵĵ�ַ�ַ���
* @param      str_len      �ַ�������
*/
const char *socketaddr_ntop(const sockaddr *sock_addr,
                            char *str_ptr,
                            size_t str_len);

/*!
* @brief      socketaddr_ntop_ex ���IP��ַ��Ϣ�Ͷ˿ں�,�˿ں�
*             ���IP��ַ��Ϣ�Լ��˿���Ϣ���ڲ��ǲ�ʹ�þ�̬�������̰߳�ȫ��BUF����IPV4���ٳ���>21.IPV6���ٳ���>45
*             ��socketaddr_ntop�����������socketaddr_ntop_exͬʱ����˶˿ں�
*             �����ο� @ref socketaddr_ntop
*/
const char *socketaddr_ntop_ex(const sockaddr *sock_addr,
                               char *str_ptr,
                               size_t str_len,
                               size_t &use_len,
                               bool out_port_info = true);

//======================================================================================================
//����������ת��IP��ַ�ļ�������
//�����������֣���һ���ֺ�����ϸ�䡶UNIX�����̡���1�ĵ�9��

/*!
* @brief      �������������õ���Ӧ��IP��ַ����������Ѿ������ر��Ƽ�����Ϊ���ֲ����⣬
*             ���粻�����룬���һ������ͬʱ��IPV4��IPV6����ô��ֻ�з���IPV4��
*             �Ƽ�ʹ��getaddrinfo�� gethostbyname��ʹ�ÿ��Բο�gethostbyname_inary����
* @return     hostent*  ���ص�IP��ַ��Ϣ
* @param      hostname  ��ѯ������
*/
hostent *gethostbyname(const char *hostname);

/*!
* @brief      GNU extensions ,����Э��أ��õ������ĵ�ַ
* @return     hostent*
* @param      hostname
* @param      af
*/
hostent *gethostbyname2(const char *hostname,
                        int af);

//��Զ��ԣ�������2����������ʹ�ø��Ӽ�һ�㣬

/*!
* @brief      �Ǳ�׼����,�õ�ĳ��������IPV4�ĵ�ַ���飬ʹ�������Ƚ����׺ͷ��㣬�ڲ�ʹ��gethostbyname
* @return        int           0�ɹ���-1ʧ��
* @param[in]     hostname      ����
* @param[in]     service_port  �˿ںţ���Ͳ�ѯ���ĵ�ַ��Ϣһ�����õ�sockaddr_in
* @param[in,out] ary_addr_num  ����ʱ����ĳ��ȣ��������ʵ�ʻ�õ����鳤��
* @param[out]    ary_sock_addr ����+�˿ڶ�Ӧ��sockaddr_in ������Ϣ
*/
int gethostbyname_inary(const char *hostname,
                        uint16_t service_port,
                        size_t *ary_addr_num,
                        sockaddr_in ary_sock_addr[]);

/*!
* @brief      �Ǳ�׼����,�õ�ĳ��������IPV6�ĵ�ַ���飬ʹ�������Ƚ����׺ͷ���
* @return        int            0�ɹ���-1ʧ��
* @param[in]     hostname       ����
* @param[in]     service_port   �˿ںţ���Ͳ�ѯ���ĵ�ַ��Ϣһ�����õ�sockaddr_in
* @param[in,out] ary_addr6_num  ����ʱ����ĳ��ȣ��������ʵ�ʻ�õĵ�ַ����
* @param[out]    ary_sock_addr6 ����+�˿ڶ�Ӧ��sockaddr_in6 ����
* @note
*/
int gethostbyname_in6ary(const char *hostname,
                         uint16_t service_port,
                         size_t *ary_addr6_num,
                         sockaddr_in6 ary_sock_addr6[]);

/*!
* @brief      ͨ��IP��ַ��Ϣ�������������������룬���Ƽ�ʹ�ã��Ƽ�ʹ��getnameinfo
* @return     hostent* ��ѯ��������
* @param[in]  addr     ָ���ַ��ָ�룬ע���ַ��in_addr��in6_addr�ȵ�
* @param[in]  len      ��ַ�ĳ���
* @param[in]  family   ��ַЭ����
*/
hostent *gethostbyaddr(const void *addr,
                       socklen_t len,
                       int family);

/*!
* @brief      �Ǳ�׼������ͨ��IPV4��ַȡ������
* @return     int        0�ɹ���-1ʧ��
* @param[in]  sock_addr  IPV4��sockaddr
* @param[out] host_name  ���ص�������Ϣ
* @param[int] name_len   ����buffer�ĳ���
*/
int gethostbyaddr_in(const sockaddr_in *sock_addr,
                     char *host_name,
                     size_t name_len);

/*!
* @brief      �Ǳ�׼������ͨ��IPV6��ַȡ������
* @return     int        0�ɹ���-1ʧ��
* @param[in]  sock_addr6 IPV6��sockaddr
* @param[out] host_name  ���ص�������Ϣ
* @param[int] name_len   ����buffer�ĳ���
*/
int gethostbyaddr_in6(const sockaddr_in6 *sock_addr6,
                      char *host_name,
                      size_t name_len);

//�������Ӹ߼�һЩ�ĵĺ�����Posix 1g�ĺ������Ƽ�ʹ�����ǣ����ǿ��ܿ������루Ҫ���ײ�ʵ�֣�����ײ����gethostbyname����ô������
//�߼����ֺ�����ϸ�䡶UNIX�����̡���1�ĵ�11��

/*!
* @brief      ͨ�������õ���������ַ��Ϣ������ͬʱ�õ�IPV4����IPV6�ĵ�ַ
* @return     int        0�ɹ�����0��ʾʧ�ܣ��Լ�����ID
* @param[in]  hostname   ������Ϣ
* @param[in]  service    ��������ƣ�����"http","ftp"�ȣ����ھ�������Ķ˿ں�
* @param[in]  hints      ����������Ҫ�ĵ�ַ����Ϣ����Ӧ�Ĳ���˵����
*                        ���Ҫͬʱ�õ�IPV4��IPV6�ĵ�ַ����ôhints.ai_family =  AF_UNSPEC
*                        ai_socktype������û�����дһ��ֵ��������ܷ���SOCK_DGRAM,SOCK_STREAM��һ����
*                        ai_flags ��0һ���OK�������۵�һ��������
*                          AI_CANONNAME��ʾ���ص�addrinfo����ĵ�һ���ڵ����ai_canoname��������addrinfo �ṹ����
*                          AI_PASSIVE��ʾ���صĵ�ַ����bind��hostnameΪNULLʱ����IP��ַ��Ϣ����0������������connect
*                          AI_NUMERICHOST ��ֹ������������Ϊhostname����ֵ��ʽ��ַ
*                          AI_ALL ��ѡ�����IPV4��IPV6��ַ��IPV4�ĵ�ַҲ��MAP��IPV6���أ�Ҳ����IPV4�ĵ�ַ����
*                          AI_V4MAPPED ���û��IPV6�ĵ�ַ���أ���IPV4�ĵ�ַMAPIPV6�ĵ�ַ���ء�
*                        ai_protocol����0��
* @param[out] result     ���صĽ������
*/
int getaddrinfo( const char *hostname,
                 const char *service,
                 const addrinfo *hints,
                 addrinfo **result );

/*!
* @brief      �ͷ�getaddrinfo�õ��Ľ��
* @param      result  getaddrinfo���صĽ��
*/
void freeaddrinfo(struct addrinfo *result);

/*!
* @brief      ������������getaddrinfo�Ľ������ȡһ��sockaddr���,
* @return     int
* @param[in]  result getaddrinfo���صĽ��
* @param[out] addr   �����������addr_len�� ȷ����sockaddr_in,����sockaddr_in6
* @param[in]  addr_len ��ַ�ĳ���
*/
int getaddrinfo_result_to_addr(addrinfo *result,
                               sockaddr *addr,
                               socklen_t addr_len);


/*!
* @brief         ������������getaddrinfo�Ľ�����мӹ��������������
* @param[in]     result
* @param[in,out] ary_addr_num   ary_addr������������ת���󣬷�������
* @param[out]    ary_addr
* @param[in,out] ary_addr6_num  ary_addr6������������ת���󣬷�������
* @param[out]    ary_addr6
*/
void getaddrinfo_result_to_addrary(addrinfo *result,
                                   size_t *ary_addr_num,
                                   sockaddr_in ary_addr[],
                                   size_t *ary_addr6_num,
                                   sockaddr_in6 ary_addr6[]);

/*!
* @brief         �Ǳ�׼����,�õ�ĳ��������IPV4�ĵ�ַ���飬ʹ�������Ƚ����׺ͷ���,�ײ�ʹ��getaddrinfo
* @return        int            0�ɹ�������ʧ��
* @param[in]     notename       ����
* @param[in,out] ary_addr_num   ����ʱ����ĳ��ȣ��������ʵ�ʻ�õ����鳤��
* @param[out]    ary_sock_addr  ������Ӧ��sockaddr_in ������Ϣ
* @param[in,out] ary_addr6_num  ����ʱ����ĳ��ȣ��������ʵ�ʻ�õĵ�ַ����
* @param[out]    ary_sock_addr6 ������Ӧ��sockaddr_in6 ����
*/
int getaddrinfo_to_addrary(const char *notename,
                           size_t *ary_addr_num,
                           sockaddr_in ary_addr[],
                           size_t *ary_addr6_num,
                           sockaddr_in6 ary_addr6[]);



/*!
* @brief
* @return     int      0�ɹ�������ʧ��
* @param[in]  notename ����OR��ֵ��ַ��ʽ���ڲ����Ƚ�����ֵ��ַת���������ʱ�����ɹ��ٽ�����������
* @param[out] addr     ���صĵ�ַ
* @param[in]  addr_len ��ַ�ĳ���
*/
int getaddrinfo_to_addr(const char *host_name,
                        sockaddr *addr,
                        socklen_t addr_len);

/*!
* @brief      ͨ��IP��ַ��Ϣ����������.�������������ܣ��������뺯����Ҫ���ײ�ʵ�֣���
* @return     int   0�ɹ�����0��ʾʧ�ܣ��Լ�����ID
* @param[in]  sa      socket��ַ��ָ�룬ͨ�������ѯ����
* @param[in]  salen   scket��ַ�ĳ���
* @param[out] host    ���ص�����
* @param[in]  hostlen ������buffer�ĳ���
* @param[out] serv    ���صķ�������buffer
* @param[in]  servlen ��������buffer�ĳ���
* @param[in]  flags   flags ����������ʹ�õ�ֵ����
*                     NI_NOFQDN        ���ڱ�������, ��������ȫ�޶������Ľڵ�������.����bear.qq.com�����ص�bear����������.qq.com
*                     NI_NUMERICHOST  host�����������ֵ�IP��ַ��Ϣ��Ϣ
*                     NI_NAMEREQD     ���IP��ַ���ܽ���Ϊ����������һ������
*                     NI_NUMERICSERV  serv����������ֵ���ַ�����Ϣ
*                     NI_DGRAM        ����������ݱ����ǻ�������������ͬһ���˿ںţ�UDP��TCP�ķ��������ǲ�һ���ģ�
*/
int getnameinfo(const struct sockaddr *sa,
                socklen_t salen,
                char *host,
                size_t hostlen,
                char *serv,
                size_t servlen,
                int flags);

/*!
* @brief      �Ǳ�׼������ͨ��IPV4��ַȡ������
* @return     int  0�ɹ�����0��ʾʧ�ܣ��Լ�����ID
* @param[in]  sock_addr  sockaddr�ĵ�ַ
* @param[out] host_name  ���ص�������Ϣ
* @param[int] name_len   ����buffer�ĳ���
*/
int getnameinfo_sockaddr(const sockaddr *sock_addr,
                         char *host_name,
                         size_t name_len);


/*!
* @brief      ���ص�ǰ��������������,2��ƽ̨Ӧ�ö�֧���������
* @return     int        0�ɹ�
* @param[out] name       ���ص��������ƣ�
* @param[in]  name_len   �������Ƶ�buffer����
*/
inline int gethostname(char *name, size_t name_len);

//-------------------------------------------------------------------------------------
//Ϊsockaddr_in���ӵ�һ�麯��������ʹ��,

//�������ú���ΪʲôҪ����sockaddr *������ֱ����Ϊ�����ڴ�������ʹ�ã�����������أ�������


/*!
* @brief
* @return     int ���󷵻�-1����ȷ����0
* @param[out] sock_addr_ipv4 �����õ�IPV4��ַ
* @param[in]  ipv4_addr_str  ��ַ��Ϣ�ַ���
* @param[in]  ipv4_port      �˿ڣ�������
*/
inline int set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                           const char *ipv4_addr_str,
                           uint16_t ipv4_port);

/*!
* @brief      ����һ��IPV4�ĵ�ַ������ַ��������ж˿���Ϣ��Ҳ��ͬʱ���ö˿�
* @return     int ���󷵻�-1����ȷ����0
* @param      sock_addr_ipv4 �����õ�IPV4��ַ
* @param      ipv4_addr_str  ��ַ�ַ���,����������ַ�'#'������Ϊ�ж˿ںţ�
*/
inline int set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                           const char *ipv4_addr_str);

/*!
* @brief      ����һ��IPV4�ĵ�ַ
* @return     int ���󷵻�-1����ȷ����0
* @param[out] sock_addr_ipv4 �����õ�IPV4��ַ
* @param[in]  ipv4_addr_val  ��ʾIPV4��32λ������������
* @param[in]  ipv4_port      �˿ںţ�������
* @note
*/
inline int set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                           uint32_t ipv4_addr_val,
                           uint16_t ipv4_port );


/*!
* @brief      ����һ��IPV6�ĵ�ַ,
* @return     int ���󷵻�-1�� ��ȷ����0
* @param[out] sock_addr_ipv6 �����õ�IPV6��ַ
* @param[in]  ipv6_addr_str  IPV6��ַ��Ϣ�ַ��������ָ�ʽ"0:0:0:0:0:0:0:0"
* @param[in]  ipv6_port      �˿ںţ�������
*/
inline int set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                            const char *ipv6_addr_str,
                            uint16_t ipv6_port);


/*!
* @brief      ����һ��IPV4�ĵ�ַ������ַ��������ж˿���Ϣ��Ҳ��ͬʱ���ö˿�
* @return     int ���󷵻�-1����ȷ����0
* @param      sock_addr_ipv6 �����õ�IPV6��ַ
* @param      ipv6_addr_str  ��ַ�ַ���,����������ַ�'#'������Ϊ�ж˿ںţ�
*/
inline int set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                            const char *ipv6_addr_str);

/*!
* @brief      ����һ��IPV6�ĵ�ַ,����һ��������������Ҫ�ڲ���˳���ϣ�ע��
* @return     int   ���󷵻�-1����ȷ����0
* @param[out] sock_addr_ipv6 �����õ�IPV6��ַ
* @param[in]  ipv6_port      �˿ںţ�������
* @param[in]  ipv6_addr_val  16���ֽڵĵ�ַ��Ϣ
*/
inline int set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                            uint16_t ipv6_port,
                            const char ipv6_addr_val[16]);

//����һЩ�����IPV4�ĺ���

//���ض˿ں�,��ָ����Ϊ������Ҫ��ϣ��ͳһ
inline uint16_t get_port_number(const sockaddr *addr);

//���ص�ַ��Ϣ
inline const char *get_host_addr(const sockaddr *addr,
                                 char *addr_buf,
                                 size_t addr_size);

//�õ�IP��ַ�Ͷ˿���Ϣ���ַ�������
inline const char *get_host_addr_port(const sockaddr *addr,
                                      char *addr_buf,
                                      size_t addr_size);

/*!
* @brief      ����IP��ַ������,������
* @return     inline uint32_t
* @param      sock_addr_ipv4
* @note
*/
inline uint32_t get_ip_address(const sockaddr_in *sock_addr_ipv4);

/*!
* @brief      ���һ����ַ�Ƿ���������ַ,��������ɨäһ�£�IPV6û�о�����������
* @return     bool true��������ַ��false���ǣ�
* @param      sock_addr_ipv4 �жϵ�sockaddr_in
*/
bool is_internal(const sockaddr_in *sock_addr_ipv4);



/*!
* @brief      ���һ��IPV4��ַ���������Ƿ���������ַ,
* @return     bool ture��������ַ��false���ǣ�
* @param[in]  ipv4_addr_val ������ʾIPV4��ַ��������
*/
bool is_internal(uint32_t ipv4_addr_val);

//uint32_t inet_lnaof(struct in_addr in); uint32_t inet_netof(struct in_addr in);
//�������������ڵõ������ַ������ID������ID���Ҿ��û���û�˻�����������ɡ����ˡ�

//-------------------------------------------------------------------------------------
//IPV4��IPV6֮���໥ת���ĺ��������ǷǱ�׼������

/*!
* @brief      ��һ��IPV4�ĵ�ַӳ��ΪIPV6�ĵ�ַ
* @return     int  0�ɹ���-1ʧ��
* @param      src  Ҫ����ת����in_addr
* @param      dst  ת���õ�IPV6�ĵ�ַin6_addr
*/
int inaddr_map_inaddr6(const in_addr *src, in6_addr *dst);

/*!
* @brief      ��һ��IPV4��Sock��ַӳ��ΪIPV6�ĵ�ַ
* @return     int  0�ɹ���-1ʧ��
* @param      src
* @param      dst
*/
int sockin_map_sockin6(const sockaddr_in *src, sockaddr_in6 *dst);

/*!
* @brief      �ж�һ����ַ�Ƿ���IPV4ӳ��ĵ�ַ
* @return     bool TRUE��ӳ����˵ģ�FALSE����
* @param      in6
*/
bool is_in6_addr_v4mapped(const in6_addr *in6);

/*!
* @brief      ���һ��IPV6�ĵ�ַ��IPV4ӳ������ģ�ת����IPV4�ĵ�ַ
* @return     int  0�ɹ���-1ʧ��
* @param      src
* @param      dst
*/
int mapped_in6_to_in(const in6_addr *src, in_addr *dst);

/*!
* @brief      ���һ��IPV6��socketaddr_in6��ַ��IPV4ӳ������ģ�ת����IPV4��socketaddr_in��ַ
* @return     int  0�ɹ���-1ʧ��
* @param[in]  src  IPV6��socketaddr_in6
* @param[out] dst  ת���ɵ�IPV4��socketaddr_in
*/
int mapped_sockin6_to_sockin(const sockaddr_in6 *src, sockaddr_in *dst);

/*!
* @brief      ���һ���˿��Ƿ�ȫ��һЩ�˿��Ǻڿ��ص�ɨ��Ķ˿ڣ�����1024���£�
* @return     bool
* @param      check_port
*/
bool check_safeport(uint16_t check_port);


//-------------------------------------------------------------------------------------
//socks 5 ������

/*!
* @brief      SOCKS5�����ʼ���������û���֤��
* @return     int ����0��ʶ�ɹ�
* @param      handle      �Ѿ�����SOCKS5�������ľ�������������� connect,����ʹ��connect_timeout����
* @param      username    ��֤ģʽ�µ��û����ƣ��������Ҫ��֤����дNULL
* @param      password    ��֤ģʽ�µ����룬�������Ҫ��֤����дNULL
* @param      timeout_tv  ��ʱʱ��
* @note       handle ����������
*/
int socks5_initialize(ZCE_SOCKET handle,
                      const char *username,
                      const char *password,
                      ZCE_Time_Value &timeout_tv);


/*!
* @brief      SOCKS5�����ʼ���������û���֤��
* @return     int  ����0��ʶ�ɹ�
* @param      handle     �Ѿ�����SOCKS5�������ľ�������������� connect
* @param      host_name  ��ת�������������͵�ַֻѡһ��������������ΪNULL
* @param      port       ��ת�Ķ˿�
* @param      addr       ��ת�ĵ�ַ
* @param      addrlen    ��ת�ĵ�ַ�ĳ���
* @param      timeout_tv ��ʱʱ��
*/
int socks5_connect_host(ZCE_SOCKET handle,
                        const char *host_name,
                        const sockaddr *host_addr,
                        int addrlen,
                        uint16_t host_port,
                        ZCE_Time_Value &timeout_tv);

//socks5����UDP��͸
int socks5_udp_associate(ZCE_SOCKET handle,
                         const sockaddr *bind_addr,
                         int addr_len,
                         sockaddr *udp_addr,
                         ZCE_Time_Value &timeout_tv);

};

//-----------------------------------------------------------------------------------------

//��ϸ����һ��Ϊʲô���е�Windows�µĵ���API������һ�δ���
//errno = ::WSAGetLastError ();
//��ΪWindows����ʵ������error��¼��һ����C��errno��һ����::GetLastError,����::WSAGetLastError ()
//���صĴ��󣬶�������error����ͬʱ���ڣ����������ǵĴ������д���������������������errno����ΪWindows��::GetLastError
//�����Ƕ�����ܴ������Ҳ��ò���������һ�£�

//
inline ZCE_SOCKET zce::socket (int family,
                               int type,
                               int proto)
{
    ZCE_SOCKET socket_hdl =::socket (family,
                                     type,
                                     proto);

#if defined (ZCE_OS_WINDOWS)

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
    if (ZCE_INVALID_SOCKET  == socket_hdl)
    {

        errno = ::WSAGetLastError ();
    }
    else
    {
        //�ر�����ط�ʵ��һ��
        //�Ѿ���ʼ���ɹ���������UDP ��ȥ����һ��
        //SIO_UDP_CONNRESET������������һ������UDP peer����һ�����ݺ�����յ�һ��RST����
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
                //�ɵ�ɶ�أ�����ɶ�����ܸ��ã�
            }
        }
    }

#endif //

    return socket_hdl;
}

// ����һ��accept�����socket
inline ZCE_SOCKET zce::accept (ZCE_SOCKET handle,
                               sockaddr *addr,
                               socklen_t *addrlen)
{

#if defined (ZCE_OS_WINDOWS)

    ZCE_SOCKET accept_hdl = ::accept (handle,
                                      addr,
                                      addrlen);

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
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

//��IP��ַЭ�飬��
inline int zce::bind (ZCE_SOCKET handle,
                      const sockaddr *addr,
                      socklen_t addrlen)
{
#if defined (ZCE_OS_WINDOWS)

    int zce_result = ::bind (handle,
                             addr,
                             addrlen);

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
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

//���ֻ����Windows���룬��Ϊ���Լ�����һ��close��������windows������������ȫ��������
inline int zce::closesocket (ZCE_SOCKET handle)
{
#if defined ZCE_OS_WINDOWS
    int zce_result = ::closesocket (handle);

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;
#elif defined (ZCE_OS_LINUX)
    return ::close(handle);
#endif
}

//���ӷ�����
inline int zce::connect (ZCE_SOCKET handle,
                         const sockaddr *addr,
                         socklen_t addrlen)
{

#if defined ZCE_OS_WINDOWS
    int zce_result = ::connect (handle,
                                addr,
                                addrlen);

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
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

//ȡ�öԶ˵ĵ�ַ��Ϣ
inline int zce::getpeername (ZCE_SOCKET handle,
                             sockaddr *addr,
                             socklen_t *addrlen)
{

#if defined ZCE_OS_WINDOWS
    int zce_result = ::getpeername (handle,
                                    addr,
                                    addrlen);

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
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

//ȡ�öԶ˵ĵ�ַ��Ϣ
inline int zce::getsockname (ZCE_SOCKET handle,
                             sockaddr *addr,
                             socklen_t *addrlen)
{
#if defined ZCE_OS_WINDOWS

    int zce_result = ::getsockname (handle,
                                    addr,
                                    addrlen);

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
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

//ȡ��ĳ��ѡ�ע��WINDOWS��LINUX�����кܶ�Ĵ������ģ���ע��
inline int zce::getsockopt (ZCE_SOCKET handle,
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

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
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

inline int zce::setsockopt (ZCE_SOCKET handle,
                            int level,
                            int optname,
                            const void *optval,
                            socklen_t optlen)
{

#if defined (ZCE_OS_WINDOWS)

    //Windowsƽ̨�£�SO_REUSEPORT ��SO_REUSEADDR Ч��һ�£�SO_REUSEADDR ��Ĭ��Ч��
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

    //ͳһ���󷵻�
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

//��һ�������˿�
inline int zce::listen (ZCE_SOCKET handle, int backlog)
{
#if defined (ZCE_OS_WINDOWS)

    int zce_result =  ::listen (handle, backlog);

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;

#elif defined (ZCE_OS_LINUX)

    return ::listen (handle, backlog);
#endif

}

//shutdown��how������LINUX�¿���,SHUT_RD,SHUT_WR,SHUT_RDWR
inline int zce::shutdown (ZCE_SOCKET handle, int how)
{
#if defined (ZCE_OS_WINDOWS)

    int zce_result = ::shutdown (handle, how);

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;

#elif defined (ZCE_OS_LINUX)

    return ::shutdown (handle, how);
#endif

}

//��������
//����flagsͨ���Էǳ��LINUX����WINDOWS�¼�����ͨ�ã�LINUX����һ���Ƚ����õĲ���MSG_DONTWAIT������WINDOWS�²��ṩ��
//����������ʹ��EWOULDBLOCK
inline ssize_t zce::recv (ZCE_SOCKET handle, void *buf, size_t len, int flags)
{

#if defined (ZCE_OS_WINDOWS)
    int zce_result = ::recv (handle,
                             static_cast<char *>(buf),
                             static_cast<int> (len),
                             flags);

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;

#elif defined (ZCE_OS_LINUX)

    ssize_t zce_result  = 0;
    zce_result = ::recv (handle, buf, len, flags);

    //ͳһ�����EWOULDBLOCK
# if (EAGAIN != EWOULDBLOCK)

    if (zce_result == -1 && errno == EAGAIN)
    {
        errno = EWOULDBLOCK;
    }

# endif //# if (EAGAIN != EWOULDBLOCK)

    return zce_result;
#endif
}

//��������
inline ssize_t zce::send (ZCE_SOCKET handle,
                          const void *buf,
                          size_t len,
                          int flags)
{

#if defined (ZCE_OS_WINDOWS)

    int zce_result = ::send (handle,
                             static_cast<const char *>(buf),
                             static_cast<int> (len),
                             flags);

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
    if ( SOCKET_ERROR == zce_result)
    {
        errno = ::WSAGetLastError ();
    }

    return zce_result;

#elif defined (ZCE_OS_LINUX)

    ssize_t const zce_result = ::send (handle, buf, len, flags);

    //ͳһ�����EWOULDBLOCK,ACE����˵������ĳЩʱ���������ʧЧ�����Ǿ�����֪����LINUXƽ̨��EAGAIN == EWOULDBLOCK
    //��δ��������Ϊһ�ַ���
# if (EAGAIN != EWOULDBLOCK)

    if (zce_result == -1 && errno == EAGAIN)
    {
        errno = EWOULDBLOCK;
    }

# endif //# if (EAGAIN != EWOULDBLOCK)

    return zce_result;

#endif
}

inline ssize_t zce::recvfrom (ZCE_SOCKET handle,
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
        //MSG_PEEK�Ľ���
        //Peek at the incoming data. The data is copied into the buffer, but is not removed from the input queue. This flag is valid only for non-overlapped sockets.

        //Ҳ����������������־flags��������Ϣ�����Ĵ��󣬷�����Ϣ���ܻ��峤�ȸ�������
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
inline ssize_t zce::sendto (ZCE_SOCKET handle,
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

    //��������Ϣ���õ�errno����ϸ��ο�����zce���ֿռ����Ľ���
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
//����timeout_tv�����Ƿ�ʱ�����TCP���պ�����
//timeout_tv !=NULL��ʶ��ʱ��������N���ֽں󷵻�,��ʱ�õ���select���鳬ʱ����
//timeout_tv ==NULL ��ʶ�����г�ʱ����,����Socket״̬�Լ�����
inline ssize_t zce::recv_n (ZCE_SOCKET handle,
                            void *buf,
                            size_t len,
                            ZCE_Time_Value *timeout_tv,
                            int flags)
{
    if (timeout_tv)
    {
        return zce::recvn_timeout(handle,
                                  buf,
                                  len,
                                  *timeout_tv,
                                  flags);
    }
    else
    {
        return zce::recvn(handle,
                          buf,
                          len,
                          flags);
    }
}

//����timeout_tv�����Ƿ�ʱ�����TCP���ͺ���
//timeout_tv !=NULL��ʶ��ʱ��������N���ֽں󷵻�,��ʱ�õ���select���鳬ʱ���� ,
//timeout_tv ==NULL ��ʶ�����г�ʱ����,����Socket״̬�Լ�����,�����������߷�����,
inline ssize_t zce::send_n (ZCE_SOCKET handle,
                            const void *buf,
                            size_t len,
                            ZCE_Time_Value *timeout_tv,
                            int flags)
{
    if (timeout_tv)
    {
        return zce::sendn_timeout(handle,
                                  buf,
                                  len,
                                  *timeout_tv,
                                  flags);
    }
    else
    {
        return zce::sendn(handle,
                          buf,
                          len,
                          flags);

    }
}

//����timeout_tv�����Ƿ�ʱ�����UDP���պ�����
//timeout_tv !=NULL��ʶ��ʱ��������N���ֽں󷵻�,��ʱ�õ���select���鳬ʱ���� ,
//timeout_tv ==NULL ��ʶ�����г�ʱ����,����Socket״̬�Լ�����,�����������߷�����,
inline ssize_t zce::recvfrom (ZCE_SOCKET handle,
                              void *buf,
                              size_t len,
                              sockaddr *from,
                              socklen_t *from_len,
                              ZCE_Time_Value *timeout_tv,
                              int flags)
{
    if (timeout_tv)
    {
        return zce::recvfrom_timeout(handle,
                                     buf,
                                     len,
                                     from,
                                     from_len,
                                     *timeout_tv,
                                     flags);
    }
    else
    {
        return zce::recvfrom(handle,
                             buf,
                             len,
                             flags,
                             from,
                             from_len);
    }
}

//UDP�ķ�����ʱ�ǲ��������ģ����ó�ʱ����д���������ȫ��Ϊ�˺�ǰ�����
//����UDP������,����ʱ�������������ʵ���Ͻ���û�г�ʱ����
inline ssize_t zce::sendto (ZCE_SOCKET handle,
                            const void *buf,
                            size_t len,
                            const sockaddr *addr,
                            int addrlen,
                            ZCE_Time_Value *timeout_tv,
                            int flags)
{
    if (timeout_tv)
    {
        return zce::sendto_timeout(handle,
                                   buf,
                                   len,
                                   addr,
                                   addrlen,
                                   *timeout_tv,
                                   flags);
    }
    else
    {
        return zce::sendto(handle,
                           buf,
                           len,
                           flags,
                           addr,
                           addrlen);
    }
}

//--------------------------------------------------------------------------------------------
//SELECT������Ϊ�˺�LINUXƽ̨���룬����ʱ��Ϊʣ��ʱ��
inline int zce::select(
    int nfds,
    fd_set *readfds,
    fd_set *writefds,
    fd_set *exceptfds,
    ZCE_Time_Value *timeout_tv
)
{
#if defined (ZCE_OS_WINDOWS)

    //����Ƿ�û�о���ڵȴ�
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

    //�������Ҫ�ȴ������ֱ��ʹ��sleep���select��
    //������ΪWINDOWS��select ����ȴ�һ�����������ͻ�return -1��������鷳��
    if (no_handle_to_wait)
    {
        zce::sleep(*timeout_tv);
        timeout_tv->set(0, 0);
        return 0;
    }

    clock_t start_clock = 0;
    timeval *select_tv = NULL;

    if (timeout_tv)
    {
        start_clock = std::clock();
        //�к������ڲ���timv_valȡ����
        select_tv = (*timeout_tv);
    }

    //WINDOWS�£�timeval��һ��const ����
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
        //�к������ڲ���timv_valȡ����
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

//���������Ϊ�˷���ƽ̨�����дд��һ�������������ò�ͬƽ̨��fd_setʵ�ּӿ��ٶȣ�
//��Ȼ����ô���κ�ͨ���ԣ�sorry��׷�������ˡ�
inline bool zce::is_ready_fds(int no_fds,
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

//���ض˿ں�
inline uint16_t zce::get_port_number(const sockaddr *addr)
{
    if (AF_INET == addr->sa_family)
    {
        return ntohs(((sockaddr_in *)(addr))->sin_port );
    }
    else if (AF_INET6 == addr->sa_family)
    {
        return ntohs(((sockaddr_in6 *)(addr))->sin6_port);
    }
    else
    {
    }

}

//���ص�ַ��Ϣ
inline const char *zce::get_host_addr(const sockaddr *addr,
                                      char *addr_buf,
                                      size_t addr_size)
{
    return zce::inet_ntop(addr->sa_family,
                          (void *)(addr),
                          addr_buf,
                          addr_size);
}

inline const char *zce::get_host_addr_port(const sockaddr *addr,
                                           char *addr_buf,
                                           size_t addr_size)
{
    uint16_t port = 0;
    if (AF_INET == addr->sa_family)
    {
        port = ntohs(((sockaddr_in *)(addr))->sin_port);
    }
    else if (AF_INET6 == addr->sa_family)
    {
        port = ntohs(((sockaddr_in6 *)(addr))->sin6_port);
    }
    else
    {
    }
    zce::inet_ntop(addr->sa_family, (void *)(addr), addr_buf, addr_size);
    size_t str_len = strlen(addr_buf);
    snprintf(addr_buf + str_len, addr_size - str_len, "#%u", port);
    return addr_buf;
}


//�Ǳ�׼�������������밲ȫ
//����ַ��Ϣ��ӡ��������ַ��ʹ��������
inline const char *zce::inet_ntoa(uint32_t in, char *addr_buf, size_t addr_size)
{
    sockaddr_in sock_addr_ipv4;
    sock_addr_ipv4.sin_addr.s_addr = ntohl(in);
    return zce::inet_ntop(AF_INET,
                          (void *)(&sock_addr_ipv4),
                          addr_buf,
                          addr_size);
};

//����������WIN32��LINUX����
//ͨ��IPV4�ĵ�ַ���õ���Ӧ���ַ�����ʾ���������뺯������̫����ʹ��
inline const char *zce::inet_ntoa(struct in_addr in)
{
    return ::inet_ntoa(in);
}

//ͨ��һ��IP��ַ���ַ�����ʾ�õ�һ��IPV4�ĵ�����
inline uint32_t zce::inet_addr(const char *cp)
{
    return ::inet_addr(cp);
};

//����IP��ַ������
inline uint32_t zce::get_ip_address(const sockaddr_in *sock_addr_ipv4)
{
    return ntohl(sock_addr_ipv4->sin_addr.s_addr);
}

//--------------------------------------------------------------------------------------------------------------------

//����һ��IPV4�ĵ�ַ,
inline int zce::set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                                const char *ipv4_addr_str,
                                uint16_t ipv4_port)
{
    sock_addr_ipv4->sin_family = AF_INET;
    //htons��ĳЩ�������һ���꣬
    //sock_addr_ipv4->sin_port = ::htons(ipv4_port);
    sock_addr_ipv4->sin_port = htons(ipv4_port);

    //��ĳЩƽ̨��sockaddr_in��һ�����ȱ�ʶ������BSD��ϵ��
#if 0
    sock_addr_ipv4->sa_len = sizeof(sockaddr_in);
#endif

    sock_addr_ipv4->sin_addr.s_addr = ::inet_addr(ipv4_addr_str);

    //ע��inet_pton�ķ���ֵ
    if (sock_addr_ipv4->sin_addr.s_addr == INADDR_NONE)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

//����һ��IPV4�ĵ�ַ,����ַ���������#������Ϊ�����ж˿ںţ���ͬʱ��ȡ�˿ںţ�����˿ں�����0
inline int zce::set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                                const char *ipv4_addr_str)
{
    int ret = zce::set_sockaddr_in(sock_addr_ipv4, ipv4_addr_str, 0);
    if (ret != 0)
    {
        return ret;
    }
    //�����#�����Һ��滹�пռ䣬
    const char *port_pos = strchr(ipv4_addr_str, '#');
    if ( port_pos != NULL && *(++port_pos) != '\0' )
    {
        //ע�⵽���pos�Ѿ�++�ˡ�
        uint16_t read_port = 0;
        int fields = sscanf(port_pos, "%hu", &read_port);
        if (fields != 1)
        {
            ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
        }
        sock_addr_ipv4->sin_port = htons(read_port);
    }

    return 0;
}


//����һ��IPV4�ĵ�ַ,���󷵻�NULL����ȷ�������õĵ�ַ�ı任
inline int zce::set_sockaddr_in(sockaddr_in *sock_addr_ipv4,
                                uint32_t ipv4_addr_val,
                                uint16_t ipv4_port
                               )
{
    //memset(sock_addr_ipv4,0,sizeof(sockaddr_in));
    sock_addr_ipv4->sin_family = AF_INET;
    sock_addr_ipv4->sin_port = htons(ipv4_port);
    sock_addr_ipv4->sin_addr.s_addr = htonl(ipv4_addr_val);

    //��ĳЩƽ̨��sockaddr_in��һ�����ȱ�ʶ������BSD��ϵ��
#if 0
    sock_addr_ipv4->sa_len = sizeof(sockaddr_in);
#endif

    return 0;
}

//����һ��IPV6�ĵ�ַ,
inline int zce::set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                                 const char *ipv6_addr_str,
                                 uint16_t ipv6_port)
{
    sock_addr_ipv6->sin6_family = AF_INET6;
    sock_addr_ipv6->sin6_port = htons(ipv6_port);

    int ret = zce::inet_pton(AF_INET6,
                             ipv6_addr_str,
                             static_cast<void *>(&(sock_addr_ipv6->sin6_addr.s6_addr)));

    //ע��inet_pton�ķ���ֵ,��ת������Ϥ�ķ���
    if (ret == 1)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

//����һ��IPV6�ĵ�ַ,����ж˿ں���Ϣ��Ҳ��
inline int zce::set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                                 const char *ipv6_addr_str)
{
    int ret = set_sockaddr_in6(sock_addr_ipv6, ipv6_addr_str, 0);
    if (ret != 0)
    {
        return ret;
    }
    //�����#�����Һ��滹�пռ䣬
    const char *port_pos = strchr(ipv6_addr_str, '#');
    if (port_pos != NULL && *(++port_pos) != '\0')
    {
        //ע�⵽���pos�Ѿ�++�ˡ�
        uint16_t read_port = 0;
        int fields = sscanf(port_pos, "%hu", &read_port);
        if (fields != 1)
        {
            ZCE_TRACE_FAIL_INFO(RS_ERROR, "sscanf");
        }
        sock_addr_ipv6->sin6_port = htons(read_port);
    }
    return 0;
}


//����һ��IPV6�ĵ�ַ,���󷵻�NULL����ȷ�������õĵ�ַ�ı任
inline int zce::set_sockaddr_in6(sockaddr_in6 *sock_addr_ipv6,
                                 uint16_t ipv6_port,
                                 const char ipv6_addr_val[16])
{
    //16bytes��128bit��IPV6�ĵ�ַ��Ϣ
    const size_t IPV6_INET6_LEN = 16;

    sock_addr_ipv6->sin6_family = AF_INET6;
    sock_addr_ipv6->sin6_port = htons(ipv6_port);
    memcpy(sock_addr_ipv6->sin6_addr.s6_addr, ipv6_addr_val, IPV6_INET6_LEN);
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------
//���ص�ǰ��������������,2��ƽ̨Ӧ�ö�֧���������
inline int zce::gethostname(char *name, size_t name_len)
{
#if defined (ZCE_OS_WINDOWS)
    return ::gethostname(name, static_cast<int>(name_len) );
#elif defined (ZCE_OS_LINUX)
    return ::gethostname(name, name_len);
#endif

}

#endif //ZCE_LIB_OS_ADAPT_SOCKET_H_

