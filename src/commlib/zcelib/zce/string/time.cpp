#include "zce/predefine.h"
#include "zce/comm/common.h"
#include "zce/os_adapt/time.h"
#include "zce/string/time.h"
#include "zce/logger/logging.h"

//将参数timeval的值作为的时间格格式化后输出打印出来
//可以控制各种格式输出
//如果成功，返回参数字符串str_date_time，如果失败返回nullptr
//timeval->tv_usec 千万不要溢出，会导致不可以预期问题
const char* zce::timeval_to_str(const timeval* timeval,
                                char* str_date_time,
                                size_t str_len,
                                size_t& use_buf,
                                TMS_FMT fmt,
                                bool out_tz,
                                bool utc_time)
{
    //这个实现没有使用strftime的原因是，我对输出精度可能有更高的要求，

    time_t now_time = timeval->tv_sec;
    tm tm_data;
    use_buf = 0;

    //Email Date域是带有时区的输出，不用UTC,而且有时区信息
    if (zce::TMS_FMT::EMAIL_DATE == fmt)
    {
        out_tz = false;
        utc_time = false;
    }
    //HTTPHEAD只用GMT时间
    if (zce::TMS_FMT::HTTP_GMT == fmt)
    {
        out_tz = false;
        utc_time = true;
    }
    if (utc_time)
    {
        zce::gmtime_r(&now_time, &tm_data);
    }
    else
    {
        zce::localtime_r(&now_time, &tm_data);
    }

    ZCE_ASSERT(fmt < SHRINK_DAY && fmt >= TMS_FMT_COUNT && str_len > TIMESTR_LEN[fmt]);
    if (fmt >= SHRINK_DAY && fmt < TMS_FMT_COUNT && str_len <= TIMESTR_LEN[fmt])
    {
        errno = EINVAL;
        return nullptr;
    }
    if (out_tz && str_len <= TIMESTR_LEN[fmt] + MAX_TZ_LEN)
    {
        errno = EINVAL;
        return nullptr;
    }
    //如果是压缩格式，精度到天，20100910
    if (zce::TMS_FMT::SHRINK_DAY == fmt)
    {
        snprintf(str_date_time,
                 str_len,
                 "%02d%02d%02d",
                 tm_data.tm_year > 100 ? tm_data.tm_year - 100 : tm_data.tm_year,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday);
    }
    else if (zce::TMS_FMT::SHRINK_SEC == fmt)
    {
        snprintf(str_date_time,
                 str_len,
                 "%02d%02d%02d %02d%02d%02d",
                 tm_data.tm_year > 100 ? tm_data.tm_year - 100 : tm_data.tm_year,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec);
    }
    else if (zce::TMS_FMT::SHRINK_USEC == fmt)
    {
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
    }
    else if (zce::TMS_FMT::COMPACT_DAY == fmt)
    {
        snprintf(str_date_time,
                 str_len,
                 "%4d%02d%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday);
    }
    //如果是压缩格式，精度到秒，20100910100318
    else if (zce::TMS_FMT::COMPACT_SEC == fmt)
    {
        snprintf(str_date_time,
                 str_len,
                 "%4d%02d%02d %02d%02d%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday,
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec);
    }
    //如果是压缩格式，精度到毫秒，20100910 100318.000213
    else if (zce::TMS_FMT::COMPACT_USEC == fmt)
    {
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
    }
    //2010-09-10
    else if (zce::TMS_FMT::ISO_TIME_SEC == fmt)
    {
        snprintf(str_date_time,
                 str_len,
                 "%4d-%02d-%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday);
    }
    //10:03:18
    else if (zce::TMS_FMT::ISO_TIME_SEC == fmt)
    {
        snprintf(str_date_time,
                 str_len,
                 "%02d:%02d:%02d",
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec);
    }
    //10:03:18.100190
    else if (zce::TMS_FMT::ISO_TIME_USEC == fmt)
    {
        snprintf(str_date_time,
                 str_len,
                 "%02d:%02d:%02d.%06ld",
                 tm_data.tm_hour,
                 tm_data.tm_min,
                 tm_data.tm_sec,
                 timeval->tv_usec);
    }
    //2010-09-10 10:03:18
    else if (zce::TMS_FMT::ISO_DATE_SEC == fmt)
    {
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
    else if (zce::TMS_FMT::ISO_DATE_USEC == fmt)
    {
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
    else if (zce::TMS_FMT::US_SEC == fmt)
    {
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
    else if (zce::TMS_FMT::US_USEC == fmt)
    {
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
    else if (zce::TMS_FMT::HTTP_GMT == fmt)
    {
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
    else if (zce::TMS_FMT::EMAIL_DATE == fmt)
    {
        //注意timezone理论上需要tzset()函数初始化
        int tz = zce::gettimezone() * -1;
        //显示的时区格式为+0630，西6区，30为分
        int tzo = (tz / 3600) * 100 + (tz % 3600) / 60;
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
    }
    //没有实现，参数错误
    else
    {
        ZCE_ASSERT(false);
        errno = EINVAL;
        return nullptr;
    }
    use_buf = TIMESTR_LEN[fmt];
    //如果需要输出时区信息
    if (out_tz)
    {
        //注意timezone理论上需要tzset()函数初始化，
        // *-1是因为tz是UTC和当前时区本地时间的差，+0800是东8区，+08:00的tz是-28800，这个是反直觉的
        int tz = zce::gettimezone() * -1;
        //显示的时区格式为+0630，西6区，30为分
        snprintf(str_date_time + use_buf,
                 str_len - use_buf,
                 "%+02d:%02d",
                 (tz / 3600),
                 (tz % 3600) / 60);
        use_buf += MAX_TZ_LEN;
    }
    return str_date_time;
}

//通过字符串翻译得到tm时间结构
int zce::str_to_ztm(const char* strtm,
                    zce::ztm* pztm)
{
    size_t len_str = ::strlen(strtm);
    ZCE_ASSERT(strtm && pztm &&
               pztm->fmt_ != zce::TMS_FMT::TMS_FMT_COUNT &&
               pztm->fmt_ != zce::TMS_FMT::TMS_FMT_INVALID &&
               len_str >= TIMESTR_LEN[pztm->fmt_]);
    //字符串长度必须>=8
    if (len_str < TIMESTR_LEN[pztm->fmt_])
    {
        return -1;
    }
    pztm->clear();
    if (zce::TMS_FMT::SHRINK_DAY == pztm->fmt_ ||
        zce::TMS_FMT::SHRINK_SEC == pztm->fmt_ ||
        zce::TMS_FMT::SHRINK_USEC == pztm->fmt_)
    {
        pztm->year_ = ((*strtm) - '0') * 10
            + (*(strtm + 1) - '0');
        if (pztm->year_ < 75)
        {
            pztm->year_ += 2000;
        }
        else if (pztm->year_ < 100)
        {
            pztm->year_ += 1900;
        }

        pztm->mon_ = (*(strtm + 2) - '0') * 10
            + (*(strtm + 3) - '0') - 1;
        pztm->day_ = (*(strtm + 4) - '0') * 10
            + (*(strtm + 5) - '0');

        //如果输入字符串精度到秒
        if (zce::TMS_FMT::SHRINK_SEC == pztm->fmt_ ||
            zce::TMS_FMT::SHRINK_USEC == pztm->fmt_)
        {
            pztm->hour_ = (*(strtm + 7) - '0') * 10
                + (*(strtm + 8) - '0');
            pztm->min_ = (*(strtm + 9) - '0') * 10
                + (*(strtm + 10) - '0');
            pztm->sec_ = (*(strtm + 11) - '0') * 10
                + (*(strtm + 12) - '0');

            if (zce::TMS_FMT::SHRINK_USEC == pztm->fmt_)
            {
                pztm->usec_ = ((*(strtm + 14)) - '0') * 100000
                    + ((*(strtm + 15) - '0')) * 10000
                    + ((*(strtm + 16) - '0')) * 1000
                    + ((*(strtm + 17) - '0')) * 100
                    + ((*(strtm + 18) - '0')) * 10
                    + ((*(strtm + 19) - '0'));
            }
        }
    }
    else if (zce::TMS_FMT::COMPACT_DAY == pztm->fmt_ ||
             zce::TMS_FMT::COMPACT_SEC == pztm->fmt_ ||
             zce::TMS_FMT::COMPACT_USEC == pztm->fmt_)
    {
        pztm->year_ = ((*strtm) - '0') * 1000
            + (*(strtm + 1) - '0') * 100
            + (*(strtm + 2) - '0') * 10
            + (*(strtm + 3) - '0');
        pztm->mon_ = (*(strtm + 4) - '0') * 10
            + (*(strtm + 5) - '0');
        pztm->day_ = (*(strtm + 6) - '0') * 10
            + (*(strtm + 7) - '0');

        //如果输入字符串精度到秒
        if (zce::TMS_FMT::COMPACT_SEC == pztm->fmt_ ||
            zce::TMS_FMT::COMPACT_USEC == pztm->fmt_)
        {
            pztm->hour_ = (*(strtm + 9) - '0') * 10
                + (*(strtm + 10) - '0');
            pztm->min_ = (*(strtm + 11) - '0') * 10
                + (*(strtm + 12) - '0');
            pztm->sec_ = (*(strtm + 13) - '0') * 10
                + (*(strtm + 14) - '0');

            if (zce::TMS_FMT::COMPACT_USEC == pztm->fmt_)
            {
                pztm->usec_ = ((*(strtm + 16)) - '0') * 100000
                    + ((*(strtm + 17) - '0')) * 10000
                    + ((*(strtm + 18) - '0')) * 1000
                    + ((*(strtm + 19) - '0')) * 100
                    + ((*(strtm + 20) - '0')) * 10
                    + ((*(strtm + 21) - '0'));
            }
        }
    }
    else if (zce::TMS_FMT::ISO_TIME_SEC == pztm->fmt_ ||
             zce::TMS_FMT::ISO_TIME_USEC == pztm->fmt_)
    {
        pztm->hour_ = (*(strtm + 0) - '0') * 10
            + (*(strtm + 1) - '0');
        pztm->min_ = (*(strtm + 3) - '0') * 10
            + (*(strtm + 4) - '0');
        pztm->sec_ = (*(strtm + 6) - '0') * 10
            + (*(strtm + 7) - '0');
        if (zce::TMS_FMT::ISO_DATE_USEC == pztm->fmt_)
        {
            pztm->usec_ = ((*(strtm + 9)) - '0') * 100000
                + ((*(strtm + 10) - '0')) * 10000
                + ((*(strtm + 11) - '0')) * 1000
                + ((*(strtm + 12) - '0')) * 100
                + ((*(strtm + 13) - '0')) * 10
                + ((*(strtm + 14) - '0'));
        }
    }
    else if (zce::TMS_FMT::ISO_DATE_DAY == pztm->fmt_ ||
             zce::TMS_FMT::ISO_DATE_SEC == pztm->fmt_ ||
             zce::TMS_FMT::ISO_DATE_USEC == pztm->fmt_)
    {
        pztm->year_ = ((*strtm) - '0') * 1000
            + (*(strtm + 1) - '0') * 100
            + (*(strtm + 2) - '0') * 10
            + (*(strtm + 3) - '0');
        pztm->mon_ = (*(strtm + 5) - '0') * 10
            + (*(strtm + 6) - '0');
        pztm->day_ = (*(strtm + 8) - '0') * 10
            + (*(strtm + 9) - '0');

        //如果输入字符串精度到微秒
        if (zce::TMS_FMT::ISO_DATE_SEC == pztm->fmt_ ||
            zce::TMS_FMT::ISO_DATE_USEC == pztm->fmt_)
        {
            pztm->hour_ = (*(strtm + 11) - '0') * 10
                + (*(strtm + 12) - '0');
            pztm->min_ = (*(strtm + 14) - '0') * 10
                + (*(strtm + 15) - '0');
            pztm->sec_ = (*(strtm + 17) - '0') * 10
                + (*(strtm + 18) - '0');
            if (zce::TMS_FMT::ISO_DATE_USEC == pztm->fmt_)
            {
                pztm->usec_ = ((*(strtm + 20)) - '0') * 100000
                    + ((*(strtm + 21) - '0')) * 10000
                    + ((*(strtm + 22) - '0')) * 1000
                    + ((*(strtm + 23) - '0')) * 100
                    + ((*(strtm + 24) - '0')) * 10
                    + ((*(strtm + 25) - '0'));
            }
        }
    }
    else if (zce::TMS_FMT::US_SEC == pztm->fmt_ ||
             zce::TMS_FMT::US_USEC == pztm->fmt_)
    {
        char mon_str[4];
        mon_str[0] = strtm[4];
        mon_str[1] = strtm[5];
        mon_str[2] = strtm[6];
        mon_str[3] = '\0';
        int i = 0;
        for (; i < 12; i++)
        {
            if (strncasecmp(mon_str, MONTH_NAME[i], 3) == 0)
            {
                pztm->mon_ = i + 1;
                break;
            }
        }
        if (i == 12)
        {
            errno = EINVAL;
            return -1;
        }
        pztm->day_ = (*(strtm + 8) - '0') * 10
            + (*(strtm + 9) - '0');
        pztm->year_ = (*(strtm + 11) - '0') * 1000
            + (*(strtm + 12) - '0') * 100
            + (*(strtm + 13) - '0') * 10
            + (*(strtm + 14) - '0');
        pztm->hour_ = (*(strtm + 16) - '0') * 10
            + (*(strtm + 17) - '0');
        pztm->min_ = (*(strtm + 19) - '0') * 10
            + (*(strtm + 20) - '0');
        pztm->sec_ = (*(strtm + 22) - '0') * 10
            + (*(strtm + 23) - '0');
        //如果输入字符串精度到微秒
        if (zce::TMS_FMT::US_USEC == pztm->fmt_)
        {
            pztm->usec_ = (*(strtm + 25) - '0') * 100000
                + (*(strtm + 26) - '0') * 10000
                + (*(strtm + 27) - '0') * 1000
                + (*(strtm + 28) - '0') * 100
                + (*(strtm + 29) - '0') * 10
                + (*(strtm + 30) - '0');
        }
    }
    //Thu, 26 Nov 2009 13:05:19 GMT
    else if (zce::TMS_FMT::HTTP_GMT == pztm->fmt_)
    {
        char mon_str[4];
        mon_str[0] = strtm[8];
        mon_str[1] = strtm[9];
        mon_str[2] = strtm[10];
        mon_str[3] = '\0';
        int i = 0;
        for (; i < 12; i++)
        {
            if (strncasecmp(mon_str, MONTH_NAME[i], 3) == 0)
            {
                pztm->mon_ = i + 1;
                break;
            }
        }
        if (i == 12)
        {
            errno = EINVAL;
            return -1;
        }
        pztm->day_ = (*(strtm + 5) - '0') * 10
            + (*(strtm + 6) - '0');
        pztm->year_ = (*(strtm + 12) - '0') * 1000
            + (*(strtm + 13) - '0') * 100
            + (*(strtm + 14) - '0') * 10
            + (*(strtm + 15) - '0');
        pztm->hour_ = (*(strtm + 17) - '0') * 10
            + (*(strtm + 18) - '0');
        pztm->min_ = (*(strtm + 20) - '0') * 10
            + (*(strtm + 21) - '0');
        pztm->sec_ = (*(strtm + 23) - '0') * 10
            + (*(strtm + 24) - '0');
        //尾部还有一个" GMT"
        pztm->tz_ = 0;
    }
    else if (zce::TMS_FMT::EMAIL_DATE == pztm->fmt_)
    {
        char mon_str[4];
        mon_str[0] = strtm[8];
        mon_str[1] = strtm[9];
        mon_str[2] = strtm[10];
        mon_str[3] = '\0';
        int i = 0;
        for (; i < 12; i++)
        {
            if (strncasecmp(mon_str, MONTH_NAME[i], 3) == 0)
            {
                pztm->mon_ = i;
                break;
            }
        }
        if (i == 12)
        {
            errno = EINVAL;
            return -1;
        }
        pztm->day_ = (*(strtm + 5) - '0') * 10
            + (*(strtm + 6) - '0');
        pztm->year_ = (*(strtm + 12) - '0') * 1000
            + (*(strtm + 13) - '0') * 100
            + (*(strtm + 14) - '0') * 10
            + (*(strtm + 15) - '0')
            - 1900;
        pztm->hour_ = (*(strtm + 17) - '0') * 10
            + (*(strtm + 18) - '0');
        pztm->min_ = (*(strtm + 20) - '0') * 10
            + (*(strtm + 21) - '0');
        pztm->sec_ = (*(strtm + 23) - '0') * 10
            + (*(strtm + 24) - '0');

        int tzi = ((*(strtm + 27) - '0') * 10
                   + (*(strtm + 28) - '0')) * 3600
            + ((*(strtm + 29) - '0') * 10
               + (*(strtm + 30) - '0')) * 60;
        // 注意 tz是UTC和当前时区本地时间的差，+0800是东8区，+08:00的tz是-28800，这个是反直觉的
        if (*(strtm + 26) == '+')
        {
            tzi = -1 * tzi;
        }
        pztm->tz_ = tzi;
    }
    else
    {
        ZCE_ASSERT(false);
        errno = EINVAL;
        return -1;
    }
    //如果输入的信息里面有时区，将时区提取出来 Z +08 ，+08:00 , +05:30
    size_t procss_len = TIMESTR_LEN[pztm->fmt_];
    if (pztm->tz_ == ztm::INVALID_TZ)
    {
        if (len_str >= procss_len + 1 && strtm[procss_len] == 'Z')
        {
            pztm->tz_ = 0;
        }
        else if (len_str >= procss_len + 3 &&
                 (strtm[procss_len] == '+' || strtm[procss_len] == '-'))
        {
            int tzi = ((*(strtm + procss_len + 1) - '0') * 10
                       + (*(strtm + procss_len + 2) - '0')) * 3600;
            if (len_str > procss_len + 6 && strtm[procss_len + 3] == ':')
            {
                tzi += ((*(strtm + procss_len + 4) - '0') * 10
                        + (*(strtm + procss_len + 5) - '0')) * 60;
            }
            if (strtm[procss_len] == '+')
            {
                tzi = -1 * tzi;
            }
            pztm->tz_ = tzi;
        }
    }

    return 0;
}

//模糊字符串翻译得到tm时间结构，不需要你
int zce::fuzzy_str_to_ztm(const char* strtm,
                          zce::ztm* pztm)
{
    if (pztm->fmt_ != zce::TMS_FMT::TMS_FMT_INVALID)
    {
        return str_to_ztm(strtm, pztm);
    }
    size_t len_str = ::strlen(strtm);
    if ((len_str >= zce::TIMESTR_LEN[SHRINK_DAY] &&
        Z_ISDIGIT(*(strtm + 0)) && Z_ISDIGIT(*(strtm + 1)) &&
        Z_ISDIGIT(*(strtm + 2)) && Z_ISDIGIT(*(strtm + 3)) &&
        Z_ISDIGIT(*(strtm + 4)) && Z_ISDIGIT(*(strtm + 5))))
    {
        pztm->fmt_ = zce::TMS_FMT::SHRINK_DAY;
        if ((len_str >= TIMESTR_LEN[SHRINK_SEC] &&
            ((*(strtm + 6) == ' ') || (*(strtm + 6) == 'T')) &&
            Z_ISDIGIT(*(strtm + 7)) && Z_ISDIGIT(*(strtm + 8)) &&
            Z_ISDIGIT(*(strtm + 9)) && Z_ISDIGIT(*(strtm + 10)) &&
            Z_ISDIGIT(*(strtm + 11)) && Z_ISDIGIT(*(strtm + 12))))
        {
            pztm->fmt_ = zce::TMS_FMT::SHRINK_SEC;
            if ((len_str >= TIMESTR_LEN[SHRINK_USEC] && (*(strtm + 13) == '.') &&
                Z_ISDIGIT(*(strtm + 14)) && Z_ISDIGIT(*(strtm + 15)) &&
                Z_ISDIGIT(*(strtm + 16)) && Z_ISDIGIT(*(strtm + 17)) &&
                Z_ISDIGIT(*(strtm + 18)) && Z_ISDIGIT(*(strtm + 19))))
            {
                pztm->fmt_ = zce::TMS_FMT::SHRINK_USEC;
            }
        }
    }
    else if (len_str >= TIMESTR_LEN[COMPACT_DAY] &&
             Z_ISDIGIT(*(strtm + 0)) && Z_ISDIGIT(*(strtm + 1)) &&
             Z_ISDIGIT(*(strtm + 2)) && Z_ISDIGIT(*(strtm + 3)) &&
             Z_ISDIGIT(*(strtm + 4)) && Z_ISDIGIT(*(strtm + 5)) &&
             Z_ISDIGIT(*(strtm + 6)) && Z_ISDIGIT(*(strtm + 7)))
    {
        pztm->fmt_ = zce::TMS_FMT::COMPACT_DAY;
        if ((len_str >= TIMESTR_LEN[COMPACT_SEC] &&
            ((*(strtm + 8) == ' ') || (*(strtm + 8) == 'T')) &&
            Z_ISDIGIT(*(strtm + 9)) && Z_ISDIGIT(*(strtm + 10)) &&
            Z_ISDIGIT(*(strtm + 11)) && Z_ISDIGIT(*(strtm + 11)) &&
            Z_ISDIGIT(*(strtm + 13)) && Z_ISDIGIT(*(strtm + 14))))
        {
            pztm->fmt_ = zce::TMS_FMT::COMPACT_SEC;
            if ((len_str >= TIMESTR_LEN[COMPACT_USEC] && (*(strtm + 15) == '.') &&
                Z_ISDIGIT(*(strtm + 16)) && Z_ISDIGIT(*(strtm + 17)) &&
                Z_ISDIGIT(*(strtm + 18)) && Z_ISDIGIT(*(strtm + 19)) &&
                Z_ISDIGIT(*(strtm + 20)) && Z_ISDIGIT(*(strtm + 21))))
            {
                pztm->fmt_ = zce::TMS_FMT::COMPACT_USEC;
            }
        }
    }
    else if (len_str >= TIMESTR_LEN[ISO_TIME_SEC] &&
             Z_ISDIGIT(*(strtm + 0)) && Z_ISDIGIT(*(strtm + 1)) &&
             Z_ISDIGIT(*(strtm + 3)) && Z_ISDIGIT(*(strtm + 4)) &&
             Z_ISDIGIT(*(strtm + 6)) && Z_ISDIGIT(*(strtm + 7)) &&
             (*(strtm + 2) == ':') && (*(strtm + 5) == ':'))
    {
        pztm->fmt_ = zce::TMS_FMT::ISO_TIME_SEC;
        if ((len_str >= TIMESTR_LEN[ISO_TIME_USEC] && (*(strtm + 8) == '.') &&
            Z_ISDIGIT(*(strtm + 9)) && Z_ISDIGIT(*(strtm + 10)) &&
            Z_ISDIGIT(*(strtm + 11)) && Z_ISDIGIT(*(strtm + 12)) &&
            Z_ISDIGIT(*(strtm + 13)) && Z_ISDIGIT(*(strtm + 14))))
        {
            pztm->fmt_ = zce::TMS_FMT::ISO_TIME_USEC;
        }
    }
    else if (len_str >= TIMESTR_LEN[ISO_DATE_DAY] &&
             Z_ISDIGIT(*(strtm + 0)) && Z_ISDIGIT(*(strtm + 1)) &&
             Z_ISDIGIT(*(strtm + 2)) && Z_ISDIGIT(*(strtm + 3)) &&
             Z_ISDIGIT(*(strtm + 5)) && Z_ISDIGIT(*(strtm + 6)) &&
             Z_ISDIGIT(*(strtm + 8)) && Z_ISDIGIT(*(strtm + 9)) &&
             ((*(strtm + 4) == '-') || (*(strtm + 4) == '/')) &&
             ((*(strtm + 7) == '-') || (*(strtm + 7) == '/')))
    {
        pztm->fmt_ = zce::TMS_FMT::ISO_DATE_DAY;
        if (len_str >= TIMESTR_LEN[ISO_DATE_SEC] &&
            ((*(strtm + 10) == ' ') || (*(strtm + 10) == 'T')) &&
            Z_ISDIGIT(*(strtm + 11)) && Z_ISDIGIT(*(strtm + 12)) &&
            Z_ISDIGIT(*(strtm + 14)) && Z_ISDIGIT(*(strtm + 15)) &&
            Z_ISDIGIT(*(strtm + 17)) && Z_ISDIGIT(*(strtm + 18)) &&
            (*(strtm + 13) == ':') && (*(strtm + 16) == ':'))
        {
            pztm->fmt_ = zce::TMS_FMT::ISO_DATE_SEC;
            if ((len_str >= TIMESTR_LEN[ISO_DATE_USEC] && (*(strtm + 19) == '.') &&
                Z_ISDIGIT(*(strtm + 20)) && Z_ISDIGIT(*(strtm + 21)) &&
                Z_ISDIGIT(*(strtm + 22)) && Z_ISDIGIT(*(strtm + 23)) &&
                Z_ISDIGIT(*(strtm + 24)) && Z_ISDIGIT(*(strtm + 25))))
            {
                pztm->fmt_ = zce::TMS_FMT::ISO_DATE_USEC;
            }
        }
    }
    else if (len_str >= TIMESTR_LEN[US_SEC] &&
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
            if (strncasecmp(week_str, DAY_OF_WEEK_NAME[i], 3) == 0)
            {
                break;
            }
        }
        if (i == 7)
        {
            errno = EINVAL;
            return -1;
        }
        pztm->fmt_ = zce::TMS_FMT::US_SEC;
        if (len_str >= TIMESTR_LEN[US_USEC] && (*(strtm + 24) == '.'))
        {
            pztm->fmt_ = zce::TMS_FMT::US_USEC;
        }
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
    return str_to_ztm(strtm, pztm);
}

#if defined ZCE_USE_MYSQL && ZCE_USE_MYSQL ==1

//
int zce::str_to_MYSQL_TIME(const char* strtm,
                           MYSQL_TIME* mysql_tm)
{
    size_t len_str = ::strlen(strtm);
    ::memset(mysql_tm, 0, sizeof(MYSQL_TIME));

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

//从字符串转换得到时间time_t函数
int zce::str_to_timeval(const char* strtm,
                        TMS_FMT fmt,
                        timeval* tval,
                        bool uct_time)
{
    //HTTP_GMT 本身就是UTC/GMT
    if (zce::TMS_FMT::HTTP_GMT == fmt)
    {
        uct_time = true;
    }
    //EMAIL_DATE本来就是本地时间
    if (zce::TMS_FMT::EMAIL_DATE == fmt)
    {
        uct_time = false;
    }
    zce::ztm ztm;
    ztm.fmt_ = fmt;
    zce::str_to_ztm(strtm,
                    &ztm);
    *tval = make_timeval(&ztm, uct_time);
    //转换失败，表示字符串有问题
    if (-1 == tval->tv_sec)
    {
        errno = EINVAL;
        return -1;
    }

    return 0;
}
