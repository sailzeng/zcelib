#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/zerg/services_id.h"

namespace soar
{
//构造函数和析构函数

//
void SERVICES_ID::set_svcid(uint16_t svrtype,uint16_t svrid)
{
    services_type_ = svrtype;
    services_id_ = svrid;
}

//比较是否相同的函数
bool SERVICES_ID::operator ==(const SERVICES_ID &others) const
{
    if (services_type_ == others.services_type_ && services_id_ == others.services_id_)
    {
        return true;
    }
    return false;
}

//比较是否不同的函数
bool SERVICES_ID::operator !=(const SERVICES_ID &others) const
{
    if (services_type_ != others.services_type_ || services_id_ != others.services_id_)
    {
        return true;
    }
    return false;
}

//比较大小的函数
bool SERVICES_ID::operator <(const SERVICES_ID &others) const
{
    if (services_type_ + services_id_ < others.services_type_ + others.services_id_)
    {
        return true;
    }

    return false;
}

///转换string
const char *SERVICES_ID::to_str(char *str_buffer,size_t buf_len)
{
    snprintf(str_buffer,
             buf_len,
             "%hu.%u",
             services_type_,
             services_id_);
    return str_buffer;
}

int SERVICES_ID::from_str(const char *str_buffer,bool check_valid)
{
    //注意.前面的%号
    int ret_num = sscanf(str_buffer,
                         "%hu.%u",
                         &services_type_,
                         &services_id_);
    if (check_valid
        && (INVALID_SERVICES_TYPE == services_type_ || INVALID_SERVICES_ID == services_id_))
    {
        return SOAR_RET::ERROR_SERVICES_ID_INVALID;
    }
    if (ret_num != 2)
    {
        return SOAR_RET::ERROR_STRING_TO_SVCID_FAIL;
    }
    else
    {
        return 0;
    }
}
}