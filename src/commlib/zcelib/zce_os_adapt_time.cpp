/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_time.cpp
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Tuesday, December 31, 2011
* @brief
*
*
* @details    时间操作的适配器层，主要还是向LINUX下靠拢
*
*
*
* @note
*
*/

#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_mutex.h"
#include "zce_lock_thread_mutex.h"
#include "zce_trace_debugging.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_time.h"

//为什么不让我用ACE，卫生棉！，卫生棉！！！！！卫生棉卫生棉卫生棉！！！！！！！！

//这个代码里面写了大量的long主要是为了兼容,迫不得已

//非标准函数，得到服务器启动的时间，gethrtime单位比较怪，类似得到CPU从启动后到现在的TICK的时间，
//缺点：如果两次调用之间的时间如果过长，超过49天，我无法保证你得到准确的值
//你老不要49天就只调用一次这个函数呀，那样我保证不了你的TICK的效果，你老至少每天调用一次吧。
//内部为了上一次调用的时间，用了static 变量，又为了保护static，给了锁，
const timeval ZCE_LIB::get_uptime()
{

#if defined (ZCE_OS_WINDOWS)

    //注意GetTickCount64和GetTickCount返回的都是milliseconds，不是CPU Tick

    timeval up_time;
    uint64_t now_cpu_tick = 0;

    //为什么不让我用GetTickCount64 ,(Vista才支持),不打开下面注释的原因是，编译会通过了，但你也没法用,XP和WINSERVER2003都无法使用，
    //VISTA,WINSERVER2008的_WIN32_WINNT都是0x0600
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1
    now_cpu_tick = ::GetTickCount64();
#else

    //GetTickCount返回的是一个32位的整数milliseconds ，但是，DWORD其实只有49多天的长度,所以只有蛋疼的封装了
    unsigned int cpu_tick =  static_cast<unsigned int>(::GetTickCount());

    //用了静态变量，防止重入
    ZCE_Thread_Light_Mutex lock_static_var;
    ZCE_Thread_Light_Mutex::LOCK_GUARD guard(lock_static_var);

    static unsigned int one_period_tick = 0;
    static uint64_t cpu_tick_count = 0;

    //如果是GetTickCount没有归零，新数据肯定大于旧数据
    if (one_period_tick  <= cpu_tick)
    {
        one_period_tick = cpu_tick;
        cpu_tick_count = (0xFFFFFFFF00000000 & cpu_tick_count) + one_period_tick;
    }
    //理论上else这儿都是转了1圈了
    else
    {
        cpu_tick_count +=  0xFFFFFFFF - one_period_tick + cpu_tick;
        one_period_tick = cpu_tick;
    }

    now_cpu_tick = cpu_tick_count;

#endif //

    up_time.tv_sec = static_cast<long>( now_cpu_tick / SEC_PER_MSEC);
    up_time.tv_usec = static_cast<long>( now_cpu_tick % SEC_PER_MSEC * MSEC_PER_USEC);

    return up_time;

#elif defined (ZCE_OS_LINUX)
    //倒霉的发现LINUX很多版本都没有支持这个gethrtime函数，我靠，，，，，
    struct timespec sp;
    timeval up_time;
    int ret = ::clock_gettime(CLOCK_MONOTONIC, &sp);

    if (ret == 0)
    {
        up_time = ZCE_LIB::make_timeval(&sp);
    }
    else
    {
        ZCE_LOG(RS_ERROR, "::clock_gettime(CLOCK_MONOTONIC, &sp) ret != 0,fail.ret = %d lasterror = %d", ret, ZCE_LIB::last_error());
        up_time.tv_sec = 0;
        up_time.tv_usec = 0;
    }

    return up_time;
#endif
}

//
//得到当前的系统时间字符串输出
const char *ZCE_LIB::timestamp (char *str_date_time, size_t datetime_strlen)
{
    timeval now_time_val (ZCE_LIB::gettimeofday());
    return ZCE_LIB::timestamp (&now_time_val, str_date_time, datetime_strlen);
}

//将参数timeval的值作为的时间格格式化后输出打印出来
const char *ZCE_LIB::timestamp (const timeval *timeval, char *str_date_time, size_t datetime_strlen)
{
    ZCE_ASSERT(datetime_strlen > ZCE_LIB::TIMESTR_ISO_USEC_LEN);

    //转换为语句
    time_t now_time = timeval->tv_sec;
    tm tm_data;
    ZCE_LIB::localtime_r(&now_time, &tm_data);

    //上面的两行代码我原来用的是这一行代码，但是会出现崩溃(Windows下的断言),你知道为啥吗，呵呵
    //tm now_tm =*localtime(static_cast<time_t *>(&(timeval->tv_sec)));

    snprintf (str_date_time,
              datetime_strlen,
              "%4d-%02d-%02d %02d:%02d:%02d.%06ld",
              tm_data.tm_year + 1900,
              tm_data.tm_mon + 1,
              tm_data.tm_mday,
              tm_data.tm_hour,
              tm_data.tm_min,
              tm_data.tm_sec,
              timeval->tv_usec);

    return str_date_time;
}


int ZCE_LIB::gettimezone()
{
#if defined ZCE_OS_WINDOWS
    return _timezone;
#else
    return timezone;
#endif
}

//将参数timeval的值作为的时间格格式化后输出打印出来
//可以控制各种格式输出
//如果成功，返回参数字符串str_date_time，如果失败返回NULL
const char *ZCE_LIB::timeval_to_str(const timeval *timeval,
                                    char *str_date_time,
                                    size_t str_len,
                                    bool uct_time,
                                    TIME_STR_FORMAT_TYPE fmt
                                   )
{
    //这个实现没有使用strftime的原因是，我对输出精度可能有更高的要求，
    static const char *DAY_OF_WEEK_NAME[] =
    {
        ("Sun"),
        ("Mon"),
        ("Tue"),
        ("Wed"),
        ("Thu"),
        ("Fri"),
        ("Sat")
    };

    static const char *MONTH_NAME[] =
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
    time_t now_time = timeval->tv_sec;
    tm tm_data;

    if (uct_time)
    {
        //Email Date域是带有时区的输出，不用UTC
        if ( ZCE_LIB::TIME_STRFMT_EMAIL_DATE == fmt)
        {
            ZCE_ASSERT(false);
            errno = EINVAL;
            return NULL;
        }
        ZCE_LIB::gmtime_r(&now_time, &tm_data);
    }
    else
    {
        //HTTPHEAD只用GMT时间
        if ( ZCE_LIB::TIME_STRFMT_HTTP_GMT == fmt)
        {
            ZCE_ASSERT(false);
            errno = EINVAL;
            return NULL;
        }
        ZCE_LIB::localtime_r(&now_time, &tm_data);
    }


    //如果是压缩格式，精度到天，20100910
    if ( ZCE_LIB::TIME_STRFMT_COMPACT_DAY == fmt)
    {
        //参数保护和检查
        ZCE_ASSERT(str_len > ZCE_LIB::TIMESTR_COMPACT_SEC_LEN);
        if (str_len <= ZCE_LIB::TIMESTR_COMPACT_SEC_LEN)
        {
            return NULL;
        }

        snprintf(str_date_time,
                 str_len,
                 "%4d%02d%02d%02d%02d%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec);

    }
    //如果是压缩格式，精度到秒，20100910100318
    else if ( ZCE_LIB::TIME_STRFMT_COMPACT_SEC == fmt)
    {
        ZCE_ASSERT(str_len > ZCE_LIB::TIMESTR_COMPACT_DAY_LEN);
        if (str_len <= ZCE_LIB::TIMESTR_COMPACT_DAY_LEN)
        {
            return NULL;
        }

        snprintf(str_date_time,
                 str_len,
                 "%4d%02d%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday);
    }
    //2010-09-10
    else if ( ZCE_LIB::TIME_STRFMT_ISO_DAY == fmt )
    {
        ZCE_ASSERT(str_len > ZCE_LIB::TIMESTR_ISO_DAY_LEN);
        if (str_len <= ZCE_LIB::TIMESTR_ISO_DAY_LEN)
        {
            return NULL;
        }

        snprintf(str_date_time,
                 str_len,
                 "%4d-%02d-%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday);
    }
    //2010-09-10 10:03:18
    else if (ZCE_LIB::TIME_STRFMT_ISO_SEC == fmt)
    {
        ZCE_ASSERT(str_len > ZCE_LIB::TIMESTR_ISO_SEC_LEN);
        if (str_len <= ZCE_LIB::TIMESTR_ISO_SEC_LEN)
        {
            return NULL;
        }

        snprintf(str_date_time,
                 str_len,
                 "%4d-%02d-%02d %02d:%02d:%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec);
    }
    //2010-09-10 10:03:18.100190
    else if ( ZCE_LIB::TIME_STRFMT_ISO_USEC == fmt )
    {
        //参数保护和检查
        ZCE_ASSERT(str_len > ZCE_LIB::TIMESTR_ISO_USEC_LEN);
        if (str_len <= ZCE_LIB::TIMESTR_ISO_USEC_LEN)
        {
            return NULL;
        }

        snprintf(str_date_time,
                 str_len,
                 "%4d-%02d-%02d %02d:%02d:%02d.%06ld",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec,
                 timeval->tv_usec);
    }
    //Fri Aug 24 2002 07:43:05
    else if ( ZCE_LIB::TIME_STRFMT_US_SEC == fmt)
    {
        ZCE_ASSERT(str_len > TIMESTR_US_SEC_LEN);
        if (str_len <= TIMESTR_US_SEC_LEN)
        {
            return NULL;
        }

        snprintf(str_date_time,
                 str_len,
                 "%3s %3s %2d %04d %02d:%02d:%02d",
                 DAY_OF_WEEK_NAME[tm_data.tm_wday],
                 MONTH_NAME[tm_data.tm_mon],
                 tm_data.tm_mday,
                 tm_data.tm_year + 1900,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec);
    }
    //Fri Aug 24 2002 07:43:05.100190
    else if ( ZCE_LIB::TIME_STRFMT_US_USEC == fmt)
    {
        ZCE_ASSERT(str_len > TIMESTR_US_USEC_LEN);
        if (str_len <= TIMESTR_US_USEC_LEN)
        {
            return NULL;
        }

        snprintf(str_date_time,
                 str_len,
                 "%3s %3s %2d %04d %02d:%02d:%02d.%06d",
                 DAY_OF_WEEK_NAME[tm_data.tm_wday],
                 MONTH_NAME[tm_data.tm_mon],
                 tm_data.tm_mday,
                 tm_data.tm_year + 1900,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 (int)tm_data.tm_sec,
                 (int)timeval->tv_usec);
    }
    //Thu, 26 Nov 2009 13:50:19 GMT
    else if ( ZCE_LIB::TIME_STRFMT_HTTP_GMT == fmt )
    {
        ZCE_ASSERT(str_len > TIMESTR_HTTP_GMT_LEN);
        if (str_len <= TIMESTR_HTTP_GMT_LEN)
        {
            return NULL;
        }

        snprintf(str_date_time,
                 str_len,
                 "%3s, %2d %3s %04d %02d:%02d:%02d GMT",
                 DAY_OF_WEEK_NAME[tm_data.tm_wday],
                 tm_data.tm_mday,
                 MONTH_NAME[tm_data.tm_mon],
                 tm_data.tm_year + 1900,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec);
    }
    //Fri, 08 Nov 2002 09:42:22 +0800
    else if (ZCE_LIB::TIME_STRFMT_EMAIL_DATE == fmt)
    {
        ZCE_ASSERT(str_len > TIMESTR_EMAIL_DATE_LEN);
        if (str_len <= TIMESTR_EMAIL_DATE_LEN)
        {
            return NULL;
        }

        //注意timezone理论上需要tzset()函数初始化
        int tz = ZCE_LIB::gettimezone();
        snprintf(str_date_time,
                 str_len,
                 "%3s, %2d %3s %04d %02d:%02d:%02d %+05d",
                 DAY_OF_WEEK_NAME[tm_data.tm_wday],
                 tm_data.tm_mday,
                 MONTH_NAME[tm_data.tm_mon],
                 tm_data.tm_year + 1900,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec,
                 tz / 360 * 10);
    }
    //没有实现，参数错误
    else
    {
        ZCE_ASSERT(false);
        errno = EINVAL;
        return NULL;
    }

    return str_date_time;

}


//通过字符串翻译得到tm时间结构
void ZCE_LIB::str_to_tm(const char *strtm,
                        TIME_STR_FORMAT_TYPE fmt,
                        tm *ptr_tm,
                        time_t *usec,
                        int *tz)
{
    static const char *MONTH_NAME[] =
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
	static const time_t CHARATER_ZERO_TIME_T = '0';

    ZCE_ASSERT(strtm && ptr_tm);
    if (usec != NULL)
    {
        *usec = 0;
    }
    if (tz != NULL)
    {
        *tz = 0;
    }

    ptr_tm->tm_isdst = 0;
    if (ZCE_LIB::TIME_STRFMT_COMPACT_DAY == fmt ||
        ZCE_LIB::TIME_STRFMT_COMPACT_SEC == fmt)
    {

        ptr_tm->tm_year = ((*strtm) - '0') * 1000
                          + (*(strtm + 1) - '0') * 100
                          + (*(strtm + 2) - '0') * 10
                          + (*(strtm + 3) - '0') - 1900;
        ptr_tm->tm_mon = (*(strtm + 4) - '0') * 10
                         + (*(strtm + 5) - '0') - 1;
        ptr_tm->tm_mday = (*(strtm + 6) - '0') * 10
                          + (*(strtm + 7) - '0');
        ptr_tm->tm_hour = 0;
        ptr_tm->tm_min = 0;
        ptr_tm->tm_sec = 0;

        //如果输入字符串精度到微秒
        if (ZCE_LIB::TIME_STRFMT_COMPACT_SEC == fmt)
        {
            ptr_tm->tm_hour = (*(strtm + 8) - '0') * 10
                              + (*(strtm + 9) - '0');
            ptr_tm->tm_min = (*(strtm + 10) - '0') * 10
                             + (*(strtm + 11) - '0');
            ptr_tm->tm_sec = (*(strtm + 12) - '0') * 10
                             + (*(strtm + 13) - '0');
        }
    }
    else if (ZCE_LIB::TIME_STRFMT_ISO_DAY == fmt ||
             ZCE_LIB::TIME_STRFMT_ISO_SEC == fmt ||
             ZCE_LIB::TIME_STRFMT_ISO_USEC == fmt)
    {
        ptr_tm->tm_year = ((*strtm) - '0') * 1000
                          + (*(strtm + 1) - '0') * 100
                          + (*(strtm + 2) - '0') * 10
                          + (*(strtm + 3) - '0')
                          - 1900;
        ptr_tm->tm_mon = (*(strtm + 5) - '0') * 10
                         + (*(strtm + 6) - '0')
                         - 1;
        ptr_tm->tm_mday = (*(strtm + 8) - '0') * 10
                          + (*(strtm + 9) - '0');
        ptr_tm->tm_hour = 0;
        ptr_tm->tm_min = 0;
        ptr_tm->tm_sec = 0;

        //如果输入字符串精度到微秒
        if (ZCE_LIB::TIME_STRFMT_ISO_SEC == fmt ||
            ZCE_LIB::TIME_STRFMT_ISO_USEC == fmt)
        {
            ptr_tm->tm_hour = (*(strtm + 11) - '0') * 10
                              + (*(strtm + 12) - '0');
            ptr_tm->tm_min = (*(strtm + 14) - '0') * 10
                             + (*(strtm + 15) - '0');
            ptr_tm->tm_sec = (*(strtm + 17) - '0') * 10
                             + (*(strtm + 18) - '0');

        }
        if (ZCE_LIB::TIME_STRFMT_ISO_USEC == fmt &&
            usec != NULL)
        {
			*usec = ((*(strtm + 20)) - CHARATER_ZERO_TIME_T) * 100000
				+ ((*(strtm + 21) - CHARATER_ZERO_TIME_T)) * 10000
				+ ((*(strtm + 22) - CHARATER_ZERO_TIME_T)) * 1000
				+ ((*(strtm + 23) - CHARATER_ZERO_TIME_T)) * 100
				+ ((*(strtm + 24) - CHARATER_ZERO_TIME_T)) * 10
				+ ((*(strtm + 25) - CHARATER_ZERO_TIME_T));
			
        }
    }
    else if (ZCE_LIB::TIME_STRFMT_US_SEC == fmt ||
             ZCE_LIB::TIME_STRFMT_US_USEC == fmt)
    {
        //Fri Aug 24 2002 07:43:05.100190
        char mon_str[4];
        mon_str[0] = strtm[4];
        mon_str[1] = strtm[5];
        mon_str[2] = strtm[6];
        mon_str[3] = '\0';
        ptr_tm->tm_mon = 0;

        for (int i = 0; i < 12; i++)
        {
            if (strncasecmp(mon_str, MONTH_NAME[i], 3) == 0)
            {
                ptr_tm->tm_mon = i;
            }
        }
        ptr_tm->tm_mday = (*(strtm + 8) - '0') * 10
                          + (*(strtm + 9) - '0') ;
        ptr_tm->tm_year = ((*strtm + 11) - '0') * 1000
                          + (*(strtm + 12) - '0') * 100
                          + (*(strtm + 13) - '0') * 10
                          + (*(strtm + 14) - '0')
                          - 1900;
        ptr_tm->tm_hour = (*(strtm + 16) - '0') * 10
                          + (*(strtm + 17) - '0');
        ptr_tm->tm_min = (*(strtm + 19) - '0') * 10
                         + (*(strtm + 20) - '0');
        ptr_tm->tm_sec = (*(strtm + 22) - '0') * 10
                         + (*(strtm + 23) - '0');
        //如果输入字符串精度到微秒
        if (ZCE_LIB::TIME_STRFMT_US_USEC == fmt &&
            usec != NULL)
        {
            *usec = (*(strtm + 25) - CHARATER_ZERO_TIME_T) * 100000
                    + (*(strtm + 26) - CHARATER_ZERO_TIME_T) * 10000
                    + (*(strtm + 27) - CHARATER_ZERO_TIME_T) * 1000
                    + (*(strtm + 28) - CHARATER_ZERO_TIME_T) * 100
                    + (*(strtm + 29) - CHARATER_ZERO_TIME_T) * 10
                    + (*(strtm + 30) - CHARATER_ZERO_TIME_T);
        }
    }
    //Thu, 26 Nov 2009 13:05:19 GMT
    else if (ZCE_LIB::TIME_STRFMT_HTTP_GMT == fmt)
    {

        char mon_str[4];
        mon_str[0] = strtm[5];
        mon_str[1] = strtm[6];
        mon_str[2] = strtm[7];
        mon_str[3] = '\0';
        ptr_tm->tm_mon = 0;

        for (int i = 0; i < 12; i++)
        {
            if (strncasecmp(mon_str, MONTH_NAME[i], 3) == 0)
            {
                ptr_tm->tm_mon = i;
            }
        }
        ptr_tm->tm_mday = (*(strtm + 9) - '0') * 10
                          + (*(strtm + 10) - '0');
        ptr_tm->tm_year = ((*strtm + 12) - '0') * 1000
                          + (*(strtm + 13) - '0') * 100
                          + (*(strtm + 14) - '0') * 10
                          + (*(strtm + 15) - '0')
                          - 1900;
        ptr_tm->tm_hour = (*(strtm + 17) - '0') * 10
                          + (*(strtm + 18) - '0');
        ptr_tm->tm_min = (*(strtm + 20) - '0') * 10
                         + (*(strtm + 21) - '0');
        ptr_tm->tm_sec = (*(strtm + 23) - '0') * 10
                         + (*(strtm + 24) - '0');
    }
    else if (ZCE_LIB::TIME_STRFMT_EMAIL_DATE == fmt)
    {
        char mon_str[4];
        mon_str[0] = strtm[5];
        mon_str[1] = strtm[6];
        mon_str[2] = strtm[7];
        mon_str[3] = '\0';
        ptr_tm->tm_mon = 0;

        for (int i = 0; i < 12; i++)
        {
            if (strncasecmp(mon_str, MONTH_NAME[i], 3) == 0)
            {
                ptr_tm->tm_mon = i;
            }
        }
        ptr_tm->tm_mday = (*(strtm + 9) - '0') * 10
                          + (*(strtm + 10) - '0');
        ptr_tm->tm_year = ((*strtm + 12) - '0') * 1000
                          + (*(strtm + 13) - '0') * 100
                          + (*(strtm + 14) - '0') * 10
                          + (*(strtm + 15) - '0')
                          - 1900;
        ptr_tm->tm_hour = (*(strtm + 17) - '0') * 10
                          + (*(strtm + 18) - '0');
        ptr_tm->tm_min = (*(strtm + 20) - '0') * 10
                         + (*(strtm + 21) - '0');
        ptr_tm->tm_sec = (*(strtm + 23) - '0') * 10
                         + (*(strtm + 24) - '0');

        if (tz)
        {
            int time_zone = (*(strtm + 27) - '0') * 1000
                            + (*(strtm + 28) - '0') * 100
                            + (*(strtm + 28) - '0') * 10
                            + (*(strtm + 30) - '0');
            if (*(strtm + 26) == '-')
            {
                time_zone = -1 * time_zone;
            }
            *tz = time_zone / 10 * 360;
        }

    }
    else
    {
        ZCE_ASSERT(false);
        errno = EINVAL;
        return;
    }
}


//从字符串转换得到时间time_t函数
int ZCE_LIB::str_to_timeval(const char *strtm,
                            TIME_STR_FORMAT_TYPE fmt,
                            bool uct_time,
                            timeval *tval)
{
    //
    if (!uct_time && ZCE_LIB::TIME_STRFMT_HTTP_GMT == fmt)
    {
        ZCE_ASSERT(false);
        errno = EINVAL;
        return -1;
    }

    if ( ZCE_LIB::TIME_STRFMT_EMAIL_DATE == fmt)
    {
        uct_time = true;
    }
    struct tm tm_value;
    time_t usec = 0;
    int tz = 0;
    ZCE_LIB::str_to_tm(strtm,
                       fmt,
                       &tm_value,
                       &usec,
                       &tz);

    if (uct_time)
    {

#if defined ZCE_OS_WINDOWS
        tval->tv_sec = static_cast<long>( ZCE_LIB::timegm(&tm_value));
        tval->tv_usec = static_cast<long>(usec);
#else
        tval->tv_sec = ZCE_LIB::timegm(&tm_value);
        tval->tv_usec = usec;
#endif
    }
    else
    {

#if defined ZCE_OS_WINDOWS
        tval->tv_sec = static_cast<long>(ZCE_LIB::timelocal(&tm_value));
        tval->tv_usec = static_cast<long>(usec);
#else
        tval->tv_sec = ZCE_LIB::timelocal(&tm_value);
        tval->tv_usec = usec;
#endif
    }
    //转换失败，表示字符串有问题
    if (-1 == tval->tv_sec )
    {
        errno = EINVAL;
        return -1;
    }

    if (ZCE_LIB::TIME_STRFMT_EMAIL_DATE == fmt)
    {
        tval->tv_sec += tz;
    }

    return 0;
}


///本地时间字符串转换为time_t
int ZCE_LIB::localtimestr_to_time_t(const char *localtime_str,
    TIME_STR_FORMAT_TYPE fmt,
    time_t *time_t_val)
{
    timeval tval;
    int ret = str_to_timeval(localtime_str, fmt, false, &tval);
    if (ret != 0)
    {
        return ret;
    }
    *time_t_val = tval.tv_sec;
    return 0;
}

//----------------------------------------------------------------------------------------------------
const timeval ZCE_LIB::timeval_zero()
{
    timeval zero_time;
    zero_time.tv_sec = 0;
    zero_time.tv_usec = 0;
    return zero_time;
}

//将tv修正为0
void ZCE_LIB::timeval_clear(timeval &tv)
{
    tv.tv_sec = 0;
    tv.tv_sec = 0;
}

//计算总计是多少毫秒
uint64_t ZCE_LIB::total_milliseconds(const timeval &tv)
{
    //这里的参数就是因为需要转换到毫秒所折腾的。
    return static_cast<uint64_t>(tv.tv_sec) * SEC_PER_MSEC + tv.tv_usec / MSEC_PER_USEC;
}

//计算timeval内部总计是多少微秒10-6
uint64_t ZCE_LIB::total_microseconds(const timeval &tv)
{
    //这里的参数就是因为需要转换到毫秒所折腾的。
    return static_cast<uint64_t>(tv.tv_sec) * SEC_PER_USEC + tv.tv_usec ;
}

//比较时间是否一致,如果一致返回0，left大，返回整数，right大返回负数
int ZCE_LIB::timeval_compare(const  timeval &left, const timeval &right)
{
    if ( left.tv_sec != right.tv_sec )
    {
        return left.tv_sec - right.tv_sec;
    }
    else
    {
        return left.tv_usec - right.tv_usec;
    }
}

//对两个时间进行想减,没有做复杂的溢出检查
const timeval ZCE_LIB::timeval_add(const timeval &left, const timeval &right)
{
    timeval plus_time_val;
    plus_time_val.tv_sec = left.tv_sec + right.tv_sec;
    plus_time_val.tv_usec = left.tv_usec + right.tv_usec;

    if (plus_time_val.tv_usec > SEC_PER_USEC)
    {
        plus_time_val.tv_sec += plus_time_val.tv_usec / SEC_PER_USEC;
        plus_time_val.tv_usec = plus_time_val.tv_usec % SEC_PER_USEC;
    }

    return plus_time_val;

}

//对两个时间进行相加,没有做复杂的溢出检查,尽量返回>0的数值
//safe == true保证返回值>=0,
const  timeval ZCE_LIB::timeval_sub(const timeval &left, const  timeval &right, bool safe)
{
    int64_t left_usec_val = (int64_t)left.tv_sec * SEC_PER_USEC + left.tv_usec;
    int64_t right_usec_val = (int64_t)right.tv_sec * SEC_PER_USEC + right.tv_usec;

    //用64位作为基准去减
    int64_t minus_usec_val = left_usec_val - right_usec_val;

    timeval minus_time_val;

    // >0 或者标识无须调整就调整成0
    if (minus_usec_val >= 0 || (minus_usec_val < 0 && safe == false))
    {
        minus_time_val.tv_sec = static_cast<long>( minus_usec_val / SEC_PER_USEC);
        minus_time_val.tv_usec = static_cast<long>(  minus_usec_val % SEC_PER_USEC);
    }
    else
    {
        minus_time_val.tv_sec = 0;
        minus_time_val.tv_usec = 0;
    }

    return minus_time_val;
}


//检查这个TIMEVALUE是否还有剩余的时间
void ZCE_LIB::timeval_adjust(timeval &tv)
{
    int64_t tv_usec_val = (int64_t)tv.tv_sec * SEC_PER_USEC + tv.tv_usec;

    tv.tv_sec = static_cast<long>( tv_usec_val / SEC_PER_USEC );
    tv.tv_usec = static_cast<long>( tv_usec_val % SEC_PER_USEC);
}

//检查这个TIMEVALUE是否还有剩余的时间
bool ZCE_LIB::timeval_havetime(const timeval &tv)
{
    int64_t tv_usec_val = (int64_t)tv.tv_sec * SEC_PER_USEC + tv.tv_usec;

    if (tv_usec_val > 0)
    {
        return true;
    }

    return false;
}

//这只timeval这个结构
const timeval ZCE_LIB::make_timeval(time_t sec, time_t usec)
{
    timeval to_timeval;
#if defined (ZCE_OS_WINDOWS)
    to_timeval.tv_sec = static_cast<long>( sec);
    to_timeval.tv_usec = static_cast<long>( usec);
#elif defined (ZCE_OS_LINUX)
    to_timeval.tv_sec = sec;
    to_timeval.tv_usec =  usec ;
#endif

    return to_timeval;
}

//转换得到timeval这个结构
const timeval ZCE_LIB::make_timeval(std::clock_t clock_value)
{
    timeval to_timeval;

    to_timeval.tv_sec = clock_value /  CLOCKS_PER_SEC;
    clock_t remain_val = clock_value %  CLOCKS_PER_SEC;

	// Windows平台下tv_sec被定义成long,所以需要转换
    to_timeval.tv_usec = static_cast<decltype(to_timeval.tv_usec)>(
		(remain_val  * SEC_PER_USEC / CLOCKS_PER_SEC)) ;

    return to_timeval;
}

//转换得到timeval这个结构
const timeval ZCE_LIB::make_timeval(const ::timespec *timespec_val)
{
    //每次我自己看见这段代码都会疑惑好半天，实际我没有错，好吧，写点注释把，
    //NSEC 纳秒 10-9s
    //USEC 微秒 10-6s
    timeval to_timeval;

    to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>( timespec_val->tv_sec);
    to_timeval.tv_usec = timespec_val->tv_nsec / USEC_PER_NSEC;
    return to_timeval;
}

const timeval ZCE_LIB::make_timeval(const std::chrono::hours& val)
{
	timeval to_timeval;
	to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>(val.count() * ZCE_LIB::ONE_HOUR_SECONDS);
	to_timeval.tv_usec = 0;
	return to_timeval;
}
const timeval ZCE_LIB::make_timeval(const std::chrono::minutes& val)
{
	timeval to_timeval;
	to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>(val.count() * ZCE_LIB::ONE_MINUTE_SECONDS);
	to_timeval.tv_usec = 0;
	return to_timeval;
}
const timeval ZCE_LIB::make_timeval(const std::chrono::seconds& val)
{
	timeval to_timeval;
	to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>(val.count());
	to_timeval.tv_usec = 0;
	return to_timeval;
}
const timeval ZCE_LIB::make_timeval(const std::chrono::milliseconds& val)
{
	timeval to_timeval;
	to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>(val.count() / SEC_PER_MSEC);
	to_timeval.tv_usec = (val.count() % SEC_PER_MSEC) *1000;
	return to_timeval;
}
const timeval ZCE_LIB::make_timeval(const std::chrono::microseconds& val)
{
	timeval to_timeval;
	to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>( val.count() / SEC_PER_USEC);
	to_timeval.tv_usec = static_cast<decltype(to_timeval.tv_usec)>((val.count() % SEC_PER_USEC));
	return to_timeval;
}
const timeval ZCE_LIB::make_timeval(const std::chrono::nanoseconds& val)
{
	timeval to_timeval;
	to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>(val.count() / SEC_PER_NSEC);
	to_timeval.tv_usec = static_cast<decltype(to_timeval.tv_usec)>((val.count() % SEC_PER_NSEC)/1000);
	return to_timeval;
}

//将CPP11的time_point的数据结构转换得到timeval结构
const timeval ZCE_LIB::make_timeval(const std::chrono::system_clock::time_point& val)
{
	std::chrono::nanoseconds tval = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(val.time_since_epoch());
	return ZCE_LIB::make_timeval(tval);
}
const timeval ZCE_LIB::make_timeval(const std::chrono::steady_clock::time_point& val)
{
	std::chrono::nanoseconds tval = 
		std::chrono::duration_cast<std::chrono::nanoseconds>(val.time_since_epoch());
	return ZCE_LIB::make_timeval(tval);
}

#if defined (ZCE_OS_WINDOWS)

//转换FILETIME到timeval
const timeval ZCE_LIB::make_timeval(const FILETIME *file_time)
{
    timeval to_timeval;

    ULARGE_INTEGER ui;
    ui.LowPart = file_time->dwLowDateTime;
    ui.HighPart = file_time->dwHighDateTime;

    //The FILETIME structure is a 64-bit value representing the number of
    //100-nanosecond intervals since January 1, 1601.

    //得到time_t部分
    to_timeval.tv_sec = static_cast<long>((ui.QuadPart - 116444736000000000) / 10000000);
    //得到微秒部分，FILETIME存放的是100-nanosecond
    to_timeval.tv_usec = static_cast<long>(((ui.QuadPart - 116444736000000000) % 10000000) / 10);

    return to_timeval;
}

//转换SYSTEMTIME到timeval
const timeval ZCE_LIB::make_timeval(const SYSTEMTIME *system_time)
{
    FILETIME ft;
    ::SystemTimeToFileTime(system_time, &ft);
    return make_timeval(&ft);
}

//转换FILETIME到timeval,这个是把FILETIME当着一个时长看待进行的
const timeval ZCE_LIB::make_timeval2(const FILETIME *file_time)
{
    timeval to_timeval;

    ULARGE_INTEGER ui;
    ui.LowPart = file_time->dwLowDateTime;
    ui.HighPart = file_time->dwHighDateTime;

    //FILETIME的单位是100-nanosecond
    to_timeval.tv_sec = static_cast<long>((ui.QuadPart) / 10000000);
    to_timeval.tv_usec = static_cast<long>(((ui.QuadPart) % 10000000) / 10);

    return to_timeval;
}

#endif

//----------------------------------------------------------------------------------------------------
//转换得到timeval这个结构
const ::timespec ZCE_LIB::make_timespec(const ::timeval *timeval_val)
{
    //每次我自己看见这段代码都会疑惑好半天，实际我没有错，好吧，写点注释把，
    //NSEC 纳秒 10-9s
    //USEC 微秒 10-6s
    ::timespec to_timespec;

    to_timespec.tv_sec =  timeval_val->tv_sec;
    to_timespec.tv_nsec = timeval_val->tv_usec * USEC_PER_NSEC;

    return to_timespec;
}

//计算总计是多少毫秒
uint64_t ZCE_LIB::total_milliseconds(const ::timespec &ts)
{
    //这里的参数就是因为需要转换到毫秒所折腾的。
    return static_cast<uint64_t>(ts.tv_sec) * SEC_PER_MSEC + ts.tv_nsec / MSEC_PER_NSEC;
}

//----------------------------------------------------------------------------------------------------
//休眠函数
//秒得休眠函数
int ZCE_LIB::sleep (uint32_t seconds)
{
#if defined (ZCE_OS_WINDOWS)
    ::Sleep (seconds * SEC_PER_MSEC);
    return 0;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::sleep (seconds);
#endif //
}

//休息一个timeval的时间
int ZCE_LIB::sleep (const timeval &tv)
{
    //
#if defined (ZCE_OS_WINDOWS)
    ::Sleep (tv.tv_sec * SEC_PER_MSEC + tv.tv_usec / MSEC_PER_USEC );
    return 0;
#endif //

#if defined (ZCE_OS_LINUX)
    return ::usleep (tv.tv_sec * SEC_PER_USEC + tv.tv_usec );
#endif //
}

//微秒的休眠函数
int ZCE_LIB::usleep (unsigned long usec)
{
#if defined (ZCE_OS_WINDOWS)
    ::Sleep (usec / MSEC_PER_USEC);
    return 0;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::usleep (usec);
#endif //
}

//----------------------------------------------------------------------------------------

uint64_t ZCE_LIB::rdtsc()
{

    uint64_t tsc_value = 0;

#if defined (ZCE_WIN32) && !defined (ZCE_WIN64)

    uint32_t hiword, loword;
    //#define rdtsc __asm __emit 0fh __asm __emit 031h
    //#define cpuid __asm __emit 0fh __asm __emit 0a2h
    __asm
    {
        //CPUID
        __emit 0fh
        __emit 0a2h
        //TSC
        __emit 0fh
        __emit 031h
        //读取edx，eax，
        mov hiword , edx
        mov loword , eax
    }
    tsc_value = (uint64_t( hiword ) << 32) + loword ;
    tsc_value = __rdtsc();
#elif defined (ZCE_WIN64)

    int registers[4];
    __cpuid(registers, 0);
    tsc_value = __rdtsc();

#elif defined (ZCE_OS_LINUX)

    uint32_t hiword, loword;
    asm("cpuid");
    asm volatile("rdtsc" : "=a" (hiword), "=d" (loword));
    tsc_value = (uint64_t( hiword ) << 32) + loword ;
#endif

    return tsc_value ;
}
