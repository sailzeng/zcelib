/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/math/big_uint.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2022年12月11日
* @brief
*
*
* @details
*
*/

#pragma once

namespace zce
{
//! B是bignumber的bit为数，必须为32的倍数
//! 必须要考虑溢出风险，比如128bits * 128 bits的数字需要256bits保存，
//! 如果您希望使用2048bit的big number，那么您要使用4096的长度，
template <std::size_t B>
class big_uint
{
public:

    typedef uint32_t unit;
    typedef big_uint<B> self;

    big_uint() = default;
    ~big_uint() = default;
    big_uint(size_t digits, uint32_t *b)
    {
        assign(digits, b);
    }
    //不加explict
    big_uint(uint32_t b)
    {
        bn_[0] = b;
    }

    //
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
    /// *this = one_unit
    big_uint &operator = (uint32_t one_unit)
    {
        this->assign(one_unit);
        return *this;
    }
    big_uint &operator = (const big_uint &other)
    {
        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            bn_[i] = other.bn_[i];
        }
        return *this;
    }

    bool operator== (const big_uint &other) const
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
    bool operator!= (const big_uint &other) const
    {
        return !(*this == other);
    }
    bool operator< (const big_uint &other) const
    {
        return cmp(other) < 0 ? true : false;
    }
    bool operator<= (const big_uint &other) const
    {
        return cmp(other) <= 0 ? true : false;
    }

    big_uint operator+ (const big_uint &other) const
    {
        uint32_t carry = 0;
        big_uint result;
        add(result, *this, other, carry);
        return result;
    }
    big_uint operator- (const big_uint &other)
    {
        uint32_t borrow = 0;
        big_uint result;
        sub(result, *this, other, borrow);
        return result;
    }
    big_uint operator* (const big_uint &other)
    {
        big_uint result;
        mul(result, *this, other);
        return result;
    }
    big_uint operator/ (const big_uint &other)
    {
        big_uint result;
        div(result, *this, other);
        return result;
    }
    big_uint operator% (const big_uint &other)
    {
        big_uint result;
        mod(result, *this, other);
        return result;
    }

    big_uint &operator+= (const big_uint &other)
    {
        uint32_t carry = 0;
        add(*this, *this, other, carry);
        return *this;
    }
    big_uint &operator-= (const big_uint &other) const
    {
        uint32_t borrow = 0;
        sub(*this, *this, other, borrow);
        return *this;
    }
    big_uint operator*= (const big_uint &other)
    {
        mul(*this, *this, other);
        return *this;
    }
    big_uint operator/= (const big_uint &other)
    {
        div(*this, *this, other);
        return *this;
    }
    big_uint operator%= (const big_uint &other)
    {
        mod(*this, *this, other);
        return *this;
    }
    big_uint operator>>= (size_t c)
    {
        this->shift_digits_r(c);
        return *this;
    }
    big_uint operator<<= (size_t c)
    {
        this->shift_digits_l(c);
        return *this;
    }

    void assign(size_t digits, uint32_t *b)
    {
        assert(digits <= LEN_OF_U32_ARY);
        if (digits > LEN_OF_U32_ARY)
        {
            return;
        }
        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            if (i < digits)
            {
                this->bn_[i] = b[i];
            }
            else
            {
                this->bn_[i] = 0;
            }
        }
    }
    //fill example: 4(digits), 0x1234,0x4321,0x3412,0x2143
    void assign(size_t digits...)
    {
        assert(digits <= LEN_OF_U32_ARY);
        if (digits > LEN_OF_U32_ARY)
        {
            return;
        }
        va_list argptr;
        va_start(argptr, digits);
        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            if (i < digits)
            {
                uint32_t ai = va_arg(argptr, uint32_t);
                this->bn_[i] = ai;
            }
            else
            {
                this->bn_[i] = 0;
            }
        }
    }
    void assign(uint32_t one_unit)
    {
        this->bn_[0] = one_unit;
        for (size_t i = 1; i < LEN_OF_U32_ARY; ++i)
        {
            this->bn_[i] = 0;
        }
    }
    // a = 0
    void zero()
    {
        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            bn_[i] = 0;
        }
    }

    // a = 0
    bool is_zero()
    {
        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            if (bn_[i] != 0)
            {
                return false;
            }
        }
        return true;
    }
    // return a - b
    static int cmp(const big_uint &a, const big_uint &b)
    {
        for (ssize_t i = LEN_OF_U32_ARY - 1; i >= 0; --i)
        {
            if (a.bn_[i] > b.bn_[i])
            {
                return 1;
            }
            if (a.bn_[i] < b.bn_[i])
            {
                return -1;
            }
        }
        return 0;
    }

    //a = b + c
    static void add(big_uint &a,
                    const big_uint &b,
                    const big_uint &c,
                    uint32_t &carry)
    {
        uint32_t ai = 0;
        carry = 0;
        for (size_t i = 0; i < LEN_OF_U32_ARY; i++)
        {
            //carry == 1 ,b.bn_[i]= 0xFFFFFFFF.
            if ((ai = b.bn_[i] + carry) < carry)
            {
                ai = c.bn_[i];
            }
            else if ((ai += c.bn_[i]) < c.bn_[i])
            {
                carry = 1;
            }
            else
            {
                carry = 0;
            }
            a.bn_[i] = ai;
        }
        return;
    }
    // add one unit
    static void add_unit(big_uint &a,
                         const big_uint &b,
                         uint32_t c)
    {
        big_uint cc = c;
        add(a, b, cc);
    }
    /// a = b - c
    static void sub(big_uint &a,
                    const big_uint &b,
                    const big_uint &c,
                    uint32_t &borrow)
    {
        uint32_t ai = 0;
        borrow = 0;
        for (size_t i = 0; i < LEN_OF_U32_ARY; i++)
        {
            if ((ai = b.bn_[i] - borrow) > (BN_UNIT_MAX_NUM - borrow))
            {
                ai = BN_UNIT_MAX_NUM - c.bn_[i];
            }
            else if ((ai -= c.bn_[i]) > (BN_UNIT_MAX_NUM - c.bn_[i]))
            {
                borrow = 1;
            }
            else
            {
                borrow = 0;
            }
            a.bn_[i] = ai;
        }
        return;
    }
    // sub one unit
    static void sub_unit(big_uint &a,
                         const big_uint &b,
                         uint32_t c)
    {
        big_uint cc = c;
        sub(a, b, cc);
    }

    //注意这不是按bit来的，是按unit进行的移位
    void shift_digits_l(size_t b)
    {
        assert(b <= LEN_OF_U32_ARY);
        if (b == 0 || b > LEN_OF_U32_ARY)
        {
            return;
        }
        size_t m_n = LEN_OF_U32_ARY - b;
        size_t z_n = b;
        for (size_t i = 0; i < m_n; ++i)
        {
            bn_[LEN_OF_U32_ARY - i - 1] = bn_[LEN_OF_U32_ARY - i - b - 1];
        }
        for (size_t i = 0; i < z_n; ++i)
        {
            bn_[i] = 0;
        }
    }

    void shift_digits_r(size_t b)
    {
        assert(b <= LEN_OF_U32_ARY);
        if (b <= 0 || b >= LEN_OF_U32_ARY)
        {
            return;
        }

        for (size_t i = 0; i < LEN_OF_U32_ARY - b; ++i)
        {
            bn_[i] = bn_[i + b];
        }
        for (size_t i = 0; i < b; ++i)
        {
            bn_[LEN_OF_U32_ARY - i - 1] = 0;
        }
    }

    // returns valid significant length of a in digits
    size_t valid_digits() const
    {
        ssize_t i = LEN_OF_U32_ARY - 1;
        for (; i >= 0; --i)
        {
            if (bn_[i])
            {
                break;
            }
        }
        return (i + 1);
    }

    // mul one unit
    static void mul_unit(big_uint &a,
                         const big_uint &b,
                         uint32_t c)
    {
        uint64_t result = 0;
        uint32_t carry = 0, rh = 0, rl = 0;
        a.zero();
        if (c == 0)
        {
            return;
        }

        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            result = (uint64_t)c * b.bn_[i];
            rl = result & BN_UNIT_MAX_NUM;
            rh = (result >> BN_UNIT_BITS) & BN_UNIT_MAX_NUM;
            a.bn_[i] = rl + carry;
            if (a.bn_[i] < carry)
            {
                carry = 1;
            }
            else
            {
                carry = 0;
            }
            carry += rh;
        }
    }

    // a = b * b
    static void mul(big_uint &a,
                    const big_uint &b,
                    const big_uint &c)
    {
        uint32_t carry = 0;
        big_uint bb = b;
        big_uint cc = c;
        size_t cvd = cc.valid_digits();
        a.zero();
        for (size_t i = 0; i < cvd; i++)
        {
            big_uint t;
            mul_unit(t, bb, cc.bn_[i]);
            add(a, a, t, carry);
            bb.shift_digits_l(1);
        }
    }

    // a = c / d, b = c % d
    static void div_i(big_uint &a,
                      big_uint &b,
                      const big_uint &c,
                      const big_uint &d)
    {
        size_t cdigits = c.valid_digits();
        size_t ddigits = d.valid_digits();
        assert(ddigits != 0);
        if (ddigits == 0)
        {
            return;
        }
        a.zero();
        if (cdigits == 0 && cdigits < ddigits)
        {
            b = c;
            return;
        }

        big_uint cc = c, dd = d;

        uint32_t td = dd.bn_[ddigits - 1];
        uint64_t tc = 0;

        uint32_t ai = 0;
        ssize_t i = cdigits - ddigits; //商最多的位数 -1
        //对齐开始除
        dd.shift_digits_l(i);
        for (; i >= 0; i--)
        {
            tc = cc.bn_[i + ddigits - 1];
            if ((size_t)i != cdigits - ddigits && cc.bn_[i + ddigits] > 0)
            {
                tc += (uint64_t)cc.bn_[i + ddigits] << 32;
            }
            if (tc > td)
            {
                uint64_t di = tc / td;
                assert(di <= BN_UNIT_MAX_NUM);
                ai = (uint32_t)di;
                big_uint t;
                mul_unit(t, dd, (uint32_t)di);
                uint32_t borrow = 0;
                sub(cc, cc, t, borrow);
                while (cmp(cc, dd) >= 0)
                {
                    ai++;
                    sub(cc, cc, dd, borrow);
                }
            }
            else
            {
                ai = 0;
            }
            a.bn_[i] = ai;
            dd.shift_digits_r(1);
        }
        b = cc;
    }

    void mod(big_uint &a,
             const big_uint &b,
             const big_uint &c)
    {
        big_uint t;
        return div_i(t, a, b, c);
    }

    void div(big_uint &a,
             const big_uint &b,
             const big_uint &c)
    {
        big_uint t;
        return div_i(a, t, b, c);
    }
    // a = b * c mod d
    void mod_mul(big_uint &a,
                 const big_uint &b,
                 const big_uint &c,
                 const big_uint *d)
    {
        big_uint t;
        mul(t, b, c);
        mod(a, t, d);
    }

    // https://labuladong.github.io/algo/4/32/117/
    // 蒙哥马利算法进行超大数字模幂运算
    // 模幂运算(为啥不叫幂模预算) a = b ^ c mod d , exp = exponentiation
    void mod_exp(big_uint &a,
                 const big_uint &b,
                 const big_uint &c,
                 const big_uint &d)
    {
        size_t cdigits = c.valid_digits();
        size_t ddigits = d.valid_digits();
        assert(ddigits != 0);
        if (ddigits == 0)
        {
            return;
        }

        big_uint t, bb = b;
        t.bn_[0] = 1;
        for (size_t i = 0; i < cdigits; ++i)
        {
            uint32_t ci = c.bn_[i];
            while (ci)
            {
                if (ci & 1)
                {
                    mod_mul(t, t, bb, d);
                }
                //bi -1 , 然后就是0，然后就是一个偶数，所以有下面这个公式
                mod_mul(bb, bb, bb, d);
                ci >>= 1;
            }
        }
        a = t;
    }

    void print()
    {
        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            printf("%08x,", bn_[i]);
        }
        printf("\n");
    }

    //
    void random_fill(zce::random_mt19937 &randomer, size_t digits)
    {
        for (size_t i = 0; i < digits && i < LEN_OF_U32_ARY; ++i)
        {
            bn_[i] = randomer.get_uint32();
        }
    }

    //! 如何判断一个数字是质数的方法
    //! https://blog.nowcoder.net/n/c0181f2b9aca4947ae40e8681fba4273?from=nowcoder_improve
    //! Miller-Rabin素性测试取是否是超大素数
    bool miller_rabin_isprime(const big_uint &x)
    {
        const big_uint BIG_UINT_1 = 1;
        const big_uint BIG_UINT_2 = 2;

        if (x == BIG_UINT_2)
        {
            return true; //2是素数
        }
        size_t xdigits = x.valid_digits();
        if (xdigits > 1)
        {
            return false;
        }
        if (x < BIG_UINT_2 || !(x.bn_[0] & 0x1))
        {
            return false; //如果x是偶数或者是0，1，那它不是素数
        }
        size_t s = 0;
        big_uint t = x - BIG_UINT_1;

        //将x-1分解成(2^s)*t的样子
        for (size_t i = 0; i < xdigits; ++i)
        {
            uint32_t ti = t[i];
            for (size_t v = 0; v < BN_UNIT_BITS; ++v)
            {
                if (!(ti & 0x1))
                {
                    s++;
                    ti >> 1;
                }
                else
                {
                    break;
                }
            }
            if (ti & 0x1)
            {
                break;
            }
        }
        t >>= s;
        big_uint k, a, b;
        for (size_t i = 0; i < 10 && prime[i] < x; i++) //随便选一个素数进行测试
        {
            a = prime[i];
            mod_exp(b, a, t, x);   //先算出a^t
            //然后进行s次平方
            for (size_t j = 1; j <= s; j++)
            {
                //求b的平方
                mod_mul(k, b, b, x);
                if (k == BIG_UINT_1 && b != BIG_UINT_1 && b != x - BIG_UINT_1)
                {
                    return false;
                }
                b = k;
            }
            //用费马小定理判断
            if (b != BIG_UINT_1)
            {
                return false;
            }
        }
        //如果进行多次测试都是对的，那么x就很有可能是素数
        return true;
    }

protected:
    //! 每个存储单元都是32bit
    static const size_t BN_UNIT_BITS = 32;
    //! 数组的长度是32
    static const size_t LEN_OF_U32_ARY = B / 32;
    //!
    static const size_t BN_UNIT_MAX_NUM = 0xFFFFFFFF;

protected:

    //
    uint32_t  bn_[LEN_OF_U32_ARY] = { 0 };
};
}