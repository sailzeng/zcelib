#ifndef ZCE_LIB_SOCKET_ACCEPTOR_CONNECTOR_H_
#define ZCE_LIB_SOCKET_ACCEPTOR_CONNECTOR_H_

#include "zce_boost_non_copyable.h"
#include "zce_os_adapt_socket.h"
#include "zce_os_adapt_time.h"

class ZCE_Socket_Stream;
class ZCE_Socket_Base;

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011��6��19��
Class           : ZCE_Socket_Acceptor
Inherit         :
Description     : Acceptor�Ĺ��������ڲ���ZCE_Socket_Stream
Other           :
Modify Record   :
************************************************************************************************************/
class ZCE_Socket_Acceptor  : public ZCE_Socket_Base
{

public:
    ZCE_Socket_Acceptor();
    ~ZCE_Socket_Acceptor();

public:

    //��һ��������ַ��Ŀǰֻ֧��AF_INET,��AFINET6
    int open(const ZCE_Sockaddr *local_addr,
             bool reuse_addr = true,
             int protocol_family = AF_UNSPEC,
             int backlog = ZCE_DEFAULT_BACKLOG,
             int protocol = 0);

    //����һ��SOCKET
    int accept (ZCE_Socket_Stream &new_stream,
                ZCE_Sockaddr *remote_addr) const;

    //���г�ʱ�Ĵ����accept
    int accept (ZCE_Socket_Stream &new_stream,
                ZCE_Time_Value &timeout,
                ZCE_Sockaddr *remote_addr) const;

};

#endif //#ifndef ZCE_LIB_SOCKET_ACCEPTOR_CONNECTOR_H_

