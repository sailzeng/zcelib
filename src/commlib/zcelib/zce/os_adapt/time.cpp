#include "zce/predefine.h"
#include "zce/comm/common.h"
#include "zce/os_adapt/define.h"
#include "zce/os_adapt/mutex.h"
#include "zce/lock/thread_mutex.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/time.h"

//这个代码里面写了大量的long主要是为了兼容,迫不得已

//非标准函数，得到服务器启动的时间，gethrtime单位比较怪，类似得到CPU从启动后到现在的TICK的时间，
//缺点：如果两次调用之间的时间如果过长，超过49天，我无法保证你得到准确的值
//你老不要49天就只调用一次这个函数呀，那样我保证不了你的TICK的效果，你老至少每天调用一次吧。
//内部为了上一次调用的时间，用了static 变量，又为了保护static，给了锁，
int zce::steady_clock(timeval* tv)
{
#if defined (ZCE_OS_WINDOWS)

    //注意GetTickCount64和GetTickCount返回的都是milliseconds，不是CPU Tick
    uint64_t now_cpu_tick = 0;

    //为什么不让我用GetTickCount64 ,(Vista才支持),不打开下面注释的原因是，编译会通过了，但你也没法用,XP和WINSERVER2003都无法使用，
    //VISTA,WINSERVER2008的_WIN32_WINNT都是0x0600
#if defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008
    now_cpu_tick = ::GetTickCount64();
#else

    //GetTickCount返回的是一个32位的整数milliseconds ，但是，DWORD其实只有49多天的长度,所以只有蛋疼的封装了
    unsigned int cpu_tick = static_cast<unsigned int>(::GetTickCount());

    //用了静态变量，防止重入
    thread_mutex lock_static_var;
    thread_mutex::LOCK_GUARD guard(lock_static_var);

    static unsigned int one_period_tick = 0;
    static uint64_t cpu_tick_count = 0;

    //如果是GetTickCount没有归零，新数据肯定大于旧数据
    if (one_period_tick <= cpu_tick)
    {
        one_period_tick = cpu_tick;
        cpu_tick_count = (0xFFFFFFFF00000000 & cpu_tick_count) + one_period_tick;
    }
    //理论上else这儿都是转了1圈了
    else
    {
        cpu_tick_count += 0xFFFFFFFF - one_period_tick + cpu_tick;
        one_period_tick = cpu_tick;
    }
    now_cpu_tick = cpu_tick_count;

#endif //

    tv->tv_sec = static_cast<long>(now_cpu_tick / SEC_PER_MSEC);
    tv->tv_usec = static_cast<long>(now_cpu_tick % SEC_PER_MSEC * MSEC_PER_USEC);

    return 0;

#elif defined (ZCE_OS_LINUX)
    //倒霉的发现LINUX很多版本都没有支持这个gethrtime函数，我靠，，，，，
    struct timespec sp;
    int ret = ::clock_gettime(CLOCK_MONOTONIC,&sp);

    if (ret == 0)
    {
        *tv = zce::make_timeval(&sp);
    }
    else
    {
        ZCE_LOG(RS_ERROR,"::clock_gettime(CLOCK_MONOTONIC, &sp) ret != 0,fail.ret = %d "
                "lasterror = %d",ret,zce::last_error());
        tv->tv_sec = 0;
        tv->tv_usec = 0;
    }

    return 0;
#endif
}

//
uint64_t zce::clock_ms(void)
{
#if defined (ZCE_OS_WINDOWS)
    return  ::GetTickCount64();
#elif defined (ZCE_OS_LINUX)
    struct timespec sp;
    int ret = ::clock_gettime(CLOCK_MONOTONIC,&sp);
    if (ret == 0)
    {
        return (uint64_t)sp.tv_sec * SEC_PER_MSEC + (uint64_t)sp.tv_nsec / MSEC_PER_NSEC;
    }
    else
    {
        ZCE_LOG(RS_ERROR,"::clock_gettime(CLOCK_MONOTONIC, &sp) ret != 0,fail.ret = %d lasterror = %d",
                ret,
                zce::last_error());
        return 0;
    }
#endif
}

//
//得到当前的系统时间字符串输出
const char* zce::timestamp(char* str_date_time,size_t datetime_strlen)
{
    timeval now_time_val(zce::gettimeofday());
    return zce::timestamp(&now_time_val,str_date_time,datetime_strlen);
}

//将参数timeval的值作为的时间格格式化后输出打印出来
const char* zce::timestamp(const timeval* timeval,char* str_date_time,size_t datetime_strlen)
{
    ZCE_ASSERT(datetime_strlen > zce::TS_ISO_USEC_LEN);

    //转换为语句
    time_t now_time = timeval->tv_sec;
    tm tm_data;
    zce::localtime_r(&now_time,&tm_data);

    //上面的两行代码我原来用的是这一行代码，但是会出现崩溃(Windows下的断言),你知道为啥吗，呵呵
    //tm now_tm =*localtime(static_cast<time_t *>(&(timeval->tv_sec)));

    snprintf(str_date_time,
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

int zce::gettimezone()
{
#if defined ZCE_OS_WINDOWS
    return _timezone;
#else
    return timezone;
#endif
}

//将参数timeval的值作为的时间格格式化后输出打印出来
//可以控制各种格式输出
//如果成功，返回参数字符串str_date_time，如果失败返回nullptr
//timeval->tv_usec 千万不要溢出，会导致不可以预期问题
const char* zce::timeval_to_str(const timeval* timeval,
                                char* str_date_time,
                                size_t str_len,
                                size_t& use_buf,
                                bool uct_time,
                                TS_FMT fmt
)
{
    //这个实现没有使用strftime的原因是，我对输出精度可能有更高的要求，

    time_t now_time = timeval->tv_sec;
    tm tm_data;
    use_buf = 0;

    //Email Date域是带有时区的输出，不用UTC
    if (zce::TS_FMT::EMAIL_DATE == fmt)
    {
        uct_time = false;
    }
    //HTTPHEAD只用GMT时间
    if (zce::TS_FMT::HTTP_GMT == fmt)
    {
        uct_time = true;
    }
    if (uct_time)
    {
        zce::gmtime_r(&now_time,&tm_data);
    }
    else
    {
        zce::localtime_r(&now_time,&tm_data);
    }

    //如果是压缩格式，精度到天，20100910
    if (zce::TS_FMT::SHRINK_DAY == fmt)
    {
        ZCE_ASSERT(str_len > zce::TS_SHRINK_DAY_LEN);
        if (str_len <= zce::TS_SHRINK_DAY_LEN)
        {
            return nullptr;
        }
        snprintf(str_date_time,
                 str_len,
                 "%02d%02d%02d",
                 tm_data.tm_year > 100 ? tm_data.tm_year - 100 : tm_data.tm_year,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday);
        use_buf = zce::TS_SHRINK_DAY_LEN;
    }
    else if (zce::TS_FMT::SHRINK_SEC == fmt)
    {
        ZCE_ASSERT(str_len > zce::TS_SHRINK_SEC_LEN);
        if (str_len <= zce::TS_SHRINK_SEC_LEN)
        {
            return nullptr;
        }
        snprintf(str_date_time,
                 str_len,
                 "%02d%02d%02d %02d%02d%02d",
                 tm_data.tm_year > 100 ? tm_data.tm_year - 100 : tm_data.tm_year,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec);
        use_buf = zce::TS_SHRINK_SEC_LEN;
    }
    else if (zce::TS_FMT::SHRINK_USEC == fmt)
    {
        ZCE_ASSERT(str_len > zce::TS_SHRINK_USEC_LEN);
        if (str_len <= zce::TS_SHRINK_USEC_LEN)
        {
            return nullptr;
        }
        snprintf(str_date_time,
                 str_len,
                 "%02d%02d%02d %02d%02d%02d.%06ld",
                 tm_data.tm_year > 100 ? tm_data.tm_year - 100 : tm_data.tm_year,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec,
                 timeval->tv_usec);
        use_buf = zce::TS_SHRINK_USEC_LEN;
    }
    else if (zce::TS_FMT::COMPACT_DAY == fmt)
    {
        ZCE_ASSERT(str_len > zce::TS_COMPACT_DAY_LEN);
        if (str_len <= zce::TS_COMPACT_DAY_LEN)
        {
            return nullptr;
        }

        snprintf(str_date_time,
                 str_len,
                 "%4d%02d%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday);
        use_buf = zce::TS_COMPACT_DAY_LEN;
    }
    //如果是压缩格式，精度到秒，20100910100318
    else if (zce::TS_FMT::COMPACT_SEC == fmt)
    {
        //参数保护和检查
        ZCE_ASSERT(str_len > zce::TS_COMPACT_SEC_LEN);
        if (str_len <= zce::TS_COMPACT_SEC_LEN)
        {
            return nullptr;
        }

        snprintf(str_date_time,
                 str_len,
                 "%4d%02d%02d %02d%02d%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec);
        use_buf = zce::TS_COMPACT_SEC_LEN;
    }
    //如果是压缩格式，精度到毫秒，20100910 100318.000213
    else if (zce::TS_FMT::COMPACT_USEC == fmt)
    {
        //参数保护和检查
        ZCE_ASSERT(str_len > zce::TS_COMPACT_USEC_LEN);
        if (str_len <= zce::TS_COMPACT_USEC_LEN)
        {
            return nullptr;
        }

        snprintf(str_date_time,
                 str_len,
                 "%4d%02d%02d %02d%02d%02d.%06ld",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec,
                 timeval->tv_usec);
        use_buf = zce::TS_COMPACT_USEC_LEN;
    }
    //2010-09-10
    else if (zce::TS_FMT::ISO_DAY == fmt)
    {
        ZCE_ASSERT(str_len > zce::TS_ISO_DAY_LEN);
        if (str_len <= zce::TS_ISO_DAY_LEN)
        {
            return nullptr;
        }

        snprintf(str_date_time,
                 str_len,
                 "%4d-%02d-%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday);
        use_buf = zce::TS_ISO_DAY_LEN;
    }
    //2010-09-10 10:03:18
    else if (zce::TS_FMT::ISO_SEC == fmt)
    {
        ZCE_ASSERT(str_len > zce::TS_ISO_SEC_LEN);
        if (str_len <= zce::TS_ISO_SEC_LEN)
        {
            return nullptr;
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
        use_buf = zce::TS_ISO_SEC_LEN;
    }
    //2010-09-10 10:03:18.100190
    else if (zce::TS_FMT::ISO_USEC == fmt)
    {
        //参数保护和检查
        ZCE_ASSERT(str_len > zce::TS_ISO_USEC_LEN);
        if (str_len <= zce::TS_ISO_USEC_LEN)
        {
            return nullptr;
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
        use_buf = zce::TS_ISO_USEC_LEN;
    }
    //Fri Aug 24 2002 07:43:05
    else if (zce::TS_FMT::US_SEC == fmt)
    {
        ZCE_ASSERT(str_len > TIMESTR_US_SEC_LEN);
        if (str_len <= TIMESTR_US_SEC_LEN)
        {
            return nullptr;
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
        use_buf = zce::TIMESTR_US_SEC_LEN;
    }
    //Fri Aug 24 2002 07:43:05.100190
    else if (zce::TS_FMT::US_USEC == fmt)
    {
        ZCE_ASSERT(str_len > TIMESTR_US_USEC_LEN);
        if (str_len <= TIMESTR_US_USEC_LEN)
        {
            return nullptr;
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
        use_buf = zce::TIMESTR_US_USEC_LEN;
    }
    //Thu, 26 Nov 2009 13:50:19 GMT
    else if (zce::TS_FMT::HTTP_GMT == fmt)
    {
        ZCE_ASSERT(str_len > TIMESTR_HTTP_GMT_LEN);
        if (str_len <= TIMESTR_HTTP_GMT_LEN)
        {
            return nullptr;
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
        use_buf = zce::TIMESTR_HTTP_GMT_LEN;
    }
    //Fri, 08 Nov 2002 09:42:22 +0800
    else if (zce::TS_FMT::EMAIL_DATE == fmt)
    {
        ZCE_ASSERT(str_len > TIMESTR_EMAIL_DATE_LEN);
        if (str_len <= TIMESTR_EMAIL_DATE_LEN)
        {
            return nullptr;
        }

        //注意timezone理论上需要tzset()函数初始化
        int tz = zce::gettimezone();
        //显示的时区格式为+0630，西6区，30为分
        int tzo = (tz / 3600) * 100 + tz % 3600 / 60;
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
                 tzo);
        use_buf = zce::TIMESTR_EMAIL_DATE_LEN;
    }
    //没有实现，参数错误
    else
    {
        ZCE_ASSERT(false);
        errno = EINVAL;
        return nullptr;
    }

    return str_date_time;
}

//通过字符串翻译得到tm时间结构
int zce::str_to_tm(const char* strtm,
                   TS_FMT fmt,
                   tm* ptr_tm,
                   time_t* usec,
                   int* tz)
{
    size_t len_str = ::strlen(strtm);
    ZCE_ASSERT(strtm && ptr_tm);
    if (usec != nullptr)
    {
        *usec = 0;
    }
    if (tz != nullptr)
    {
        *tz = 0;
    }

    memset(ptr_tm,0,sizeof(tm));
    if (zce::TS_FMT::SHRINK_DAY == fmt ||
        zce::TS_FMT::SHRINK_SEC == fmt ||
        zce::TS_FMT::SHRINK_USEC == fmt)
    {
        //字符串长度必须>=8
        if (len_str < zce::TS_SHRINK_DAY_LEN)
        {
            return -1;
        }
        ptr_tm->tm_year = ((*strtm) - '0') * 10
            + (*(strtm + 1) - '0');
        if (ptr_tm->tm_year < 75)
        {
            ptr_tm->tm_year += 100;
        }
        ptr_tm->tm_mon = (*(strtm + 2) - '0') * 10
            + (*(strtm + 3) - '0') - 1;
        ptr_tm->tm_mday = (*(strtm + 4) - '0') * 10
            + (*(strtm + 5) - '0');

        //如果输入字符串精度到秒
        if (zce::TS_FMT::SHRINK_SEC == fmt ||
            zce::TS_FMT::SHRINK_USEC == fmt)
        {
            //字符串长度必须>=15
            if (len_str < zce::TS_SHRINK_SEC_LEN)
            {
                return -1;
            }
            ptr_tm->tm_hour = (*(strtm + 7) - '0') * 10
                + (*(strtm + 8) - '0');
            ptr_tm->tm_min = (*(strtm + 9) - '0') * 10
                + (*(strtm + 10) - '0');
            ptr_tm->tm_sec = (*(strtm + 11) - '0') * 10
                + (*(strtm + 12) - '0');

            if (zce::TS_FMT::SHRINK_USEC == fmt &&
                usec != nullptr)
            {
                if (len_str < zce::TS_SHRINK_USEC_LEN)
                {
                    return -1;
                }
                *usec = ((*(strtm + 14)) - '0') * 100000
                    + ((*(strtm + 15) - '0')) * 10000
                    + ((*(strtm + 16) - '0')) * 1000
                    + ((*(strtm + 17) - '0')) * 100
                    + ((*(strtm + 18) - '0')) * 10
                    + ((*(strtm + 19) - '0'));
            }
        }
    }
    else if (zce::TS_FMT::COMPACT_DAY == fmt ||
             zce::TS_FMT::COMPACT_SEC == fmt ||
             zce::TS_FMT::COMPACT_USEC == fmt)
    {
        //字符串长度必须>=8
        if (len_str < zce::TS_COMPACT_DAY_LEN)
        {
            return -1;
        }
        ptr_tm->tm_year = ((*strtm) - '0') * 1000
            + (*(strtm + 1) - '0') * 100
            + (*(strtm + 2) - '0') * 10
            + (*(strtm + 3) - '0') - 1900;
        ptr_tm->tm_mon = (*(strtm + 4) - '0') * 10
            + (*(strtm + 5) - '0') - 1;
        ptr_tm->tm_mday = (*(strtm + 6) - '0') * 10
            + (*(strtm + 7) - '0');

        //如果输入字符串精度到秒
        if (zce::TS_FMT::COMPACT_SEC == fmt ||
            zce::TS_FMT::COMPACT_USEC == fmt)
        {
            //字符串长度必须>=15
            if (len_str < zce::TS_COMPACT_SEC_LEN)
            {
                return -1;
            }
            ptr_tm->tm_hour = (*(strtm + 9) - '0') * 10
                + (*(strtm + 10) - '0');
            ptr_tm->tm_min = (*(strtm + 11) - '0') * 10
                + (*(strtm + 12) - '0');
            ptr_tm->tm_sec = (*(strtm + 13) - '0') * 10
                + (*(strtm + 14) - '0');

            if (zce::TS_FMT::COMPACT_USEC == fmt &&
                usec != nullptr)
            {
                if (len_str < zce::TS_COMPACT_USEC_LEN)
                {
                    return -1;
                }
                *usec = ((*(strtm + 16)) - '0') * 100000
                    + ((*(strtm + 17) - '0')) * 10000
                    + ((*(strtm + 18) - '0')) * 1000
                    + ((*(strtm + 19) - '0')) * 100
                    + ((*(strtm + 20) - '0')) * 10
                    + ((*(strtm + 21) - '0'));
            }
        }
    }
    else if (zce::TS_FMT::ISO_DAY == fmt ||
             zce::TS_FMT::ISO_SEC == fmt ||
             zce::TS_FMT::ISO_USEC == fmt)
    {
        if (len_str < zce::TS_ISO_DAY_LEN)
        {
            return -1;
        }
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

        //如果输入字符串精度到微秒
        if (zce::TS_FMT::ISO_SEC == fmt ||
            zce::TS_FMT::ISO_USEC == fmt)
        {
            if (len_str < zce::TS_ISO_SEC_LEN)
            {
                return -1;
            }
            ptr_tm->tm_hour = (*(strtm + 11) - '0') * 10
                + (*(strtm + 12) - '0');
            ptr_tm->tm_min = (*(strtm + 14) - '0') * 10
                + (*(strtm + 15) - '0');
            ptr_tm->tm_sec = (*(strtm + 17) - '0') * 10
                + (*(strtm + 18) - '0');
        }
        if (zce::TS_FMT::ISO_USEC == fmt &&
            usec != nullptr)
        {
            if (len_str < zce::TS_ISO_USEC_LEN)
            {
                return -1;
            }
            *usec = ((*(strtm + 20)) - '0') * 100000
                + ((*(strtm + 21) - '0')) * 10000
                + ((*(strtm + 22) - '0')) * 1000
                + ((*(strtm + 23) - '0')) * 100
                + ((*(strtm + 24) - '0')) * 10
                + ((*(strtm + 25) - '0'));
        }
    }
    else if (zce::TS_FMT::US_SEC == fmt ||
             zce::TS_FMT::US_USEC == fmt)
    {
        //Fri Aug 24 2002 07:43:05.100190
        if (len_str < zce::TIMESTR_US_SEC_LEN)
        {
            return -1;
        }
        char mon_str[4];
        mon_str[0] = strtm[4];
        mon_str[1] = strtm[5];
        mon_str[2] = strtm[6];
        mon_str[3] = '\0';
        ptr_tm->tm_mon = 0;
        int i = 0;
        for (; i < 12; i++)
        {
            if (strncasecmp(mon_str,MONTH_NAME[i],3) == 0)
            {
                ptr_tm->tm_mon = i;
                break;
            }
        }
        if (i == 12)
        {
            errno = EINVAL;
            return -1;
        }
        ptr_tm->tm_mday = (*(strtm + 8) - '0') * 10
            + (*(strtm + 9) - '0');
        ptr_tm->tm_year = (*(strtm + 11) - '0') * 1000
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
        if (zce::TS_FMT::US_USEC == fmt &&
            usec != nullptr)
        {
            if (len_str < zce::TIMESTR_US_USEC_LEN)
            {
                return -1;
            }
            *usec = (*(strtm + 25) - '0') * 100000
                + (*(strtm + 26) - '0') * 10000
                + (*(strtm + 27) - '0') * 1000
                + (*(strtm + 28) - '0') * 100
                + (*(strtm + 29) - '0') * 10
                + (*(strtm + 30) - '0');
        }
    }
    //Thu, 26 Nov 2009 13:05:19 GMT
    else if (zce::TS_FMT::HTTP_GMT == fmt)
    {
        if (len_str < zce::TIMESTR_HTTP_GMT_LEN)
        {
            return -1;
        }
        char mon_str[4];
        mon_str[0] = strtm[8];
        mon_str[1] = strtm[9];
        mon_str[2] = strtm[10];
        mon_str[3] = '\0';
        ptr_tm->tm_mon = 0;
        int i = 0;
        for (; i < 12; i++)
        {
            if (strncasecmp(mon_str,MONTH_NAME[i],3) == 0)
            {
                ptr_tm->tm_mon = i;
                break;
            }
        }
        if (i == 12)
        {
            errno = EINVAL;
            return -1;
        }
        ptr_tm->tm_mday = (*(strtm + 5) - '0') * 10
            + (*(strtm + 6) - '0');
        ptr_tm->tm_year = (*(strtm + 12) - '0') * 1000
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
        //尾部还有一个" GMT"
    }
    else if (zce::TS_FMT::EMAIL_DATE == fmt)
    {
        if (len_str < 31)
        {
            return -1;
        }
        char mon_str[4];
        mon_str[0] = strtm[8];
        mon_str[1] = strtm[9];
        mon_str[2] = strtm[10];
        mon_str[3] = '\0';
        ptr_tm->tm_mon = 0;
        int i = 0;
        for (; i < 12; i++)
        {
            if (strncasecmp(mon_str,MONTH_NAME[i],3) == 0)
            {
                ptr_tm->tm_mon = i;
                break;
            }
        }
        if (i == 12)
        {
            errno = EINVAL;
            return -1;
        }
        ptr_tm->tm_mday = (*(strtm + 5) - '0') * 10
            + (*(strtm + 6) - '0');
        ptr_tm->tm_year = (*(strtm + 12) - '0') * 1000
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
            int tzi = ((*(strtm + 27) - '0') * 10
                       + (*(strtm + 28) - '0')) * 3600
                + ((*(strtm + 29) - '0') * 10
                   + (*(strtm + 30) - '0')) * 60;
            if (*(strtm + 26) == '-')
            {
                tzi = -1 * tzi;
            }
            *tz = tzi;
        }
    }
    else
    {
        ZCE_ASSERT(false);
        errno = EINVAL;
        return -1;
    }
    return 0;
}

//模糊字符串翻译得到tm时间结构，不需要你
int zce::fuzzy_str_to_tm(const char* strtm,
                         tm* ptr_tm,
                         time_t* usec,
                         int* tz)
{
    size_t len_str = ::strlen(strtm);
    if ((len_str >= TS_SHRINK_DAY_LEN &&
        Z_ISDIGIT(*(strtm + 0)) && Z_ISDIGIT(*(strtm + 1)) &&
        Z_ISDIGIT(*(strtm + 2)) && Z_ISDIGIT(*(strtm + 3)) &&
        Z_ISDIGIT(*(strtm + 4)) && Z_ISDIGIT(*(strtm + 5))))
    {
        if ((len_str >= TS_SHRINK_SEC_LEN && (*(strtm + 6) == ' ') &&
            Z_ISDIGIT(*(strtm + 7)) && Z_ISDIGIT(*(strtm + 8)) &&
            Z_ISDIGIT(*(strtm + 9)) && Z_ISDIGIT(*(strtm + 10)) &&
            Z_ISDIGIT(*(strtm + 11)) && Z_ISDIGIT(*(strtm + 12))))
        {
            if ((len_str >= TS_SHRINK_USEC_LEN && (*(strtm + 13) == '.') &&
                Z_ISDIGIT(*(strtm + 14)) && Z_ISDIGIT(*(strtm + 15)) &&
                Z_ISDIGIT(*(strtm + 16)) && Z_ISDIGIT(*(strtm + 17)) &&
                Z_ISDIGIT(*(strtm + 18)) && Z_ISDIGIT(*(strtm + 19))))
            {
                return str_to_tm(strtm,zce::TS_FMT::SHRINK_USEC,ptr_tm,usec,tz);
            }
            return str_to_tm(strtm,zce::TS_FMT::SHRINK_SEC,ptr_tm,usec,tz);
        }
        return str_to_tm(strtm,zce::TS_FMT::SHRINK_DAY,ptr_tm,usec,tz);
    }
    else if (len_str >= TS_COMPACT_DAY_LEN &&
             Z_ISDIGIT(*(strtm + 0)) && Z_ISDIGIT(*(strtm + 1)) &&
             Z_ISDIGIT(*(strtm + 2)) && Z_ISDIGIT(*(strtm + 3)) &&
             Z_ISDIGIT(*(strtm + 4)) && Z_ISDIGIT(*(strtm + 5)) &&
             Z_ISDIGIT(*(strtm + 6)) && Z_ISDIGIT(*(strtm + 7)))
    {
        if ((len_str >= TS_COMPACT_SEC_LEN && (*(strtm + 8) == ' ') &&
            Z_ISDIGIT(*(strtm + 9)) && Z_ISDIGIT(*(strtm + 10)) &&
            Z_ISDIGIT(*(strtm + 11)) && Z_ISDIGIT(*(strtm + 11)) &&
            Z_ISDIGIT(*(strtm + 13)) && Z_ISDIGIT(*(strtm + 14))))
        {
            if ((len_str >= TS_COMPACT_USEC_LEN && (*(strtm + 15) == '.') &&
                Z_ISDIGIT(*(strtm + 16)) && Z_ISDIGIT(*(strtm + 17)) &&
                Z_ISDIGIT(*(strtm + 18)) && Z_ISDIGIT(*(strtm + 19)) &&
                Z_ISDIGIT(*(strtm + 20)) && Z_ISDIGIT(*(strtm + 21))))
            {
                return str_to_tm(strtm,zce::TS_FMT::COMPACT_USEC,ptr_tm,usec,tz);
            }
            return str_to_tm(strtm,zce::TS_FMT::COMPACT_SEC,ptr_tm,usec,tz);
        }
        return str_to_tm(strtm,zce::TS_FMT::COMPACT_DAY,ptr_tm,usec,tz);
    }
    else if (len_str >= TS_ISO_DAY_LEN &&
             Z_ISDIGIT(*(strtm + 0)) && Z_ISDIGIT(*(strtm + 1)) &&
             Z_ISDIGIT(*(strtm + 2)) && Z_ISDIGIT(*(strtm + 3)) &&
             Z_ISDIGIT(*(strtm + 5)) && Z_ISDIGIT(*(strtm + 6)) &&
             Z_ISDIGIT(*(strtm + 8)) && Z_ISDIGIT(*(strtm + 9)))
    {
        if ((len_str >= TS_ISO_SEC_LEN && (*(strtm + 10) == ' ') &&
            Z_ISDIGIT(*(strtm + 11)) && Z_ISDIGIT(*(strtm + 12)) &&
            Z_ISDIGIT(*(strtm + 14)) && Z_ISDIGIT(*(strtm + 15)) &&
            Z_ISDIGIT(*(strtm + 17)) && Z_ISDIGIT(*(strtm + 18))))
        {
            if ((len_str >= TS_ISO_USEC_LEN && (*(strtm + 19) == '.') &&
                Z_ISDIGIT(*(strtm + 20)) && Z_ISDIGIT(*(strtm + 21)) &&
                Z_ISDIGIT(*(strtm + 22)) && Z_ISDIGIT(*(strtm + 23)) &&
                Z_ISDIGIT(*(strtm + 24)) && Z_ISDIGIT(*(strtm + 25))))
            {
                return str_to_tm(strtm,zce::TS_FMT::ISO_USEC,ptr_tm,usec,tz);
            }
            return str_to_tm(strtm,zce::TS_FMT::ISO_SEC,ptr_tm,usec,tz);
        }
        return str_to_tm(strtm,zce::TS_FMT::ISO_DAY,ptr_tm,usec,tz);
    }
    else if (len_str >= TIMESTR_US_SEC_LEN &&
             Z_ISALPHA(*(strtm + 0)) && Z_ISALPHA(*(strtm + 1)) &&
             Z_ISALPHA(*(strtm + 2)))
    {
        char week_str[4];
        week_str[0] = strtm[0];
        week_str[1] = strtm[1];
        week_str[2] = strtm[2];
        week_str[3] = '\0';
        size_t i = 0;
        for (; i < 7; i++)
        {
            if (strncasecmp(week_str,DAY_OF_WEEK_NAME[i],3) == 0)
            {
                break;
            }
        }
        if (i == 7)
        {
            errno = EINVAL;
            return -1;
        }
        return str_to_tm(strtm,zce::TS_FMT::US_SEC,ptr_tm,usec,tz);
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
}

//从字符串转换得到时间time_t函数
int zce::str_to_timeval(const char* strtm,
                        TS_FMT fmt,
                        bool uct_time,
                        timeval* tval)
{
    //HTTP_GMT 本身就是UTC/GMT
    if (zce::TS_FMT::HTTP_GMT == fmt)
    {
        uct_time = true;
    }
    //EMAIL_DATE本来就是本地时间
    if (zce::TS_FMT::EMAIL_DATE == fmt)
    {
        uct_time = false;
    }
    struct tm tm_value;
    time_t usec = 0;
    int tz = 0;
    zce::str_to_tm(strtm,
                   fmt,
                   &tm_value,
                   &usec,
                   &tz);

    if (uct_time)
    {
#if defined ZCE_OS_WINDOWS
        tval->tv_sec = static_cast<long>(zce::timegm(&tm_value));
        tval->tv_usec = static_cast<long>(usec);
#else
        tval->tv_sec = zce::timegm(&tm_value);
        tval->tv_usec = usec;
#endif
    }
    else
    {
#if defined ZCE_OS_WINDOWS
        tval->tv_sec = static_cast<long>(zce::timelocal(&tm_value));
        tval->tv_usec = static_cast<long>(usec);
#else
        tval->tv_sec = zce::timelocal(&tm_value);
        tval->tv_usec = usec;
#endif
    }
    //转换失败，表示字符串有问题
    if (-1 == tval->tv_sec)
    {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL ==1

//
int zce::str_to_MYSQL_TIME(const char* strtm,
                           MYSQL_TIME* mysql_tm)
{
    size_t len_str = ::strlen(strtm);
    ::memset(mysql_tm,0,sizeof(MYSQL_TIME));

    if (len_str >= 10 && *(strtm + 4) == '-')
    {
        mysql_tm->time_type = MYSQL_TIMESTAMP_DATE;

        mysql_tm->year = (*(strtm)-'0') * 1000
            + (*(strtm + 1) - '0') * 100
            + (*(strtm + 2) - '0') * 10
            + (*(strtm + 3) - '0');
        mysql_tm->month = (*(strtm + 5) - '0') * 10
            + (*(strtm + 6) - '0');
        mysql_tm->day = (*(strtm + 8) - '0') * 10
            + (*(strtm + 9) - '0');

        if (len_str >= 19 && *(strtm + 13) == ':')
        {
            mysql_tm->time_type = MYSQL_TIMESTAMP_DATETIME;
            mysql_tm->hour = (*(strtm + 11) - '0') * 10
                + (*(strtm + 12) - '0');
            mysql_tm->minute = (*(strtm + 14) - '0') * 10
                + (*(strtm + 15) - '0');
            mysql_tm->second = (*(strtm + 17) - '0') * 10
                + (*(strtm + 18) - '0');
        }
        //如果后续还有毫秒
        if (len_str >= 26 && *(strtm + 19) == '.')
        {
            mysql_tm->second_part = ((*(strtm + 20)) - '0') * 100000
                + ((*(strtm + 21) - '0')) * 10000
                + ((*(strtm + 22) - '0')) * 1000
                + ((*(strtm + 23) - '0')) * 100
                + ((*(strtm + 24) - '0')) * 10
                + ((*(strtm + 25) - '0'));
        }
    }
    else if ((len_str >= 9 && *(strtm) == '-' && *(strtm + 3) == ':') ||
             (len_str >= 10 && *(strtm) == '-' && *(strtm + 4) == ':') ||
             (len_str >= 8 && *(strtm + 2) == ':') ||
             (len_str >= 9 && *(strtm + 3) == ':'))
    {
        //TIME values may range from '-838:59:59' to '838:59:59'.
        mysql_tm->time_type = MYSQL_TIMESTAMP_TIME;
        size_t offset = 0;
        if (*(strtm + offset) == '-')
        {
            mysql_tm->neg = true;
            offset = 1;
        }
        //
        if (*(strtm + offset + 3) == ':')
        {
            mysql_tm->hour = (*(strtm + offset + 0) - '0') * 100
                + (*(strtm + offset + 1) - '0') * 10
                + (*(strtm + offset + 2) - '0');
            offset += 3;
        }
        else if (*(strtm + offset + 2) == ':')
        {
            mysql_tm->hour = (*(strtm + offset + 0) - '0') * 10
                + (*(strtm + offset + 1) - '0');
            offset += 2;
        }
        else
        {
            return -1;
        }
        mysql_tm->minute = (*(strtm + offset + 0) - '0') * 10
            + (*(strtm + offset + 1) - '0');
        mysql_tm->second = (*(strtm + offset + 3) - '0') * 10
            + (*(strtm + offset + 4) - '0');
    }
    else
    {
        return -1;
    }
    return 0;
}

#endif

///本地时间字符串转换为time_t
int zce::localtimestr_to_time_t(const char* localtime_str,
                                TS_FMT fmt,
                                time_t* time_t_val)
{
    timeval tval;
    int ret = str_to_timeval(localtime_str,fmt,false,&tval);
    if (ret != 0)
    {
        return ret;
    }
    *time_t_val = tval.tv_sec;
    return 0;
}

//----------------------------------------------------------------------------------------------------
const timeval zce::timeval_zero()
{
    timeval zero_time;
    zero_time.tv_sec = 0;
    zero_time.tv_usec = 0;
    return zero_time;
}

//将tv修正为0
void zce::timeval_clear(timeval& tv)
{
    tv.tv_sec = 0;
    tv.tv_usec = 0;
}

//计算总计是多少毫秒
uint64_t zce::total_milliseconds(const timeval& tv)
{
    //这里的参数就是因为需要转换到毫秒所折腾的。
    return static_cast<uint64_t>(tv.tv_sec) * SEC_PER_MSEC + tv.tv_usec / MSEC_PER_USEC;
}

//计算timeval内部总计是多少微秒10-6
uint64_t zce::total_microseconds(const timeval& tv)
{
    //这里的参数就是因为需要转换到毫秒所折腾的。
    return static_cast<uint64_t>(tv.tv_sec) * SEC_PER_USEC + tv.tv_usec;
}

//比较时间是否一致,如果一致返回0，left大，返回整数，right大返回负数
int zce::timeval_compare(const timeval& left,const timeval& right)
{
    if (left.tv_sec != right.tv_sec)
    {
        return left.tv_sec - right.tv_sec;
    }
    else
    {
        return left.tv_usec - right.tv_usec;
    }
}

//对两个时间进行想减,没有做复杂的溢出检查
const timeval zce::timeval_add(const timeval& left,const timeval& right)
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
const  timeval zce::timeval_sub(const timeval& left,const  timeval& right,bool safe)
{
    int64_t left_usec_val = (int64_t)left.tv_sec * SEC_PER_USEC + left.tv_usec;
    int64_t right_usec_val = (int64_t)right.tv_sec * SEC_PER_USEC + right.tv_usec;

    //用64位作为基准去减
    int64_t minus_usec_val = left_usec_val - right_usec_val;

    timeval minus_time_val;

    // >0 或者标识无须调整就调整成0
    if (minus_usec_val >= 0 || (minus_usec_val < 0 && safe == false))
    {
        minus_time_val.tv_sec = static_cast<long>(minus_usec_val / SEC_PER_USEC);
        minus_time_val.tv_usec = static_cast<long>(minus_usec_val % SEC_PER_USEC);
    }
    else
    {
        minus_time_val.tv_sec = 0;
        minus_time_val.tv_usec = 0;
    }

    return minus_time_val;
}

//检查这个TIMEVALUE是否还有剩余的时间
void zce::timeval_adjust(timeval& tv)
{
    int64_t tv_usec_val = (int64_t)tv.tv_sec * SEC_PER_USEC + tv.tv_usec;

    tv.tv_sec = static_cast<long>(tv_usec_val / SEC_PER_USEC);
    tv.tv_usec = static_cast<long>(tv_usec_val % SEC_PER_USEC);
}

//检查这个TIMEVALUE是否还有剩余的时间
bool zce::timeval_havetime(const timeval& tv)
{
    int64_t tv_usec_val = (int64_t)tv.tv_sec * SEC_PER_USEC + tv.tv_usec;

    if (tv_usec_val > 0)
    {
        return true;
    }

    return false;
}

//这只timeval这个结构
const timeval zce::make_timeval(time_t sec,time_t usec) noexcept
{
    timeval to_timeval;
#if defined (ZCE_OS_WINDOWS)
    to_timeval.tv_sec = static_cast<long>(sec);
    to_timeval.tv_usec = static_cast<long>(usec);
#elif defined (ZCE_OS_LINUX)
    to_timeval.tv_sec = sec;
    to_timeval.tv_usec = usec;
#endif

    return to_timeval;
}

//转换得到timeval这个结构
const timeval zce::make_timeval(std::clock_t clock_value) noexcept
{
    timeval to_timeval;

    to_timeval.tv_sec = clock_value / CLOCKS_PER_SEC;
    clock_t remain_val = clock_value % CLOCKS_PER_SEC;

    // Windows平台下tv_sec被定义成long,所以需要转换
    to_timeval.tv_usec = static_cast<decltype(to_timeval.tv_usec)>(
        (remain_val * SEC_PER_USEC / CLOCKS_PER_SEC));

    return to_timeval;
}

//转换得到timeval这个结构
const timeval zce::make_timeval(const ::timespec* timespec_val) noexcept
{
    //每次我自己看见这段代码都会疑惑好半天，实际我没有错，好吧，写点注释把，
    //NSEC 纳秒 10-9s
    //USEC 微秒 10-6s
    timeval to_timeval;

    to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>(timespec_val->tv_sec);
    to_timeval.tv_usec = timespec_val->tv_nsec / USEC_PER_NSEC;
    return to_timeval;
}

#if defined (ZCE_OS_WINDOWS)

//转换FILETIME到timeval
const timeval zce::make_timeval(const FILETIME* file_time) noexcept
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
const timeval zce::make_timeval(const SYSTEMTIME* system_time) noexcept
{
    FILETIME ft;
    ::SystemTimeToFileTime(system_time,&ft);
    return make_timeval(&ft);
}

//转换FILETIME到timeval,这个是把FILETIME当着一个时长看待进行的
const timeval zce::make_timeval2(const FILETIME* file_time) noexcept
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
const ::timespec zce::make_timespec(const ::timeval* timeval_val)
{
    //每次我自己看见这段代码都会疑惑好半天，实际我没有错，好吧，写点注释把，
    //NSEC 纳秒 10-9s
    //USEC 微秒 10-6s
    ::timespec to_timespec;

    to_timespec.tv_sec = timeval_val->tv_sec;
    to_timespec.tv_nsec = timeval_val->tv_usec * USEC_PER_NSEC;

    return to_timespec;
}

//计算总计是多少毫秒
uint64_t zce::total_milliseconds(const ::timespec& ts)
{
    //这里的参数就是因为需要转换到毫秒所折腾的。
    return static_cast<uint64_t>(ts.tv_sec) * SEC_PER_MSEC + ts.tv_nsec / MSEC_PER_NSEC;
}

//----------------------------------------------------------------------------------------------------
//休眠函数
//秒得休眠函数
int zce::sleep(uint32_t seconds)
{
#if defined (ZCE_OS_WINDOWS)
    ::Sleep(seconds * SEC_PER_MSEC);
    return 0;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::sleep(seconds);
#endif //
}

//休息一个timeval的时间
int zce::sleep(const timeval& tv)
{
    //
#if defined (ZCE_OS_WINDOWS)
    ::Sleep(tv.tv_sec * SEC_PER_MSEC + tv.tv_usec / MSEC_PER_USEC);
    return 0;
#endif //

#if defined (ZCE_OS_LINUX)
    return ::usleep(tv.tv_sec * SEC_PER_USEC + tv.tv_usec);
#endif //
}

//微秒的休眠函数
int zce::usleep(unsigned long usec)
{
#if defined (ZCE_OS_WINDOWS)
    //::Sleep(usec / MSEC_PER_USEC);
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
    return 0;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::usleep(usec);
#endif //
}

//----------------------------------------------------------------------------------------

uint64_t zce::rdtsc()
{
    uint64_t tsc_value = 0;

#if defined (ZCE_WIN32) && !defined (ZCE_WIN64)

    uint32_t hiword,loword;
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
        mov hiword,edx
        mov loword,eax
    }
    tsc_value = (uint64_t(hiword) << 32) + loword;
    tsc_value = __rdtsc();
#elif defined (ZCE_WIN64)

    int registers[4];
    __cpuid(registers,0);
    tsc_value = __rdtsc();

#elif defined (ZCE_OS_LINUX)

    uint32_t hiword,loword;
    asm("cpuid");
    asm volatile("rdtsc" : "=a" (hiword),"=d" (loword));
    tsc_value = (uint64_t(hiword) << 32) + loword;
#endif

    return tsc_value;
}
