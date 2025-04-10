#pragma once

namespace zce
{
///时间格式化输出的格式类型
enum class TS_FMT : int
{
    ///用2位年的收缩的格式进行输出 格式举例如下:
    /// 240910
    /// 240910 100318
    /// 240910 100318.100190
    SHRINK_DAY = 11,
    SHRINK_SEC = 12,
    SHRINK_USEC = 13,

    ///用紧凑的格式进行输出 格式举例如下:
    /// 20100910
    /// 20100910 100318
    /// 20100910 100318.100190
    COMPACT_DAY = 11,
    COMPACT_SEC = 12,
    COMPACT_USEC = 13,

    // 只有时间的格式
    /// 10:03:18
    /// 10:03:18.100190
    TIME_SEC = 21,
    TIME_USEC = 22,

    ///用ISO的格式进行时间输出，格式举例如下:
    /// 2010-09-10
    /// 2010-09-10 10:03:18
    /// 2010-09-10 10:03:18.100190
    ISO_DAY = 31,
    ISO_SEC = 32,
    ISO_USEC = 33,

    ///用美国的时间格式进行输出，格式举例如下:
    /// Fri Aug 24 2002 07:43:05
    /// Fri Aug 24 2002 07:43:05.100190
    US_SEC = 41,
    US_USEC = 42,

    ///用HTTP头部GMT的时间格式进行输出, Thu, 26 Nov 2009 13:50:19 GMT
    HTTP_GMT = 1001,
    ///用EMAIL头部DATE的时间格式进行输出, Fri, 08 Nov 2002 09:42:22 +0800
    EMAIL_DATE = 1002,
};

/*
100910 100318.100190                      SHRINK,收缩
20100910 100318.100190                    COMPACT,紧凑
2010-09-10 10:03:18.100190                ISO
Fri Aug 24 2002 07:43:05.100190           US
Thu, 26 Nov 2009 13:50:19 GMT             GMT(GMT一般不输出毫秒，在HTTP头中应用)
Fri, 08 Nov 2002 09:42:22 +0800           EMail Date
1234567890123456789012345678901234567890
*/

///注意下面的长度不包括包括'\0'，申请的空间要 + 1，最简单的记法就是保证有32字节的空间
///（除了GMT精确到us），不采用+1的长度记录，这样写的目的是方便某些计算，

///COMPACT 紧凑时间格式字符串的长度，包括精度到DAY，SEC，USEC
static const size_t TS_SHRINK_DAY_LEN = 6;
static const size_t TS_SHRINK_SEC_LEN = 13;
static const size_t TS_SHRINK_USEC_LEN = 20;

///COMPACT 紧凑时间格式字符串的长度，包括精度到DAY，SEC，USEC
static const size_t TS_COMPACT_DAY_LEN = 8;
static const size_t TS_COMPACT_SEC_LEN = 15;
static const size_t TS_COMPACT_USEC_LEN = 22;

///ISO 时间格式化字符串的长度，包括精度到DAY，SEC，USEC
static const size_t TS_ISO_DAY_LEN = 10;
static const size_t TS_ISO_SEC_LEN = 19;
static const size_t TS_ISO_USEC_LEN = 26;  //最常用

///US 米国时间格式字符串的长度，包括精度到SEC，USEC
static const size_t TIMESTR_US_SEC_LEN = 24;
static const size_t TIMESTR_US_USEC_LEN = 31;

///GMT 时间格式字符串的长度，精度到秒
static const size_t TIMESTR_HTTP_GMT_LEN = 29;

///邮件时间格式字符串的长度，精度到秒
static const size_t TIMESTR_EMAIL_DATE_LEN = 31;

///最大格式化长度,一般而言32足够了
static const size_t MAX_TIMEVAL_STRING_LEN = TIMESTR_EMAIL_DATE_LEN;

struct ::timeval;

/*!
* @brief      将参数timeval的值作为的时间格格式化后输出打印出来，可以控制各种格式输出
* @return     const char*   返回的字符串，其实就是str_date_time
* @param[in]  timeval       打印的时间timeval
* @param[out] str_date_time 得到的时间字符串
* @param[in]  str_len       字符串的长度，最简单的记法就是保证有32字节的空间
* @param[in]  uct_time      将timeval视为UCT/GMT时间还是本地时间Local Time，true
*                           表示视为UCT/GMT时间，false表示视为本地时间
* @param      fmt           参数清参考@ref TS_FMT ，
* @note       时间戳打印格式说明,TS_FMT
*/
const char* timeval_to_str(const ::timeval* timeval,
                           char* str_date_time,
                           size_t str_len,
                           size_t& use_buf,
                           bool uct_time = false,
                           TS_FMT fmt = zce::TS_FMT::ISO_USEC
);

struct ztm;
/*!
* @brief      从字符串中高速的得到ztm的结构的结果
* @param[in]  strtm   字符串，字符串的正确性你自己要保证
* @param[in]  fmt     字符串的格式，参考枚举值 @ref TS_FMT
* @param[out] pztm    返回的ztm结构的指针，注意，如果字符串错误，可能会导致ztm错误喔
包括时区，年，月，日，时，分，秒，微秒，时区，如果有尽量填充
*/
int str_to_ztm(const char* strtm,
               TS_FMT fmt,
               zce::ztm* pztm);

int fuzzy_str_to_ztm(const char* strtm,
                     zce::ztm* pztm);

/*!
* @brief      从字符串转换得到本地时间timeval函数
* @return     int == 0 表示成功
* @param[in]  strtm    字符串参数
* @param[in]  uct_time 将strtm字符串视为UCT/GMT时间还是本地时间Local Time
*                      true表示视为UCT/GMT时间，false表示视为本地时间
* @param[in]  fmt      字符串的格式，参考枚举值 @ref TS_FMT,
* @param[out] tval     返回的时间,
*/
int str_to_timeval(const char* strtm,
                   TS_FMT fmt,
                   bool uct_time,
                   ::timeval* tval);

///本地时间字符串转换为time_t
int localtimestr_to_time_t(const char* localtime_str,
                           TS_FMT fmt,
                           ::time_t* time_t_val);

#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL ==1

int str_to_MYSQL_TIME(const char* strtm,
                      MYSQL_TIME* ptr_tm);
#endif
}
