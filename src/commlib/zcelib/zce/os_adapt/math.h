/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/math.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年8月12日
* @brief      数据函数集合，目前没有多少料
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_OS_ADAPT_MATH_H_
#define ZCE_LIB_OS_ADAPT_MATH_H_

#include "zce/os_adapt/define.h"

namespace zce
{
/*!
* @brief      计算两个数字的最大公约数
*/
uint32_t gcd(uint32_t x, uint32_t y);

/*!
* @brief      检查一个数值是否是质数
*/
inline bool is_prime(const size_t n)
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

//! B是bignumber的bit为数，必须为32的倍数
template <std::size_t B>
class bignumber
{
    typedef uint64_t dbn_t;
    typedef uint32_t bn_t;
    typedef bignumber<B> self;

#define BN_DIGIT_BITS               32      // For uint32_t
#define BN_MAX_DIGITS               65      // RSA_MAX_MODULUS_LEN + 1

#define BN_MAX_DIGIT                0xFFFFFFFF

#define DIGIT_2MSB(x)               (uint32_t)(((x) >> (BN_DIGIT_BITS - 2)) & 0x03)

    void bn_mul(bn_t *a, bn_t *b, bn_t *c, uint32_t digits);                                    // a = b * c
    void bn_div(bn_t *a, bn_t *b, bn_t *c, uint32_t cdigits, bn_t *d, uint32_t ddigits);        // a = b / c, d = b % c
    bn_t bn_shift_l(bn_t *a, bn_t *b, uint32_t c, uint32_t digits);                             // a = b << c (a = b * 2^c)
    bn_t bn_shift_r(bn_t *a, bn_t *b, uint32_t c, uint32_t digits);                             // a = b >> c (a = b / 2^c)

    void bn_mod(bn_t *a, bn_t *b, uint32_t bdigits, bn_t *c, uint32_t cdigits);                 // a = b mod c
    void bn_mod_mul(bn_t *a, bn_t *b, bn_t *c, bn_t *d, uint32_t digits);                       // a = b * c mod d
    void bn_mod_exp(bn_t *a, bn_t *b, bn_t *c, uint32_t cdigits, bn_t *d, uint32_t ddigits);    // a = b ^ c mod d

    int bn_cmp(bn_t *a, bn_t *b, uint32_t digits);                                              // returns sign of a - b

    uint32_t bn_digits(bn_t *a, uint32_t digits);                                               // returns significant length of a in digits

    static bn_t bn_sub_digit_mul(bn_t *a, bn_t *b, bn_t c, bn_t *d, uint32_t digits);
    static bn_t bn_add_digit_mul(bn_t *a, bn_t *b, bn_t c, bn_t *d, uint32_t digits);
    static uint32_t bn_digit_bits(bn_t a);

    void bn_decode(bn_t *bn, uint32_t digits, uint8_t *hexarr, uint32_t size)
    {
        bn_t t;
        int j;
        uint32_t i, u;
        for (i = 0, j = size - 1; i < digits && j >= 0; i++)
        {
            t = 0;
            for (u = 0; j >= 0 && u < BN_DIGIT_BITS; j--, u += 8)
            {
                t |= ((bn_t)hexarr[j]) << u;
            }
            bn[i] = t;
        }

        for (; i < digits; i++)
        {
            bn[i] = 0;
        }
    }

    void bn_encode(uint8_t *hexarr, uint32_t size, bn_t *bn, uint32_t digits)
    {
        bn_t t;
        int j;
        uint32_t i, u;

        for (i = 0, j = size - 1; i < digits && j >= 0; ++i)
        {
            t = bn[i];
            for (u = 0; j >= 0 && u < BN_DIGIT_BITS; j--, u += 8)
            {
                hexarr[j] = (uint8_t)(t >> u);
            }
        }

        for (; j >= 0; j--)
        {
            hexarr[j] = 0;
        }
    }

    /// *this = other
    self &operator = (const self &other)
    {
        for (size_t i = 0; i < NUM_OF_UINT32; ++i)
        {
            bn_[i] = other.bn_[i];
        }
    }

    bool operator== (const self &other) const
    {
        for (size_t i = 0; i < NUM_OF_UINT32; ++i)
        {
            if (bn_[i] != other.bn_[i])
            {
                return false;
            }
        }
        return true;
    }

    bool operator!= (const self &other) const
    {
        return !(*this == other);
    }

    bool operator> (const self &other) const
    {
        for (size_t i = 0; i < NUM_OF_UINT32; ++i)
        {
            if (bn_[NUM_OF_UINT32 - i - 1] > other.bn_[NUM_OF_UINT32 - i - 1])
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        return false;
    }

    self operator+ (const self &other)
    {
        return !(*this == other);
    }

    self operator- (const self &other)
    {
        return !(*this == other);
    }

    // a = 0
    void zero()
    {
        for (size_t i = 0; i < NUM_OF_UINT32; ++i)
        {
            bn_[i] = 0;
        }
    }

    /// return  *this + other
    self add(const self &other, uint32_t &carry)
    {
        bignumber a;
        uint32_t ai;
        carry = 0;
        for (size_t i = 0; i < NUM_OF_UINT32; ++i)
        {
            if ((ai = bn_[i] + carry) < carry) //carry == 1 ,bn_[i]= 0xFFFFFFFF
            {
                ai = other.bn_[i];
            }
            else if ((ai += other.bn_[i]) < other.bn_[i])
            {
                carry = 1;
            }
            else
            {
                carry = 0;
            }
            a[i] = ai;
        }

        return a;
    }

    /// return a =（*this - other)
    self sub(const self &other, uint32_t &borrow)
    {
        bignumber a;
        uint32_t ai;
        borrow = 0;
        for (size_t i = 0; i < NUM_OF_UINT32; i++)
        {
            if ((ai = bn_[i] - borrow) > (BN_MAX_DIGIT - borrow))
            {
                ai = BN_MAX_DIGIT - other.bn_[i];
            }
            else if ((ai -= other.bn_[i]) > (BN_MAX_DIGIT - other.bn_[i]))
            {
                borrow = 1;
            }
            else
            {
                borrow = 0;
            }
            a[i] = ai;
        }

        return a;
    }

    void bn_mul(bn_t *a, bn_t *b, bn_t *c, uint32_t digits)
    {
        bn_t t[2 * BN_MAX_DIGITS];
        uint32_t bdigits, cdigits, i;

        bn_assign_zero(t, 2 * digits);
        bdigits = bn_digits(b, digits);
        cdigits = bn_digits(c, digits);

        for (i = 0; i < bdigits; i++)
        {
            t[i + cdigits] += bn_add_digit_mul(&t[i], &t[i], b[i], c, cdigits);
        }

        bn_assign(a, t, 2 * digits);

        // Clear potentially sensitive information
        memset((uint8_t *)t, 0, sizeof(t));
    }

    void bn_div(bn_t *a, bn_t *b, bn_t *c, uint32_t cdigits, bn_t *d, uint32_t ddigits)
    {
        dbn_t tmp;
        bn_t ai, t, cc[2 * BN_MAX_DIGITS + 1], dd[BN_MAX_DIGITS];
        int i;
        uint32_t dddigits, shift;

        dddigits = bn_digits(d, ddigits);
        if (dddigits == 0)
            return;

        shift = BN_DIGIT_BITS - bn_digit_bits(d[dddigits - 1]);
        bn_assign_zero(cc, dddigits);
        cc[cdigits] = bn_shift_l(cc, c, shift, cdigits);
        bn_shift_l(dd, d, shift, dddigits);
        t = dd[dddigits - 1];

        bn_assign_zero(a, cdigits);
        i = cdigits - dddigits;
        for (; i >= 0; i--)
        {
            if (t == BN_MAX_DIGIT)
            {
                ai = cc[i + dddigits];
            }
            else
            {
                tmp = cc[i + dddigits - 1];
                tmp += (dbn_t)cc[i + dddigits] << BN_DIGIT_BITS;
                ai = tmp / (t + 1);
            }

            cc[i + dddigits] -= bn_sub_digit_mul(&cc[i], &cc[i], ai, dd, dddigits);
            // printf("cc[%d]: %08X\n", i, cc[i+dddigits]);
            while (cc[i + dddigits] || (bn_cmp(&cc[i], dd, dddigits) >= 0))
            {
                ai++;
                cc[i + dddigits] -= bn_sub(&cc[i], &cc[i], dd, dddigits);
            }
            a[i] = ai;
            // printf("ai[%d]: %08X\n", i, ai);
        }

        bn_assign_zero(b, ddigits);
        bn_shift_r(b, cc, shift, dddigits);

        // Clear potentially sensitive information
        memset((uint8_t *)cc, 0, sizeof(cc));
        memset((uint8_t *)dd, 0, sizeof(dd));
    }

    bn_t bn_shift_l(bn_t *a, bn_t *b, uint32_t c, uint32_t digits)
    {
        bn_t bi, carry;
        uint32_t i, t;

        if (c >= BN_DIGIT_BITS)
            return 0;

        t = BN_DIGIT_BITS - c;
        carry = 0;
        for (i = 0; i < digits; i++)
        {
            bi = b[i];
            a[i] = (bi << c) | carry;
            carry = c ? (bi >> t) : 0;
        }

        return carry;
    }

    bn_t bn_shift_r(bn_t *a, bn_t *b, uint32_t c, uint32_t digits)
    {
        bn_t bi, carry;
        int i;
        uint32_t t;

        if (c >= BN_DIGIT_BITS)
            return 0;

        t = BN_DIGIT_BITS - c;
        carry = 0;
        i = digits - 1;
        for (; i >= 0; i--)
        {
            bi = b[i];
            a[i] = (bi >> c) | carry;
            carry = c ? (bi << t) : 0;
        }

        return carry;
    }

    void bn_mod(bn_t *a, bn_t *b, uint32_t bdigits, bn_t *c, uint32_t cdigits)
    {
        bn_t t[2 * BN_MAX_DIGITS] = { 0 };

        bn_div(t, a, b, bdigits, c, cdigits);

        // Clear potentially sensitive information
        memset((uint8_t *)t, 0, sizeof(t));
    }

    void bn_mod_mul(bn_t *a, bn_t *b, bn_t *c, bn_t *d, uint32_t digits)
    {
        bn_t t[2 * BN_MAX_DIGITS];

        bn_mul(t, b, c, digits);
        bn_mod(a, t, 2 * digits, d, digits);

        // Clear potentially sensitive information
        memset((uint8_t *)t, 0, sizeof(t));
    }

    void bn_mod_exp(bn_t *a, bn_t *b, bn_t *c, uint32_t cdigits, bn_t *d, uint32_t ddigits)
    {
        bn_t bpower[3][BN_MAX_DIGITS], ci, t[BN_MAX_DIGITS];
        int i;
        uint32_t ci_bits, j, s;

        bn_assign(bpower[0], b, ddigits);
        bn_mod_mul(bpower[1], bpower[0], b, d, ddigits);
        bn_mod_mul(bpower[2], bpower[1], b, d, ddigits);

        BN_ASSIGN_DIGIT(t, 1, ddigits);

        cdigits = bn_digits(c, cdigits);
        i = cdigits - 1;
        for (; i >= 0; i--)
        {
            ci = c[i];
            ci_bits = BN_DIGIT_BITS;

            if (i == (int)(cdigits - 1))
            {
                while (!DIGIT_2MSB(ci))
                {
                    ci <<= 2;
                    ci_bits -= 2;
                }
            }

            for (j = 0; j < ci_bits; j += 2)
            {
                bn_mod_mul(t, t, t, d, ddigits);
                bn_mod_mul(t, t, t, d, ddigits);
                if ((s = DIGIT_2MSB(ci)) != 0)
                {
                    bn_mod_mul(t, t, bpower[s - 1], d, ddigits);
                }
                ci <<= 2;
            }
        }

        bn_assign(a, t, ddigits);

        // Clear potentially sensitive information
        memset((uint8_t *)bpower, 0, sizeof(bpower));
        memset((uint8_t *)t, 0, sizeof(t));
    }

    int cmp(bn_t *a, bn_t *b, uint32_t digits)
    {
        int i;
        for (i = digits - 1; i >= 0; i--)
        {
            if (a[i] > b[i])     return 1;
            if (a[i] < b[i])     return -1;
        }

        return 0;
    }

    uint32_t bn_digits(bn_t *a, uint32_t digits)
    {
        int i;
        for (i = digits - 1; i >= 0; i--)
        {
            if (a[i])    break;
        }

        return (i + 1);
    }

    static bn_t bn_add_digit_mul(bn_t *a, bn_t *b, bn_t c, bn_t *d, uint32_t digits)
    {
        dbn_t result;
        bn_t carry, rh, rl;
        uint32_t i;

        if (c == 0)
            return 0;

        carry = 0;
        for (i = 0; i < digits; i++)
        {
            result = (dbn_t)c * d[i];
            rl = result & BN_MAX_DIGIT;
            rh = (result >> BN_DIGIT_BITS) & BN_MAX_DIGIT;
            if ((a[i] = b[i] + carry) < carry)
            {
                carry = 1;
            }
            else
            {
                carry = 0;
            }
            if ((a[i] += rl) < rl)
            {
                carry++;
            }
            carry += rh;
        }

        return carry;
    }

    static bn_t bn_sub_digit_mul(bn_t *a, bn_t *b, bn_t c, bn_t *d, uint32_t digits)
    {
        dbn_t result;
        bn_t borrow, rh, rl;
        uint32_t i;

        if (c == 0)
            return 0;

        borrow = 0;
        for (i = 0; i < digits; i++)
        {
            result = (dbn_t)c * d[i];
            rl = result & BN_MAX_DIGIT;
            rh = (result >> BN_DIGIT_BITS) & BN_MAX_DIGIT;
            if ((a[i] = b[i] - borrow) > (BN_MAX_DIGIT - borrow))
            {
                borrow = 1;
            }
            else
            {
                borrow = 0;
            }
            if ((a[i] -= rl) > (BN_MAX_DIGIT - rl))
            {
                borrow++;
            }
            borrow += rh;
        }

        return borrow;
    }

    static uint32_t bn_digit_bits(bn_t a)
    {
        uint32_t i;
        for (i = 0; i < BN_DIGIT_BITS; i++)
        {
            if (a == 0)  break;
            a >>= 1;
        }

        return i;
    }

protected:
    //
    static const size_t NUM_OF_UINT32 = B / 32;
    //
    uint32_t  bn_[NUM_OF_UINT32] = {};
};
}

#endif //ZCE_LIB_OS_ADAPT_MATH_H_
