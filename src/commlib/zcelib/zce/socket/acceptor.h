#ifndef ZCE_LIB_SOCKET_ACCEPTOR_CONNECTOR_H_
#define ZCE_LIB_SOCKET_ACCEPTOR_CONNECTOR_H_

#include "zce/util/non_copyable.h"
#include "zce/os_adapt/socket.h"
#include "zce/os_adapt/time.h"

class ZCE_Socket_Stream;
class ZCE_Socket_Base;

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011年6月19日
Class           : ZCE_Socket_Acceptor
Inherit         :
Description     : Acceptor的工厂，用于产生ZCE_Socket_Stream
Other           :
Modify Record   :
************************************************************************************************************/
class ZCE_Socket_Acceptor  : public ZCE_Socket_Base
{

public:
    ZCE_Socket_Acceptor();
    ~ZCE_Socket_Acceptor();

public:

    //打开一个监听地址，目前只支持AF_INET,和AFINET6
    int open(const ZCE_Sockaddr *local_addr,
             bool reuse_addr = true,
             int protocol_family = AF_UNSPEC,
             int backlog = ZCE_DEFAULT_BACKLOG,
             int protocol = 0);

    //接受一个SOCKET
    int accept (ZCE_Socket_Stream &new_stream,
                ZCE_Sockaddr *remote_addr) const;

    //带有超时的处理的accept
    int accept (ZCE_Socket_Stream &new_stream,
                ZCE_Time_Value &timeout,
                ZCE_Sockaddr *remote_addr) const;

};

#endif //#ifndef ZCE_LIB_SOCKET_ACCEPTOR_CONNECTOR_H_

