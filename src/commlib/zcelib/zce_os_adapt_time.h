/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_time.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年5月1日
* @brief      时间操作的适配器层，主要还是向LINUX下靠拢
*
* @details
*
* @note       timeval在整套库内部使用较多，本来打算彻底用他标识内部的时间，但后来还是整理了自己的Time_Value类，
*             可能是因为最后写代码时，timeval的很多操作不封装写起来还是繁琐。
*/

#ifndef ZCE_LIB_OS_ADAPT_TIME_H_
#define ZCE_LIB_OS_ADAPT_TIME_H_

#include "zce_os_adapt_predefine.h"




#if defined ZCE_OS_WINDOWS

struct timezone
{
    //minutes west of Greenwich,注意这儿是分钟
    int tz_minuteswest;
    // type of DST correction ，夏令时，这个值在LINUX下都不再使用了，就饶了我吧
    //int tz_dsttime;
};

#endif

namespace ZCE_LIB
{
    //一个小时的时间
    static const time_t ONE_HOUR_SECONDS = 3600;
    //一分钟的秒数
    static const time_t ONE_MINUTE_SECONDS = 60;
    //
    static const time_t ONE_QUARTER_SECONDS = 15 * 60;

    static const time_t FIVE_MINUTE_SECONDS = 5 * 60;
    //一天的秒数86400
    static const time_t ONE_DAY_SECONDS = 86400;
    //各种关于时间的定义
    static const time_t ONE_WEEK_DAYS = 7;
    //一周的秒数
    static const time_t ONE_WEEK_SECONDS = 604800;

#if defined ZCE_OS_WINDOWS
    static const time_t TIMEZONE_SECONDS = _timezone;
#else
    static const time_t TIMEZONE_SECONDS = timezone;
#endif

//在时区的角度，从1970.1.1到现在多少秒
#ifndef HOW_MANY_SECONDS_TZ
#define HOW_MANY_SECONDS_TZ(x)  (x + ZCE_LIB::TIMEZONE_SECONDS)
#endif
//在时区的角度，取从1970.1.1到现在的天数量,用于判断是不是同一天之类的问题
#ifndef HOW_MANY_DAYS_TZ
#define HOW_MANY_DAYS_TZ(x)  (HOW_MANY_SECONDS_TZ(x)/ZCE_LIB::ONE_DAY_SECONDS)
#endif
//在时区的角度，取从1970.1.1到现在的小时数量
#ifndef HOW_MANY_HOURS_TZ 
#define HOW_MANY_HOURS_TZ(x) (HOW_MANY_SECONDS_TZ(x) / ZCE_LIB::ONE_HOUR_SECONDS)
#endif
//在时区的角度，取从1970.1.1到现在的小时数量 这里为啥要+3，因为19700101是礼拜四。还是注释清楚吧，从周一到现在
#ifndef HOW_MANY_WEEKS_TZ
#define HOW_MANY_WEEKS_TZ(x)  (( HOW_MANY_SECONDS_TZ(x) + ZCE_LIB::ONE_DAY_SECONDS * 3)/ZCE_LIB::ONE_WEEK_SECONDS)
#endif

/*!
* @brief      非标准函数，得到服务器启动到现在的时间，这个时间是个绝对递增的值，不会调整
* @detail     原来模拟的函数是gethrtime的行为，但其返回单位比较怪，类似得到CPU从启动后到现在的TICK的时间，
*             后来pascal改了改了函数名称，也好行为不一样，不如这样
* @return     const timeval
* @note       WINDOWS (不支持WIN2008的环境上，包括编译) 使用的是GetTickCount 函数，这个函数返回毫秒整数，
*             49天就会溢出，内部为了上一次调用的时间，用了static 变量，又为了保护static，给了锁，，所以你要
*             保证调用频度问题。如果两次调用之间的时间如果过长，超过49天，我无法保证你得到准确的值
*             你老不要49天就只调用一次这个函数呀，那样我保证不了你的TICK的效果，你老至少每天调用一次吧。
*             LINUX 下，测试后发现很多系统没有gethrtime函数，用POSIX的新函数clock_gettime代替。
*/
const timeval  get_uptime(void);

/*!
* @brief      取得当前的时间，（墙上时钟）
* @return     inline int  ==0表示成功
* @param      [out]tv     当前的时间
* @param      [out]tz     时区，内部没有处理夏令时问题，LINUX的系统目前也不支持了
* @note       gettimeofday 这个函数在POSIX中已经被抛弃，但是大量的LINUX还会继续使用，所以暂时留下来，
*             POSIX推荐的clock_gettime好像还没有看见多少人呢用
*/
inline int gettimeofday(struct timeval *tv, struct timezone *tz = NULL);


/*!
* @brief      取得当前的时钟
* @return     inline const timeval 将当前时间timeval作为结果返回
*/
inline const timeval gettimeofday();


/*!
* @brief      取得当前的时区,非标准函数，为当前时区和GMT相差的秒数，
* @return     int 为当前时区和GMT相差的秒数，
* @note       为什么是秒，而不是第一时区，第二时区，如果你真了解时区，
*             你就明白了，时区还有+0630这样的时区
*             理论上timezone要先调用tzset才能初始化。
*/
int gettimezone();


/*!
* @brief      将参数timeval的值作为的时间格格式化后输出打印出来
*             时间戳打印函数,字符串的输出格式是ISO-8601 format. 例子 2010-09-10 10:03:18.100190
*             输出的时间戳格式为2010-09-10 10:03:18.100190       注意末尾还有一个\0
*                           123456789012345678901234567890
* @return     const char*         得到的时间字符串
* @param[out] str_date_time   字符串
* @param[in]  datetime_strlen 字符串长度
* @note
*/
const char *timestamp(const timeval *timeval,
                      char *str_date_time,
                      size_t datetime_strlen);

/*!
* @brief      得到当前的系统时间字符串输出
* @return     const char*
* @param      str_date_time
* @param      datetime_strlen
* @note
*/
const char *timestamp(char *str_date_time,
                      size_t datetime_strlen);


///时间格式化输出的格式类型
enum TIME_STR_FORMAT_TYPE
{

    ///用紧凑的格式进行输出 20100910
    TIME_STRFMT_COMPACT_DAY  = 1,
    ///用紧凑的格式进行输出 20100910100318
    TIME_STRFMT_COMPACT_SEC  = 2,

    ///用ISO的格式进行时间输出，精度到天 2010-09-10
    TIME_STRFMT_ISO_DAY      = 5,
    ///用ISO的格式进行时间输出，精度到秒，2010-09-10 10:03:18
    TIME_STRFMT_ISO_SEC      = 6,
    ///用ISO的格式进行时间输出，精度到微秒，2010-09-10 10:03:18.100190
    TIME_STRFMT_ISO_USEC     = 7,

    ///用美国的时间格式进行输出 Fri Aug 24 2002 07:43:05
    TIME_STRFMT_US_SEC       = 10,
    ///用美国的时间格式进行输出 Fri Aug 24 2002 07:43:05.100190
    TIME_STRFMT_US_USEC      = 11,


    ///用HTTP头部GMT的时间格式进行输出, Thu, 26 Nov 2009 13:50:19 GMT
    TIME_STRFMT_HTTP_GMT     = 1001,
    ///用EMAIL头部DATE的时间格式进行输出, Fri, 08 Nov 2002 09:42:22 +0800
    TIME_STRFMT_EMAIL_DATE   = 1002,
};

/*
20100910100318                            紧凑
2010-09-10 10:03:18.100190                ISO
Fri Aug 24 2002 07:43:05.100190           US
Thu, 26 Nov 2009 13:50:19 GMT             GMT(GMT一般不输出毫秒，在HTTP头中应用)
Fri, 08 Nov 2002 09:42:22 +0800
1234567890123456789012345678901234567890
*/

///注意下面的长度不包括包括'\0'，申请的空间要 + 1，最简单的记法就是保证有32字节的空间
///（除了GMT精确到us），不采用+1的长度记录，这样写的目的是方便某些计算，

///COMPACT 紧凑时间格式字符串的长度，
///输出字符串精度到日期的字符串长度
static const size_t TIMESTR_COMPACT_DAY_LEN = 8;
///输出字符串精度到秒的字符串长度
static const size_t TIMESTR_COMPACT_SEC_LEN = 14;

///ISO 时间格式化字符串的长度，
///输出字符串精度到日期的字符串长度
static const size_t TIMESTR_ISO_DAY_LEN = 10;
///输出字符串精度到秒的字符串长度
static const size_t TIMESTR_ISO_SEC_LEN = 19;
///[常用]精度到微秒的
static const size_t TIMESTR_ISO_USEC_LEN = 26;

///US 米国时间格式字符串的长度，精度到秒
static const size_t TIMESTR_US_SEC_LEN = 24;
///US 米国时间格式字符串的长度，精度到微秒
static const size_t TIMESTR_US_USEC_LEN = 31;

///GMT 时间格式字符串的长度，精度到秒
static const size_t TIMESTR_HTTP_GMT_LEN = 29;

///GMT 时间格式字符串的长度，精度到秒
static const size_t TIMESTR_EMAIL_DATE_LEN = 31;






/*!
* @brief      将参数timeval的值作为的时间格格式化后输出打印出来，可以控制各种格式输出
* @return     const char*   返回的字符串，其实就是str_date_time
* @param[in]  timeval       打印的时间timeval
* @param[out] str_date_time 得到的时间字符串
* @param[in]  str_len       字符串的长度，最简单的记法就是保证有32字节的空间
* @param[in]  uct_time      将timeval视为UCT/GMT时间还是本地时间Local Time，true
*                           表示视为UCT/GMT时间，false表示视为本地时间
* @param      fmt           参数清参考@ref TIME_STR_FORMAT_TYPE ，
* @note       时间戳打印格式说明,TIME_STR_FORMAT_TYPE
*/
const char *timeval_to_str(const timeval *timeval,
                           char *str_date_time,
                           size_t str_len,
                           bool uct_time = false,
                           TIME_STR_FORMAT_TYPE fmt = ZCE_LIB::TIME_STRFMT_ISO_USEC
                          );

/*!
* @brief      从字符串中高速的得到tm的结构的结果
* @param[in]  strtm   字符串，字符串的正确性你自己要保证
* @param[in]  fmt     字符串的格式，参考枚举值 @ref TIME_STR_FORMAT_TYPE
* @param[out] ptr_tm  返回的tm结构的指针，注意，如果字符串错误，可能会导致tm错误喔
* @param[out] usec    返回的的微秒的时间，默认为NULL，表示不需要返回，
* @param[out] tz      返回的的时区,默认为NULL，表示不需要返回，很多种格式里面没有时区信息
*/
void str_to_tm(const char *strtm,
               TIME_STR_FORMAT_TYPE fmt,
               tm *ptr_tm,
               time_t *usec = NULL,
               int *tz = NULL);


/*!
* @brief      从字符串转换得到本地时间timeval函数
* @return     int == 0 表示成功
* @param[in]  strtm    字符串参数
* @param[in]  uct_time 将strtm字符串视为UCT/GMT时间还是本地时间Local Time
*                      true表示视为UCT/GMT时间，false表示视为本地时间
* @param[in]  fmt      字符串的格式，参考枚举值 @ref TIME_STR_FORMAT_TYPE,

* @param[out] tval     返回的时间,
*/
int str_to_timeval(const char *strtm,
                   TIME_STR_FORMAT_TYPE fmt,
                   bool uct_time,
                   timeval *tval);


///本地时间字符串转换为time_t
int localtimestr_to_time_t(const char *localtime_str,
                           TIME_STR_FORMAT_TYPE fmt,
                           time_t *time_t_val);

/*!
* @brief      计算timeval内部总计是多少毫秒
* @return     uint64_t
* @param      tv  要计算的timeval
*/
uint64_t total_milliseconds(const timeval &tv);

/*!
* @brief      计算timeval内部总计是多少微秒
* @return     uint64_t
* @param      tv  要计算的timeval
*/
uint64_t total_microseconds(const timeval &tv);

//扩展timeval的一些函数，没有之间重载操作符号的原因是避免污染整个空间

/*!
* @brief      返回一个0,0的的timeval的结构，
* @return     const timeval 要计算的timeval
*/
const timeval timeval_zero();

/*!
* @brief      将tv修正为0
* @param      tv  清0的timeval
*/
void timeval_clear(timeval &tv);

/*!
* @brief      比较时间是否一致,
* @return     int     如果一致返回0，left大，返回正数，right大返回负数,
* @param      left    左值
* @param      right   右值
* @note       注意他和timercmp是不一样的功能，timercmp主要是利用最后一个函数参数完成比较内容
*/
int timeval_compare(const timeval &left, const timeval &right);

/*!
* @brief      对两个时间进行相加，将结果返回，非标准函数
* @return     const timeval
* @param      left          左值
* @param      right         右值
* @note
*/
const timeval timeval_add(const timeval &left, const timeval &right);

/*!
* @brief      对两个时间进行想减，将结果返回，非标准函数,safe==true如果小于0，返回0
* @return     const timeval 相减的结果
* @param      left          左
* @param      right         右
* @param      safe          是否进行安全保护，如果进行保护，结果小于0时，返回0
* @note
*/
const timeval timeval_sub(const timeval &left, const timeval &right, bool safe = true);

/*!
* @brief      如果你设置的usec 总长度>1s，我帮你调整，，非标准函数
* @return     void
* @param      tv      调整的timeval
*/
void timeval_adjust(timeval &tv);

/*!
* @brief      检查这个TIMEVALUE是否还有剩余的时间，非标准函数
* @return     bool
* @param      tv
*/
bool timeval_havetime(const timeval &tv);

/*!
* @brief      生成timeval这个结构
* @return     const timeval
* @param      sec           秒
* @param      usec          微秒
*/
const timeval make_timeval(time_t sec, time_t usec);

/*!
* @brief      将类型为std::clock_t值 转换得到timeval这个结构
* @return     const timeval 转换后的timeval结果
* @param      clock_value   进行转换的参数
*/
const timeval make_timeval(std::clock_t clock_value);

/*!
* @brief      将timespec结构转换得到timeval结构
* @return     const timeval 转换后的timeval结果
* @param      timespec_val  进行转换的参数
*/
const timeval make_timeval(const ::timespec *timespec_val);

//WINDOWS API常用的几个参数
#if defined (ZCE_OS_WINDOWS)

/*!
* @brief      将FILETIME的参数视为一个时间（绝对时间 如2013-01-01 01:53:29），转换得到timeval
* @return     const timeval
* @param      file_time
*/
const timeval make_timeval(const FILETIME *file_time);

/*!
* @brief      转换SYSTEMTIME到timeval
* @return     const timeval
* @param      system_time
* @note
*/
const timeval make_timeval(const SYSTEMTIME *system_time);

/*!
* @brief      将FILETIME的参数视为一个时长（相对时间 如25s），转换FILETIME到timeval ，
* @return     const timeval
* @param      file_time
*/
const timeval make_timeval2(const FILETIME *file_time);

#endif

//我整体对timespec不想做太多支持，

/*!
* @brief      POSIX 新的推荐使用的时间函数，
* @return     int
* @param      clk_id  Linux 下支持很多参数包括时间，打点，启动时间，进程，线程时间等
*                     WIN 下有有效的是， CLOCK_REALTIME 高精度实际时间，CLOCK_MONOTONIC，
*                     单调（打点）计数器，其他的的不考虑
* @param      ts      返回的时间
*/
inline int clock_gettime(clockid_t clk_id, timespec *ts);

/*!
* @brief      计算timespec内部总计是多少毫秒
* @return     uint64_t
* @param      ts
*/
uint64_t total_milliseconds(const timespec &ts);

/*!
* @brief      将timespec 结构，转换得到timeval这个结构
* @return     const ::timespec
* @param      timeval_val
*/
const ::timespec make_timespec(const timeval *timeval_val);

//-------------------------------------------------------------------------------
//休息一下，休息一下

/*!
* @brief      SLEEP 秒
* @return     int     0成功，-1失败
* @param      seconds 休息的秒数
* @note
*/
int sleep (uint32_t seconds);

/*!
* @brief      SLEEP timeval的时间
* @return     int  0成功，-1失败
* @param      tv   休息的timeval时间长度的时间
* @note
*/
int sleep (const timeval &tv);

/*!
* @brief      休息微秒，但实际休息时间，肯定长于这个，呵呵
* @return     int   0成功，-1失败
* @param      usec  微秒时间长度
*/
int usleep (unsigned long usec);

//-------------------------------------------------------------------------------
//重入安全的时间转换函数的函数

/*!
* @brief      得到本地时间的tm结构
* @return     inline struct tm*
* @param      timep
* @param      result
* @note       重入安全
*/
inline struct tm *localtime_r(const time_t *timep, struct tm *result);

/*!
* @brief      根据GM时间，得到tm结构
* @return     inline struct tm*
* @param      timep
* @param      result
* @note
*/
inline struct tm *gmtime_r(const time_t *timep, struct tm *result);

/*!
* @brief      打印TM内部的时间信息
* @return     inline char*
* @param      tm
* @param      buf            输出的字符串长度，长度你自己保证至少26个
* @note       输出字符串的格式是Wed Jan 02 02:03:55 1980\n\0.强烈不建议使用,
*             唉，我实在不知道这样的时间输出有啥用处，完全是为了兼容才写这两个函数，
*/
inline char *asctime_r(const struct tm *tm, char *buf);

/*!
* @brief      打印time_t所表示的时间信息
* @return     inline char* 返回的时间字符串
* @param      timep        时间
* @param      buf          输出的字符串长度，长度你自己保证至少26个
* @note       输出字符串的格式是Wed Jan 02 02:03:55 1980\n\0.强烈不建议使用,
*             唉，我实在不知道这样的时间输出有啥用处，完全是为了兼容才写这两个函数，
*/
inline char *ctime_r(const time_t *timep, char *buf);


/*!
* @brief      等同于mktime,将tm视为本地时间，转换为time_t
* @return     time_t 转换得到的UTC世界时间
* @param      tm 视为本地时间的tm
*/
inline time_t timelocal(struct tm *tm);

/*!
* @brief      类似mktime，但是是把tm视为GMT时间，转换为time_t
*             WINDOWS下有个mkgmtime，
* @return     time_t 转换得到的UTC世界时间
* @param      tm 视为GMT时间的tm
*/
inline time_t timegm(struct tm *tm);

/*!
* @brief      读取TSC，Read Time-Stamp Counter
* @return     uint64_t TSC值
*/
uint64_t rdtsc();


};

//-------------------------------------------------------------------------------

//兼容LINUX下的gettimeofday
inline int ZCE_LIB::gettimeofday(struct timeval *tv, struct timezone *tz)
{
    //
#if defined ZCE_OS_WINDOWS
    //
    if (tv == NULL && tz == NULL)
    {
        return 0;
    }

    //如果输入了时间
    if (tv)
    {
        //得到系统时间,
        //测试比较::GetSystemTime 比这个函数耗时，从字面理解有点怪异
        FILETIME   tfile;
        ::GetSystemTimeAsFileTime (&tfile);

        ULARGE_INTEGER ui;
        ui.LowPart = tfile.dwLowDateTime;
        ui.HighPart = tfile.dwHighDateTime;

        //The FILETIME structure is a 64-bit value representing the number of
        //100-nanosecond intervals since January 1, 1601.

        //得到time_t部分
        tv->tv_sec = static_cast<long>((ui.QuadPart - 116444736000000000) / 10000000);
        //得到微秒部分，FILETIME存放的是100-nanosecond
        tv->tv_usec = static_cast<long>(((ui.QuadPart - 116444736000000000) % 10000000) / 10);
    }

    //得到时区相关数据
    if (tz)
    {
        //得到时区信息
        TIME_ZONE_INFORMATION tzone;
        DWORD  ret = ::GetTimeZoneInformation(&tzone);

        if (ret == TIME_ZONE_ID_INVALID)
        {
            return -1;
        }

        //
        tz->tz_minuteswest = tzone.Bias;
        //夏令时问题回避，LINUX下已经不处理这个问题了

    }

    return 0;
#endif //

    //LINUX下得到时间
#if defined ZCE_OS_LINUX
    //直接掉用系统的
    return ::gettimeofday(tv, tz);
#endif //

}


//得到时间各种时间
inline int ZCE_LIB::clock_gettime(clockid_t clk_id, timespec *ts)
{
#if defined ZCE_OS_WINDOWS
    timeval tv;
    if (CLOCK_REALTIME  == clk_id)
    {
        ZCE_LIB::gettimeofday(&tv);
    }
    else if (CLOCK_MONOTONIC  == clk_id)
    {
        tv = ZCE_LIB::get_uptime();
    }
    else
    {
        return EINVAL;
    }
    ts->tv_sec = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;
    return 0;
#else
    return ::clock_gettime(clk_id, ts);
#endif //
}


inline const timeval ZCE_LIB::gettimeofday()
{
    timeval now_time;
    ZCE_LIB::gettimeofday(&now_time);
    return now_time;
}

//得到本地时间
inline struct tm *ZCE_LIB::localtime_r(const time_t *timep, struct tm *result)
{
#if defined (ZCE_OS_WINDOWS)
    //WINDOWS下使用默认的_s 系列的API
    errno_t convert_err =  ::localtime_s(result, timep);

    if (convert_err)
    {
        return NULL;
    }
    else
    {
        return result;
    }

#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::localtime_r(timep, result);
#endif //#if defined (ZCE_OS_LINUX)
}

//GM时间
inline struct tm *ZCE_LIB::gmtime_r(const time_t *timep, struct tm *result)
{
#if defined (ZCE_OS_WINDOWS)
    //WINDOWS下使用默认的_s 系列的API
    errno_t convert_err =  ::gmtime_s(result, timep);

    if (convert_err)
    {
        return NULL;
    }
    else
    {
        return result;
    }

#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::gmtime_r(timep, result);
#endif //#if defined (ZCE_OS_LINUX)
}

//打印TM内部的时间信息
inline char *ZCE_LIB::asctime_r(const struct tm *tm_data, char *buf)
{
#if defined (ZCE_OS_WINDOWS)

    //你至少要保证buf里面有26个字符,这种写法实属无奈，因为LINUX的API并没有保证真正的安全，只是解决了重入问题
    //很少有的发现WINDOWS的API好于LINUX
    const size_t I_GUESS_BUF_HAVE_ROOM_FOR_AT_LEAST_26_BYTES = 26;

    //WINDOWS下使用默认的_s 系列的API
    errno_t convert_err =  ::asctime_s(buf, I_GUESS_BUF_HAVE_ROOM_FOR_AT_LEAST_26_BYTES, tm_data);

    if (convert_err)
    {
        return NULL;
    }
    else
    {
        return buf;
    }

#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::asctime_r(tm_data, buf);
#endif //#if defined (ZCE_OS_LINUX)
}

//打印time_t锁标识的时间信息
inline char *ZCE_LIB::ctime_r(const time_t *timep, char *buf)
{
#if defined (ZCE_OS_WINDOWS)

    //你至少要保证buf里面有26个字符,，因为LINUX的API并没有保证真正的安全，只是解决了重入问题
    const size_t I_GUESS_BUF_HAVE_ROOM_FOR_AT_LEAST_26_BYTES = 26;

    //WINDOWS下使用默认的_s 系列的API
    errno_t convert_err =  ::ctime_s(buf, I_GUESS_BUF_HAVE_ROOM_FOR_AT_LEAST_26_BYTES, timep);

    if (convert_err)
    {
        return NULL;
    }
    else
    {
        return buf;
    }

#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::ctime_r(timep, buf);
#endif //#if defined (ZCE_OS_LINUX)
}



//等同于mktime,将tm视为本地时间，转换为世界时间time_t
inline time_t ZCE_LIB::timelocal(struct tm *tm)
{
#if defined (ZCE_OS_WINDOWS)
    return ::mktime(tm);
#endif

#if defined (ZCE_OS_LINUX)
    return ::timelocal(tm);
#endif
}


//类似mktime，但是是把tm视为GMT时间，转换为世界时间time_t
inline time_t ZCE_LIB::timegm(struct tm *tm)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_mkgmtime(tm);
#endif

#if defined (ZCE_OS_LINUX)
    return ::timegm(tm);
#endif
}

#endif //ZCE_LIB_SHARE_MEM_PRE_DEFINE_H_

