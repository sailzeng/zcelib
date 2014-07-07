/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_uuid_64_generator.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年2月4日
* @brief      64bit的UUID，当然其实UUID其实是128bit的，这儿就是产生一个
*             64位的唯一表示，
*             这个地方主要是为了提供给大家一种方法，产生一个唯一标识，
*             在游戏项目组，主要是在游戏道具标识等范围需要这样的东东，主要用于
*             跟踪某个物品的流转，（我个人对道具使用这个东东作为ID的效果持保守意见）
*             效果并不明显
*
* @details    支持两种产生方式，随机数和以时间为基数的方式
*
* @note
*
*/
#ifndef _ZCE_LIB_UUID_64_GENERATOR_H_
#define _ZCE_LIB_UUID_64_GENERATOR_H_

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
struct ZCELIB_EXPORT ZCE_UUID64_16_48
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
class ZCELIB_EXPORT ZCE_UUID64
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
    const char *to_string(char *buffer, size_t buf_len) const;

    /// 转换为字符串，同时返回输出字符串的长度
    inline const char *to_string(char *buffer, size_t buf_len, size_t &use_buf) const
    {
        const char *ret_ptr = to_string(buffer, buf_len);
        if (ret_ptr)
        {
            use_buf = LEN_OF_ZCE_UUID64_STR;
        }
        else
        {
            use_buf = 0;
        }
        return 0;
    }

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
class ZCELIB_EXPORT ZCE_UUID64_Generator
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
    ZCE_LIB::random_mt19937       mt_19937_random_;
    ///随机发生器2
    ZCE_LIB::random_mt11213b      mt_11213b_random_;

    ///随机数的种子
    ZCE_UUID64                    time_radix_seed_;

protected:

    ///发生器实例指针
    static ZCE_UUID64_Generator  *instance_;

};

#endif //# ZCE_LIB_UUID_64_GENERATOR_H_

