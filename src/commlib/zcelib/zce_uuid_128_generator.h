/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_uuid_128_generator.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年2月4日
* @brief      128为的UUID，真正的UUID，
*             Universally Unique Identifier 缩写UUID，
* 
* @details    有2种产生方法，
*             随机数的方法
*             基数的方式，（你用其他基数也可以，不用时间基数）也可以。
* 
* 出门
* 词：卡夫卡 曲：尹吾
* 
* 我吩咐把我的马儿从马棚里牵出来。
* 仆人没有听懂我的话，我便自己走到马棚，
* 给马备好鞍，骑了上去。
* 远处传来了号角声，我问他，这是什么意思。
* 他说不知道，他什么也没听到，什么也没听到。
* 
* 在大门口，他叫住我，
* 问：“您骑马上哪儿去呢，我的主人？”
* “我不知道，”我说，“只是离开这儿，离开这儿。
* 离开这儿向前走，向前走，这就是我达到目标的唯一办法。”
* “那么您知道您的目标了？”他问。
* “是的”我回答，
* “我刚刚告诉你了，离开这儿，离开这儿，这就是我的目标。”
* “您还没有带上口粮呢，”他说。“什么口粮我也不要。”我说，
* “旅途是那么的漫长啊，如果一路上我得不到东西，
* 那我一定会，死的。
* 
* 什么口粮也不能搭救我，
* 幸运的是，这可是一次，真正没有尽头的旅程啊！”
* 
*/
#ifndef ZCE_LIB_UUID_128_GENERATOR_H_
#define ZCE_LIB_UUID_128_GENERATOR_H_

#include "zce_boost_random.h"

/************************************************************************************************************
Class           : ZCE_UUID128
************************************************************************************************************/
///32bit整数+32整数+64位整数的表示方法
struct ZCELIB_EXPORT ZCE_UUID128_32_32_64
{

    //
    uint32_t       data1_;
    uint32_t       data2_;
    uint64_t       data3_;
};

///标准的UUID的格式
struct ZCELIB_EXPORT ZCE_UUID128_32_16_16_16_48
{
    uint64_t       data1_: 32;
    uint64_t       data2_: 16;
    uint64_t       data3_: 16;
    uint64_t       data4_: 16;
    uint64_t       data5_: 48;
};

///微软的GUID的格式
struct ZCELIB_EXPORT ZCE_UUID128_32_16_16_64
{
    uint64_t       data1_: 32;
    uint64_t       data2_: 16;
    uint64_t       data3_: 16;
    uint64_t       data4_;

};

/*!
* @brief      UUID128，128位的唯一标识，
*             有5中表示方法，
*             @li 16个字节
*             @li 4个32为的整数
*             @li 32位整数+32位整数+64位整数的表示方法
*             @li 32位整数+16位整数+16位整数+16位整数+48位整数，标准的UUID表示方法
*             @li 32位整数+16位整数+16位整数+48位整数，微软的UUID表示方法，GUID，
* 
*/
class ZCELIB_EXPORT ZCE_UUID128
{
public:
    ///构造函数
    ZCE_UUID128();
    ///析构函数
    ~ZCE_UUID128();

    /// < 运算符重载
    bool operator < (const ZCE_UUID128 &others) const;
    /// == 运算符
    bool operator == (const ZCE_UUID128 &others) const;

    /// 以UUID8-4-4-4-12的格式进行转换为字符串
    const char *to_string(char *buffer, size_t buf_len) const;

    /// 转换为字符串，同时返回输出字符串的长度
    inline const char *to_string(char *buffer, size_t buf_len, size_t &use_buf) const
    {
        const char *ret_ptr = to_string(buffer, buf_len);
        if (ret_ptr)
        {
            use_buf = LEN_OF_ZCE_UUID128_STR;
        }
        else
        {
            use_buf = 0;
        }
        return 0;
    }

public:

    ///5种数据表示方法
    union
    {
        ///16个字节的组成
        uint8_t                      u_16uint8_[16];
        ///4个32为字节的组成
        uint32_t                     u_4uint32_[4];
        ///32位整数+32位整数+64位整数
        ZCE_UUID128_32_32_64         u_32_32_64_;
        ///标准的UUID的标识方法
        ZCE_UUID128_32_16_16_16_48   u_32_16_16_16_48_;
        ///微软的GUID的标识方法
        ZCE_UUID128_32_16_16_64      u_32_16_16_64_;

    };

public:
    ///UUID的字符串表示的长度
    static const size_t LEN_OF_ZCE_UUID128_STR = 36;
};

/************************************************************************************************************
Class           : ZCE_UUID128_Generator UUID的发生器
************************************************************************************************************/

/*!
* @brief      ZCE_UUID128_Generator UUID的发生器
* 
*/
class ZCELIB_EXPORT ZCE_UUID128_Generator
{
    //
    enum UUID128_GENERATOR_TYPE
    {
        //
        UUID128_GENERATOR_INVALID,

        ///用随机数的方法产生
        UUID128_GENERATOR_RANDOM,
        ///用事件作为基数触发
        UUID128_GENERATOR_TIME,
    };

public:

    ///构造函数
    ZCE_UUID128_Generator();
    ///析构函数
    ~ZCE_UUID128_Generator();

public:

    /*!
    * @brief      注册随机数的种子
    * @param      seed 种子
    */
    void random_seed(uint32_t seed);
    /*!
    * @brief      产生一个UUID128，
    * @return     ZCE_UUID128
    */
    ZCE_UUID128 random_gen();

    /*!
    * @brief      以时间为基数的初始化，radix一般可以用当前时间time_t（当然也可以不用，你用另外一个标识）
    * @param      identity
    * @param      radix
    */
    void time_radix(uint32_t identity, uint32_t radix = static_cast<uint32_t> (time(NULL)));

    /*!
    * @brief      以时间为基数产生UUID64
    * @return     ZCE_UUID128
    */
    ZCE_UUID128 timeradix_gen();

protected:

    ///
    static ZCE_UUID128_Generator *instance();
    ///
    static void clean_instance();

protected:

    ///发生器使用什么发生方式
    UUID128_GENERATOR_TYPE        generator_type_;

    ///随机发生器1
    ZCE_LIB::random_mt19937       mt_19937_random_;
    ///随机发生器2
    ZCE_LIB::random_mt11213b      mt_11213b_random_;

    ///
    ZCE_UUID128                   time_radix_seed_;

protected:

    //发生器实例指针
    static ZCE_UUID128_Generator *instance_;

};

#endif // ZCE_LIB_UUID_128_GENERATOR_H_

