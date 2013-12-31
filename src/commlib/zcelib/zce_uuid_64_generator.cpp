#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_uuid_64_generator.h"

/************************************************************************************************************
Class           : ZCE_UUID64
************************************************************************************************************/
ZCE_UUID64::ZCE_UUID64():
    u_uint64_(0)
{
}

ZCE_UUID64::~ZCE_UUID64()
{
}

// < 运算符
bool ZCE_UUID64::operator < (const ZCE_UUID64 &others) const
{
    if (this->u_uint64_ < others.u_uint64_)
    {
        return true;
    }

    return false;
}

// == 运算符
bool ZCE_UUID64::operator == (const ZCE_UUID64 &others) const
{
    if (this->u_uint64_ == others.u_uint64_)
    {
        return true;
    }

    return false;
}

//转换为字符串
const char *ZCE_UUID64::to_string(char *buffer, size_t buf_len) const
{

    //如果传递的BUFFER空间不够，直接返回NULL
    if (buf_len < LEN_OF_ZCE_UUID64_STR + 1)
    {
        return NULL;
    }

    int ret = snprintf(buffer, buf_len, "%08x-%08x", this->u_2uint32_[1], this->u_2uint32_[0]);

    if (ret < 0 || ret > static_cast<int>( buf_len) )
    {
        return NULL;
    }

    return buffer;
}

// 转移成一个uint64_t的结构
ZCE_UUID64::operator uint64_t ()
{
    return this->u_uint64_;
}

/************************************************************************************************************
Class           : ZCE_UUID64_Generator
************************************************************************************************************/

//实例指针
ZCE_UUID64_Generator *ZCE_UUID64_Generator::instance_ = NULL;

//构造函数
ZCE_UUID64_Generator::ZCE_UUID64_Generator():
    generator_type_(UUID64_GENERATOR_INVALID)
{
}
ZCE_UUID64_Generator::~ZCE_UUID64_Generator()
{
}

//随机数的种子，注意如果是是在一个分布式系统中找种子，
//时间未必特别合理，也许用IP地址计算得到一个种子更好一些
void ZCE_UUID64_Generator::random_seed(uint32_t seed)
{
    generator_type_ = UUID64_GENERATOR_RANDOM;
    mt_19937_random_.srand(seed);
    mt_11213b_random_.srand(seed);
}

//产生一个UUID64
ZCE_UUID64 ZCE_UUID64_Generator::random_gen()
{
    ZCE_ASSERT(UUID64_GENERATOR_RANDOM == generator_type_);

    //分别用2个随机数发生器得到随机数
    ZCE_UUID64 uuid64_ret;

    uuid64_ret.u_2uint32_[0] = mt_19937_random_.rand();
    uuid64_ret.u_2uint32_[1] = mt_11213b_random_.rand();

    return uuid64_ret;
}

//以时间为基数的初始化，
void ZCE_UUID64_Generator::time_radix(uint16_t identity, uint32_t radix )
{
    generator_type_ = UUID64_GENERATOR_TIME;

    time_radix_seed_.u_16_48_.data1_ = identity;
    time_radix_seed_.u_16_48_.data2_ = radix * 0x10;
}

//以时间为基数产生UUID64
ZCE_UUID64 ZCE_UUID64_Generator::timeradix_gen()
{
    ZCE_ASSERT(UUID64_GENERATOR_TIME == generator_type_);

    ++time_radix_seed_.u_16_48_.data2_;
    return time_radix_seed_;
}

