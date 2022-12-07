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
#include "zce/bytes/base_encode.h"

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

#define BN_MAX_DIGITS               65      // RSA_MAX_MODULUS_LEN + 1

#define BN_MAX_DIGIT                0xFFFFFFFF

#define DIGIT_2MSB(x)               (uint32_t)(((x) >> (BN_UNIT_BITS - 2)) & 0x03)

    //
    static const size_t LEN_OF_U32_ARY = B / 32;
    static const size_t LEN_OF_U8_ARY = B / 8;
    static const size_t BN_UNIT_BITS = 32;

    int decode(const char *hexarr, size_t arr_szie)
    {
        assert(arr_szie < LEN_OF_U32_ARY * 4 * 2);
        size_t out_len = sizeof(bn_);
        return zce::base16_decode(hexarr, arr_szie, bn_, out_len);
    }

    int encode(char *hexarr, size_t *arr_szie)
    {
        assert(*arr_szie <= LEN_OF_U32_ARY * 4 * 2);
        size_t in_len = sizeof(bn_);
        return zce::base16_encode(bn_, in_len, hexarr, arr_szie);
    }

    /// *this = other
    bignumber &operator = (const bignumber &other)
    {
        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            bn_[i] = other.bn_[i];
        }
    }

    bool operator== (const bignumber &other) const
    {
        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            if (bn_[i] != other.bn_[i])
            {
                return false;
            }
        }
        return true;
    }

    bool operator!= (const bignumber &other) const
    {
        return !(*this == other);
    }

    bool operator< (const bignumber &other) const
    {
        return cmp(other) < 0 ? true : false;
    }
    bool operator<= (const bignumber &other) const
    {
        return cmp(other) <= 0 ? true : false;
    }

    bignumber operator+ (const bignumber &other) const
    {
        uint32_t carry = 0;
        bignumber result;
        add(other, result, carry);
        return result;
    }

    bignumber operator- (const bignumber &other) const
    {
        uint32_t borrow = 0;
        bignumber result;
        sub(other, result, borrow);
        return result;
    }

    /// returns sign of a(*this) - b
    int cmp(const bignumber &b)
    {
        for (size_t i = LEN_OF_U32_ARY - 1; i >= 0; --i)
        {
            if (bn_[i] > b.bn_[i])
            {
                return 1;
            }
            if (bn_[i] < b.bn_[i])
            {
                return -1;
            }
        }
        return 0;
    }

    // a = 0
    void zero()
    {
        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            bn_[i] = 0;
        }
    }

    /// c = a(this) + b
    void add(const bignumber &b, bignumber &c, uint32_t &carry)
    {
        uint32_t ai = 0;
        carry = 0;
        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            if ((ai = bn_[i] + carry) < carry) //carry == 1 ,bn_[i]= 0xFFFFFFFF
            {
                ai = b.bn_[i];
            }
            else if ((ai += b.bn_[i]) < b.bn_[i])
            {
                carry = 1;
            }
            else
            {
                carry = 0;
            }
            c.bn_[i] = ai;
        }
        return;
    }

    /// c = a(this) - b
    void sub(const bignumber &b, bignumber &c, uint32_t &borrow)
    {
        uint32_t ai;
        borrow = 0;
        for (size_t i = 0; i < LEN_OF_U32_ARY; i++)
        {
            if ((ai = bn_[i] - borrow) > (BN_MAX_DIGIT - borrow))
            {
                ai = BN_MAX_DIGIT - b.bn_[i];
            }
            else if ((ai -= b.bn_[i]) > (BN_MAX_DIGIT - b.bn_[i]))
            {
                borrow = 1;
            }
            else
            {
                borrow = 0;
            }
            c.bn_[i] = ai;
        }
        return;
    }

    // c = a(this) << b (c = a * 2^b)
    void shift_l(size_t b, bignumber &c, uint32_t &carry, bool roll) const
    {
        assert(b <= BN_UNIT_BITS);
        if (b > BN_UNIT_BITS)
        {
            return;
        }
        uint32_t t = BN_UNIT_BITS - b, bi = 0;
        carry = 0;
        for (size_t i = 0; i < LEN_OF_U32_ARY; i++)
        {
            bi = bn_[i];
            c.bn_[i] = (bi << b) | carry;
            carry = b ? (bi >> t) : 0;
        }
        if (roll)
        {
            c.bn_[0] |= carry;
        }
        return;
    }
    // c = a(this) >> b (c = a / 2^b)
    void shift_r(size_t b, bignumber &c, uint32_t &carry, bool roll) const
    {
        assert(b <= BN_UNIT_BITS);
        if (b > BN_UNIT_BITS)
        {
            return;
        }

        uint32_t t = BN_UNIT_BITS - b, bi = 0;
        carry = 0;
        for (size_t i = BN_UNIT_BITS - 1; i >= 0; i--)
        {
            bi = bn_[i];
            c.bn_[i] = (bi >> b) | carry;
            carry = b ? (bi << t) : 0;
        }
        if (roll)
        {
            c.bn_[BN_UNIT_BITS - 1] |= carry;
        }
        return;
    }

    // returns significant length of a in digits
    size_t digits() const
    {
        size_t i = BN_UNIT_BITS - 1;
        for (; i >= 0; --i)
        {
            if (bn_[i])
            {
                break;
            }
        }
        return (i + 1);
    }

    static uint32_t add_digit_mul(uint32_t *a, uint32_t b, const bignumber &c, size_t digits)
    {
        uint64_t result = 0;
        uint32_t carry = 0, rh = 0, rl = 0;
        if (b == 0)
        {
            return 0;
        }
        for (size_t i = 0; i < digits; i++)
        {
            result = (uint64_t)b * c.bn_[i];
            rl = result & BN_MAX_DIGIT;
            rh = (result >> BN_UNIT_BITS) & BN_MAX_DIGIT;
            if ((a[i] = a[i] + carry) < carry)
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

    // c = a(this) * b
    void mul(const bignumber &b, uint32_t c[LEN_OF_U32_ARY * 2]) const
    {
        memset(c, 0, LEN_OF_U32_ARY * 2 * 4);
        size_t adigits = this->digits();
        size_t bdigits = b.digits();

        for (size_t i = 0; i < adigits; i++)
        {
            c[i + bdigits] += add_digit_mul(&c[i], bn_[i], b, bdigits);
        }
        return;
    }

    static bn_t sub_digit_mul(bn_t *a, bn_t *b, bn_t c, bn_t *d, uint32_t digits)
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
            rh = (result >> BN_UNIT_BITS) & BN_MAX_DIGIT;
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

    // c = a(this) / b, d = a(this) % b
    void div(const bignumber &b, bignumber &c, bignumber &d, uint32_t ddigits)
    {
        uint64_t tmp;
        bn_t ai, t, cc[2 * BN_MAX_DIGITS + 1], dd[BN_MAX_DIGITS];
        int i;
        uint32_t dddigits, shift;

        dddigits = d.digits();
        if (dddigits == 0)
            return;

        shift = BN_UNIT_BITS - bn_digit_bits(d[dddigits - 1]);
        bn_assign_zero(cc, dddigits);
        cc[cdigits] = bn_shift_l(cc, b, shift, cdigits);
        bn_shift_l(dd, d, shift, dddigits);
        t = dd[dddigits - 1];

        bn_assign_zero(c, cdigits);
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
                tmp += (dbn_t)cc[i + dddigits] << BN_UNIT_BITS;
                ai = tmp / (t + 1);
            }

            cc[i + dddigits] -= bn_sub_digit_mul(&cc[i], &cc[i], ai, dd, dddigits);
            // printf("cc[%d]: %08X\n", i, cc[i+dddigits]);
            while (cc[i + dddigits] || (bn_cmp(&cc[i], dd, dddigits) >= 0))
            {
                ai++;
                cc[i + dddigits] -= bn_sub(&cc[i], &cc[i], dd, dddigits);
            }
            c[i] = ai;
            // printf("ai[%d]: %08X\n", i, ai);
        }

        this->zero();
        bn_shift_r(this, cc, shift, dddigits);

        // Clear potentially sensitive information
        memset((uint8_t *)cc, 0, sizeof(cc));
        memset((uint8_t *)dd, 0, sizeof(dd));
    }

    // c = a(this) mod b
    void mod(const bignumber *b, bignumber &c)
    {
        bn_t t[2 * BN_MAX_DIGITS] = { 0 };

        div(t, c, this, bdigits, b, cdigits);

        // Clear potentially sensitive information
        memset((uint8_t *)t, 0, sizeof(t));
    }
    // a = b * c mod d
    void bn_mod_mul(bn_t *a, bn_t *b, bn_t *c, bn_t *d, uint32_t digits)
    {
        bn_t t[2 * BN_MAX_DIGITS];

        bn_mul(t, b, c, digits);
        bn_mod(a, t, 2 * digits, d, digits);

        // Clear potentially sensitive information
        memset((uint8_t *)t, 0, sizeof(t));
    }
    // a = b ^ c mod d
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
            ci_bits = BN_UNIT_BITS;

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

    static uint32_t bn_digit_bits(bn_t a)
    {
        uint32_t i;
        for (i = 0; i < BN_UNIT_BITS; i++)
        {
            if (a == 0)  break;
            a >>= 1;
        }

        return i;
    }

protected:

    //
    //
    //
    uint32_t  bn_[LEN_OF_U32_ARY] = {};
};
}

#endif //ZCE_LIB_OS_ADAPT_MATH_H_
