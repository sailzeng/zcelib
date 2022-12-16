/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/math/big_uint.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2022年12月11日
* @brief      大整数
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
//! B是bignumber的bit为数，必须为32的倍数
//! 必须要考虑溢出风险，比如128bits * 128 bits的数字需要256bits保存，
//! 如果您希望使用2048bit的big number，那么您要使用4096的长度，
template <std::size_t B>
class big_uint
{
public:

    typedef uint32_t UNIT_TYPE;
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
    bool operator> (const big_uint &other) const
    {
        return cmp(*this, other) > 0 ? true : false;
    }
    bool operator>= (const big_uint &other) const
    {
        return cmp(*this, other) >= 0 ? true : false;
    }

    big_uint operator+ (const big_uint &other) const
    {
        uint32_t carry = 0;
        big_uint result = add(*this, other, carry);
        return result;
    }
    big_uint operator- (const big_uint &other) const
    {
        uint32_t borrow = 0;
        big_uint result = sub(*this, other, borrow);
        return result;
    }
    big_uint operator* (const big_uint &other) const
    {
        big_uint result = mul(*this, other);
        return result;
    }
    big_uint operator/ (const big_uint &other) const
    {
        big_uint result = div(*this, other);
        return result;
    }
    big_uint operator% (const big_uint &other) const
    {
        big_uint result = mod(*this, other);
        return result;
    }

    big_uint operator+= (const big_uint &other)
    {
        uint32_t carry = 0;
        *this = add(*this, other, carry);
        return *this;
    }
    big_uint operator-= (const big_uint &other)
    {
        uint32_t borrow = 0;
        *this = sub(*this, other, borrow);
        return *this;
    }
    big_uint operator*= (const big_uint &other)
    {
        *this = mul(*this, other);
        return *this;
    }
    big_uint operator/= (const big_uint &other)
    {
        *this = div(*this, other);
        return *this;
    }
    big_uint operator%= (const big_uint &other)
    {
        *this = mod(*this, other);
        return *this;
    }
    big_uint operator>>= (size_t b)
    {
        shift_bits_r(b);
        return *this;
    }
    big_uint operator<<= (size_t b)
    {
        shift_bits_l(b);
        return *this;
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

    //! 赋值函数，用数组长度digits的数组b赋值
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
    //!赋值函数，动态填充digits个uint32_t，例子，assign(4, 0x1234,0x4321,0x3412,0x2143)
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
    //!赋值函数,赋值一个整数
    void assign(uint32_t one_unit)
    {
        this->bn_[0] = one_unit;
        for (size_t i = 1; i < LEN_OF_U32_ARY; ++i)
        {
            this->bn_[i] = 0;
        }
    }
    // a(*this) = 0
    void zero()
    {
        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            bn_[i] = 0;
        }
    }
    // a(*this) == 0
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

    //！uint32_t单元左移，注意这不是按bit来的，是按单元unit进行的移位
    void shift_units_l(size_t b)
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
    //！uint32_t单元右移
    void shift_units_r(size_t b)
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

    //! 按bit位左移 a(this) << b
    void shift_bits_l(size_t b)
    {
        assert(b <= BN_ALL_BITS);
        if (b > BN_ALL_BITS)
        {
            return;
        }
        uint32_t b1 = (uint32_t)(b / BN_UNIT_BITS);
        if (b1)
        {
            shift_units_l(b1);
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
    //! 按bit位右移 a >> b
    void shift_bits_r(size_t b)
    {
        assert(b <= BN_ALL_BITS);
        if (b > BN_ALL_BITS)
        {
            return;
        }
        uint32_t b1 = (uint32_t)b / BN_UNIT_BITS;
        if (b1)
        {
            shift_units_r(b1);
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

    //取得某个index bit位置上是否为1
    bool get_bit(size_t b_i) const
    {
        assert(b_i < BN_ALL_BITS);
        if (b_i >= BN_ALL_BITS)
        {
            return false;
        }
        size_t ui = b_i / BN_UNIT_BITS;
        size_t bi = b_i % BN_UNIT_BITS;
        return bn_[ui] & (0x1 << bi);
    }
    //某个index bit位置上置1
    void set_bit_1(size_t b_i)
    {
        assert(b_i < BN_ALL_BITS);
        if (b_i >= BN_ALL_BITS)
        {
            return;
        }
        size_t ui = b_i / BN_UNIT_BITS;
        size_t bi = b_i % BN_UNIT_BITS;
        bn_[ui] = bn_[ui] | (0x1 << bi);
    }
    //某个index bit位置上置0
    void set_bit_0(size_t b_i)
    {
        assert(b_i < BN_ALL_BITS);
        if (b_i >= BN_ALL_BITS)
        {
            return;
        }
        size_t ui = b_i / BN_UNIT_BITS;
        size_t bi = b_i % BN_UNIT_BITS;
        bn_[ui] = bn_[ui] & (~(0x1 << bi));
    }
    //!取出某个单元的数据
    uint32_t get_unit(size_t u_i)
    {
        assert(u_i < LEN_OF_U32_ARY);
        if (u_i >= LEN_OF_U32_ARY)
        {
            return 0;
        }
        return bn_[u_i];
    }
    //!设置某个单元的数值
    void set_unit(size_t u_i, uint32_t b)
    {
        assert(u_i < LEN_OF_U32_ARY);
        if (u_i >= LEN_OF_U32_ARY)
        {
            return 0;
        }
        bn_[u_i] = b;
    }

    // returns valid significant length of a in digits
    size_t valid_units() const
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
    bool scanbit_msb2lsb(size_t &index) const
    {
        size_t vu = valid_units();
        if (vu == 0)
        {
            return false;
        }
        index = (vu - 1) * BN_UNIT_BITS + zce::scanbit_msb2lsb32(bn_[vu - 1]);
        return true;
    }

    //！从低位lsb到高位msb数，第一个二进制1是第几个bit，如果为值为0，返回0
    bool scanbit_lsb2msb(size_t &index)
    {
        index = 0;
        size_t vu = valid_units();
        for (size_t i = 0; i < vu; ++i)
        {
            uint32_t ai = bn_[i];
            for (size_t v = 0; v < BN_UNIT_BITS; ++v)
            {
                if (!(ai & 0x1))
                {
                    index++;
                    ai >>= 1;
                }
                else
                {
                    return true;
                }
            }
        }
        return false;
    }

    //! 加一个unit32_t add one unit32_t
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

    // sub one unit
    void sub_unit(uint32_t c)
    {
        big_uint cc = c;
        *this = sub(*this, cc);
    }

    //! mul one unit
    static big_uint mul_unit(const big_uint &b,
                             uint32_t c)
    {
        big_uint a;
        uint64_t result = 0;
        uint32_t carry = 0, rh = 0, rl = 0;
        if (c == 0)
        {
            return a;
        }

        for (size_t i = 0; i < LEN_OF_U32_ARY; ++i)
        {
            result = (uint64_t)c * b.bn_[i];
            rl = result & BN_UNIT_MAX_NUM;
            rh = (result >> BN_UNIT_BITS);
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
        return a;
    }

    //@note : return a = b + c 这种返回值模式的缺点就是不好调试，返回值对象a经常被VC++优化了

    //return a = b + c
    static big_uint add(const big_uint &b,
                        const big_uint &c,
                        uint32_t &carry)
    {
        big_uint a;
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
        return a;
    }

    //! 加法 return a = b - c
    static big_uint sub(const big_uint &b,
                        const big_uint &c,
                        uint32_t &borrow)
    {
        big_uint a;
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
        return a;
    }

    // 减法return a = b * b
    static big_uint mul(const big_uint &b,
                        const big_uint &c)
    {
        uint32_t carry = 0;
        big_uint a, bb = b, cc = c;
        size_t cvu = cc.valid_units();

        big_uint t;
        for (size_t i = 0; i < cvu; i++)
        {
            t = mul_unit(bb, cc.bn_[i]);
            a = add(a, t, carry);
            bb.shift_units_l(1);
        }
        return a;
    }

    //! 采用移动递减的方式实现的整数除法，如果位数相差大，也会耗时，但时间消耗可以预估，
    //! 最后，仍然采用这个函数
    //! a = b / c, m = b % c
    static big_uint div_i(const big_uint &b,
                          const big_uint &c,
                          big_uint &m)
    {
        big_uint a, aa;

        size_t bvb = 0, cvb = 0;
        bool bvu = b.scanbit_msb2lsb(bvb);
        bool cvu = c.scanbit_msb2lsb(cvb);
        //c could n't == 0
        assert(cvu);
        if (cvu == false)
        {
            return a;
        }
        if (bvu == false || bvb < cvb)
        {
            m = b;
            return a;
        }

        big_uint bb = b, cc = c;
        size_t difb = bvb - cvb;
        cc.shift_bits_l(difb);
        uint32_t borrow = 0;
        for (ssize_t i = difb; i >= 0; --i)
        {
            if (cmp(bb, cc) >= 0)
            {
                bb = sub(bb, cc, borrow);
                aa.set_bit_1(i);
            }
            cc.shift_bits_r(1);
        }
        m = bb;
        a = aa;
        return a;
    }

    //! return a = b % c
    static big_uint mod(const big_uint &b,
                        const big_uint &c)
    {
        big_uint t, a;
        t = div_i(b, c, a);
        return a;
    }
    //! return a = b / c
    static big_uint div(const big_uint &b,
                        const big_uint &c)
    {
        big_uint t;
        return div_i(b, c, t);
    }
    // return a = b * c mod d 幂乘
    static big_uint mod_mul(const big_uint &b,
                            const big_uint &c,
                            const big_uint &d)
    {
        big_uint t;
        t = mul(b, c);
        return mod(t, d);
    }

    // https://labuladong.github.io/algo/4/32/117/
    // 蒙哥马利算法进行超大数字模幂运算
    // 模幂运算(为啥不叫幂模预算) a = b ^ c mod d , exp = exponentiation
    static big_uint mod_exp2(const big_uint &b,
                             const big_uint &c,
                             const big_uint &d)
    {
        big_uint a, t = 1;
        size_t cvb = 0;
        bool cvu = c.scanbit_msb2lsb(cvb);
        size_t dvu = d.valid_units();
        assert(dvu != 0);
        if (dvu == 0 || cvu == false)
        {
            return a;
        }

        for (ssize_t i = cvb; i >= 0; --i)
        {
            t = mod_mul(t, t, d);
            if (c.get_bit(i))
            {
                t = mod_mul(t, b, d);
            }
        }
        a = t;
        return a;
    }

    static big_uint mod_exp(const big_uint &b,
                            const big_uint &c,
                            const big_uint &d)
    {
        big_uint a, t = 1, bb = mod(b, d);
        size_t cvb = 0;
        bool cvu = c.scanbit_msb2lsb(cvb);
        size_t dvu = d.valid_units();
        assert(dvu != 0);
        if (dvu == 0 || cvu == false)
        {
            return a;
        }

        for (size_t i = 0; i <= cvb; ++i)
        {
            if (c.get_bit(i))
            {
                t = mod_mul(t, bb, d);
            }
            bb = mod_mul(bb, bb, d);
        }
        a = t;
        return a;
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

    /*!
     * @brief 创造一个质数，选择一个随机数，然后累加进行预算
     * @tparam RANDOM_ENGINE 随机数引擎
     * @param engine 随机数引擎
     * @param digits 质数的 unit 数量，就是所少个uint32_t
     * @param rounds 使用 miller_rabin 判断的质数轮数，轮数越多，概率越高
     * @param counter 测试的数字数量，
    */
    template <class RANDOM_ENGINE>
    void create_prime(RANDOM_ENGINE &engine,
                      size_t digits,
                      size_t rounds,
                      size_t &counter)
    {
        counter = 0;
        random_tight(engine, digits);
        print();
        while (true)
        {
            //只处理尾数，1,3,7,9尾数的值
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
            counter++;
        }
    }

    //! 判断一个数字是质数的方法
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
        size_t vu = valid_units();
        //此函数只对2个以上的单元进行大质数进行判断
        assert(vu > 1);
        if (vu <= 1)
        {
            return false;
        }
        //如果x是偶数或者是0，1，那它不是素数
        if (*this < BIG_UINT_2 || !(bn_[0] & 0x1))
        {
            return false;
        }
        //随便选一个素数进行测试
        for (size_t i = 0; i < rounds; i++)
        {
            big_uint a = engine();
            if (miller_rabin(a))
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

    //https://www.cnblogs.com/RioTian/p/13927952.html
    //Miller - Rabin素性测试取是否是超大素数
    bool miller_rabin(const big_uint &a) const
    {
        static const big_uint BIG_UINT_1 = 1;
        big_uint t = *this - BIG_UINT_1;
        big_uint tt = t;

        big_uint k, b;
        b = mod_exp(a, t, *this);   //先算出a^t
        //用费马小定理判断
        if (b != BIG_UINT_1)
        {
            return false;
        }

        //用2次探测，
        //将x-1分解成(2^s)*t的样子
        size_t s = 0;
        t.scanbit_lsb2msb(s);
        //然后进行s次平方
        for (size_t j = 1; j <= s; j++)
        {
            t.shift_bits_r(1);
            k = mod_exp(a, t, *this);
            if (k != BIG_UINT_1 && k != tt)
            {
                return false;
            }
        }
        return true;
    }
protected:
    //! bits 长度
    static const size_t BN_ALL_BITS = B;
    //! 每个存储单元都是32bit
    static const size_t BN_UNIT_BITS = 32;
    //! 数组的长度是32
    static const size_t LEN_OF_U32_ARY = B / 32;
    //! 每个单元的最大长度
    static const size_t BN_UNIT_MAX_NUM = 0xFFFFFFFF;

protected:

    //
    uint32_t  bn_[LEN_OF_U32_ARY] = { 0 };
};
}

//// 除法的第二种方式，采用了试商，在除数比较大时间有优势，但如果除数比较小（相对被除数），
//// 可能发生需要很多时间的情况。所以没有使用这个方式，
//// a = b / c, m = b % c
//static big_uint div_i_2(const big_uint &b,
//                        const big_uint &c,
//                        big_uint &m)
//{
//    big_uint aa, a;
//    size_t bvu = b.valid_units();
//    size_t cvu = c.valid_units();
//    assert(cvu != 0);
//    if (cvu == 0)
//    {
//        return a;
//    }
//    if (bvu == 0 || bvu < cvu)
//    {
//        m = b;
//        return a;
//    }
//
//    big_uint bb = b, cc = c;
//
//    uint64_t td = cc.bn_[cvu - 1];
//    uint64_t tc = 0;
//
//    uint32_t ai = 0;
//    ssize_t i = bvu - cvu; //商最多的位数 -1
//    //对齐开始除
//    cc.shift_digits_l(i);
//    for (; i >= 0; i--)
//    {
//        tc = bb.bn_[i + cvu - 1];
//        if ((size_t)i != bvu - cvu && bb.bn_[i + cvu] > 0)
//        {
//            tc += (uint64_t)bb.bn_[i + cvu] << 32;
//        }
//        if (tc >= td)
//        {
//            //td 必须加1，因为 tc （可能）小于实际被除数，td 也小于实际除数，得到的结果可能超出
//            //@note 所以这儿如果被除数很小，很可能很慢
//            uint64_t di = tc / (td + 1);
//            assert((di & 0xFFFFFFFF00000000) == 0);
//            ai = (uint32_t)di;
//            big_uint t = mul_unit(cc, (uint32_t)di);
//
//            uint32_t borrow = 0;
//            bb = sub(bb, t, borrow);
//            while (cmp(bb, cc) >= 0)
//            {
//                ai++;
//                bb = sub(bb, cc, borrow);
//            }
//        }
//        else
//        {
//            ai = 0;
//        }
//        aa.bn_[i] = ai;
//        cc.shift_digits_r(1);
//    }
//    m = bb;
//    a = aa;
//    return a;
//}