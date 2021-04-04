/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_math.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��8��12��
* @brief      ���ݺ������ϣ�Ŀǰû�ж�����
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_OS_ADAPT_MATH_H_
#define ZCE_LIB_OS_ADAPT_MATH_H_

#include "zce_os_adapt_predefine.h"

namespace zce
{

/*!
* @brief      �����������ֵ����Լ��
*/
uint32_t gcd (uint32_t x, uint32_t y);

/*!
* @brief      ���һ����ֵ�Ƿ�������
*/
inline bool is_prime (const size_t n)
{
    //�ж��Ƿ���������
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
* @brief      �ҵ��������n���(����)��һ��������
* @return     size_t ����������Ǹ�����
* @param      n      �������ֵ
*/
size_t nearest_prime(const size_t n);


/*!
* @brief      ����LSB����Сλ����to MSB�����λ������һ��������Ϊ1��λ�ã�ǰ���ж��ٸ�0
* @return     int  ���ص���1ǰ���0�ĸ���
* @param      mask ����ĵ�32λ���ݣ�mask ������0
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
* @brief      ����MSB�����λ����to LSB����Сλ�������ֵ�һ��������Ϊ1��λ�ã�ǰ���ж��ٸ�0
* @return     int    ���ص���1ǰ���0�ĸ���
* @param      mask   ����ĵ�32λ���ݣ�mask ������0��������û�ж���
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

//_BitScanForward64 _BitScanReverse64 ����ֻ����x64�Ļ������ܱ����룬Linux���ҹ���Ҳһ����
//��������intel��ָ��
#if defined ZCE_OS64

/*!
* @brief      ����LSB����Сλ����to MSB�����λ������һ��������Ϊ1��λ�ã�ǰ���ж��ٸ�0
* @return     int  ���ص���1ǰ���0�ĸ���
* @param      mask ����ĵ�64λ���ݣ�mask ������0
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
* @brief      ����MSB�����λ����to LSB����Сλ�������ֵ�һ��������Ϊ1��λ�ã�ǰ���ж��ٸ�0
* @return     int    ���ص���1ǰ���0�ĸ���
* @param      mask   ����ĵ�64λ���ݣ�mask ������0��������û�ж���
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

#endif //ZCE_LIB_OS_ADAPT_MATH_H_

