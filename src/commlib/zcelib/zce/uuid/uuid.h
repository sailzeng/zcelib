#pragma once

/************************************************************************************************************
Class           : uuid64
************************************************************************************************************/
namespace zce
{
#pragma pack(push,1)

/*!
* @brief      uuid64的一种表示方法
*             UUID64_16_48 用1个16bit的数值+48位的数值
*             表示uuid64，可以用于一些特殊场合产生唯一表示，
*/
struct UUID64_16_48
{
    uint64_t       data1_ : 16;
    uint64_t       data2_ : 48;
};

struct UUID64_16_32_16
{
    uint64_t       data1_ : 16;
    uint64_t       data2_ : 32;
    uint64_t       data3_ : 16;
};

/*!
* @brief      64位的UUID，内部分成3种表示方法，也提供了比较==，<等操作
*             @li 64位的整数
*             @li 16位整数+48为的整数
*             @li 2个32位的整数
*
*/
class uuid64
{
public:

    enum FMT
    {
        FMT_INVALID = -1,
        FMT_64,
        FMT_32_32,
        FMT_16_48,
        FMT_16_32_16,
        FMT_COUNTER
    };

    /// UUID64 string lengths for different formats
    static constexpr size_t UUID64_STR_LEN[(size_t)FMT_COUNTER] =
    {
        16, // FMT_64
        17, // FMT_32_32
        17, // FMT_16_48
        18  // FMT_16_32_16
    };

    ///构造函数
    uuid64() = default;
    ///析构函数
    ~uuid64() = default;

    /// < 运算符重载
    bool operator < (const uuid64& others) const;
    /// == 运算符
    bool operator == (const uuid64& others) const;

    /// 转移成一个uint64_t的结构
    operator uint64_t();

    //! 转换为字符串
    const char* to_str(char* buffer,
                       size_t buf_len,
                       size_t& use_buf,
                       zce::uuid64::FMT fmt = FMT_16_32_16) const;

    //! 从字符串转换为UUID64，返回EOF表示失败
    int from_str(char* buffer);

public:

    ///结构数据的几种不同的表达方式
    union
    {
        ///单个64字节的表示方法
        uint64_t      u_uint64_ = 0;
        ///2个32字节的表示方法
        uint32_t      u_2uint32_[2];
        ///16bit+48Bit的表示方法
        UUID64_16_48  u_16_48_;
        ///16+16+32bit的表示方法
        UUID64_16_32_16 u_16_32_16_;
    };

public:

    
    static const size_t   LEN_OF_ZCE_UUID64_STR = 17;
};

/************************************************************************************************************
Class           : uuid128
************************************************************************************************************/
///32bit整数+32整数+64位整数的表示方法
struct UUID128_32_32_64
{
    //
    uint32_t       data1_;
    uint32_t       data2_;
    uint64_t       data3_;
};

///标准的UUID的格式
struct UUID128_32_16_16_16_48
{
    uint64_t       data1_ : 32;
    uint64_t       data2_ : 16;
    uint64_t       data3_ : 16;
    uint64_t       data4_ : 16;
    uint64_t       data5_ : 48;
};

///微软的GUID的格式
struct UUID128_32_16_16_64
{
    uint64_t       data1_ : 32;
    uint64_t       data2_ : 16;
    uint64_t       data3_ : 16;
    uint64_t       data4_;
};

/*!
* @brief      uuid128，128位的唯一标识，
*             有5中表示方法，
*             @li 16个字节
*             @li 4个32为的整数
*             @li 32位整数+32位整数+64位整数的表示方法
*             @li 32位整数+16位整数+16位整数+16位整数+48位整数，标准的UUID表示方法
*             @li 32位整数+16位整数+16位整数+64位整数，微软的UUID表示方法，GUID，
*
*/
class uuid128
{
public:
    enum FMT
    {
        FMT_INVALID = -1,
        FMT_32_32_32_32,
        FMT_64_64,
        FMT_32_32_64,
        FMT_32_16_16_16_48,
        FMT_32_16_16_64,
        FMT_COUNTER
    };

    /// UUID64 string lengths for different formats
    static constexpr size_t UUID128_STR_LEN[(size_t)FMT_COUNTER] =
    {
        35, // FMT_32_32_32_32
        33, // FMT_64_64
        34, // FMT_32_32_64
        36,  // FMT_32_16_16_16_48
        35, // FMT_32_16_16_64
    };

    ///构造函数
    uuid128();
    ///析构函数
    ~uuid128();

    /// < 运算符重载
    bool operator < (const uuid128& others) const;
    /// == 运算符
    bool operator == (const uuid128& others) const;

    /// 以UUID8-4-4-4-12的格式进行转换为字符串
    const char* to_str(char* buffer, size_t buf_len, size_t& use_buf,
                       zce::uuid128::FMT fmt = FMT_32_16_16_16_48) const;

    //! 以UUID8-4-4-4-12的格式读取字符串,返回EOF表示失败
    int from_str(char* buffer);

public:
    ///UUID的字符串表示的长度
    static const size_t LEN_OF_ZCE_UUID128_STR = 36;

public:

    ///5种数据表示方法
    union
    {
        ///16个字节的组成
        uint8_t                  u_16uint8_[16];
        ///4个32为字节的组成
        uint32_t                 u_4uint32_[4];
        ///2个64为字节的组成 32+1
        uint64_t                 u_2uint64_[2];
        ///32位整数+32位整数+64位整数
        UUID128_32_32_64         u_32_32_64_;
        ///标准的UUID的标识方法 32+ 4
        UUID128_32_16_16_16_48   u_32_16_16_16_48_;
        ///微软的GUID的格式，（非标准）
        UUID128_32_16_16_64      u_32_16_16_64_;
    };
};
}
#pragma pack(pop)
