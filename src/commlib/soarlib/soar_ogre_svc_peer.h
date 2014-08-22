#ifndef SOARING_LIB_SERVICES_PEER_H_
#define SOARING_LIB_SERVICES_PEER_H_

#pragma pack (1)
/****************************************************************************************************
class  Socket_Peer_Info 　SOCKET PEER信息,适应IPV4的代码。
****************************************************************************************************/
class SOARING_EXPORT Socket_Peer_Info
{
public:
    //无效的SERVICE 类型
    static const unsigned short  INVALID_PERR_IP_ADDRESS = 0;
    //无效的SERVICE ID
    static const unsigned int   INVALID_PERR_PORT   = 0;

public:

    //IP地址
    unsigned int            peer_ip_address_;
    //端口号码
    unsigned short          peer_port_;

public:
    //构造,析构函数,默认为0
    explicit Socket_Peer_Info(unsigned int peer_ip_address = INVALID_PERR_IP_ADDRESS, unsigned short peer_port = INVALID_PERR_PORT);
    explicit Socket_Peer_Info(const ZCE_Sockaddr_In &);
    //
    ~Socket_Peer_Info();

    //设置
    void SetSocketPeerInfo(unsigned int peer_ip_address, unsigned short peer_port );
    //根据ZCE_Sockaddr_In的IP地址设置
    void SetSocketPeerInfo(const ZCE_Sockaddr_In &);

    //比较函数,services_type_,services_id_
    bool operator ==(const Socket_Peer_Info &others) const;
    //比较不同函数,
    bool operator !=(const Socket_Peer_Info &others) const;
    //有个排序需求,
    bool operator <(const Socket_Peer_Info &others) const;
};

#pragma pack ()

class SOARING_EXPORT HashofSocketPeerInfo
{
public:
    size_t operator()(const Socket_Peer_Info &peer_info) const
    {
        return (size_t (peer_info.peer_port_) << 16) + peer_info.peer_ip_address_ ;
    }
};

#endif //#ifndef SOARING_LIB_SERVICES_PEER_H_

