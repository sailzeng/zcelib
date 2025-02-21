/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_uuid_64_generator.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
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
#pragma once

#include "zce/math/random.h"
#include "zce/uuid/uuid.h"

namespace zce
{
//UUID产生方法，
enum class UUID_GENERATOR
{
    ///无效的产生方式
    INVALID,
    ///用随机数的方法产生
    RANDOM,
    ///用时间作为基数触发
    TIME,
};

/************************************************************************************************************
Class           : uuid64_gen
************************************************************************************************************/

/*!
* @brief      uuid64的产生器，提供随机数产生，以时间为基数产生随机数的方法
*             @li 随机数产生，产生一个随机的uint64_t的数字,当然大家方向这个重复律很低
*             @li 以时间为基数的产生长发，16位数值你自己定义，后面48位整数，以时间为基数
*                 ，然后后面每次产生就累计一次，这种方法适合小规模的分布，16位表示分布的
*                 服务器，每个服务器自己产生UUID的情况，只要1s产生的UUID的数量不超过16
*                 位，就比较很安全。这个方法始作蛹者应该是 sonicmao，你可以根据UUID回溯
*                 是什么服务器产生的数据
*
*/
class uuid64_gen
{
public:

    /*!
    * @brief      构造函数
    */
    uuid64_gen();

    /*!
    * @brief      析构函数
    */
    ~uuid64_gen();

public:

    /*!
    * @brief      随机数的种子,调用这个函数后，
    *             就会确认产生方式是UUID64_GENERATOR::RANDOM，不能更改了。
    * @param      seed 随机数的种子,
    */
    void random_seed(uint32_t seed);
    /*!
    @brief      通过随机数产生一个uuid64
    @return     uuid64 产生的UUID
    */
    uuid64 random_gen();

    /*!
    * @brief      以时间为基数的初始化，radix一般可以用当前时间time_t
    *             调用这个函数后，就会确认产生方式是UUID64_GENERATOR_TIME，不能更改了。
    * @param      identity 当前的唯一表示，比如服务器ID等信息
    * @param      radix    时间基数，一般时间作为基础
    */
    void time_radix(uint16_t identity,
                    uint32_t radix = static_cast<uint32_t> (time(nullptr)));

    /*!
    * @brief      以时间为基数产生uuid64
    * @return     uuid64 产生的uuid64
    */
    uuid64 timeradix_gen();

protected:

    ///发生器实例指针
    static uuid64_gen* instance_;

protected:

    ///发生器使用什么发生方式
    UUID_GENERATOR        generator_type_;

    ///随机发生器1
    zce::random_mt19937   mt_19937_random_;
    ///随机发生器2
    zce::random_mt11213b  mt_11213b_random_;

    ///以时间作为基数的UUID种子
    zce::uuid64           time_radix_seed_;
};

//! @brief 实例
typedef zce::singleton<uuid64_gen> uuid64_gen_inst;

/************************************************************************************************************
Class           : uuid128_gen UUID的发生器
************************************************************************************************************/

/*!
* @brief      uuid128_gen UUID的发生器
*
*/
class uuid128_gen
{
public:

    ///构造函数
    uuid128_gen();
    ///析构函数
    ~uuid128_gen();

public:

    /*!
    * @brief      注册随机数的种子
    * @param      seed 种子
    */
    void random_seed(uint32_t seed);
    /*!
    * @brief      产生一个uuid128，
    * @return     uuid128
    */
    uuid128 random_gen();

    /*!
    * @brief      以时间为基数的初始化，radix一般可以用当前时间time_t（当然也可以不用，你用另外一个标识）
    * @param      identity
    * @param      radix
    */
    void time_radix(uint32_t identity,
                    uint32_t radix = static_cast<uint32_t> (time(nullptr)));

    /*!
    * @brief      以时间为基数产生uuid64
    * @return     uuid128
    */
    uuid128 timeradix_gen();

protected:

    ///发生器使用什么发生方式
    UUID_GENERATOR            generator_type_;

    ///随机发生器1
    zce::random_mt19937       mt_19937_random_;
    ///随机发生器2
    zce::random_mt11213b      mt_11213b_random_;

    ///以时间作为基数的UUID种子
    zce::uuid128              time_radix_seed_;
};

//! @brief 实例
typedef zce::singleton<uuid64_gen> uuid128_gen_inst;
}
