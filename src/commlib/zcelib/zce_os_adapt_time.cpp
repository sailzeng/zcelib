/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_time.cpp
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       Tuesday, December 31, 2011
* @brief
*
*
* @details    ʱ��������������㣬��Ҫ������LINUX�¿�£
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
#include "zce_log_logging.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_time.h"

//Ϊʲô��������ACE�������ޣ��������ޣ��������������������������ޣ���������������

//�����������д�˴�����long��Ҫ��Ϊ�˼���,�Ȳ�����

//�Ǳ�׼�������õ�������������ʱ�䣬gethrtime��λ�ȽϹ֣����Ƶõ�CPU�����������ڵ�TICK��ʱ�䣬
//ȱ�㣺������ε���֮���ʱ���������������49�죬���޷���֤��õ�׼ȷ��ֵ
//���ϲ�Ҫ49���ֻ����һ���������ѽ�������ұ�֤�������TICK��Ч������������ÿ�����һ�ΰɡ�
//�ڲ�Ϊ����һ�ε��õ�ʱ�䣬����static ��������Ϊ�˱���static����������
const timeval zce::get_uptime()
{

#if defined (ZCE_OS_WINDOWS)

    //ע��GetTickCount64��GetTickCount���صĶ���milliseconds������CPU Tick

    timeval up_time;
    uint64_t now_cpu_tick = 0;

    //Ϊʲô��������GetTickCount64 ,(Vista��֧��),��������ע�͵�ԭ���ǣ������ͨ���ˣ�����Ҳû����,XP��WINSERVER2003���޷�ʹ�ã�
    //VISTA,WINSERVER2008��_WIN32_WINNT����0x0600
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1
    now_cpu_tick = ::GetTickCount64();
#else

    //GetTickCount���ص���һ��32λ������milliseconds �����ǣ�DWORD��ʵֻ��49����ĳ���,����ֻ�е��۵ķ�װ��
    unsigned int cpu_tick =  static_cast<unsigned int>(::GetTickCount());

    //���˾�̬��������ֹ����
    ZCE_Thread_Light_Mutex lock_static_var;
    ZCE_Thread_Light_Mutex::LOCK_GUARD guard(lock_static_var);

    static unsigned int one_period_tick = 0;
    static uint64_t cpu_tick_count = 0;

    //�����GetTickCountû�й��㣬�����ݿ϶����ھ�����
    if (one_period_tick  <= cpu_tick)
    {
        one_period_tick = cpu_tick;
        cpu_tick_count = (0xFFFFFFFF00000000 & cpu_tick_count) + one_period_tick;
    }
    //������else�������ת��1Ȧ��
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
    //��ù�ķ���LINUX�ܶ�汾��û��֧�����gethrtime�������ҿ�����������
    struct timespec sp;
    timeval up_time;
    int ret = ::clock_gettime(CLOCK_MONOTONIC, &sp);

    if (ret == 0)
    {
        up_time = zce::make_timeval(&sp);
    }
    else
    {
        ZCE_LOG(RS_ERROR, "::clock_gettime(CLOCK_MONOTONIC, &sp) ret != 0,fail.ret = %d lasterror = %d", ret, zce::last_error());
        up_time.tv_sec = 0;
        up_time.tv_usec = 0;
    }

    return up_time;
#endif
}

//
//�õ���ǰ��ϵͳʱ���ַ������
const char *zce::timestamp (char *str_date_time, size_t datetime_strlen)
{
    timeval now_time_val (zce::gettimeofday());
    return zce::timestamp (&now_time_val, str_date_time, datetime_strlen);
}

//������timeval��ֵ��Ϊ��ʱ����ʽ���������ӡ����
const char *zce::timestamp (const timeval *timeval, char *str_date_time, size_t datetime_strlen)
{
    ZCE_ASSERT(datetime_strlen > zce::TIMESTR_ISO_USEC_LEN);

    //ת��Ϊ���
    time_t now_time = timeval->tv_sec;
    tm tm_data;
    zce::localtime_r(&now_time, &tm_data);

    //��������д�����ԭ���õ�����һ�д��룬���ǻ���ֱ���(Windows�µĶ���),��֪��Ϊɶ�𣬺Ǻ�
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


int zce::gettimezone()
{
#if defined ZCE_OS_WINDOWS
    return _timezone;
#else
    return timezone;
#endif
}

//������timeval��ֵ��Ϊ��ʱ����ʽ���������ӡ����
//���Կ��Ƹ��ָ�ʽ���
//����ɹ������ز����ַ���str_date_time�����ʧ�ܷ���NULL
//timeval->tv_usec ǧ��Ҫ������ᵼ�²�����Ԥ������
const char *zce::timeval_to_str(const timeval *timeval,
                                char *str_date_time,
                                size_t str_len,
                                size_t &use_buf,
                                bool uct_time,
                                TIME_STR_FORMAT_TYPE fmt
                               )
{
    //���ʵ��û��ʹ��strftime��ԭ���ǣ��Ҷ�������ȿ����и��ߵ�Ҫ��
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
    use_buf = 0;
    if (uct_time)
    {
        //Email Date���Ǵ���ʱ�������������UTC
        if ( zce::TIME_STRFMT_EMAIL_DATE == fmt)
        {
            ZCE_ASSERT(false);
            errno = EINVAL;
            return NULL;
        }
        zce::gmtime_r(&now_time, &tm_data);
    }
    else
    {
        //HTTPHEADֻ��GMTʱ��
        if ( zce::TIME_STRFMT_HTTP_GMT == fmt)
        {
            ZCE_ASSERT(false);
            errno = EINVAL;
            return NULL;
        }
        zce::localtime_r(&now_time, &tm_data);
    }


    //�����ѹ����ʽ�����ȵ��죬20100910
    if ( zce::TIME_STRFMT_COMPACT_DAY == fmt)
    {
        ZCE_ASSERT(str_len > zce::TIMESTR_COMPACT_DAY_LEN);
        if (str_len <= zce::TIMESTR_COMPACT_DAY_LEN)
        {
            return NULL;
        }

        snprintf(str_date_time,
                 str_len,
                 "%4d%02d%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday);
        use_buf = zce::TIMESTR_COMPACT_DAY_LEN;
    }
    //�����ѹ����ʽ�����ȵ��룬20100910100318
    else if ( zce::TIME_STRFMT_COMPACT_SEC == fmt)
    {
        //���������ͼ��
        ZCE_ASSERT(str_len > zce::TIMESTR_COMPACT_SEC_LEN);
        if (str_len <= zce::TIMESTR_COMPACT_SEC_LEN)
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
        use_buf = zce::TIMESTR_COMPACT_SEC_LEN;
    }
    //2010-09-10
    else if ( zce::TIME_STRFMT_ISO_DAY == fmt )
    {
        ZCE_ASSERT(str_len > zce::TIMESTR_ISO_DAY_LEN);
        if (str_len <= zce::TIMESTR_ISO_DAY_LEN)
        {
            return NULL;
        }

        snprintf(str_date_time,
                 str_len,
                 "%4d-%02d-%02d",
                 tm_data.tm_year + 1900,
                 tm_data.tm_mon + 1,
                 tm_data.tm_mday);
        use_buf = zce::TIMESTR_ISO_DAY_LEN;
    }
    //2010-09-10 10:03:18
    else if (zce::TIME_STRFMT_ISO_SEC == fmt)
    {
        ZCE_ASSERT(str_len > zce::TIMESTR_ISO_SEC_LEN);
        if (str_len <= zce::TIMESTR_ISO_SEC_LEN)
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
        use_buf = zce::TIMESTR_ISO_SEC_LEN;
    }
    //2010-09-10 10:03:18.100190
    else if ( zce::TIME_STRFMT_ISO_USEC == fmt )
    {
        //���������ͼ��
        ZCE_ASSERT(str_len > zce::TIMESTR_ISO_USEC_LEN);
        if (str_len <= zce::TIMESTR_ISO_USEC_LEN)
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
        use_buf = zce::TIMESTR_ISO_USEC_LEN;
    }
    //Fri Aug 24 2002 07:43:05
    else if ( zce::TIME_STRFMT_US_SEC == fmt)
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
        use_buf = zce::TIME_STRFMT_US_SEC;
    }
    //Fri Aug 24 2002 07:43:05.100190
    else if ( zce::TIME_STRFMT_US_USEC == fmt)
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
        use_buf = zce::TIMESTR_US_USEC_LEN;
    }
    //Thu, 26 Nov 2009 13:50:19 GMT
    else if ( zce::TIME_STRFMT_HTTP_GMT == fmt )
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
        use_buf = zce::TIMESTR_HTTP_GMT_LEN;
    }
    //Fri, 08 Nov 2002 09:42:22 +0800
    else if (zce::TIME_STRFMT_EMAIL_DATE == fmt)
    {
        ZCE_ASSERT(str_len > TIMESTR_EMAIL_DATE_LEN);
        if (str_len <= TIMESTR_EMAIL_DATE_LEN)
        {
            return NULL;
        }

        //ע��timezone��������Ҫtzset()������ʼ��
        int tz = zce::gettimezone();
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
        use_buf = zce::TIMESTR_EMAIL_DATE_LEN;
    }
    //û��ʵ�֣���������
    else
    {
        ZCE_ASSERT(false);
        errno = EINVAL;
        return NULL;
    }

    return str_date_time;

}


//ͨ���ַ�������õ�tmʱ��ṹ
void zce::str_to_tm(const char *strtm,
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
    if (zce::TIME_STRFMT_COMPACT_DAY == fmt ||
        zce::TIME_STRFMT_COMPACT_SEC == fmt)
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

        //��������ַ������ȵ�΢��
        if (zce::TIME_STRFMT_COMPACT_SEC == fmt)
        {
            ptr_tm->tm_hour = (*(strtm + 8) - '0') * 10
                              + (*(strtm + 9) - '0');
            ptr_tm->tm_min = (*(strtm + 10) - '0') * 10
                             + (*(strtm + 11) - '0');
            ptr_tm->tm_sec = (*(strtm + 12) - '0') * 10
                             + (*(strtm + 13) - '0');
        }
    }
    else if (zce::TIME_STRFMT_ISO_DAY == fmt ||
             zce::TIME_STRFMT_ISO_SEC == fmt ||
             zce::TIME_STRFMT_ISO_USEC == fmt)
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

        //��������ַ������ȵ�΢��
        if (zce::TIME_STRFMT_ISO_SEC == fmt ||
            zce::TIME_STRFMT_ISO_USEC == fmt)
        {
            ptr_tm->tm_hour = (*(strtm + 11) - '0') * 10
                              + (*(strtm + 12) - '0');
            ptr_tm->tm_min = (*(strtm + 14) - '0') * 10
                             + (*(strtm + 15) - '0');
            ptr_tm->tm_sec = (*(strtm + 17) - '0') * 10
                             + (*(strtm + 18) - '0');

        }
        if (zce::TIME_STRFMT_ISO_USEC == fmt &&
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
    else if (zce::TIME_STRFMT_US_SEC == fmt ||
             zce::TIME_STRFMT_US_USEC == fmt)
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
        //��������ַ������ȵ�΢��
        if (zce::TIME_STRFMT_US_USEC == fmt &&
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
    else if (zce::TIME_STRFMT_HTTP_GMT == fmt)
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
    else if (zce::TIME_STRFMT_EMAIL_DATE == fmt)
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


//���ַ���ת���õ�ʱ��time_t����
int zce::str_to_timeval(const char *strtm,
                        TIME_STR_FORMAT_TYPE fmt,
                        bool uct_time,
                        timeval *tval)
{
    //
    if (!uct_time && zce::TIME_STRFMT_HTTP_GMT == fmt)
    {
        ZCE_ASSERT(false);
        errno = EINVAL;
        return -1;
    }

    if ( zce::TIME_STRFMT_EMAIL_DATE == fmt)
    {
        uct_time = true;
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
        tval->tv_sec = static_cast<long>( zce::timegm(&tm_value));
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
    //ת��ʧ�ܣ���ʾ�ַ���������
    if (-1 == tval->tv_sec )
    {
        errno = EINVAL;
        return -1;
    }

    if (zce::TIME_STRFMT_EMAIL_DATE == fmt)
    {
        tval->tv_sec += tz;
    }

    return 0;
}


///����ʱ���ַ���ת��Ϊtime_t
int zce::localtimestr_to_time_t(const char *localtime_str,
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
const timeval zce::timeval_zero()
{
    timeval zero_time;
    zero_time.tv_sec = 0;
    zero_time.tv_usec = 0;
    return zero_time;
}

//��tv����Ϊ0
void zce::timeval_clear(timeval &tv)
{
    tv.tv_sec = 0;
    tv.tv_sec = 0;
}

//�����ܼ��Ƕ��ٺ���
uint64_t zce::total_milliseconds(const timeval &tv)
{
    //����Ĳ���������Ϊ��Ҫת�������������ڵġ�
    return static_cast<uint64_t>(tv.tv_sec) * SEC_PER_MSEC + tv.tv_usec / MSEC_PER_USEC;
}

//����timeval�ڲ��ܼ��Ƕ���΢��10-6
uint64_t zce::total_microseconds(const timeval &tv)
{
    //����Ĳ���������Ϊ��Ҫת�������������ڵġ�
    return static_cast<uint64_t>(tv.tv_sec) * SEC_PER_USEC + tv.tv_usec ;
}

//�Ƚ�ʱ���Ƿ�һ��,���һ�·���0��left�󣬷���������right�󷵻ظ���
int zce::timeval_compare(const  timeval &left, const timeval &right)
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

//������ʱ��������,û�������ӵ�������
const timeval zce::timeval_add(const timeval &left, const timeval &right)
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

//������ʱ��������,û�������ӵ�������,��������>0����ֵ
//safe == true��֤����ֵ>=0,
const  timeval zce::timeval_sub(const timeval &left, const  timeval &right, bool safe)
{
    int64_t left_usec_val = (int64_t)left.tv_sec * SEC_PER_USEC + left.tv_usec;
    int64_t right_usec_val = (int64_t)right.tv_sec * SEC_PER_USEC + right.tv_usec;

    //��64λ��Ϊ��׼ȥ��
    int64_t minus_usec_val = left_usec_val - right_usec_val;

    timeval minus_time_val;

    // >0 ���߱�ʶ��������͵�����0
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


//������TIMEVALUE�Ƿ���ʣ���ʱ��
void zce::timeval_adjust(timeval &tv)
{
    int64_t tv_usec_val = (int64_t)tv.tv_sec * SEC_PER_USEC + tv.tv_usec;

    tv.tv_sec = static_cast<long>( tv_usec_val / SEC_PER_USEC );
    tv.tv_usec = static_cast<long>( tv_usec_val % SEC_PER_USEC);
}

//������TIMEVALUE�Ƿ���ʣ���ʱ��
bool zce::timeval_havetime(const timeval &tv)
{
    int64_t tv_usec_val = (int64_t)tv.tv_sec * SEC_PER_USEC + tv.tv_usec;

    if (tv_usec_val > 0)
    {
        return true;
    }

    return false;
}

//��ֻtimeval����ṹ
const timeval zce::make_timeval(time_t sec, time_t usec)
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

//ת���õ�timeval����ṹ
const timeval zce::make_timeval(std::clock_t clock_value)
{
    timeval to_timeval;

    to_timeval.tv_sec = clock_value /  CLOCKS_PER_SEC;
    clock_t remain_val = clock_value %  CLOCKS_PER_SEC;

    // Windowsƽ̨��tv_sec�������long,������Ҫת��
    to_timeval.tv_usec = static_cast<decltype(to_timeval.tv_usec)>(
                             (remain_val  * SEC_PER_USEC / CLOCKS_PER_SEC)) ;

    return to_timeval;
}

//ת���õ�timeval����ṹ
const timeval zce::make_timeval(const ::timespec *timespec_val)
{
    //ÿ�����Լ�������δ��붼���ɻ�ð��죬ʵ����û�д��ðɣ�д��ע�Ͱѣ�
    //NSEC ���� 10-9s
    //USEC ΢�� 10-6s
    timeval to_timeval;

    to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>( timespec_val->tv_sec);
    to_timeval.tv_usec = timespec_val->tv_nsec / USEC_PER_NSEC;
    return to_timeval;
}

const timeval zce::make_timeval(const std::chrono::hours &val)
{
    timeval to_timeval;
    to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>(val.count() * zce::ONE_HOUR_SECONDS);
    to_timeval.tv_usec = 0;
    return to_timeval;
}
const timeval zce::make_timeval(const std::chrono::minutes &val)
{
    timeval to_timeval;
    to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>(val.count() * zce::ONE_MINUTE_SECONDS);
    to_timeval.tv_usec = 0;
    return to_timeval;
}
const timeval zce::make_timeval(const std::chrono::seconds &val)
{
    timeval to_timeval;
    to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>(val.count());
    to_timeval.tv_usec = 0;
    return to_timeval;
}
const timeval zce::make_timeval(const std::chrono::milliseconds &val)
{
    timeval to_timeval;
    to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>(val.count() / SEC_PER_MSEC);
    to_timeval.tv_usec = (val.count() % SEC_PER_MSEC) * 1000;
    return to_timeval;
}
const timeval zce::make_timeval(const std::chrono::microseconds &val)
{
    timeval to_timeval;
    to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>( val.count() / SEC_PER_USEC);
    to_timeval.tv_usec = static_cast<decltype(to_timeval.tv_usec)>((val.count() % SEC_PER_USEC));
    return to_timeval;
}
const timeval zce::make_timeval(const std::chrono::nanoseconds &val)
{
    timeval to_timeval;
    to_timeval.tv_sec = static_cast<decltype(to_timeval.tv_sec)>(val.count() / SEC_PER_NSEC);
    to_timeval.tv_usec = static_cast<decltype(to_timeval.tv_usec)>((val.count() % SEC_PER_NSEC) / 1000);
    return to_timeval;
}

//��CPP11��time_point�����ݽṹת���õ�timeval�ṹ
const timeval zce::make_timeval(const std::chrono::system_clock::time_point &val)
{
    std::chrono::nanoseconds tval =
        std::chrono::duration_cast<std::chrono::nanoseconds>(val.time_since_epoch());
    return zce::make_timeval(tval);
}
const timeval zce::make_timeval(const std::chrono::steady_clock::time_point &val)
{
    std::chrono::nanoseconds tval =
        std::chrono::duration_cast<std::chrono::nanoseconds>(val.time_since_epoch());
    return zce::make_timeval(tval);
}

#if defined (ZCE_OS_WINDOWS)

//ת��FILETIME��timeval
const timeval zce::make_timeval(const FILETIME *file_time)
{
    timeval to_timeval;

    ULARGE_INTEGER ui;
    ui.LowPart = file_time->dwLowDateTime;
    ui.HighPart = file_time->dwHighDateTime;

    //The FILETIME structure is a 64-bit value representing the number of
    //100-nanosecond intervals since January 1, 1601.

    //�õ�time_t����
    to_timeval.tv_sec = static_cast<long>((ui.QuadPart - 116444736000000000) / 10000000);
    //�õ�΢�벿�֣�FILETIME��ŵ���100-nanosecond
    to_timeval.tv_usec = static_cast<long>(((ui.QuadPart - 116444736000000000) % 10000000) / 10);

    return to_timeval;
}

//ת��SYSTEMTIME��timeval
const timeval zce::make_timeval(const SYSTEMTIME *system_time)
{
    FILETIME ft;
    ::SystemTimeToFileTime(system_time, &ft);
    return make_timeval(&ft);
}

//ת��FILETIME��timeval,����ǰ�FILETIME����һ��ʱ���������е�
const timeval zce::make_timeval2(const FILETIME *file_time)
{
    timeval to_timeval;

    ULARGE_INTEGER ui;
    ui.LowPart = file_time->dwLowDateTime;
    ui.HighPart = file_time->dwHighDateTime;

    //FILETIME�ĵ�λ��100-nanosecond
    to_timeval.tv_sec = static_cast<long>((ui.QuadPart) / 10000000);
    to_timeval.tv_usec = static_cast<long>(((ui.QuadPart) % 10000000) / 10);

    return to_timeval;
}

#endif

//----------------------------------------------------------------------------------------------------
//ת���õ�timeval����ṹ
const ::timespec zce::make_timespec(const ::timeval *timeval_val)
{
    //ÿ�����Լ�������δ��붼���ɻ�ð��죬ʵ����û�д��ðɣ�д��ע�Ͱѣ�
    //NSEC ���� 10-9s
    //USEC ΢�� 10-6s
    ::timespec to_timespec;

    to_timespec.tv_sec =  timeval_val->tv_sec;
    to_timespec.tv_nsec = timeval_val->tv_usec * USEC_PER_NSEC;

    return to_timespec;
}

//�����ܼ��Ƕ��ٺ���
uint64_t zce::total_milliseconds(const ::timespec &ts)
{
    //����Ĳ���������Ϊ��Ҫת�������������ڵġ�
    return static_cast<uint64_t>(ts.tv_sec) * SEC_PER_MSEC + ts.tv_nsec / MSEC_PER_NSEC;
}

//----------------------------------------------------------------------------------------------------
//���ߺ���
//������ߺ���
int zce::sleep (uint32_t seconds)
{
#if defined (ZCE_OS_WINDOWS)
    ::Sleep (seconds * SEC_PER_MSEC);
    return 0;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::sleep (seconds);
#endif //
}

//��Ϣһ��timeval��ʱ��
int zce::sleep (const timeval &tv)
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

//΢������ߺ���
int zce::usleep (unsigned long usec)
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

uint64_t zce::rdtsc()
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
        //��ȡedx��eax��
        mov hiword, edx
        mov loword, eax
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
