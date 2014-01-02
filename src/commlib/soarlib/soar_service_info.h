#ifndef SOARING_LIB_SERVICES_INFO_H_
#define SOARING_LIB_SERVICES_INFO_H_

#include "soar_enum_define.h"

#pragma pack (1)
/****************************************************************************************************
class  SERVICES_ID 　服务编号信息,
****************************************************************************************************/
struct SOARING_EXPORT SERVICES_ID
{
public:

    //无效的SERVICE 类型
    static const unsigned short  INVALID_SERVICES_TYPE = 0;

    //无效的SERVICE ID
    static const unsigned int    INVALID_SERVICES_ID   = 0;
    //表示是动态分片的SERVICE ID
    static const unsigned int    DYNAMIC_ALLOC_SERVICES_ID = static_cast<unsigned int>(-1);

public:

    //服务的类型,参考枚举值SERVICES_TYPE，无效时用INVALID_SERVICES_TYPE
    unsigned short           services_type_;

    //服务的编号,采用两种表现方式,服务编号可以是一个服务器定义值,也可以是一个UIN.
    unsigned int             services_id_;

public:
    //构造,析构函数,默认为0
    explicit SERVICES_ID(unsigned short svrtype = INVALID_SERVICES_TYPE, unsigned int svrno = INVALID_SERVICES_ID);
    //
    ~SERVICES_ID();

    void  clear()
    {
        services_type_ = 0;
        services_id_ = 0;
    }

    //
    void set_serviceid(unsigned short svrtype, unsigned int svrid );

    //比较函数,services_type_,services_id_
    bool operator ==(const SERVICES_ID &others) const;
    //比较不同函数,
    bool operator !=(const SERVICES_ID &others) const;
    //有个排序需求,
    bool operator <(const SERVICES_ID &others) const;
};

#pragma pack ()

//HASH函数,用于得到HASH Key

class SOARING_EXPORT HashofSvrInfo
{
public:
    size_t operator()(const SERVICES_ID &svrinfo) const
    {
        return (size_t (svrinfo.services_type_) << 16) + svrinfo.services_id_ ;
    }
};

/******************************************************************************************
struct SERVICES_IP_INFO 服务ID信息 + IP信息
******************************************************************************************/
struct SOARING_EXPORT SERVICES_IP_INFO
{
public:
    //服务ID信息
    SERVICES_ID              services_info_;

    //服务IP
    unsigned int             ip_addr_;

    //服务端口
    unsigned short           port_;

    //服务器所属 IDC的编号
    unsigned int             idc_no_;

    //服务器编号
    unsigned int             server_guid_no_;

    //额外的配置信息,是一个字符串，各个模块自己解析
    char                     cfg_info_[MAX_NAME_LEN_STRING + 1];

public:
    //
    SERVICES_IP_INFO();
    ~SERVICES_IP_INFO();
};

//得到KEY的HASH函数
class SOARING_EXPORT HashofSvrIPInfo
{
public:
    size_t operator()(const SERVICES_IP_INFO &svripinfo) const
    {
        return (size_t (svripinfo.services_info_.services_type_) << 16) + svripinfo.services_info_.services_id_ ;
    }
};

class SOARING_EXPORT EqualSvrIPInfo
{
public:
    //注意判断条件不是所有的变量
    bool operator()(const SERVICES_IP_INFO &right, const SERVICES_IP_INFO &left) const
    {
        //检查SVC INFO的相等,就认为相等
        if (right.services_info_ == left.services_info_ )
        {
            return true;
        }

        return false;
    }
};

#endif //#ifndef SOARING_LIB_SERVICES_INFO_H_

