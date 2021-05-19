/*!
* @filename   zce/util/random.h
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年10月1日
* @brief      若干个产生随机数的类，包括随机数发生器等
*             此代码名为boost，其实和boost几乎没有多大关系，只是boost
*             有类似的代码。C++11后来也有相关的的库了，但我主要目标是伺候
*             好游戏开发，所以还是保留了这个代码。
* @details
* 为什么要自己写rand的代码，因为2个原因，
* 1.系统的rand不太好，比如他们的循环长度级别一般是2^32,而且WIN的random值
* 产生小于65535的数值，我深刻记得一次测试用随机数模拟大量HASH的种子，结果冲
* 突到死。
* 2.我们所有的rand都是伪随机，伪随机是什么意思？就是你用产生的一个数据可以推算
* 后面一个产生的数据，所以为了安全还是不用他们吧。
* （比如你用随机数产生一个电话卡密码，如果你不做一些变化，那么买了一张卡的人，
* 可以推算你产生的所有电话卡密码）
*  综上所述，我觉得还是自己实现一个比较好，特别是游戏，需要有大量的随机数的地方
*
* 随机数的算法基本分成几种
* @li 线性同余 效果一般
* @li 平方取中 效果很一般
* @li 数学天才们想出来的算法，下面就是这些算法的集合汇总，
*
* 关于随机数的算法问题，俺的数学不好，基本都是从BOOST中间知道的名字，然后从
* 网上爬的算法，
* 本来想BOOST的代码想提取出来，发现要费太大的力气（模版中毒太深），不合算，
* 算了，（其实我的代码最后为了简洁也用了部分模版）
*
* 随机数的一些参考实现代码
* GNU Scientific Library (GSL) 里面也有很多随机数的函数，有兴趣可以看看，
* 我估计BOOST的带来也来自这儿。GNU真是好人呀
* http://www.koders.com/c/fid694A621D3301A5177FEF0181F19D64D75CDC9CCC.aspx?s=ranlux#L1
* 这儿也有少量几个实现
* http://www.koders.com/cpp/fid63AE02B1E8D46B18CBAC4338C47735991DBCBFC0.aspx?s=Chebyshev
*
* 我想使用的产生随机数的算法包括如下几个，同时给出BOOST给出的他们的算法的速度以及循环周期对比
* 算法                      循环长度       生成数字的范围         BOOST给出的算法时间比     我的实现给出的算法时间比
*                   x                                          （100%最快），           产生1亿用时 每s产生数量     和bsdrand比较   和（第二名） mt19937比较
* VC++Rand                               2^16                 无                      2.304s    43402777.7    10.67%         18.57%
* bsdrand                  2^31          (uint32)0 - 2^31     无                      0.216s    462962962.6   100%           198.14%
* rand48                   2^48          (uint32)0 - 2^32     59%                     0.681s    146842878.1   31.71%         62.84%
* taus88                   ~2^88         (uint32)0 - 2^32     100%                    0.4610    216919739.6   46.85%         92.84
* mt19937                  2^19937       (uint32)0 - 2^32     81%                     0.4280    233644859.8   50.46%         100%
* mt11231a                 2^11231       (uint32)0 - 2^32     81%                     0.4290    233100233.1   50.34%         99.76%
* mt11231b                 2^11231       (uint32)0 - 2^32     81%                     0.4330    230946882.2   49.88%         98.84%
* ranlux223(ranlux3)       ~10^171       (uint32)0 - 2^24     6%                      8.8760    1126636.1     0.24%          0.48%
* ranlux389(ranlux4)       ~10^171       (uint32)0 - 2^24     3%                      15.582    641761.4      0.13%          0.27%
*
* 对于循环长度我的理解是，如果算法产生一个0x1111的数值，第二个产生0x2222,如果循环长度是2^31，那么2^31次之后
* ，就会再次出现0x1111，0x2222这样的值。
*
* 补充说明rand48在BOOST里面的算法说明生成数字范围是0-2^31,但在GSL里面的说明是0-2^32,而且两者算法不太一样
* 我的测试数据是产生1亿个随机数，计算耗时，以及每秒可以产生的数量,测试环境i5,Win7 64 ,VC++2010,realse
*
* 参考BOOST的说明在
* http://www.boost.org/doc/libs/1_48_0/doc/html/boost_random/performance.html
*
* 另外BOOST的随机数库已经进入了tr1和C++1x的标准，所以不久的将来（也许5年），我们就可以直接使用类库了，
* 再者，BOOST的随机数库的实现过于…………，感觉违背了尽量简单的原则，其包括3个部分，随机数发生器，分布器（限制随机数产生的范围），
* 变量发生器（包装）,当然从设计的角度，这样的封装很优美，但学习成本会限制大家的使用的…………
*
* 这个代码的名字是BOOST，实际上对BOOST设计有借鉴和简化，我不打算搞的太复杂，我需要的随机数分布大约也就是uint16_t,uint32_t,double,01,的平均分布
* 我不想搞的太复杂，我打算就用2个部分，随机数发生器，随机数发生器产生的数据都是uint32_t
* 变量产生器(里面包装了各种平均分布的函数),（默认包装一个随机数发生器），你使用变量产生器的typedef就OK了。
*
* @note
* 代码里面有大量都魔术数字，不是我要搞这种风格，是我和你们一样，对这些数字的含义是白痴
* 另外好奇一下，这些常量是发明算法实践出来的，还是有一定的科学背景的？
*/
#ifndef ZCE_LIB_BOOST_RANDOM_H_
#define ZCE_LIB_BOOST_RANDOM_H_

#include "zce/logger/logging.h"
#include "zce/util/singleton.h"

namespace zce
{
//==============================================================================================================

/*!
* @brief      随机数的基类，定义一些基本函数，如果没有特殊情况，一些函数继承者就不用重载了
* @note       注意我们的随机数函数rand只考虑输出的类型是uint32_t，
*/
class random_base
{
public:

    ///产生的随机数类型
    enum class RAND_STRING
    {
        ///产生数字字符串，字符0-9
        NUMBER,
        ///产生小写字符串，字符a-z
        LOWER,
        ///产生大写字符串，字符A-Z
        UPPER,
        ///产生数字+小写字符串，字符0-9，a-z
        NUMBER_LOWER,
        ///产生数字大写字符串，字符0-9，A-Z
        NUMBER_UPPER,
        ///产生数字+小写+大写字符串，字符0-9，a-z,A-Z
        NUMBER_LOWER_UPPER,
        ///产生0-127的ASCII(非扩展)字符串
        ASCII,
        ///产生0-255二进制字符串
        BINARY,
    };

    //构造函数和析构函数，避免你声明的写法
protected:
    ///构造函数不能产生实例，因为不起作用，只能让各位继承后做点事情
    random_base()
    {
    }
public:
    virtual ~random_base()
    {
    }

public:

    /*!
    @brief      放弃若干个随机数，当为了防止对方猜测时，可以使用
    @param      discard_num 放弃的次数
    */
    virtual void discard(size_t discard_num)
    {
        for (size_t i = 0; i < discard_num; ++i)
        {
            rand();
        }
    }

    ///@brief  你必须实现的函数,产生随机数,
    ///@note   注意rand产生的随机数是32位的，得到其他数值类型的随机数函数都是根据32位去适配
    virtual uint32_t rand() = 0;
    ///你必须实现的函数,如何处理随机数种子
    virtual void srand(uint32_t seed) = 0;

    ///产生一个0~0xFFFFFFFF的随机数值，一些随机数函数产生的范围都并不是整数范围，
    ///如果是这种情况要进行转换
    inline virtual uint32_t get_uint32()
    {
        return rand();
    }

    ///得到一个64bit的数字,将两个数字uin32_t合并,
    ///这些算法写的比较简单，相对而言，Boost在这方面的处理要复杂很多，他一方面要考虑通用性，一方面……
    ///我的认知是这样的，如果这个随机数的发生器的产生的数字是足够随机的，那么应该满足我们的需求，
    inline virtual uint64_t get_uint64()
    {
        uint64_t x1 = get_uint32();
        uint64_t x2 = get_uint32();
        return (x1 << 32) + x2;
    }

    ///得到一个double随机数,其实就是根据32bit拼接一个double（绝对C语言写法），BOOST的实现相对复杂很多
    inline virtual double get_double()
    {
        double x = 0.0;
        uint64_t x_64 = get_uint64();
        //这TMD绝对是个C语言的写法。但假如每一位都是足够随机的，那么……这个吼吼
        memcpy(&x, &x_64, sizeof(double));
        return x;
    }

    ///得到[0-1)之间浮点数，用于某些概率控制
    inline virtual double get_double01()
    {
        return double(get_uint32()) / (double((std::numeric_limits<uint32_t>::max)()) + 1);
    }

    /*!
    * @brief      根据要求，生成一个随机数字符串，
    * @param[in]  str_type  随机数字符串的类型，@ref random_base::RAND_STRING
    * @param[out] rand_str  返回的生成随机数字符串
    * @param[in]  str_len   要求生成的字符串的长度
    */
    virtual void get_string(RAND_STRING str_type, char* rand_str, size_t str_len);

protected:

    ///默认的种子,你看的明白的，写在这儿，不是为了纪念这场运动，而是提出一个反思，
    ///他给中国带来了什么？大概除了民族意思的觉醒以外，什么都没有改变，而民族问题却成为了
    ///这个国家屡试不爽的壮阳药。
    ///而这个运动中提出的对于这个民族国家最应该解决的两个问题，德先生，赛先生的问题，在快
    ///100年后的今天仍然没有解决。
    ///我们回想和纪念5.4，不应该只因为他是某些事的开端，而应该思考，这近百年的曲折，艰辛。
    static const uint32_t DEFAULT_SEED = 19190504UL;
};

/*!
* @brief      在随机数的类里面，记录最大值和最小值，注意不同的随机数算法产生的随机数是有范围的
* @tparam     min_result rand函数产生的随机数的最小值
* @tparam     max_result rand函数产生的随机数的最大值
* @note
*/
template <uint32_t min_result, uint32_t max_result >
class t_random_base : public random_base
{
public:

    static uint32_t min_value()
    {
        return MIN_RESULT;
    }

    static uint32_t max_value()
    {
        return MAX_RESULT;
    }

public:

    ///最大的最小的，的结果值
    static const uint32_t MIN_RESULT = min_result;
    static const uint32_t MAX_RESULT = max_result;
};

//==============================================================================================================

/*!
* @brief      BSD的RAND代码,他100%是线性同余
*/
class random_bsdrand : public t_random_base<0, 0x7FFFFFFF>
{
public:
    ///构造函数
    explicit random_bsdrand(uint32_t seed = DEFAULT_SEED) :
        seed_x_(0)
    {
        srand(seed);
    }
    ///析构函数
    virtual ~random_bsdrand()
    {
    }
    ///计算随机数
    virtual uint32_t rand()
    {
        seed_x_ = (1103515245 * seed_x_ + 12345) & 0x7FFFFFFF;
        return seed_x_;
    }

    ///得到种子
    virtual void srand(uint32_t s)
    {
        seed_x_ = s;
    }

    ///BSD的算法最高值是0x7FFFFFFF,所以补充最高位
    virtual uint32_t get_uint32()
    {
        uint32_t x = rand();
        uint32_t highest_bit = ((x & 0x2) ? 1 : 0) ^ ((x & 0x4)) ? 1 : 0;

        if (highest_bit)
        {
            x = 0x80000000 | x;
        }

        return x;
    }

protected:
    ///记录的随机数种子数据，
    uint32_t seed_x_;
};

//==============================================================================================================

/*!
* @brief      rand48 算法随机数发生器，其的循环长度是2^48,故此有此名字，
*/
class random_rand48 : public t_random_base<0, 0xFFFFFFFF>
{
public:
    ///构造函数
    explicit random_rand48(uint32_t seed = DEFAULT_SEED) :
        rand48_x0_(0),
        rand48_x1_(0),
        rand48_x2_(0)
    {
        srand(seed);
    }
    ///析构函数
    virtual ~random_rand48()
    {
    }

    ///产生随机数
    virtual uint32_t rand()
    {
        rand48_advance();

        uint32_t x2 = rand48_x2_;
        uint32_t x1 = rand48_x1_;
        return (x2 << 16) + x1;
    }

    ///计算得到种子
    virtual void srand(uint32_t s)
    {
        if (s == 0)
        {
            rand48_x0_ = 0x330E;
            rand48_x1_ = 0xABCD;
            rand48_x2_ = 0x1234;
        }
        else
        {
            rand48_x0_ = 0x330E;
            rand48_x1_ = static_cast<uint16_t>(s & 0xFFFF);
            rand48_x2_ = static_cast<uint16_t>((s >> 16) & 0xFFFF);
        }

        return;
    }

protected:
    //
    inline void rand48_advance()
    {
        /* work with unsigned long ints throughout to get correct integer promotions of any unsigned short ints */
        const uint32_t x0 = rand48_x0_;
        const uint32_t x1 = rand48_x1_;
        const uint32_t x2 = rand48_x2_;

        uint32_t a = RAND48_A0 * x0 + RAND48_C0;
        rand48_x0_ = static_cast<uint16_t>(a & 0xFFFF);

        a >>= 16;

        /* although the next line may overflow we only need the top 16 bits in the following stage, so it does not matter */

        a += RAND48_A0 * x1 + RAND48_A1 * x0;
        rand48_x1_ = static_cast<uint16_t>(a & 0xFFFF);

        a >>= 16;
        a += RAND48_A0 * x2 + RAND48_A1 * x1 + RAND48_A2 * x0;
        rand48_x2_ = static_cast<uint16_t>(a & 0xFFFF);
    }

protected:
    //魔术数字大集合
    static const uint16_t RAND48_A0 = 0xE66D;
    static const uint16_t RAND48_A1 = 0xDEEC;
    static const uint16_t RAND48_A2 = 0x0005;
    static const uint16_t RAND48_C0 = 0x000B;

protected:
    ///计算rand48的几个数据
    uint16_t rand48_x0_, rand48_x1_, rand48_x2_;
};

//==============================================================================================================

//这一个内部宏，外部不要使用，放在.h文件完全是因为我不想搞个.cpp麻烦。
#ifndef __ZCE_TAUSWORTHE
#define __ZCE_TAUSWORTHE(arg,stage1,stage2,stage3,limit) ((arg&limit)<<stage1) ^ (((arg <<stage2) ^ arg)>>stage3)
#endif

/*!
@brief      taus88 完整名称应该是Tausworthe，因为其循环长度是~2^88，有此名字，
            其参考文档是
            "Maximally Equidistributed Combined Tausworthe Generators", Pierre L'Ecuyer, Mathematics of Computation, Volume 65, Number 213, January 1996, Pages 203-213
*/
class random_taus88 : public t_random_base<0, 0xFFFFFFFF>
{
public:
    //构造函数还是把默认种子搞出来把
    explicit random_taus88(uint32_t seed) :
        seed_arg_{ 0 }
    {
        srand(seed);
    }
    virtual ~random_taus88()
    {
    }

public:

    ///处理种子的函数
    virtual void srand(uint32_t seed)
    {
        if (seed == 0)
        {
            seed = static_cast<uint32_t>(time(NULL));
        }

        seed_arg_[0] = 69069 * seed;
        seed_arg_[1] = 69069 * seed_arg_[0];
        seed_arg_[2] = 69069 * seed_arg_[1];

        //Tausworthe对参数有要求
        if (seed_arg_[0] < 2)
        {
            seed_arg_[0] += 2;
        }

        if (seed_arg_[1] < 8)
        {
            seed_arg_[1] += 8;
        }

        if (seed_arg_[2] < 16)
        {
            seed_arg_[2] += 16;
        }
    }

    ///得到随机数函数
    virtual inline uint32_t rand()
    {
        seed_arg_[0] = __ZCE_TAUSWORTHE(seed_arg_[0], 12, 13, 19, 4294967294UL);
        seed_arg_[1] = __ZCE_TAUSWORTHE(seed_arg_[1], 4, 2, 25, 4294967288UL);
        seed_arg_[2] = __ZCE_TAUSWORTHE(seed_arg_[2], 17, 3, 11, 4294967280UL);
        return (seed_arg_[0] ^ seed_arg_[1] ^ seed_arg_[2]);
    }

protected:

    //__RANDOM_TAUSWORTHE宏相当于
    uint32_t tausworthe(uint32_t arg, uint32_t stage1, uint32_t stage2, uint32_t stage3, uint32_t limit)
    {
        return ((arg & limit) << stage1) ^ (((arg << stage2) ^ arg) >> stage3);
    }

protected:

    //taus88所需要的3个参数,别问我这些算法为啥是这样，我不是数学系出生，
    uint32_t seed_arg_[3];
};

//==============================================================================================================

#ifndef __ZCE_MT_TWIST
#define __ZCE_MT_TWIST(u,v,a,b,c) (( ( ((u) & (a)) | ((v) & (b) )) >> 1) ^ ((v)&0x1 ? (c) : 0))
#endif

/*!
* @brief      MT算法的随机数发生器
*             Mersenne Twister梅森旋转算法，这真不是在大家都反日的时候给大家添乱，这个算法的确是日本人发明的。
*             而且还是目前可以看到最好随机数算法，欢迎爱国青年灭了他（找出比他更好的算法）
*             Makoto Matsumoto（松本真） 和Takuji Nishimura（西村拓士）在1997年开发的，
*             基于有限二进制字段上的矩阵线性递归field F_{2}。 可以快速产生高质量的伪随机数， 修正了古典随机数发生算法的很多缺陷。
*             http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/emt19937ar.html
*
* @tparam     mt_n  都是一些算法常量,多少轮计算一次，也是保存随机数数组的长度，
* @tparam     mt_m  都是一些算法常量,部分循环常量
* @tparam     mt_a  都是一些算法常量，根据这些常量组成了mt19937等算法
* @tparam     mt_b  算法常量
* @tparam     mt_c  算法常量
* @tparam     mt_o  算法常量
* @tparam     mt_p  算法常量
* @tparam     mt_q  算法常量
* @tparam     mt_r  算法常量
* @tparam     mt_s  算法常量
* @tparam     mt_t  还是一些算法常量
* @tparam     mt_u  都TNND是一些算法常量
* @note       值得一提的是这些日本人又搞了一个提速的版本，SFMT SIMD-oriented Fast Mersenne Twister SFMT
*             利用SSE2的功能提供了速度，我对SSE2的代码的可移植性没有把握，暂时放弃
*/
template < size_t mt_n, size_t mt_m, uint32_t mt_a, uint32_t mt_b, uint32_t mt_c,
    uint32_t mt_o, uint32_t mt_p, uint32_t mt_q, uint32_t mt_r, uint32_t mt_s, uint32_t mt_t, uint32_t mt_u >
    class random_mt : public t_random_base<0, 0xFFFFFFFF>
{
public:

    explicit random_mt(uint32_t seed = DEFAULT_SEED) :
        state_{ 0 },
        postion_(0)
    {
        srand(seed);
    }
    virtual ~random_mt()
    {
    }

    ///得到随机数
    virtual void srand(uint32_t seed)
    {
        state_[0] = seed;

        for (size_t j = 1; j < mt_n; j++)
        {
            state_[j] = (1812433253 * (state_[j - 1] ^ (state_[j - 1] >> 30)) + static_cast<uint32_t>(j));
        }

        //为了在第一次就触发gen_state
        postion_ = mt_n;
    }

    ///得到随机数，其实MT的算法非常非常快,他只在mt_n轮后才计算一次，计算结构保存在数组里面
    inline virtual uint32_t rand()
    {
        // new state vector needed
        if (postion_ == mt_n)
        {
            gen_state();
            // reset position
            postion_ = 0;
        }

        // gen_state() is split off to be non-inline, because it is only called once
        // in every 624 calls and otherwise irand() would become too big to get inlined
        uint32_t x = state_[postion_++];

        x ^= (x >> mt_o) & mt_p;
        x ^= (x << mt_q) & mt_r;
        x ^= (x << mt_s) & mt_t;
        return x ^ (x >> mt_u);
    }

protected:

    // generate new state vector
    void gen_state()
    {
        uint32_t y_data = 0;

        for (size_t i = 0; i < (mt_n - mt_m); ++i)
        {
            y_data = (state_[i] & mt_a) | (state_[i + 1] & mt_b);
            state_[i] = state_[i + mt_m] ^ (y_data >> 1) ^ ((state_[i + 1] & 1) * mt_c);
        }

        for (size_t i = mt_n - mt_m; i < (mt_n - 1); ++i)
        {
            y_data = (state_[i] & mt_a) | (state_[i + 1] & mt_b);
            state_[i] = state_[i + mt_m - mt_n] ^ (y_data >> 1) ^ ((state_[i + 1] & 1) * mt_c);
        }

        y_data = (state_[mt_n - 1] & mt_a) | (state_[0] & mt_b);
        state_[mt_n - 1] = state_[mt_m - 1] ^ (y_data >> 1) ^ ((state_[0] & 1) * mt_c);
    }

protected:

    //state数组存放了每次生成的随机数
    uint32_t     state_[mt_n];
    //记录每次取得的随机数的位置
    size_t       postion_;
};

//看不懂吧，其实我也看不懂，mt[数字]，里面的数字其实是说循环长度是2^数字
///后缀为A这个里面的魔术数字是我从网上爬到的 http://www.quadibloc.com/crypto/co4814.htm
typedef random_mt<351, 175, 0xFFF80000, 0x0007FFFF, 0xE4BD75F5, 11, 0xFFFFFFFF, 7, 0x655E5280, 15, 0xFFD58000, 17>  random_mt11213a;
///为啥名字中要一个B的后缀呢，因为对应有一个A，BOOST只选择了mt11213b.
typedef random_mt<351, 175, 0xFFF80000, 0x0007FFFF, 0xCCAB8EE7, 11, 0xFFFFFFFF, 7, 0x31B6AB00, 15, 0xFFE50000, 17>  random_mt11213b;
///@brief mt19937大概是现在大家最推荐的算法
///这儿要介绍一下的是我们一般默认使用的算法mt19937的算法，
///这种算法速度快，而且循环周期长2^19937，在不那么大（2^623,你有需要创建比这个大的数值？）的数值中平均分布能力好，
///所以是群众们的最爱，所以大家默认选择这算法，
typedef random_mt<624, 397, 0x80000000, 0x7FFFFFFF, 0x9908B0DF, 11, 0xFFFFFFFF, 7, 0x9D2C5680, 15, 0xEFC60000, 18>  random_mt19937;

//==============================================================================================================
//

/*!
* @brief      ranlux 算法 的随机数发生器 This is a lagged fibonacci(斐波纳契) generator with skipping developed by Luescher.
*             其产生的数值的范围是0~2^24 ,其循环长度是 ~10^171
* @tparam     一个循环的常量
*/
template <size_t luxury>
class random_ranlux : public t_random_base<0, 0x00FFFFFF>
{
public:

    ///构造函数
    explicit random_ranlux(uint32_t seed = DEFAULT_SEED) :
        ranlux_i_(0),
        ranlux_j_(0),
        ranlux_n_(0),
        carry_(0)

    {
        srand(seed);
    }
    virtual ~random_ranlux()
    {
    }

    ///种下种子
    virtual void srand(uint32_t seed)
    {
        if (seed == 0)
        {
            /* default seed is 314159265 */
            seed = 314159265;
        }

        // This is the initialization algorithm of F. James, widely in use  for RANLUX.
        for (size_t i = 0; i < 24; i++)
        {
            uint32_t k = seed / 53668;
            seed = 40014 * (seed - k * 53668) - k * 12211;

            if (seed <= 0)
            {
                seed += 2147483563;
            }

            u_[i] = seed % 16777216;
        }

        ranlux_i_ = 23;
        ranlux_j_ = 9;
        ranlux_n_ = 0;

        if (u_[23] & 0XFF000000)
        {
            carry_ = 1;
        }
        else
        {
            carry_ = 0;
        }
    }

    ///得到随机数
    ///你要看算法就知道ranlux的算法其实比前面那些算法慢不少，因为每24次就会计算一次,而且计算量不小
    virtual inline uint32_t rand()
    {
        const size_t  skip = luxury - 24;
        uint32_t r = increment_state();

        ranlux_n_++;

        if (ranlux_n_ == 24)
        {
            ranlux_n_ = 0;

            for (size_t i = 0; i < skip; i++)
            {
                increment_state();
            }
        }

        return r;
    }

    //补充最高位
    virtual uint32_t get_uint32()
    {
        uint32_t x1 = rand();
        uint32_t x2 = rand();
        return (x1 << 24) + x2;
    }

protected:

    //
    uint32_t increment_state()
    {
        int32_t delta = u_[ranlux_j_] - u_[ranlux_i_] - carry_;

        //这儿我不懂，
        if (delta & 0xFF000000)
        {
            carry_ = 1;
            delta &= 0x00FFFFFF;
        }
        else
        {
            carry_ = 0;
        }

        u_[ranlux_i_] = delta;

        if (ranlux_i_ == 0)
        {
            ranlux_i_ = 23;
        }
        else
        {
            --ranlux_i_;
        }

        if (ranlux_j_ == 0)
        {
            ranlux_j_ = 23;
        }
        else
        {
            --ranlux_j_;
        }

        return delta;
    }

protected:
    //反正我都不明白
    uint32_t ranlux_i_;
    uint32_t ranlux_j_;
    uint32_t ranlux_n_;

    uint32_t carry_;
    uint32_t u_[24];
};

///ranlux223 在BOOST中间被称为ranlux3
typedef  random_ranlux<223>  random_ranlux223;
///ranlux389 在BOOST中间被称为ranlux4
typedef  random_ranlux<389>  random_ranlux389;

//=======================================================================================================

///定义了不同随机数发生器的变量产生器，如果没有多线程要求，可以直接使用他们

typedef   zce::Singleton<random_bsdrand>      bsdrand_instance;
typedef   zce::Singleton<random_rand48>       rand48_instance;
typedef   zce::Singleton<random_taus88>       taus88_instance;
typedef   zce::Singleton<random_mt11213a>     mt11213a_instance;
typedef   zce::Singleton<random_mt11213b>     mt11213b_instance;
typedef   zce::Singleton<random_mt19937>      mt19937_instance;
typedef   zce::Singleton<random_ranlux223>    ranlux223_instance;
typedef   zce::Singleton<random_ranlux389>    ranlux389_instance;

//=======================================================================================================

/*!
* @brief      好吧，搞一个针对[输出uint32_t的随机数发生器]变量产生器，里面包括各种分布函数。
* @tparam     uin32_random 随机数发生器
* @note       这儿还是要提一下BOOST的实现，BOOST的实现分成，随机数发生器，分布器，变量发生器3个对象
*             从某种角度上讲他的实现更加精巧，但有一个问题是，如果最终使用的是变量发生器，那么你需要在
*             每个地方需要的地方都放置一个变量发生器
*             我的方法是在变量发生器里面使用了一些分布函数。
*             还有一个不同是BOOST的分布器是根据随机数算法的最大值，最小值通过运算得到分布，比如随机数发生器
*             产生的是32bit的数值，最大值是0x00FFFFFF，要产生一个64bit的小于0xFFFFFFFFFF的数值，BOOST的
*             算法大约是发现最大值0xFFFFFFFFFF是随机数发生器最大值的N倍，N也用随机数产生，然后和一个随机数运
*             算云云，大约如此
*             我的方法是在每个随机数发生器里面都提供一个产生uint32的函数get_uint32，然后根据这个去得到其他分布，
*/
template <typename uin32_random>
class random_var_gen
{
public:

    ///构造函数，并不完全让你使用单子，还是给你机会能创建实例
    random_var_gen()
    {
    }
    ///析构函数
    ~random_var_gen()
    {
    }

public:
    ///给随机数发生器的种子
    void srand(uint32_t seed)
    {
        uin32_rand_gen_.srand(seed);
    }

    ///在int32数值范围给产生一个随机数变量
    inline int32_t uniform_int32(int32_t min, int32_t max)
    {
        ZCE_ASSERT(max > min);
        uint32_t u32_max = max - min;
        uint32_t u32_rand = uniform_uint32(0, u32_max);
        return min + u32_rand;
    }
    ///在 uint32数值范围给产生一个随机数变量
    inline uint32_t uniform_uint32(int64_t min, int64_t max)
    {
        ZCE_ASSERT(max > min);
        uint32_t u32_rand = uin32_rand_gen_.get_uint32();
        return (u32_rand + min) % max;
    }

    ///在 int64数值范围给产生一个随机数变量
    inline int64_t uniform_int64(int64_t min, int64_t max)
    {
        ZCE_ASSERT(max > min);
        uint64_t u64_max = max - min;
        uint64_t u64_rand = uniform_uint64(0, u64_max);
        return min + u64_rand;
    }
    ///在 uint64数值范围给产生一个随机数变量
    inline uint64_t uniform_uint64(uint64_t min, uint64_t max)
    {
        ZCE_ASSERT(max > min);
        uint64_t u64_rand = uin32_rand_gen_.get_uint64();
        return (u64_rand + min) % max;
    }

    //
    inline double uniform_01()
    {
        return uin32_rand_gen_.get_double01();
    }

protected:

    ///随机发生器
    uin32_random                         uin32_rand_gen_;
};

///定义了不同随机数发生器的变量产生器，如果没有多线程要求，可以直接使用他们
///推荐mt19937_var_gen
typedef   random_var_gen<random_bsdrand>      bsdrand_var_gen;
typedef   random_var_gen<random_rand48>       rand48_var_gen;
typedef   random_var_gen<random_taus88>       taus88_var_gen;
typedef   random_var_gen<random_mt11213a>     mt11213a_var_gen;
typedef   random_var_gen<random_mt11213b>     mt11213b_var_gen;
typedef   random_var_gen<random_mt19937>      mt19937_var_gen;
typedef   random_var_gen<random_ranlux223>    ranlux223_var_gen;
typedef   random_var_gen<random_ranlux389>    ranlux389_var_gen;

typedef   zce::Singleton<bsdrand_var_gen>      bsdrand_vargen_inst;
typedef   zce::Singleton<rand48_var_gen>       rand48_vargen_inst;
typedef   zce::Singleton<taus88_var_gen>       taus88_vargen_inst;
typedef   zce::Singleton<mt11213a_var_gen>     mt11213a_vargen_inst;
typedef   zce::Singleton<mt11213b_var_gen>     mt11213b_vargen_inst;
typedef   zce::Singleton<mt19937_var_gen>      mt19937_vargen_inst;
typedef   zce::Singleton<ranlux223_var_gen>    ranlux223_vargen_inst;
typedef   zce::Singleton<ranlux389_var_gen>    ranlux389_vargen_inst;
};// end of namespace    zce

#endif
