/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_math.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012年8月12日
* @brief      数据函数集合，目前没有多少料
*
* @details
*
* @note
*
*/

#pragma once

#include "zce_os_adapt_predefine.h"

namespace ZCE_LIB
{

/*!
* @brief      计算两个数字的最大公约数
*/
uint32_t gcd (uint32_t x, uint32_t y);

/*!
* @brief      检查一个数值是否是质数
*/
inline bool is_prime (const size_t n)
{
    //判断是否是质数，
    if (2 == n || 3 == n)
    {
        return true;
    }
    size_t test_max = static_cast<size_t>(::sqrt(static_cast<double>(n)) + 1);
    for (size_t i = 2; i <= test_max; ++i)
    {
        if (n % i == 0)
        {
            return false;
        }
    }

    return true;
}



/*!
* @brief      找到输入参数n最近(大于)的一个质数，
* @return     size_t 返回最近的那个质数
* @param      n      输入的数值
*/
size_t nearest_prime(const size_t n);


/*!
* @brief      检查从LSB（最小位），to MSB（最大位），到一个被设置为1的位置，前面有多少个0
* @return     int  返回的是1前面的0的个数
* @param      mask 处理的的32位数据，mask 不能是0
*/
inline int scanbit_lsb2msb32(uint32_t mask)
{
#if defined ZCE_OS_WINDOWS
    unsigned long index = 0;
    ::_BitScanForward(&index, mask);
    return index;
#elif defined ZCE_OS_LINUX
    return ::__builtin_ctz(mask);
#endif
}

/*!
* @brief      检查从MSB（最大位），to LSB（最小位），发现第一个被设置为1的位置，前面有多少个0
* @return     int    返回的是1前面的0的个数
* @param      mask   处理的的32位数据，mask 不能是0，否则结果没有定义
*/
inline int scanbit_msb2lsb32(uint32_t mask)
{
#if defined ZCE_OS_WINDOWS
    unsigned long index = 0;
    ::_BitScanReverse(&index, mask);
    return index;
#elif defined ZCE_OS_LINUX
    return ::__builtin_clz(mask);
#endif
}

//_BitScanForward64 _BitScanReverse64 函数只能在x64的环境才能被编译，Linux的我估计也一样，
//都是利用intel的指令
#if defined ZCE_OS64

/*!
* @brief      检查从LSB（最小位），to MSB（最大位），到一个被设置为1的位置，前面有多少个0
* @return     int  返回的是1前面的0的个数
* @param      mask 处理的的64位数据，mask 不能是0
*/
inline int scanbit_lsb2msb64(uint64_t mask)
{
#if defined ZCE_OS_WINDOWS
    unsigned long index = 0;
    ::_BitScanForward64(&index, mask);
    return index;
#elif defined ZCE_OS_LINUX
    return ::__builtin_ctzll(mask);
#endif
}

/*!
* @brief      检查从MSB（最大位），to LSB（最小位），发现第一个被设置为1的位置，前面有多少个0
* @return     int    返回的是1前面的0的个数
* @param      mask   处理的的64位数据，mask 不能是0，否则结果没有定义
*/
inline int scanbit_msb2lsb64(uint64_t mask)
{
#if defined ZCE_OS_WINDOWS
    unsigned long index = 0;
    ::_BitScanReverse64(&index, mask);
    return index;
#elif defined ZCE_OS_LINUX
    return ::__builtin_clzll(mask);
#endif
}

#endif

};



