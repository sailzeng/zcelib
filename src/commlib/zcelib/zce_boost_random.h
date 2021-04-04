/*!
* @filename   zce_boost_random.h
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��10��1��
* @brief      ���ɸ�������������࣬�����������������
*             �˴�����Ϊboost����ʵ��boost����û�ж���ϵ��ֻ��boost
*             �����ƵĴ��롣C++11����Ҳ����صĵĿ��ˣ�������ҪĿ�����ź�
*             ����Ϸ���������Ի��Ǳ�����������롣
* @details
* ΪʲôҪ�Լ�дrand�Ĵ��룬��Ϊ2��ԭ��
* 1.ϵͳ��rand��̫�ã��������ǵ�ѭ�����ȼ���һ����2^32,����WIN��randomֵ
* ����С��65535����ֵ������̼ǵ�һ�β����������ģ�����HASH�����ӣ������
* ͻ������
* 2.�������е�rand����α�����α�����ʲô��˼���������ò�����һ�����ݿ�������
* ����һ�����������ݣ�����Ϊ�˰�ȫ���ǲ������ǰɡ�
* �������������������һ���绰�����룬����㲻��һЩ�仯����ô����һ�ſ����ˣ�
* ������������������е绰�����룩
*  �����������Ҿ��û����Լ�ʵ��һ���ȽϺã��ر�����Ϸ����Ҫ�д�����������ĵط�
*
* ��������㷨�����ֳɼ���
* @li ����ͬ�� Ч��һ��
* @li ƽ��ȡ�� Ч����һ��
* @li ��ѧ�������������㷨�����������Щ�㷨�ļ��ϻ��ܣ�
*
* ������������㷨���⣬������ѧ���ã��������Ǵ�BOOST�м�֪�������֣�Ȼ���
* ���������㷨��
* ������BOOST�Ĵ�������ȡ����������Ҫ��̫���������ģ���ж�̫��������㣬
* ���ˣ�����ʵ�ҵĴ������Ϊ�˼��Ҳ���˲���ģ�棩
*
* �������һЩ�ο�ʵ�ִ���
* GNU Scientific Library (GSL) ����Ҳ�кܶ�������ĺ���������Ȥ���Կ�����
* �ҹ���BOOST�Ĵ���Ҳ���������GNU���Ǻ���ѽ
* http://www.koders.com/c/fid694A621D3301A5177FEF0181F19D64D75CDC9CCC.aspx?s=ranlux#L1
* ���Ҳ����������ʵ��
* http://www.koders.com/cpp/fid63AE02B1E8D46B18CBAC4338C47735991DBCBFC0.aspx?s=Chebyshev
*
* ����ʹ�õĲ�����������㷨�������¼�����ͬʱ����BOOST���������ǵ��㷨���ٶ��Լ�ѭ�����ڶԱ�
* �㷨                      ѭ������       �������ֵķ�Χ         BOOST�������㷨ʱ���     �ҵ�ʵ�ָ������㷨ʱ���
*                   x                                          ��100%��죩��           ����1����ʱ ÿs��������     ��bsdrand�Ƚ�   �ͣ��ڶ����� mt19937�Ƚ�
* VC++Rand                               2^16                 ��                      2.304s    43402777.7    10.67%         18.57%
* bsdrand                  2^31          (uint32)0 - 2^31     ��                      0.216s    462962962.6   100%           198.14%
* rand48                   2^48          (uint32)0 - 2^32     59%                     0.681s    146842878.1   31.71%         62.84%
* taus88                   ~2^88         (uint32)0 - 2^32     100%                    0.4610    216919739.6   46.85%         92.84
* mt19937                  2^19937       (uint32)0 - 2^32     81%                     0.4280    233644859.8   50.46%         100%
* mt11231a                 2^11231       (uint32)0 - 2^32     81%                     0.4290    233100233.1   50.34%         99.76%
* mt11231b                 2^11231       (uint32)0 - 2^32     81%                     0.4330    230946882.2   49.88%         98.84%
* ranlux223(ranlux3)       ~10^171       (uint32)0 - 2^24     6%                      8.8760    1126636.1     0.24%          0.48%
* ranlux389(ranlux4)       ~10^171       (uint32)0 - 2^24     3%                      15.582    641761.4      0.13%          0.27%
*
* ����ѭ�������ҵ�����ǣ�����㷨����һ��0x1111����ֵ���ڶ�������0x2222,���ѭ��������2^31����ô2^31��֮��
* ���ͻ��ٴγ���0x1111��0x2222������ֵ��
*
* ����˵��rand48��BOOST������㷨˵���������ַ�Χ��0-2^31,����GSL�����˵����0-2^32,���������㷨��̫һ��
* �ҵĲ��������ǲ���1�ڸ�������������ʱ���Լ�ÿ����Բ���������,���Ի���i5,Win7 64 ,VC++2010,realse
*
* �ο�BOOST��˵����
* http://www.boost.org/doc/libs/1_48_0/doc/html/boost_random/performance.html
*
* ����BOOST����������Ѿ�������tr1��C++1x�ı�׼�����Բ��õĽ�����Ҳ��5�꣩�����ǾͿ���ֱ��ʹ������ˣ�
* ���ߣ�BOOST����������ʵ�ֹ��ڡ����������о�Υ���˾����򵥵�ԭ�������3�����֣���������������ֲ�������������������ķ�Χ����
* ��������������װ��,��Ȼ����ƵĽǶȣ������ķ�װ����������ѧϰ�ɱ������ƴ�ҵ�ʹ�õġ�������
*
* ��������������BOOST��ʵ���϶�BOOST����н���ͼ򻯣��Ҳ�������̫���ӣ�����Ҫ��������ֲ���ԼҲ����uint16_t,uint32_t,double,01,��ƽ���ֲ�
* �Ҳ�����̫���ӣ��Ҵ������2�����֣����������������������������������ݶ���uint32_t
* ����������(�����װ�˸���ƽ���ֲ��ĺ���),��Ĭ�ϰ�װһ�������������������ʹ�ñ�����������typedef��OK�ˡ�
*
* @note
* ���������д�����ħ�����֣�������Ҫ�����ַ�����Һ�����һ��������Щ���ֵĺ����ǰ׳�
* �������һ�£���Щ�����Ƿ����㷨ʵ�������ģ�������һ���Ŀ�ѧ�����ģ�
*/
#ifndef ZCE_LIB_BOOST_RANDOM_H_
#define ZCE_LIB_BOOST_RANDOM_H_

#include "zce_log_logging.h"
#include "zce_boost_singleton.h"

namespace zce
{

//==============================================================================================================

/*!
* @brief      ������Ļ��࣬����һЩ�������������û�����������һЩ�����̳��߾Ͳ���������
* @note       ע�����ǵ����������randֻ���������������uint32_t��
*/
class random_base
{
public:

    ///���������������
    enum RAND_STRING_TYPE
    {
        ///���������ַ������ַ�0-9
        RAND_STR_NUMBER,
        ///����Сд�ַ������ַ�a-z
        RAND_STR_LOWER,
        ///������д�ַ������ַ�A-Z
        RAND_STR_UPPER,
        ///��������+Сд�ַ������ַ�0-9��a-z
        RAND_STR_NUMBER_LOWER,
        ///�������ִ�д�ַ������ַ�0-9��A-Z
        RAND_STR_NUMBER_UPPER,
        ///��������+Сд+��д�ַ������ַ�0-9��a-z,A-Z
        RAND_STR_NUMBER_LOWER_UPPER,
        ///����0-127��ASCII(����չ)�ַ���
        RAND_STR_ASCII,
        ///����0-255�������ַ���
        RAND_STR_BINARY,
    };

    //���캯��������������������������д��
protected:
    ///���캯�����ܲ���ʵ������Ϊ�������ã�ֻ���ø�λ�̳к���������
    random_base()
    {
    }
public:
    virtual ~random_base()
    {
    }

public:

    /*!
    @brief      �������ɸ����������Ϊ�˷�ֹ�Է��²�ʱ������ʹ��
    @param      discard_num �����Ĵ���
    */
    virtual void discard(size_t discard_num)
    {
        for (size_t i = 0; i < discard_num; ++i)
        {
            rand();
        }
    }

    ///@brief  �����ʵ�ֵĺ���,���������,
    ///@note   ע��rand�������������32λ�ģ��õ�������ֵ���͵�������������Ǹ���32λȥ����
    virtual uint32_t rand() = 0;
    ///�����ʵ�ֵĺ���,��δ������������
    virtual void srand(uint32_t seed) = 0;

    ///����һ��0~0xFFFFFFFF�������ֵ��һЩ��������������ķ�Χ��������������Χ��
    ///������������Ҫ����ת��
    inline virtual uint32_t get_uint32()
    {
        return rand();
    }

    ///�õ�һ��64bit������,����������uin32_t�ϲ�,
    ///��Щ�㷨д�ıȽϼ򵥣���Զ��ԣ�Boost���ⷽ��Ĵ���Ҫ���Ӻܶ࣬��һ����Ҫ����ͨ���ԣ�һ���桭��
    ///�ҵ���֪�������ģ�������������ķ������Ĳ������������㹻����ģ���ôӦ���������ǵ�����
    inline virtual uint64_t get_uint64()
    {
        uint64_t x1 = get_uint32();
        uint64_t x2 = get_uint32();
        return (x1 << 32) + x2;
    }

    ///�õ�һ��double�����,��ʵ���Ǹ���32bitƴ��һ��double������C����д������BOOST��ʵ����Ը��Ӻܶ�
    inline virtual double get_double()
    {
        double x = 0.0;
        uint64_t x_64 = get_uint64();
        //��TMD�����Ǹ�C���Ե�д����������ÿһλ�����㹻����ģ���ô����������
        memcpy(&x, &x_64, sizeof(double));
        return x;
    }

    ///�õ�[0-1)֮�両����������ĳЩ���ʿ���
    inline virtual double get_double01()
    {
        return double (get_uint32()) / (double( (std::numeric_limits<uint32_t>::max)() ) + 1);
    }

    /*!
    * @brief      ����Ҫ������һ��������ַ�����
    * @param[in]  str_type  ������ַ��������ͣ�@ref random_base::RAND_STRING_TYPE
    * @param[out] rand_str  ���ص�����������ַ���
    * @param[in]  str_len   Ҫ�����ɵ��ַ����ĳ���
    */
    virtual void get_string(RAND_STRING_TYPE str_type, char *rand_str, size_t str_len);

protected:

    ///Ĭ�ϵ�����,�㿴�����׵ģ�д�����������Ϊ�˼����ⳡ�˶����������һ����˼��
    ///�����й�������ʲô����ų���������˼�ľ������⣬ʲô��û�иı䣬����������ȴ��Ϊ��
    ///����������Բ�ˬ��׳��ҩ��
    ///������˶�������Ķ���������������Ӧ�ý�����������⣬�������������������⣬�ڿ�
    ///100���Ľ�����Ȼû�н����
    ///���ǻ���ͼ���5.4����Ӧ��ֻ��Ϊ����ĳЩ�µĿ��ˣ���Ӧ��˼���������������ۣ�������
    static const uint32_t DEFAULT_SEED = 19190504UL;
};

/*!
* @brief      ��������������棬��¼���ֵ����Сֵ��ע�ⲻͬ��������㷨��������������з�Χ��
* @tparam     min_result rand�������������������Сֵ
* @tparam     max_result rand��������������������ֵ
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

    ///������С�ģ��Ľ��ֵ
    static const uint32_t MIN_RESULT = min_result;
    static const uint32_t MAX_RESULT = max_result;
};

//==============================================================================================================

/*!
* @brief      BSD��RAND����,��100%������ͬ��
*/
class random_bsdrand : public t_random_base<0, 0x7FFFFFFF>
{

public:
    ///���캯��
    explicit random_bsdrand(uint32_t seed = DEFAULT_SEED):
        seed_x_(0)
    {
        srand(seed);
    }
    ///��������
    virtual ~random_bsdrand()
    {
    }
    ///���������
    virtual uint32_t rand()
    {
        seed_x_ = (1103515245 * seed_x_ + 12345) & 0x7FFFFFFF;
        return seed_x_;
    }

    ///�õ�����
    virtual void srand(uint32_t s)
    {
        seed_x_ = s;
    }

    ///BSD���㷨���ֵ��0x7FFFFFFF,���Բ������λ
    virtual uint32_t get_uint32()
    {
        uint32_t x = rand();
        uint32_t highest_bit = ((x & 0x2 ) ? 1 : 0) ^ ((x & 0x4 )) ? 1 : 0;

        if (highest_bit)
        {
            x = 0x80000000 | x;
        }

        return x;
    }

protected:
    ///��¼��������������ݣ�
    uint32_t seed_x_;
};

//==============================================================================================================

/*!
* @brief      rand48 �㷨����������������ѭ��������2^48,�ʴ��д����֣�
*/
class random_rand48 : public t_random_base<0, 0xFFFFFFFF>
{
public:
    ///���캯��
    explicit random_rand48(uint32_t seed = DEFAULT_SEED):
        rand48_x0_(0),
        rand48_x1_(0),
        rand48_x2_(0)
    {
        srand(seed);
    }
    ///��������
    virtual ~random_rand48()
    {
    }

    ///���������
    virtual uint32_t rand()
    {
        rand48_advance () ;

        uint32_t x2 = rand48_x2_;
        uint32_t x1 = rand48_x1_;
        return (x2 << 16) + x1;
    }

    ///����õ�����
    virtual void srand (uint32_t s)
    {
        if (s == 0)
        {
            rand48_x0_ = 0x330E ;
            rand48_x1_ = 0xABCD ;
            rand48_x2_ = 0x1234 ;
        }
        else
        {
            rand48_x0_ = 0x330E ;
            rand48_x1_ = static_cast<uint16_t>(s & 0xFFFF );
            rand48_x2_ = static_cast<uint16_t>((s >> 16) & 0xFFFF );
        }

        return;
    }

protected:
    //
    inline void rand48_advance ()
    {

        /* work with unsigned long ints throughout to get correct integer promotions of any unsigned short ints */
        const uint32_t x0 = rand48_x0_;
        const uint32_t x1 = rand48_x1_;
        const uint32_t x2 = rand48_x2_;

        uint32_t a = RAND48_A0 * x0 + RAND48_C0 ;
        rand48_x0_ = static_cast<uint16_t>(a & 0xFFFF) ;

        a >>= 16 ;

        /* although the next line may overflow we only need the top 16 bits in the following stage, so it does not matter */

        a += RAND48_A0 * x1 + RAND48_A1 * x0 ;
        rand48_x1_ = static_cast<uint16_t>(a & 0xFFFF) ;

        a >>= 16 ;
        a += RAND48_A0 * x2 + RAND48_A1 * x1 + RAND48_A2 * x0 ;
        rand48_x2_ = static_cast<uint16_t>(a & 0xFFFF) ;
    }

protected:
    //ħ�����ִ󼯺�
    static const uint16_t RAND48_A0 = 0xE66D ;
    static const uint16_t RAND48_A1 = 0xDEEC ;
    static const uint16_t RAND48_A2 = 0x0005 ;
    static const uint16_t RAND48_C0 = 0x000B ;

protected:
    ///����rand48�ļ�������
    uint16_t rand48_x0_, rand48_x1_, rand48_x2_;

};

//==============================================================================================================

//��һ���ڲ��꣬�ⲿ��Ҫʹ�ã�����.h�ļ���ȫ����Ϊ�Ҳ�����.cpp�鷳��
#ifndef __ZCE_TAUSWORTHE
#define __ZCE_TAUSWORTHE(arg,stage1,stage2,stage3,limit) ((arg&limit)<<stage1) ^ (((arg <<stage2) ^ arg)>>stage3)
#endif

/*!
@brief      taus88 ��������Ӧ����Tausworthe����Ϊ��ѭ��������~2^88���д����֣�
            ��ο��ĵ���
            "Maximally Equidistributed Combined Tausworthe Generators", Pierre L'Ecuyer, Mathematics of Computation, Volume 65, Number 213, January 1996, Pages 203-213
*/
class random_taus88 : public t_random_base<0, 0xFFFFFFFF>
{

public:
    //���캯�����ǰ�Ĭ�����Ӹ������
    explicit random_taus88(uint32_t seed):
        seed_arg_{0}
    {
        srand(seed);
    }
    virtual ~random_taus88()
    {
    }

public:

    ///�������ӵĺ���
    virtual void srand(uint32_t seed)
    {
        if (seed == 0)
        {
            seed = static_cast< uint32_t >( time(NULL) );
        }

        seed_arg_[0] = 69069 * seed;
        seed_arg_[1] = 69069 * seed_arg_[0];
        seed_arg_[2] = 69069 * seed_arg_[1];

        //Tausworthe�Բ�����Ҫ��
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

    ///�õ����������
    virtual inline uint32_t rand()
    {
        seed_arg_[0] = __ZCE_TAUSWORTHE(seed_arg_[0], 12, 13, 19, 4294967294UL);
        seed_arg_[1] = __ZCE_TAUSWORTHE(seed_arg_[1], 4, 2, 25, 4294967288UL);
        seed_arg_[2] = __ZCE_TAUSWORTHE(seed_arg_[2], 17, 3, 11, 4294967280UL);
        return (seed_arg_[0] ^ seed_arg_[1] ^ seed_arg_[2]);
    }

protected:

    //__RANDOM_TAUSWORTHE���൱��
    uint32_t tausworthe(uint32_t arg, uint32_t stage1, uint32_t stage2, uint32_t stage3, uint32_t limit)
    {
        return ((arg & limit) << stage1) ^ (((arg << stage2) ^ arg) >> stage3);
    }

protected:

    //taus88����Ҫ��3������,��������Щ�㷨Ϊɶ���������Ҳ�����ѧϵ������
    uint32_t seed_arg_[3];
};

//==============================================================================================================

#ifndef __ZCE_MT_TWIST
#define __ZCE_MT_TWIST(u,v,a,b,c) (( ( ((u) & (a)) | ((v) & (b) )) >> 1) ^ ((v)&0x1 ? (c) : 0))
#endif

/*!
* @brief      MT�㷨�������������
*             Mersenne Twister÷ɭ��ת�㷨�����治���ڴ�Ҷ����յ�ʱ���������ң�����㷨��ȷ���ձ��˷����ġ�
*             ���һ���Ŀǰ���Կ������������㷨����ӭ�����������������ҳ��������õ��㷨��
*             Makoto Matsumoto���ɱ��棩 ��Takuji Nishimura��������ʿ����1997�꿪���ģ�
*             �������޶������ֶ��ϵľ������Եݹ�field F_{2}�� ���Կ��ٲ�����������α������� �����˹ŵ�����������㷨�ĺܶ�ȱ�ݡ�
*             http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/emt19937ar.html
*
* @tparam     mt_n  ����һЩ�㷨����,�����ּ���һ�Σ�Ҳ�Ǳ������������ĳ��ȣ�
* @tparam     mt_m  ����һЩ�㷨����,����ѭ������
* @tparam     mt_a  ����һЩ�㷨������������Щ���������mt19937���㷨
* @tparam     mt_b  �㷨����
* @tparam     mt_c  �㷨����
* @tparam     mt_o  �㷨����
* @tparam     mt_p  �㷨����
* @tparam     mt_q  �㷨����
* @tparam     mt_r  �㷨����
* @tparam     mt_s  �㷨����
* @tparam     mt_t  ����һЩ�㷨����
* @tparam     mt_u  ��TNND��һЩ�㷨����
* @note       ֵ��һ�������Щ�ձ����ָ���һ�����ٵİ汾��SFMT SIMD-oriented Fast Mersenne Twister SFMT
*             ����SSE2�Ĺ����ṩ���ٶȣ��Ҷ�SSE2�Ĵ���Ŀ���ֲ��û�а��գ���ʱ����
*/
template < size_t mt_n, size_t mt_m, uint32_t mt_a, uint32_t mt_b, uint32_t mt_c,
           uint32_t mt_o, uint32_t mt_p, uint32_t mt_q, uint32_t mt_r, uint32_t mt_s, uint32_t mt_t, uint32_t mt_u >
class random_mt : public t_random_base<0, 0xFFFFFFFF>
{

public:

    explicit random_mt(uint32_t seed = DEFAULT_SEED):
        state_{0},
        postion_(0)
    {
        srand(seed);
    }
    virtual ~random_mt()
    {
    }

    ///�õ������
    virtual void srand(uint32_t seed)
    {

        state_[0] = seed ;

        for (size_t j = 1; j < mt_n; j++)
        {
            state_[j] = (1812433253 * (state_[j - 1] ^ (state_[j - 1] >> 30)) + static_cast<uint32_t>(j));
        }

        //Ϊ���ڵ�һ�ξʹ���gen_state
        postion_ = mt_n;
    }

    ///�õ����������ʵMT���㷨�ǳ��ǳ���,��ֻ��mt_n�ֺ�ż���һ�Σ�����ṹ��������������
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

        x ^= (x >> mt_o)&mt_p;
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

    //state��������ÿ�����ɵ������
    uint32_t     state_[mt_n];
    //��¼ÿ��ȡ�õ��������λ��
    size_t       postion_;

};

//�������ɣ���ʵ��Ҳ��������mt[����]�������������ʵ��˵ѭ��������2^����
///��׺ΪA��������ħ���������Ҵ����������� http://www.quadibloc.com/crypto/co4814.htm
typedef random_mt<351, 175, 0xFFF80000, 0x0007FFFF, 0xE4BD75F5, 11, 0xFFFFFFFF, 7, 0x655E5280, 15, 0xFFD58000, 17>  random_mt11213a;
///Ϊɶ������Ҫһ��B�ĺ�׺�أ���Ϊ��Ӧ��һ��A��BOOSTֻѡ����mt11213b.
typedef random_mt<351, 175, 0xFFF80000, 0x0007FFFF, 0xCCAB8EE7, 11, 0xFFFFFFFF, 7, 0x31B6AB00, 15, 0xFFE50000, 17>  random_mt11213b;
///@brief mt19937��������ڴ�����Ƽ����㷨
///���Ҫ����һ�µ�������һ��Ĭ��ʹ�õ��㷨mt19937���㷨��
///�����㷨�ٶȿ죬����ѭ�����ڳ�2^19937���ڲ���ô��2^623,������Ҫ��������������ֵ��������ֵ��ƽ���ֲ������ã�
///������Ⱥ���ǵ�������Դ��Ĭ��ѡ�����㷨��
typedef random_mt<624, 397, 0x80000000, 0x7FFFFFFF, 0x9908B0DF, 11, 0xFFFFFFFF, 7, 0x9D2C5680, 15, 0xEFC60000, 18>  random_mt19937;

//==============================================================================================================
//

/*!
* @brief      ranlux �㷨 ������������� This is a lagged fibonacci(쳲�����) generator with skipping developed by Luescher.
*             ���������ֵ�ķ�Χ��0~2^24 ,��ѭ�������� ~10^171
* @tparam     һ��ѭ���ĳ���
*/
template <size_t luxury>
class random_ranlux : public t_random_base<0, 0x00FFFFFF>
{
public:

    ///���캯��
    explicit random_ranlux(uint32_t seed = DEFAULT_SEED):
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

    ///��������
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

    ///�õ������
    ///��Ҫ���㷨��֪��ranlux���㷨��ʵ��ǰ����Щ�㷨�����٣���Ϊÿ24�ξͻ����һ��,���Ҽ�������С
    virtual inline uint32_t rand()
    {
        const size_t  skip = luxury - 24;
        uint32_t r = increment_state ();

        ranlux_n_++;

        if (ranlux_n_ == 24)
        {
            ranlux_n_ = 0;

            for (size_t i = 0; i < skip; i++)
            {
                increment_state ();
            }
        }

        return r;
    }

    //�������λ
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

        //����Ҳ�����
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
    //�����Ҷ�������
    uint32_t ranlux_i_;
    uint32_t ranlux_j_;
    uint32_t ranlux_n_;

    uint32_t carry_;
    uint32_t u_[24];
};

///ranlux223 ��BOOST�м䱻��Ϊranlux3
typedef  random_ranlux<223>  random_ranlux223;
///ranlux389 ��BOOST�м䱻��Ϊranlux4
typedef  random_ranlux<389>  random_ranlux389;

//=======================================================================================================

///�����˲�ͬ������������ı��������������û�ж��߳�Ҫ�󣬿���ֱ��ʹ������

typedef   ZCE_Singleton<random_bsdrand>      bsdrand_instance;
typedef   ZCE_Singleton<random_rand48>       rand48_instance;
typedef   ZCE_Singleton<random_taus88>       taus88_instance;
typedef   ZCE_Singleton<random_mt11213a>     mt11213a_instance;
typedef   ZCE_Singleton<random_mt11213b>     mt11213b_instance;
typedef   ZCE_Singleton<random_mt19937>      mt19937_instance;
typedef   ZCE_Singleton<random_ranlux223>    ranlux223_instance;
typedef   ZCE_Singleton<random_ranlux389>    ranlux389_instance;

//=======================================================================================================

/*!
* @brief      �ðɣ���һ�����[���uint32_t�������������]����������������������ֲַ�������
* @tparam     uin32_random �����������
* @note       �������Ҫ��һ��BOOST��ʵ�֣�BOOST��ʵ�ֳַɣ���������������ֲ���������������3������
*             ��ĳ�ֽǶ��Ͻ�����ʵ�ָ��Ӿ��ɣ�����һ�������ǣ��������ʹ�õ��Ǳ�������������ô����Ҫ��
*             ÿ���ط���Ҫ�ĵط�������һ������������
*             �ҵķ������ڱ�������������ʹ����һЩ�ֲ�������
*             ����һ����ͬ��BOOST�ķֲ����Ǹ���������㷨�����ֵ����Сֵͨ������õ��ֲ������������������
*             ��������32bit����ֵ�����ֵ��0x00FFFFFF��Ҫ����һ��64bit��С��0xFFFFFFFFFF����ֵ��BOOST��
*             �㷨��Լ�Ƿ������ֵ0xFFFFFFFFFF����������������ֵ��N����NҲ�������������Ȼ���һ���������
*             �����ƣ���Լ���
*             �ҵķ�������ÿ����������������涼�ṩһ������uint32�ĺ���get_uint32��Ȼ��������ȥ�õ������ֲ���
*/
template <typename uin32_random>
class random_var_gen
{

public:

    ///���캯����������ȫ����ʹ�õ��ӣ����Ǹ�������ܴ���ʵ��
    random_var_gen()
    {
    }
    ///��������
    ~random_var_gen()
    {
    }

public:
    ///�������������������
    void srand(uint32_t seed)
    {
        uin32_rand_gen_.srand(seed);
    }

    ///��int32��ֵ��Χ������һ�����������
    inline int32_t uniform_int32(int32_t min, int32_t max)
    {
        ZCE_ASSERT( max > min);
        uint32_t u32_max = max - min;
        uint32_t u32_rand = uniform_uint32(0, u32_max);
        return min + u32_rand;
    }
    ///�� uint32��ֵ��Χ������һ�����������
    inline uint32_t uniform_uint32(int64_t min, int64_t max)
    {
        ZCE_ASSERT( max > min);
        uint32_t u32_rand = uin32_rand_gen_.get_uint32();
        return (u32_rand + min ) % max;
    }

    ///�� int64��ֵ��Χ������һ�����������
    inline int64_t uniform_int64(int64_t min, int64_t max)
    {
        ZCE_ASSERT( max > min);
        uint64_t u64_max = max - min;
        uint64_t u64_rand = uniform_uint64(0, u64_max);
        return min + u64_rand;
    }
    ///�� uint64��ֵ��Χ������һ�����������
    inline uint64_t uniform_uint64(uint64_t min, uint64_t max)
    {
        ZCE_ASSERT( max > min);
        uint64_t u64_rand = uin32_rand_gen_.get_uint64();
        return (u64_rand + min ) % max;
    }

    //
    inline double uniform_01()
    {
        return uin32_rand_gen_.get_double01();
    }

protected:

    ///���������
    uin32_random                         uin32_rand_gen_;

};

///�����˲�ͬ������������ı��������������û�ж��߳�Ҫ�󣬿���ֱ��ʹ������
///�Ƽ�mt19937_var_gen
typedef   random_var_gen<random_bsdrand>      bsdrand_var_gen;
typedef   random_var_gen<random_rand48>       rand48_var_gen;
typedef   random_var_gen<random_taus88>       taus88_var_gen;
typedef   random_var_gen<random_mt11213a>     mt11213a_var_gen;
typedef   random_var_gen<random_mt11213b>     mt11213b_var_gen;
typedef   random_var_gen<random_mt19937>      mt19937_var_gen;
typedef   random_var_gen<random_ranlux223>    ranlux223_var_gen;
typedef   random_var_gen<random_ranlux389>    ranlux389_var_gen;

typedef   ZCE_Singleton<bsdrand_var_gen>      bsdrand_vargen_inst;
typedef   ZCE_Singleton<rand48_var_gen>       rand48_vargen_inst;
typedef   ZCE_Singleton<taus88_var_gen>       taus88_vargen_inst;
typedef   ZCE_Singleton<mt11213a_var_gen>     mt11213a_vargen_inst;
typedef   ZCE_Singleton<mt11213b_var_gen>     mt11213b_vargen_inst;
typedef   ZCE_Singleton<mt19937_var_gen>      mt19937_vargen_inst;
typedef   ZCE_Singleton<ranlux223_var_gen>    ranlux223_vargen_inst;
typedef   ZCE_Singleton<ranlux389_var_gen>    ranlux389_vargen_inst;

};// end of namespace    zce

#endif

