/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   soar/zerg/frame_zerg.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2021年4月17日
* @brief      很无奈，我还是讲所有的类，整理成了struct，因为如果要兼容多个版本，
*             类实在不是一个好的写法。
*
* @details
*
*
*
* @note
*
*/

#pragma once

#include "soar/enum/enum_define.h"
#include "soar/zerg/frame_command.h"
#include "soar/enum/error_code.h"

namespace soar
{
#pragma pack (1)

/*!
* @brief      服务编号信息
*
* note
*/
class  SERVICES_ID
{
public:

    SERVICES_ID(uint16_t svc_type,uint32_t svc_id):
        services_type_(svc_type),
        services_id_(svc_id)
    {
    }
    SERVICES_ID() = default;
    ~SERVICES_ID() = default;
    SERVICES_ID(const SERVICES_ID &) = default;
    SERVICES_ID &operator=(const SERVICES_ID &) = default;

    void  clear()
    {
        services_type_ = 0;
        services_id_ = 0;
    }

    /*!
    * @brief      从string中得到SVC ID
    * @return     int == 0 表示成功
    * @param      str 转换的字符串
    * @param      check_valid 检查是否有效,svcid的type和id 不能是0
    */
    int from_str(const char *str,
                 bool check_valid = false);

    ///转换string
    const char *to_str(char *str_buffer,size_t buf_len);

    ///设置
    void set_svcid(uint16_t svrtype,uint16_t svrid);

    ///比较函数,services_type_,services_id_
    bool operator ==(const SERVICES_ID &others) const;
    ///比较不同函数,
    bool operator !=(const SERVICES_ID &others) const;
    ///有个排序需求,
    bool operator <(const SERVICES_ID &others) const;

public:

    ///无效的SERVICE 类型
    static const uint16_t INVALID_SERVICES_TYPE = 0;

    ///无效的SERVICE ID
    static const uint32_t INVALID_SERVICES_ID = 0;

    ///让ZERG帮忙动态分配一个SERVICES_ID ，用于某些客户端请求
    static const uint32_t DYNAMIC_ALLOC_SERVICES_ID = static_cast<uint16_t>(-1);

    ///对某个type进行广播。
    static const uint32_t BROADCAST_SERVICES_ID = static_cast<uint16_t>(-2);

    ///使用均衡负载的方式动态分片的SERVICE ID
    static const uint32_t LOAD_BALANCE_DYNAMIC_ID = static_cast<uint16_t>(-3);

    ///使用主从的方式（根据AUTO CONNECT的顺序）动态分配的SERVICE ID
    static const uint32_t MAIN_STANDBY_DYNAMIC_ID = static_cast<uint16_t>(-4);

public:

    ///服务的类型,参考枚举值SERVICES_TYPE，无效时用INVALID_SERVICES_TYPE
    uint16_t           services_type_ = INVALID_SERVICES_TYPE;

    ///服务的编号,采用两种表现方式,服务编号可以是一个服务器定义值,也可以是一个UIN.
    uint32_t           services_id_ = INVALID_SERVICES_ID;
};

#pragma pack ()
}
