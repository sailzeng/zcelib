#pragma once

#include <array>

namespace zce
{
static const char* DAY_OF_WEEK_NAME[] =
{
    ("Sun"),
    ("Mon"),
    ("Tue"),
    ("Wed"),
    ("Thu"),
    ("Fri"),
    ("Sat")
};

static const char* MONTH_NAME[] =
{
    ("Jan"),
    ("Feb"),
    ("Mar"),
    ("Apr"),
    ("May"),
    ("Jun"),
    ("Jul"),
    ("Aug"),
    ("Sep"),
    ("Oct"),
    ("Nov"),
    ("Dec")
};

///时间格式化输出的格式类型
enum TMS_FMT
{
    TMS_FMT_INVALID = -1,
    ///用2位年的收缩的格式进行输出 格式举例如下:
    /// 240910
    /// 240910 100318
    /// 240910 100318.100190
    SHRINK_DAY,
    SHRINK_SEC,
    SHRINK_USEC,
    ///用紧凑的格式进行输出 格式举例如下:
    /// 20100910
    /// 20100910 100318
    /// 20100910 100318.100190
    COMPACT_DAY,
    COMPACT_SEC,
    COMPACT_USEC,
    ///用ISO的格式进行时间输出，格式举例如下:
    /// 10:03:18  , 只有时间的格式
    /// 10:03:18.100190
    /// 2010-09-10
    /// 2010-09-10 10:03:18
    /// 2010-09-10 10:03:18.100190
    ISO_TIME_SEC,
    ISO_TIME_USEC,
    ISO_DATE_DAY,
    ISO_DATE_SEC,
    ISO_DATE_USEC,
    ///用美国的时间格式进行输出，格式举例如下:
    /// Fri Aug 24 2002 07:43:05
    /// Fri Aug 24 2002 07:43:05.100190
    US_SEC,
    US_USEC,

    ///用HTTP头部GMT的时间格式进行输出, Thu, 26 Nov 2009 13:50:19 GMT
    HTTP_GMT,
    ///用EMAIL头部DATE的时间格式进行输出, Fri, 08 Nov 2002 09:42:22 +0800
    EMAIL_DATE,

    //! 枚举数量
    TMS_FMT_COUNT,
};

//! 为什么最后还是搞了一个自己的时间定义，因为所有的结构都有短板。
//! tm 没有时区和微妙
//! time_t 没有微秒
//! MYSQL_TIME结构只能在MySQL环境下使用
struct ztm
{
    void clear()
    {
        tz_ = INVALID_TZ;
        year_ = 0;
        mon_ = 0;
        day_ = 0;
        hour_ = 0;
        min_ = 0;
        sec_ = 0;
        usec_ = 0;
    }

    static const int INVALID_TZ = 0x7FFFFFFF; //无效的时区

    TMS_FMT fmt_ = TMS_FMT_INVALID; //格式化的类型
    // 注意 tz是UTC和当前时区本地时间的差，+0800是东8区，+08:00的tz是-28800，这个是反直觉的
    // -05:00是西5区，-05:00的tz是18000，这个也是反直觉的, 还有尼泊尔的时区	+05:45
    int tz_ = INVALID_TZ;    // time zone [-12+14]
    int year_ = 0;  // years since 0
    int mon_ = 0;   // months since January - [1, 12]
    int day_ = 0;   // day of the month - [1, 31]
    int hour_ = 0;  // hours since midnight - [0, 23]
    int min_ = 0;   // minutes after the hour - [0, 59]
    int sec_ = 0;   // seconds after the minute - [0, 60] including leap second
    time_t usec_ = 0;  // microseconds after the second - [0, 999999]
};

/*
100910 100318.100190                      SHRINK,收缩
20100910 100318.100190                    COMPACT,紧凑
10:03:18.100190                           TIME
2010-09-10 10:03:18.100190+08:00          ISO 8601
Fri Aug 24 2002 07:43:05.100190           US
Thu, 26 Nov 2009 13:50:19 GMT             GMT(GMT一般不输出毫秒，在HTTP头中应用)
Fri, 08 Nov 2002 09:42:22 +0800           EMail Date
1234567890123456789012345678901234567890
*/

///注意下面的长度不包括包括'\0'，申请的空间要 + 1，最简单的记法就是保证有32字节的空间
///（除了GMT精确到us），不采用+1的长度记录，这样写的目的是方便某些计算，
constexpr std::array<size_t, (size_t)zce::TMS_FMT::TMS_FMT_COUNT> TIMESTR_LEN = {
   6,  // SHRINK_DAY
   13, // SHRINK_SEC
   20, // SHRINK_USEC
   8,  // COMPACT_DAY
   15, // COMPACT_SEC
   22, // COMPACT_USEC
   8,  // ISO_TIME_SEC
   15, // ISO_TIME_USEC
   10, // ISO_DATE_DAY
   19, // ISO_DATE_SEC
   26, // ISO_DATE_USEC
   24, // US_SEC
   31, // US_USEC
   29, // HTTP_GMT
   31, // EMAIL_DATE
};

//时区字符串的最大长度，+08  +08:00, -08:00
static const size_t MAX_TZ_LEN = 6;
///最大格式化长度,一般而言32足够了
static const size_t MAX_TIMEVAL_STRING_LEN = TIMESTR_LEN[static_cast<size_t>(zce::TMS_FMT::US_USEC)];

struct ::timeval;

/*!
* @brief      将参数timeval的值作为的时间格格式化后输出打印出来，可以控制各种格式输出
* @return     const char*   返回的字符串，其实就是str_date_time
* @param[in]  timeval       打印的时间timeval
* @param[out] str_date_time 得到的时间字符串
* @param[in]  str_len       字符串的长度，最简单的记法就是保证有32字节的空间
* @param[in]  fmt           参数清参考@ref TS_FMT ，
* @param[in]  uct_time      视字符串为UCT/GMT时间还是本地时间Local Time，
* @param[in]  out_tz        是否输出时区信息
*                           true表示视为UCT/GMT时间，false表示视为本地时间
* @note       时间戳打印格式说明,TS_FMT
*/
const char* timeval_to_str(const ::timeval* timeval,
                           char* str_date_time,
                           size_t str_len,
                           size_t& use_buf,
                           TMS_FMT fmt = zce::TMS_FMT::ISO_DATE_USEC,
                           bool utc_time = false,
                           bool out_tz = false);

//! 尝试取得fuzzy str 的时间格式类型
TMS_FMT fuzzy_str_fmt(const char* strtm);

struct ztm;
/*!
* @brief      从字符串中高速的得到ztm的结构的结果
* @param[in]  strtm    字符串，字符串的正确性你自己要保证
* @param[in/out] pztm  输入时ztm 的TMS_FMT fmt_，表示字符串的格式，参考枚举值 @ref TS_FMT
*                      返回时的时间放入ztm结构的指针，
*                      包括时区，年，月，日，时，分，秒，微秒，时区，如果有尽量填充
* @note       这个函数是一个快速的函数，主要是为了提高性能，字符串的正确性你自己要保证
*/
int str_to_ztm(const char* strtm,
               zce::ztm* pztm);

/*!
 * @brief 对字符串进行（模糊）匹配，选择最合适的格式进行转换
 *        参数和返回都请参考 @str_to_ztm
 * @param pztm   如果希望模糊匹配，输入时ztm 的TMS_FMT fmt_
 *               填写为TMS_FMT_INVALID，表示不确定的格式，否则按照你的要求进行转换
 */
int fuzzy_str_to_ztm(const char* strtm,
                     zce::ztm* pztm);

/*!
* @brief      从字符串转换得到本地时间timeval函数
* @return     int == 0 表示成功
* @param[in]  strtm    字符串参数
* @param[in]  fmt      字符串的格式，参考枚举值 @ref TS_FMT,
* @param[in]  uct_time 将strtm字符串视为UCT/GMT时间还是本地时间Local Time
*                      true表示视为UCT/GMT时间，false表示视为本地时间
* @param[out] tval     返回的时间,
*/
int str_to_timeval(const char* strtm,
                   TMS_FMT fmt,
                   ::timeval* tval,
                   bool uct_time = false);

//! @brief   采用模糊识别的方式，从字符串转换得到本地时间timeval函数,
int fuzzy_str_to_timeval(const char* strtm,
                         ::timeval* tval,
                         bool uct_time = false);

#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL ==1

int str_to_MYSQL_TIME(const char* strtm,
                      MYSQL_TIME* ptr_tm);
#endif
}
