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

#include "zce/os_adapt/math.h"
#include "zce/math/random.h"
#include "zce/bytes/base_encode.h"

namespace zce
{
class min_prime_table
{
public:
    static const size_t TABLE_LEN = 64;
    static const uint32_t PRIME_TABLE[TABLE_LEN];
};

//! B是bignumber的bit为数，必须为32的倍数
//! 必须要考虑溢出风险，比如128bits * 128 bits的数字需要256bits保存，
//! 如果您希望使用2048bit的big number，那么您要使用4096的长度，
template <std::size_t B>
class big_uint : public min_prime_table
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
        return cmp(*this, other) < 0 ? true : false;
    }
    bool operator<= (const big_uint &other) const
    {
        return cmp(*this, other) <= 0 ? true : false;
    }

    big_uint operator+ (const big_uint &other) const
    {
        uint32_t carry = 0;
        big_uint result;
        add(result, *this, other, carry);
        return result;
    }
    big_uint operator- (const big_uint &other) const
    {
        uint32_t borrow = 0;
        big_uint result;
        sub(result, *this, other, borrow);
        return result;
    }
    big_uint operator* (const big_uint &other) const
    {
        big_uint result;
        mul(result, *this, other);
        return result;
    }
    big_uint operator/ (const big_uint &other) const
    {
        big_uint result;
        div(result, *this, other);
        return result;
    }
    big_uint operator% (const big_uint &other) const
    {
        big_uint result;
        mod(result, *this, other);
        return result;
    }

    big_uint operator+= (const big_uint &other)
    {
        uint32_t carry = 0;
        add(*this, *this, other, carry);
        return *this;
    }
    big_uint operator-= (const big_uint &other)
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
        //考虑大小端，还是不用memcmp了，
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
    void add_unit(uint32_t c)
    {
        uint64_t result = 0;
        uint32_t cc = c;
        for (size_t i = 0; i < LEN_OF_U32_ARY; i++)
        {
            result = cc + bn_[i];
            bn_[i] = result & BN_UNIT_MAX_NUM;
            if (result > (uint64_t)BN_UNIT_MAX_NUM)
            {
                cc = 1;
            }
            else
            {
                break;
            }
        }
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

    // a(this) << b
    void shift_bits_l(size_t b)
    {
        assert(b <= BN_UNIT_BITS * LEN_OF_U32_ARY);
        if (b > BN_UNIT_BITS * LEN_OF_U32_ARY)
        {
            return;
        }
        uint32_t b1 = (uint32_t)(b / BN_UNIT_BITS);
        if (b1)
        {
            shift_digits_l(b1);
        }
        uint32_t b2 = b % BN_UNIT_BITS;
        if (b2)
        {
            uint32_t t = BN_UNIT_BITS - b2, ai = 0;
            uint32_t carry = 0;
            for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
            {
                ai = bn_[i];
                bn_[i] = (ai << b2) | carry;
                carry = b2 ? (ai >> t) : 0;
            }
        }
        return;
    }
    // a >> b
    void shift_bits_r(size_t b)
    {
        assert(b <= BN_UNIT_BITS * LEN_OF_U32_ARY);
        if (b > BN_UNIT_BITS * LEN_OF_U32_ARY)
        {
            return;
        }
        uint32_t b1 = (uint32_t)b / BN_UNIT_BITS;
        if (b1)
        {
            shift_digits_r(b1);
        }
        uint32_t b2 = b % BN_UNIT_BITS;
        if (b2)
        {
            uint32_t t = BN_UNIT_BITS - b2, ai = 0;
            uint32_t carry = 0;
            for (ssize_t i = LEN_OF_U32_ARY - 1; i >= 0; --i)
            {
                ai = bn_[i];
                bn_[i] = (ai >> b2) | carry;
                carry = b2 ? (ai << t) : 0;
            }
        }
        return;
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

    //从低位lsb到高位msb，第一个二进制1 是第几位bit(仍然从0位开始计数)
    size_t scanbit_msb2lsb() const
    {
        size_t vd = valid_digits();
        if (vd == 0)
        {
            return 0;
        }
        return (vd - 1) * BN_UNIT_BITS + zce::scanbit_msb2lsb32(bn_[vd - 1]);
    }

    //！从低位lsb到高位msb数，第一个二进制1是第几个bit，如果为值为0，返回0
    size_t scanbit_lsb2msb()
    {
        size_t s = 0;
        size_t vd = valid_digits();
        for (size_t i = 0; i < vd; ++i)
        {
            uint32_t ai = bn_[i];
            for (size_t v = 0; v < BN_UNIT_BITS; ++v)
            {
                if (!(ai & 0x1))
                {
                    s++;
                    ai >>= 1;
                }
                else
                {
                    return s;
                }
            }
        }
        return 0;
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
        if (cdigits == 0 || cdigits < ddigits)
        {
            b = c;
            return;
        }

        big_uint cc = c, dd = d;

        uint64_t td = dd.bn_[ddigits - 1];
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
            if (tc >= td)
            {
                uint64_t di = tc / (td + 1);
                assert((di & 0xFFFFFFFF00000000) == 0);
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

    static void mod(big_uint &a,
                    const big_uint &b,
                    const big_uint &c)
    {
        big_uint t;
        return div_i(t, a, b, c);
    }

    static void div(big_uint &a,
                    const big_uint &b,
                    const big_uint &c)
    {
        big_uint t;
        return div_i(a, t, b, c);
    }
    // a = b * c mod d
    static void mod_mul2(big_uint &a,
                         const big_uint &b,
                         const big_uint &c,
                         const big_uint &d)
    {
        big_uint t;
        mul(t, b, c);
        mod(a, t, d);
    }

    static void  mod_mul(big_uint &a,
                         const big_uint &b,
                         const big_uint &c,
                         const big_uint &d)
    {
        big_uint rt;
        size_t cdigits = c.valid_digits();
        big_uint bb = b;
        for (size_t i = 0; i < cdigits; ++i)
        {
            uint32_t ci = c.bn_[i];
            while (ci)
            {
                if (ci & 1)
                {
                    rt = (rt + bb) % d;
                }
                bb = (bb + bb) % d;
                ci >>= 1;
            }
        }
        a = rt;
    }

    // https://labuladong.github.io/algo/4/32/117/
    // 蒙哥马利算法进行超大数字模幂运算
    // 模幂运算(为啥不叫幂模预算) a = b ^ c mod d , exp = exponentiation
    static void mod_exp(big_uint &a,
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
            printf("0x%08X,", bn_[i]);
        }
        printf("\n");
    }

    //!生成一个随机数,普通填充，用一些32位数填充，但不够紧密，不能用于质数选择
    template <class random_engine>
    void random_fill(random_engine &engine, size_t digits)
    {
        for (size_t i = 0; i < digits && i < LEN_OF_U32_ARY; ++i)
        {
            bn_[i] = engine();
        }
    }

    //!生成一个随机数，紧密填充，
    template <class random_engine>
    void random_tight(random_engine &engine, size_t digits)
    {
        for (size_t i = 0; i < digits && i < LEN_OF_U32_ARY; ++i)
        {
            for (size_t j = 0; j < 8; ++j)
            {
                bn_[i] <<= 4;
                bn_[i] |= engine() & 0xF;
            }
        }
    }

    //根据当前的数字制造一个素数
    //! RE random_engine
    template <class RANDOM_ENGINE>
    void create_prime(RANDOM_ENGINE &engine, size_t digits, size_t rounds)
    {
        size_t test_count = 0;
        random_tight(engine, digits);
        print();
        while (true)
        {
            uint32_t m = bn_[0] % 10;
            switch (m)
            {
            case 0:
            case 2:
            case 6:
            case 8:
                add_unit(1);
                break;
            case 1:
            case 5:
            case 7:
            case 9:
                add_unit(2);
                break;
            case 3:
                add_unit(4);
                break;
            case 4:
                add_unit(3);
                break;
            }
            if (isprime(engine, rounds))
            {
                break;
            }
            test_count++;
            if (test_count % 50000 == 0)
            {
                printf("test continue %zu times.\n", test_count);
                print();
            }
        }
        printf("test finished %zu times.\n", test_count);
    }

    //! 如何判断一个数字是质数的方法
    //! https://blog.nowcoder.net/n/c0181f2b9aca4947ae40e8681fba4273?from=nowcoder_improve
    //! https://www.zhihu.com/question/293656940/answer/512820832
    //! Miller-Rabin素性测试取是否是超大素数
    //! https://www.cnblogs.com/RioTian/p/13927952.html
    //! https://blog.csdn.net/ECNU_LZJ/article/details/72675595
    //! https://zhuanlan.zhihu.com/p/349360074
    //! https://bindog.github.io/blog/2014/07/19/how-to-generate-big-primes/?
    //! https://blog.nowcoder.net/n/c0181f2b9aca4947ae40e8681fba4273?from=nowcoder_improve
    template <class RANDOM_ENGINE>
    bool isprime(RANDOM_ENGINE &engine, size_t rounds) const
    {
        assert(rounds > 0);
        static const big_uint BIG_UINT_1 = 1;
        static const big_uint BIG_UINT_2 = 2;
        //2是素数
        if (*this == BIG_UINT_2)
        {
            return true;
        }
        size_t vd = valid_digits();
        //只对大质数进行判断
        assert(vd > 1);
        if (vd <= 1)
        {
            return false;
        }
        if (*this < BIG_UINT_2 || !(bn_[0] & 0x1))
        {
            return false; //如果x是偶数或者是0，1，那它不是素数
        }

        big_uint a;
        for (size_t i = 0; i < rounds; i++) //随便选一个素数进行测试
        {
            a = engine();
            if (miller_rabin2(a))
            {
                continue;
            }
            else
            {
                return false;
            }
        }
        //如果进行多次测试都是对的，那么x就很有可能是素数
        return true;
    }

    //
    bool miller_rabin(const big_uint &a) const
    {
        static const big_uint BIG_UINT_1 = 1;
        big_uint d = *this - BIG_UINT_1;
        big_uint dd = d, v;
        while (d != BIG_UINT_1)
        {
            mod_exp(v, a, d, *this);
            if (v == 1)
            {
                if (d.bn_[0] & 0x1)
                {
                    return true;
                }
                d.shift_bits_r(1);
                mod_exp(v, a, d, *this);
                if (v == dd)
                {
                    return true;
                }
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    //
    bool miller_rabin2(const big_uint &a) const
    {
        static const big_uint BIG_UINT_1 = 1;
        big_uint t = *this - BIG_UINT_1;
        big_uint tt = t;
        //将x-1分解成(2^s)*t的样子

        size_t s = t.scanbit_lsb2msb();
        if (s)
        {
            t.shift_bits_r(s);
        }

        big_uint k, b;

        mod_exp(b, a, t, *this);   //先算出a^t
        //然后进行s次平方
        for (size_t j = 1; j <= s; j++)
        {
            //求b的平方
            mod_mul(k, b, b, *this);
            if (k == BIG_UINT_1 && b != BIG_UINT_1 && b != tt)
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