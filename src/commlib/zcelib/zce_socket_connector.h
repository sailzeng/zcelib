#ifndef ZCE_LIB_SOCKET_CONNECTOR_H_
#define ZCE_LIB_SOCKET_CONNECTOR_H_

#include "zce_boost_non_copyable.h"

class ZCE_Time_Value;
class ZCE_Sockaddr;

/************************************************************************************************************
Author          : Sailzeng ZENGXING  Date Of Creation: 2011年6月19日
Class           : ZCE_Socket_Connector
Inherit         :
Description     : connect的工厂，成功连接后产生,用于产生ZCE_Socket_Stream
Other           :
Modify Record   :
************************************************************************************************************/
class ZCELIB_EXPORT ZCE_Socket_Connector
{
public:
    //构造函数和析构函数
    ZCE_Socket_Connector (void);
    ~ZCE_Socket_Connector (void);

    //带超时的连接
    int connect (ZCE_Socket_Stream &new_stream,
                 const ZCE_Sockaddr *remote_addr ,
                 ZCE_Time_Value  &timeout,
                 bool reuse_addr = false,
                 int protocol = 0,
                 const ZCE_Sockaddr *local_addr = NULL);

    //阻塞或者非阻塞的连接
    int connect (ZCE_Socket_Stream &new_stream,
                 const ZCE_Sockaddr *remote_addr ,
                 bool non_blocing,
                 bool reuse_addr = false,
                 int protocol = 0,
                 const ZCE_Sockaddr *local_addr = NULL);

};

#endif //#ifndef ZCE_LIB_SOCKET_CONNECTOR_H_

