#include "zce/predefine.h"
#include "zce/uuid/uuid.h"

namespace zce
{
/************************************************************************************************************
Class           : uuid64
************************************************************************************************************/

// < 运算符
bool uuid64::operator < (const uuid64& others) const
{
    if (this->u_uint64_ < others.u_uint64_)
    {
        return true;
    }

    return false;
}

// == 运算符
bool uuid64::operator == (const uuid64& others) const
{
    if (this->u_uint64_ == others.u_uint64_)
    {
        return true;
    }

    return false;
}

//转换为字符串
const char* uuid64::to_str(char* buffer, size_t buf_len, size_t& use_buf) const
{
    //如果传递的BUFFER空间不够，直接返回nullptr
    if (buf_len < LEN_OF_ZCE_UUID64_STR + 1)
    {
        return nullptr;
    }

    int ret = snprintf(buffer, buf_len, "%08x-%08x", this->u_2uint32_[1], this->u_2uint32_[0]);
    if (ret < 0 || ret > static_cast<int>(buf_len))
    {
        return nullptr;
    }
    use_buf = LEN_OF_ZCE_UUID64_STR;
    return buffer;
}

// 转移成一个uint64_t的结构
uuid64::operator uint64_t ()
{
    return this->u_uint64_;
}

/************************************************************************************************************
Class           : uuid128
************************************************************************************************************/
uuid128::uuid128()
{
    u_4uint32_[0] = 0;
    u_4uint32_[1] = 0;
    u_4uint32_[2] = 0;
    u_4uint32_[3] = 0;
}

uuid128::~uuid128()
{
}

// < 运算符
bool uuid128::operator < (const uuid128& others) const
{
    if (u_4uint32_[3] < others.u_4uint32_[3])
    {
        return true;
    }
    else if (u_4uint32_[3] > others.u_4uint32_[3])
    {
        return false;
    }

    //else (u_4uint32_[3] == others.u_4uint32_[3])

    if (u_4uint32_[2] < others.u_4uint32_[2])
    {
        return true;
    }
    else if (u_4uint32_[2] > others.u_4uint32_[2])
    {
        return false;
    }

    //else (u_4uint32_[2] == others.u_4uint32_[2])

    if (u_4uint32_[1] < others.u_4uint32_[1])
    {
        return true;
    }
    else if (u_4uint32_[1] > others.u_4uint32_[1])
    {
        return false;
    }

    //else (u_4uint32_[1] == u_4uint32_[1])

    if (u_4uint32_[0] < others.u_4uint32_[0])
    {
        return true;
    }
    else
    {
        return false;
    }
}

// == 运算符
bool uuid128::operator == (const uuid128& others) const
{
#if defined ZCE_OS32
    if (this->u_4uint32_[0] == others.u_4uint32_[0] &&
        this->u_4uint32_[1] == others.u_4uint32_[1] &&
        this->u_4uint32_[2] == others.u_4uint32_[2] &&
        this->u_4uint32_[3] == others.u_4uint32_[3])
    {
        return true;
    }
#elif defined ZCE_OS64
    if (this->u_2uint64_[0] == others.u_2uint64_[0] &&
        this->u_2uint64_[1] == others.u_2uint64_[1])
    {
        return true;
    }
#endif
    return false;
}

//转换为字符串,这儿采用的格式是标准的8-4-4-4-12，而不是GUID的8-4-4-16的格式
const char* uuid128::to_str(char* buffer,
                            size_t buf_len,
                            size_t& use_buf) const
{
    //如果传递的BUFFER空间不够，干脆什么都不做,直接返回nullptr,长度要考虑'\0'
    if (buf_len < LEN_OF_ZCE_UUID128_STR + 1)
    {
        return nullptr;
    }

    //输出8-4-4-4-12的格式
    int ret = snprintf(buffer, buf_len, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                       this->u_16uint8_[15],
                       this->u_16uint8_[14],
                       this->u_16uint8_[13],
                       this->u_16uint8_[12],
                       this->u_16uint8_[11],
                       this->u_16uint8_[10],
                       this->u_16uint8_[9],
                       this->u_16uint8_[8],
                       this->u_16uint8_[7],
                       this->u_16uint8_[6],
                       this->u_16uint8_[5],
                       this->u_16uint8_[4],
                       this->u_16uint8_[3],
                       this->u_16uint8_[2],
                       this->u_16uint8_[1],
                       this->u_16uint8_[0]);

    if (ret < 0 || ret > static_cast<int>(buf_len))
    {
        return nullptr;
    }
    use_buf = LEN_OF_ZCE_UUID128_STR;
    return buffer;
}
}