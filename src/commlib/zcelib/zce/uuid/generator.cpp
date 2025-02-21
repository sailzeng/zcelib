#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/uuid/generator.h"

namespace zce
{
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

//产生一个uuid64
uuid64 uuid64_gen::random_gen()
{
    ZCE_ASSERT(UUID_GENERATOR::RANDOM == generator_type_);

    //分别用2个随机数发生器得到随机数
    uuid64 uuid64_ret;

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

//以时间为基数产生uuid64
uuid64 uuid64_gen::timeradix_gen()
{
    ZCE_ASSERT(UUID_GENERATOR::TIME == generator_type_);

    ++time_radix_seed_.u_16_48_.data2_;
    return time_radix_seed_;
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

//产生一个uuid64
uuid128 uuid128_gen::random_gen()
{
    ZCE_ASSERT(UUID_GENERATOR::RANDOM == generator_type_);

    //分别用2个随机数发生器得到随机数
    uuid128 uuid64_ret;

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

//以时间为基数产生uuid64
uuid128 uuid128_gen::timeradix_gen()
{
    ZCE_ASSERT(UUID_GENERATOR::TIME == generator_type_);

    ++time_radix_seed_.u_32_32_64_.data3_;
    return time_radix_seed_;
}
}