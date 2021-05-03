#ifndef SOARING_LIB_SERVICES_INFO_H_
#define SOARING_LIB_SERVICES_INFO_H_

#include "soar/enum/enum_define.h"
#include "soar/zerg/services_id.h"


//HASH函数,用于得到HASH Key

namespace soar
{


struct  HASH_OF_SVCID
{
public:
    size_t operator()(const soar::SERVICES_ID &svrinfo) const
    {
        return (size_t (svrinfo.services_type_) << 16) + svrinfo.services_id_ ;
    }
};

/******************************************************************************************
struct SERVICES_IP_INFO 服务ID信息 + IP信息
******************************************************************************************/
struct  SERVICES_INFO
{
public:

    ///从字符串中获取
    int from_str(const char *str,
                 bool check_valid = false);

    ///转换string
    const char *to_str(char *str_buffer, size_t buf_len);

public:

    ///服务ID信息
    soar::SERVICES_ID svc_id_;


    ///服务IP,服务端口
    zce::Sockaddr_In  ip_address_;

    ///业务ID,游戏ID
    unsigned int     business_id_ = 0;

    ///服务器所属 IDC的编号
    unsigned int     idc_no_ = 0;
};

//得到KEY的HASH函数
struct  HASH_OF_SVCINFO
{
public:
    size_t operator()(const SERVICES_INFO &svripinfo) const
    {
        return (size_t (svripinfo.svc_id_.services_type_) << 16) + svripinfo.svc_id_.services_id_ ;
    }
};

struct  EQUAL_OF_SVCINFO
{
public:
    //注意判断条件不是所有的变量
    bool operator()(const SERVICES_INFO &right, const SERVICES_INFO &left) const
    {
        //检查SVC INFO的相等,就认为相等
        if (right.svc_id_ == left.svc_id_ )
        {
            return true;
        }

        return false;
    }
};

}

#endif //#ifndef SOARING_LIB_SERVICES_INFO_H_

