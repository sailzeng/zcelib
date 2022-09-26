#ifndef SOARING_LIB_SERVICES_PEER_H_
#define SOARING_LIB_SERVICES_PEER_H_

#pragma pack (1)
/****************************************************************************************************
class  OGRE_PEER_ID 　SOCKET PEER信息,适应IPV4的代码。
****************************************************************************************************/
class OGRE_PEER_ID
{
public:
    //构造,析构函数,默认为0
    explicit OGRE_PEER_ID(unsigned int peer_ip_address = INVALID_PERR_IP_ADDRESS,
                          unsigned short peer_port = INVALID_PERR_PORT);
    explicit OGRE_PEER_ID(const zce::skt::addr_in&);
    //
    ~OGRE_PEER_ID();

    //设置
    void set(unsigned int peer_ip_address, unsigned short peer_port);
    //根据zce::skt::addr_in的IP地址设置
    void set(const zce::skt::addr_in&);

    //比较函数,services_type_,services_id_
    bool operator ==(const OGRE_PEER_ID& others) const;
    //比较不同函数,
    bool operator !=(const OGRE_PEER_ID& others) const;
    //有个排序需求,
    bool operator <(const OGRE_PEER_ID& others) const;

public:
    //无效的SERVICE 类型
    static const uint16_t  INVALID_PERR_IP_ADDRESS = 0;
    //无效的SERVICE ID
    static const uint32_t  INVALID_PERR_PORT = 0;

public:

    //IP地址
    uint32_t          peer_ip_address_ = INVALID_PERR_IP_ADDRESS;
    //端口号码
    uint16_t          peer_port_ = INVALID_PERR_PORT;
};

#pragma pack ()

class HASH_OF_OGREPEERID
{
public:
    size_t operator()(const OGRE_PEER_ID& peer_info) const
    {
        return (size_t(peer_info.peer_port_) << 16) + peer_info.peer_ip_address_;
    }
};

#endif //#ifndef SOARING_LIB_SERVICES_PEER_H_
