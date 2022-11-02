#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/uuid/generator.h"

namespace zce
{
/************************************************************************************************************
Class           : UUID64
************************************************************************************************************/

// < 运算符
bool UUID64::operator < (const UUID64& others) const
{
    if (this->u_uint64_ < others.u_uint64_)
    {
        return true;
    }

    return false;
}

// == 运算符
bool UUID64::operator == (const UUID64& others) const
{
    if (this->u_uint64_ == others.u_uint64_)
    {
        return true;
    }

    return false;
}

//转换为字符串
const char* UUID64::to_string(char* buffer, size_t buf_len, size_t& use_buf) const
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
UUID64::operator uint64_t ()
{
    return this->u_uint64_;
}

/************************************************************************************************************
Class           : UUID64_Generator
************************************************************************************************************/

//实例指针
uuid64_gen* uuid64_gen::instance_ = nullptr;

//构造函数
uuid64_gen::uuid64_gen() :
    generator_type_(UUID_GENERATOR::INVALID)
{
}
uuid64_gen::~uuid64_gen()
{
}

//随机数的种子，注意如果是是在一个分布式系统中找种子，
//时间未必特别合理，也许用IP地址计算得到一个种子更好一些
void uuid64_gen::random_seed(uint32_t seed)
{
    generator_type_ = UUID_GENERATOR::RANDOM;
    mt_19937_random_.srand(seed);
    mt_11213b_random_.srand(seed);
}

//产生一个UUID64
UUID64 uuid64_gen::random_gen()
{
    ZCE_ASSERT(UUID_GENERATOR::RANDOM == generator_type_);

    //分别用2个随机数发生器得到随机数
    UUID64 uuid64_ret;

    uuid64_ret.u_2uint32_[0] = mt_19937_random_.rand();
    uuid64_ret.u_2uint32_[1] = mt_11213b_random_.rand();

    return uuid64_ret;
}

//以时间为基数的初始化，
void uuid64_gen::time_radix(uint16_t identity, uint32_t radix)
{
    generator_type_ = UUID_GENERATOR::TIME;

    time_radix_seed_.u_16_48_.data1_ = identity;
    time_radix_seed_.u_16_48_.data2_ = static_cast<uint64_t>(radix) * 0x10;
}

//以时间为基数产生UUID64
UUID64 uuid64_gen::timeradix_gen()
{
    ZCE_ASSERT(UUID_GENERATOR::TIME == generator_type_);

    ++time_radix_seed_.u_16_48_.data2_;
    return time_radix_seed_;
}

/************************************************************************************************************
Class           : UUID128
************************************************************************************************************/
UUID128::UUID128()
{
    u_4uint32_[0] = 0;
    u_4uint32_[1] = 0;
    u_4uint32_[2] = 0;
    u_4uint32_[3] = 0;
}

UUID128::~UUID128()
{
}

// < 运算符
bool UUID128::operator < (const UUID128& others) const
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
bool UUID128::operator == (const UUID128& others) const
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
const char* UUID128::to_string(char* buffer, size_t buf_len, size_t& use_buf) const
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

/************************************************************************************************************
Class           : UUID128_Generator
************************************************************************************************************/

//构造函数
uuid128_gen::uuid128_gen() :
    generator_type_(UUID_GENERATOR::INVALID)
{
}
uuid128_gen::~uuid128_gen()
{
}

//随机数的种子，注意如果是是在一个分布式系统中找种子，
//时间未必特别合理，也许用IP地址计算得到一个种子更好一些
void uuid128_gen::random_seed(uint32_t seed)
{
    generator_type_ = UUID_GENERATOR::RANDOM;
    mt_19937_random_.srand(seed);
    mt_11213b_random_.srand(seed);
}

//产生一个UUID64
UUID128 uuid128_gen::random_gen()
{
    ZCE_ASSERT(UUID_GENERATOR::RANDOM == generator_type_);

    //分别用2个随机数发生器得到随机数
    UUID128 uuid64_ret;

    uuid64_ret.u_4uint32_[0] = mt_19937_random_.rand();
    uuid64_ret.u_4uint32_[1] = mt_19937_random_.rand();
    uuid64_ret.u_4uint32_[2] = mt_11213b_random_.rand();
    uuid64_ret.u_4uint32_[3] = mt_11213b_random_.rand();

    return uuid64_ret;
}

//以时间为基数的初始化，
void uuid128_gen::time_radix(uint32_t identity, uint32_t radix)
{
    generator_type_ = UUID_GENERATOR::TIME;

    time_radix_seed_.u_32_32_64_.data1_ = identity;
    time_radix_seed_.u_32_32_64_.data1_ = radix;
}

//以时间为基数产生UUID64
UUID128 uuid128_gen::timeradix_gen()
{
    ZCE_ASSERT(UUID_GENERATOR::TIME == generator_type_);

    ++time_radix_seed_.u_32_32_64_.data3_;
    return time_radix_seed_;
}
}