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
const char* uuid64::to_str(char* buf, size_t buf_len, size_t& use_buf,
                           zce::uuid64::FMT fmt) const
{
    static_assert(sizeof(uuid64) == 8);
    //如果传递的BUFFER空间不够，直接返回nullptr
    if (fmt < zce::uuid64::FMT_INVALID || fmt >= zce::uuid64::FMT_COUNTER)
    {
        return nullptr;
    }
    if (buf_len < UUID64_STR_LEN[fmt] + 1)
    {
        return nullptr;
    }
    use_buf = 0;
    int ret = 0;
    if (fmt == zce::uuid64::FMT_64)
    {
        ret = snprintf(buf, buf_len, "%016" PRIx64,
                       this->u_uint64_);
    }
    else if (fmt == zce::uuid64::FMT_32_32)
    {
        ret = snprintf(buf, buf_len, "%08x-%08x",
                       this->u_2uint32_[1],
                       this->u_2uint32_[0]);
    }
    else if (fmt == zce::uuid64::FMT_16_48)
    {
        ret = snprintf(buf, buf_len, "%04" PRIx64 "-%012" PRIx64,
                       this->u_16_48_.data1_,
                       this->u_16_48_.data2_);
    }
    else if (fmt == zce::uuid64::FMT_16_32_16)
    {
        ret = snprintf(buf, buf_len, "%04" PRIx64 "-%08" PRIx64 "-%04" PRIx64,
                       this->u_16_32_16_.data1_,
                       this->u_16_32_16_.data2_,
                       this->u_16_32_16_.data3_);
    }
    else
    {
        assert(false);
    }

    if (ret < 0 || ret > static_cast<int>(buf_len))
    {
        return nullptr;
    }
    use_buf = UUID64_STR_LEN[fmt];
    return buf;
}

int uuid64::from_str(char* buf)
{
    size_t len = strlen(buf);
    int ret = 0;
    if (len >= UUID64_STR_LEN[zce::uuid64::FMT_16_32_16] && buf[4] == '-' &&
        buf[13] == '-')
    {
        uint64_t data1 = 0, data2 = 0,data3= 0;
        ret = sscanf(buf, "%04" PRIx64 "-%08" PRIx64 "-%04" PRIx64,
                     &data1, &data2, &data3);
        if (ret == 3)
        {
            this->u_16_32_16_.data1_ = data1;
            this->u_16_32_16_.data2_ = data2;
            this->u_16_32_16_.data3_ = data3;
        }
    }
    else if (len >= UUID64_STR_LEN[zce::uuid64::FMT_16_48] && buf[4] == '-' )
    {
        uint64_t data1 = 0, data2 = 0;
        ret = sscanf(buf, "%04" PRIx64 "-%012" PRIx64,
               &data1,&data2);
        if (ret == 2)
        {
            this->u_16_48_.data1_ = data1;
            this->u_16_48_.data2_ = data2;
        }
    }
    else if (len >= UUID64_STR_LEN[zce::uuid64::FMT_32_32] && buf[8] == '-')
    {
        ret = sscanf(buf, "%08x-%08x",
                      &(this->u_2uint32_[1]),
                      &(this->u_2uint32_[0]));
    }
    else
    {
        ret = sscanf(buf, "%016" PRIx64,
                      &(this->u_uint64_));
    }
    return ret;
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
const char* uuid128::to_str(char* buf,
                            size_t buf_len,
                            size_t& use_buf,
                            zce::uuid128::FMT fmt) const
{

    static_assert(sizeof(uuid128) == 16);
    //如果传递的BUFFER空间不够，直接返回nullptr
    if (fmt < zce::uuid128::FMT_INVALID || fmt >= zce::uuid128::FMT_COUNTER)
    {
        return nullptr;
    }
    if (buf_len < UUID128_STR_LEN[fmt] + 1)
    {
        return nullptr;
    }
    use_buf = 0;
    int ret = 0;
    if (fmt == zce::uuid128::FMT_32_32_32_32)
    {
        ret = snprintf(buf, buf_len, "%08x-%08x-%08x-%08x",
                       this->u_4uint32_[3],
                       this->u_4uint32_[2],
                       this->u_4uint32_[1],
                       this->u_4uint32_[0]);
    }
    else if (fmt == zce::uuid128::FMT_64_64)
    {
        ret = snprintf(buf, buf_len, "%016" PRIx64 "-%016" PRIx64,
                       this->u_2uint64_[1],
                       this->u_2uint64_[0]);
    }
    else if (fmt == zce::uuid128::FMT_32_32_64)
    {
        ret = snprintf(buf, buf_len, "%08x-%08x-%016" PRIx64,
                       this->u_32_32_64_.data1_,
                       this->u_32_32_64_.data2_,
                       this->u_32_32_64_.data3_);
    }
    else if (fmt == zce::uuid128::FMT_32_16_16_16_48)
    {
        ret = snprintf(buf, buf_len, "%08" PRIx64 "-%04" PRIx64 "-%04" PRIx64 "-%04" PRIx64 "-%012" PRIx64,
                       this->u_32_16_16_16_48_.data1_,
                       this->u_32_16_16_16_48_.data2_,
                       this->u_32_16_16_16_48_.data3_,
                       this->u_32_16_16_16_48_.data4_,
                       this->u_32_16_16_16_48_.data5_);
    }
    else if (fmt == zce::uuid128::FMT_32_16_16_64)
    {
        ret = snprintf(buf, buf_len, "%08" PRIx64 "-%04" PRIx64 "-%04" PRIx64 "-%016" PRIx64,
                       this->u_32_16_16_64_.data1_,
                       this->u_32_16_16_64_.data2_,
                       this->u_32_16_16_64_.data3_,
                       this->u_32_16_16_64_.data4_);
    }
    else
    {
        assert(false);
    }

    if (ret < 0 || ret > static_cast<int>(buf_len))
    {
        return nullptr;
    }
    use_buf = UUID128_STR_LEN[fmt];
    return buf;


}

//! 以UUID8-4-4-4-12的格式读取字符串
int uuid128::from_str(char* buf)
{
    size_t len = strlen(buf);
    int ret = 0;
    if (len >= UUID128_STR_LEN[zce::uuid128::FMT_32_16_16_16_48] && buf[8] == '-' &&
        buf[13] == '-' && buf[18] == '-' && buf[23] == '-')
    {
        uint64_t data1 = 0, data2 = 0, data3 = 0, data4=0, data5 = 0;
        ret = sscanf(buf, "%08" PRIx64 "-%04" PRIx64 "-%04" PRIx64 "-%04" PRIx64 "-%012" PRIx64,
                     &data1, &data2, &data3, &data4, &data5);
        if (ret == 5)
        {
            this->u_32_16_16_16_48_.data1_ = data1;
            this->u_32_16_16_16_48_.data2_ = data2;
            this->u_32_16_16_16_48_.data3_ = data3;
            this->u_32_16_16_16_48_.data4_ = data4;
            this->u_32_16_16_16_48_.data5_ = data5;
        }
    }
    else if (len >= UUID128_STR_LEN[zce::uuid128::FMT_32_16_16_64] && buf[8] == '-' &&
        buf[13] == '-' && buf[18] == '-' )
    {
        uint64_t data1 = 0, data2 = 0, data3 = 0, data4 = 0;
        ret = sscanf(buf, "%08" PRIx64 "-%04" PRIx64 "-%04" PRIx64 "-%016" PRIx64,
                     &data1, &data2, &data3, &data4);
        if (ret == 4)
        {
            this->u_32_16_16_64_.data1_ = data1;
            this->u_32_16_16_64_.data2_ = data2;
            this->u_32_16_16_64_.data3_ = data3;
            this->u_32_16_16_64_.data4_ = data4;
        }
    }
    else if (len >= UUID128_STR_LEN[zce::uuid128::FMT_32_32_32_32] && buf[8] == '-' &&
        buf[17] == '-')
    {
        ret = sscanf(buf, "%08x-%08x-%08x-%08x",
                     &(this->u_4uint32_[3]),
                     &(this->u_4uint32_[2]),
                     &(this->u_4uint32_[1]),
                     &(this->u_4uint32_[0]));
    }
    if (len >= UUID128_STR_LEN[zce::uuid128::FMT_32_32_64] && buf[8] == '-' &&
        buf[17] == '-')
    {
        ret = sscanf(buf, "%08x-%08x-%016" PRIx64,
                     &(this->u_32_32_64_.data1_),
                     &(this->u_32_32_64_.data2_),
                     &(this->u_32_32_64_.data3_));
    }
    if (len >= UUID128_STR_LEN[zce::uuid128::FMT_64_64] && buf[16] == '-')
    {
        ret = sscanf(buf, "%016" PRIx64 "-%016" PRIx64,
                     &(this->u_2uint64_[1]),
                     &(this->u_2uint64_[0]));
    }
    else
    {
        
    }
    return ret;
}

}
