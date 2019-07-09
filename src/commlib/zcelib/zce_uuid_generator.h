/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_uuid_64_generator.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年2月4日
* @brief      Universally Unique Identifier 缩写UUID，支持2种UUID
*             64bit的UUID，这个不标准，主要用于一些特殊需要一个短的ID标示的特殊环境
*             64位的唯一表示，
*             128为的UUID，真正的UUID，
*
*             这个地方主要是为了提供给大家一种方法，产生一个唯一标识，
*             在游戏项目组，主要是在游戏道具标识等范围需要这样的东东，主要用于
*             跟踪某个物品的流转，（我个人对道具使用这个东东作为ID的效果持保守意见）
*             效果并不明显
*
* @details    支持两种产生方式，随机数和以时间为基数的方式
*
* @note
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
#ifndef _ZCE_LIB_UUID_GENERATOR_H_
#define _ZCE_LIB_UUID_GENERATOR_H_

#include "zce_boost_random.h"

/************************************************************************************************************
Class           : ZCE_UUID64
************************************************************************************************************/

#pragma pack(push,1)

/*!
* @brief      UUID64的一种表示方法
*             ZCE_UUID64_16_48 用1个16bit的数值+48位的数值
*             表示UUID64，可以用于一些特殊场合产生唯一表示，
*/
struct ZCE_UUID64_16_48
{
    uint64_t       data1_: 16;
    uint64_t       data2_: 48;
};

/*!
* @brief      64位的UUID，内部分成3种表示方法，也提供了比较==，<等操作
*             @li 64位的整数
*             @li 16位整数+48为的整数
*             @li 2个32位的整数
*
*/
class ZCE_UUID64
{
public:
    ///构造函数
    ZCE_UUID64();
    ///析构函数
    ~ZCE_UUID64();

    /// < 运算符重载
    bool operator < (const ZCE_UUID64 &others) const;
    /// == 运算符
    bool operator == (const ZCE_UUID64 &others) const;

    /// 转移成一个uint64_t的结构
    operator uint64_t();

    /// 转换为字符串
    const char *to_string(char *buffer, size_t buf_len, size_t &use_buf) const;


public:

    ///结构数据的几种不同的表达方式
    union
    {
        ///单个64字节的表示方法
        uint64_t          u_uint64_;
        ///2个32字节的表示方法
        uint32_t          u_2uint32_[2];
        ///16bit+48Bit的表示方法
        ZCE_UUID64_16_48  u_16_48_;
    };

public:

    ///UUID输出字符串的最大长度，不包括'\0',格式XXXXXXXX-XXXXXXXX
    static const size_t   LEN_OF_ZCE_UUID64_STR = 17;

};

#pragma pack(pop)

/************************************************************************************************************
Class           : ZCE_UUID64_Generator
************************************************************************************************************/

/*!
* @brief      UUID64的产生器，提供随机数产生，以时间为基数产生随机数的方法
*             @li 随机数产生，产生一个随机的uint64_t的数字,当然大家方向这个重复律很低
*             @li 以时间为基数的产生长发，16位数值你自己定义，后面48位整数，以时间为基数
*                 ，然后后面每次产生就累计一次，这种方法适合小规模的分布，16位表示分布的
*                 服务器，每个服务器自己产生UUID的情况，只要1s产生的UUID的数量不超过16
*                 位，就比较很安全。这个方法始作蛹者应该是 sonicmao，你可以根据UUID回溯
*                 是什么服务器产生的数据
*
*/
class ZCE_UUID64_Generator
{
    //UUID产生方法，
    enum UUID64_GENERATOR_TYPE
    {
        ///无效的产生方式
        UUID64_GENERATOR_INVALID,

        ///用随机数的方法产生
        UUID64_GENERATOR_RANDOM,
        ///用事件作为基数触发
        UUID64_GENERATOR_TIME,
    };

public:

    /*!
    * @brief      构造函数
    */
    ZCE_UUID64_Generator();

    /*!
    * @brief      析构函数
    */
    ~ZCE_UUID64_Generator();

public:

    /*!
    * @brief      随机数的种子,调用这个函数后，
    *             就会确认产生方式是UUID64_GENERATOR_RANDOM，不能更改了。
    * @param      seed 随机数的种子,
    */
    void random_seed(uint32_t seed);
    /*!
    @brief      通过随机数产生一个UUID64
    @return     ZCE_UUID64 产生的UUID
    */
    ZCE_UUID64 random_gen();

    /*!
    * @brief      以时间为基数的初始化，radix一般可以用当前时间time_t
    *             调用这个函数后，就会确认产生方式是UUID64_GENERATOR_TIME，不能更改了。
    * @param      identity 当前的唯一表示，比如服务器ID等信息
    * @param      radix    时间基数
    */
    void time_radix(uint16_t identity, uint32_t radix = static_cast<uint32_t> (time(NULL)));

    /*!
    * @brief      以时间为基数产生UUID64
    * @return     ZCE_UUID64 产生的UUID64
    */
    ZCE_UUID64 timeradix_gen();

protected:

    ///发生器实例指针
    static ZCE_UUID64_Generator *instance_;

protected:

    /*!
    * @brief      单子函数
    * @return     ZCE_UUID64_Generator* 返回的实例指针
    */
    static ZCE_UUID64_Generator *instance();
    /*!
    * @brief      清理实例指针
    */
    static void clean_instance();

protected:

    ///发生器使用什么发生方式
    UUID64_GENERATOR_TYPE         generator_type_;

    ///随机发生器1
    zce::random_mt19937       mt_19937_random_;
    ///随机发生器2
    zce::random_mt11213b      mt_11213b_random_;

    ///随机数的种子
    ZCE_UUID64                    time_radix_seed_;


};







/************************************************************************************************************
Class           : ZCE_UUID128
************************************************************************************************************/
///32bit整数+32整数+64位整数的表示方法
struct ZCE_UUID128_32_32_64
{

    //
    uint32_t       data1_;
    uint32_t       data2_;
    uint64_t       data3_;
};

///标准的UUID的格式
struct ZCE_UUID128_32_16_16_16_48
{
    uint64_t       data1_ : 32;
    uint64_t       data2_ : 16;
    uint64_t       data3_ : 16;
    uint64_t       data4_ : 16;
    uint64_t       data5_ : 48;
};

///微软的GUID的格式
struct ZCE_UUID128_32_16_16_64
{
    uint64_t       data1_ : 32;
    uint64_t       data2_ : 16;
    uint64_t       data3_ : 16;
    uint64_t       data4_;

};

/*!
* @brief      UUID128，128位的唯一标识，
*             有5中表示方法，
*             @li 16个字节
*             @li 4个32为的整数
*             @li 32位整数+32位整数+64位整数的表示方法
*             @li 32位整数+16位整数+16位整数+16位整数+48位整数，标准的UUID表示方法
*             @li 32位整数+16位整数+16位整数+64位整数，微软的UUID表示方法，GUID，
*
*/
class ZCE_UUID128
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
    const char *to_string(char *buffer, size_t buf_len, size_t &use_buf) const;

public:
    ///UUID的字符串表示的长度
    static const size_t LEN_OF_ZCE_UUID128_STR = 36;

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


};



/************************************************************************************************************
Class           : ZCE_UUID128_Generator UUID的发生器
************************************************************************************************************/

/*!
* @brief      ZCE_UUID128_Generator UUID的发生器
*
*/
class ZCE_UUID128_Generator
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

    ///发生器使用什么发生方式
    UUID128_GENERATOR_TYPE        generator_type_;

    ///随机发生器1
    zce::random_mt19937       mt_19937_random_;
    ///随机发生器2
    zce::random_mt11213b      mt_11213b_random_;

    ///
    ZCE_UUID128                   time_radix_seed_;

protected:

    //发生器实例指针
    static ZCE_UUID128_Generator *instance_;

};

#endif //# ZCE_LIB_UUID_64_GENERATOR_H_

