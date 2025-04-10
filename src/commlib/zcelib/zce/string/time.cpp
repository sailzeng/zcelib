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
        zce::gmtime_r(&now_time, &tm_data);
    }
    else
    {
        zce::localtime_r(&now_time, &tm_data);
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
int zce::str_to_ztm(const char* strtm,
                    TS_FMT fmt,
                    zce::ztm* pztm)
{
    size_t len_str = ::strlen(strtm);
    ZCE_ASSERT(strtm && pztm);

    memset(pztm, 0, sizeof(zce::ztm));
    if (zce::TS_FMT::SHRINK_DAY == fmt ||
        zce::TS_FMT::SHRINK_SEC == fmt ||
        zce::TS_FMT::SHRINK_USEC == fmt)
    {
        //字符串长度必须>=8
        if (len_str < zce::TS_SHRINK_DAY_LEN)
        {
            return -1;
        }
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
        if (zce::TS_FMT::SHRINK_SEC == fmt ||
            zce::TS_FMT::SHRINK_USEC == fmt)
        {
            //字符串长度必须>=15
            if (len_str < zce::TS_SHRINK_SEC_LEN)
            {
                return -1;
            }
            pztm->hour_ = (*(strtm + 7) - '0') * 10
                + (*(strtm + 8) - '0');
            pztm->min_ = (*(strtm + 9) - '0') * 10
                + (*(strtm + 10) - '0');
            pztm->sec_ = (*(strtm + 11) - '0') * 10
                + (*(strtm + 12) - '0');

            if (zce::TS_FMT::SHRINK_USEC == fmt)
            {
                if (len_str < zce::TS_SHRINK_USEC_LEN)
                {
                    return -1;
                }
                pztm->usec_ = ((*(strtm + 14)) - '0') * 100000
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
        pztm->year_ = ((*strtm) - '0') * 1000
            + (*(strtm + 1) - '0') * 100
            + (*(strtm + 2) - '0') * 10
            + (*(strtm + 3) - '0');
        pztm->mon_ = (*(strtm + 4) - '0') * 10
            + (*(strtm + 5) - '0');
        pztm->day_ = (*(strtm + 6) - '0') * 10
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
            pztm->hour_ = (*(strtm + 9) - '0') * 10
                + (*(strtm + 10) - '0');
            pztm->min_ = (*(strtm + 11) - '0') * 10
                + (*(strtm + 12) - '0');
            pztm->sec_ = (*(strtm + 13) - '0') * 10
                + (*(strtm + 14) - '0');

            if (zce::TS_FMT::COMPACT_USEC == fmt)
            {
                if (len_str < zce::TS_COMPACT_USEC_LEN)
                {
                    return -1;
                }
                pztm->usec_ = ((*(strtm + 16)) - '0') * 100000
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
        pztm->year_ = ((*strtm) - '0') * 1000
            + (*(strtm + 1) - '0') * 100
            + (*(strtm + 2) - '0') * 10
            + (*(strtm + 3) - '0');
        pztm->mon_ = (*(strtm + 5) - '0') * 10
            + (*(strtm + 6) - '0');
        pztm->day_ = (*(strtm + 8) - '0') * 10
            + (*(strtm + 9) - '0');

        //如果输入字符串精度到微秒
        if (zce::TS_FMT::ISO_SEC == fmt ||
            zce::TS_FMT::ISO_USEC == fmt)
        {
            if (len_str < zce::TS_ISO_SEC_LEN)
            {
                return -1;
            }
            pztm->hour_ = (*(strtm + 11) - '0') * 10
                + (*(strtm + 12) - '0');
            pztm->min_ = (*(strtm + 14) - '0') * 10
                + (*(strtm + 15) - '0');
            pztm->sec_ = (*(strtm + 17) - '0') * 10
                + (*(strtm + 18) - '0');
        }
        if (zce::TS_FMT::ISO_USEC == fmt)
        {
            if (len_str < zce::TS_ISO_USEC_LEN)
            {
                return -1;
            }
            pztm->usec_ = ((*(strtm + 20)) - '0') * 100000
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
        if (zce::TS_FMT::US_USEC == fmt)
        {
            if (len_str < zce::TIMESTR_US_USEC_LEN)
            {
                return -1;
            }
            pztm->usec_ = (*(strtm + 25) - '0') * 100000
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
        if (*(strtm + 26) == '-')
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
    return 0;
}

//模糊字符串翻译得到tm时间结构，不需要你
int zce::fuzzy_str_to_ztm(const char* strtm,
                          zce::ztm* pztm)
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
                return str_to_ztm(strtm, zce::TS_FMT::SHRINK_USEC, pztm);
            }
            return str_to_ztm(strtm, zce::TS_FMT::SHRINK_SEC, pztm);
        }
        return str_to_ztm(strtm, zce::TS_FMT::SHRINK_DAY, pztm);
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
                return str_to_ztm(strtm, zce::TS_FMT::COMPACT_USEC, pztm);
            }
            return str_to_ztm(strtm, zce::TS_FMT::COMPACT_SEC, pztm);
        }
        return str_to_ztm(strtm, zce::TS_FMT::COMPACT_DAY, pztm);
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
                return str_to_ztm(strtm, zce::TS_FMT::ISO_USEC, pztm);
            }
            return str_to_ztm(strtm, zce::TS_FMT::ISO_SEC, pztm);
        }
        return str_to_ztm(strtm, zce::TS_FMT::ISO_DAY, pztm);
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
        return str_to_ztm(strtm, zce::TS_FMT::US_SEC, pztm);
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
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

///本地时间字符串转换为time_t
int zce::localtimestr_to_time_t(const char* localtime_str,
                                TS_FMT fmt,
                                time_t* time_t_val)
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
    zce::ztm ztm;
    zce::str_to_ztm(strtm,
                    fmt,
                    &ztm);
    *tval = make_timeval(uct_time, &ztm);
    //转换失败，表示字符串有问题
    if (-1 == tval->tv_sec)
    {
        errno = EINVAL;
        return -1;
    }

    return 0;
}
