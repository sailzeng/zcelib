/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_time.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��5��1��
* @brief      ʱ��������������㣬��Ҫ������LINUX�¿�£
*
* @details
*
* @note       timeval�����׿��ڲ�ʹ�ý϶࣬�������㳹��������ʶ�ڲ���ʱ�䣬�����������������Լ���Time_Value�࣬
*             ��������Ϊ���д����ʱ��timeval�ĺܶ��������װд�������Ƿ�����
*/

#ifndef ZCE_LIB_OS_ADAPT_TIME_H_
#define ZCE_LIB_OS_ADAPT_TIME_H_

#include "zce_os_adapt_predefine.h"




#if defined ZCE_OS_WINDOWS

struct timezone
{
    //minutes west of Greenwich,ע������Ƿ���
    int tz_minuteswest;
    // type of DST correction ������ʱ�����ֵ��LINUX�¶�����ʹ���ˣ��������Ұ�
    //int tz_dsttime;
};

#endif

namespace zce
{
//һ��Сʱ��ʱ��
static const time_t ONE_HOUR_SECONDS = 3600;
//һ���ӵ�����
static const time_t ONE_MINUTE_SECONDS = 60;
//
static const time_t ONE_QUARTER_SECONDS = 15 * 60;

static const time_t FIVE_MINUTE_SECONDS = 5 * 60;
//һ�������86400
static const time_t ONE_DAY_SECONDS = 86400;
//���ֹ���ʱ��Ķ���
static const time_t ONE_WEEK_DAYS = 7;
//һ�ܵ�����
static const time_t ONE_WEEK_SECONDS = 604800;


static const time_t SEC_PER_MSEC  = 1000;
static const time_t USEC_PER_NSEC = 1000;
static const time_t MSEC_PER_USEC = 1000;
static const time_t SEC_PER_USEC  = 1000000;
static const time_t SEC_PER_NSEC  = 1000000000;
static const time_t MSEC_PER_NSEC = 1000000;


#if defined ZCE_OS_WINDOWS
static const time_t TIMEZONE_SECONDS = _timezone;
#else
static const time_t TIMEZONE_SECONDS = timezone;
#endif

//��ʱ���ĽǶȣ���1970.1.1�����ڶ�����
#ifndef HOW_MANY_SECONDS_TZ
#define HOW_MANY_SECONDS_TZ(x)  (x + zce::TIMEZONE_SECONDS)
#endif
//��ʱ���ĽǶȣ�ȡ��1970.1.1�����ڵ�������,�����ж��ǲ���ͬһ��֮�������
#ifndef HOW_MANY_DAYS_TZ
#define HOW_MANY_DAYS_TZ(x)  (HOW_MANY_SECONDS_TZ(x)/zce::ONE_DAY_SECONDS)
#endif
//��ʱ���ĽǶȣ�ȡ��1970.1.1�����ڵ�Сʱ����
#ifndef HOW_MANY_HOURS_TZ
#define HOW_MANY_HOURS_TZ(x) (HOW_MANY_SECONDS_TZ(x) / zce::ONE_HOUR_SECONDS)
#endif
//��ʱ���ĽǶȣ�ȡ��1970.1.1�����ڵ�Сʱ���� ����ΪɶҪ+3����Ϊ19700101������ġ�����ע������ɣ�����һ������
#ifndef HOW_MANY_WEEKS_TZ
#define HOW_MANY_WEEKS_TZ(x)  (( HOW_MANY_SECONDS_TZ(x) + zce::ONE_DAY_SECONDS * 3)/zce::ONE_WEEK_SECONDS)
#endif

/*!
* @brief      �Ǳ�׼�������õ����������������ڵ�ʱ�䣬���ʱ���Ǹ����Ե�����ֵ���������
* @detail     ԭ��ģ��ĺ�����gethrtime����Ϊ�����䷵�ص�λ�ȽϹ֣����Ƶõ�CPU�����������ڵ�TICK��ʱ�䣬
*             ����pascal���˸��˺������ƣ�Ҳ����Ϊ��һ������������
* @return     const timeval
* @note       WINDOWS (��֧��WIN2008�Ļ����ϣ���������) ʹ�õ���GetTickCount ����������������غ���������
*             49��ͻ�������ڲ�Ϊ����һ�ε��õ�ʱ�䣬����static ��������Ϊ�˱���static������������������Ҫ
*             ��֤����Ƶ�����⡣������ε���֮���ʱ���������������49�죬���޷���֤��õ�׼ȷ��ֵ
*             ���ϲ�Ҫ49���ֻ����һ���������ѽ�������ұ�֤�������TICK��Ч������������ÿ�����һ�ΰɡ�
*             LINUX �£����Ժ��ֺܶ�ϵͳû��gethrtime��������POSIX���º���clock_gettime���档
*/
const timeval  get_uptime(void);

/*!
* @brief      ȡ�õ�ǰ��ʱ�䣬��ǽ��ʱ�ӣ�
* @return     inline int  ==0��ʾ�ɹ�
* @param      [out]tv     ��ǰ��ʱ��
* @param      [out]tz     ʱ�����ڲ�û�д�������ʱ���⣬LINUX��ϵͳĿǰҲ��֧����
* @note       gettimeofday ���������POSIX���Ѿ������������Ǵ�����LINUX�������ʹ�ã�������ʱ��������
*             POSIX�Ƽ���clock_gettime����û�п�������������
*/
inline int gettimeofday(struct timeval *tv, struct timezone *tz = NULL);


/*!
* @brief      ȡ�õ�ǰ��ʱ��
* @return     inline const timeval ����ǰʱ��timeval��Ϊ�������
*/
inline const timeval gettimeofday();


/*!
* @brief      ȡ�õ�ǰ��ʱ��,�Ǳ�׼������Ϊ��ǰʱ����GMT����������
* @return     int Ϊ��ǰʱ����GMT����������
* @note       Ϊʲô���룬�����ǵ�һʱ�����ڶ�ʱ������������˽�ʱ����
*             ��������ˣ�ʱ������+0630������ʱ��
*             ������timezoneҪ�ȵ���tzset���ܳ�ʼ����
*/
int gettimezone();


/*!
* @brief      ������timeval��ֵ��Ϊ��ʱ����ʽ���������ӡ����
*             ʱ�����ӡ����,�ַ����������ʽ��ISO-8601 format. ���� 2010-09-10 10:03:18.100190
*             �����ʱ�����ʽΪ2010-09-10 10:03:18.100190       ע��ĩβ����һ��\0
*                           123456789012345678901234567890
* @return     const char*         �õ���ʱ���ַ���
* @param[out] str_date_time   �ַ���
* @param[in]  datetime_strlen �ַ�������
* @note
*/
const char *timestamp(const timeval *timeval,
                      char *str_date_time,
                      size_t datetime_strlen);

/*!
* @brief      �õ���ǰ��ϵͳʱ���ַ������
* @return     const char*
* @param      str_date_time
* @param      datetime_strlen
* @note
*/
const char *timestamp(char *str_date_time,
                      size_t datetime_strlen);


///ʱ���ʽ������ĸ�ʽ����
enum TIME_STR_FORMAT_TYPE
{

    ///�ý��յĸ�ʽ������� 20100910
    TIME_STRFMT_COMPACT_DAY  = 1,
    ///�ý��յĸ�ʽ������� 20100910100318
    TIME_STRFMT_COMPACT_SEC  = 2,

    ///��ISO�ĸ�ʽ����ʱ����������ȵ��� 2010-09-10
    TIME_STRFMT_ISO_DAY      = 5,
    ///��ISO�ĸ�ʽ����ʱ����������ȵ��룬2010-09-10 10:03:18
    TIME_STRFMT_ISO_SEC      = 6,
    ///��ISO�ĸ�ʽ����ʱ����������ȵ�΢�룬2010-09-10 10:03:18.100190
    TIME_STRFMT_ISO_USEC     = 7,

    ///��������ʱ���ʽ������� Fri Aug 24 2002 07:43:05
    TIME_STRFMT_US_SEC       = 10,
    ///��������ʱ���ʽ������� Fri Aug 24 2002 07:43:05.100190
    TIME_STRFMT_US_USEC      = 11,


    ///��HTTPͷ��GMT��ʱ���ʽ�������, Thu, 26 Nov 2009 13:50:19 GMT
    TIME_STRFMT_HTTP_GMT     = 1001,
    ///��EMAILͷ��DATE��ʱ���ʽ�������, Fri, 08 Nov 2002 09:42:22 +0800
    TIME_STRFMT_EMAIL_DATE   = 1002,
};

/*
20100910100318                            ����
2010-09-10 10:03:18.100190                ISO
Fri Aug 24 2002 07:43:05.100190           US
Thu, 26 Nov 2009 13:50:19 GMT             GMT(GMTһ�㲻������룬��HTTPͷ��Ӧ��)
Fri, 08 Nov 2002 09:42:22 +0800
1234567890123456789012345678901234567890
*/

///ע������ĳ��Ȳ���������'\0'������Ŀռ�Ҫ + 1����򵥵ļǷ����Ǳ�֤��32�ֽڵĿռ�
///������GMT��ȷ��us����������+1�ĳ��ȼ�¼������д��Ŀ���Ƿ���ĳЩ���㣬

///COMPACT ����ʱ���ʽ�ַ����ĳ��ȣ�
///����ַ������ȵ����ڵ��ַ�������
static const size_t TIMESTR_COMPACT_DAY_LEN = 8;
///����ַ������ȵ�����ַ�������
static const size_t TIMESTR_COMPACT_SEC_LEN = 14;

///ISO ʱ���ʽ���ַ����ĳ��ȣ�
///����ַ������ȵ����ڵ��ַ�������
static const size_t TIMESTR_ISO_DAY_LEN = 10;
///����ַ������ȵ�����ַ�������
static const size_t TIMESTR_ISO_SEC_LEN = 19;
///[����]���ȵ�΢���
static const size_t TIMESTR_ISO_USEC_LEN = 26;

///US �׹�ʱ���ʽ�ַ����ĳ��ȣ����ȵ���
static const size_t TIMESTR_US_SEC_LEN = 24;
///US �׹�ʱ���ʽ�ַ����ĳ��ȣ����ȵ�΢��
static const size_t TIMESTR_US_USEC_LEN = 31;

///GMT ʱ���ʽ�ַ����ĳ��ȣ����ȵ���
static const size_t TIMESTR_HTTP_GMT_LEN = 29;

///GMT ʱ���ʽ�ַ����ĳ��ȣ����ȵ���
static const size_t TIMESTR_EMAIL_DATE_LEN = 31;

///����ʽ������
static const size_t MAX_TIMEVAL_STRING_LEN = TIMESTR_EMAIL_DATE_LEN;



/*!
* @brief      ������timeval��ֵ��Ϊ��ʱ����ʽ���������ӡ���������Կ��Ƹ��ָ�ʽ���
* @return     const char*   ���ص��ַ�������ʵ����str_date_time
* @param[in]  timeval       ��ӡ��ʱ��timeval
* @param[out] str_date_time �õ���ʱ���ַ���
* @param[in]  str_len       �ַ����ĳ��ȣ���򵥵ļǷ����Ǳ�֤��32�ֽڵĿռ�
* @param[in]  uct_time      ��timeval��ΪUCT/GMTʱ�仹�Ǳ���ʱ��Local Time��true
*                           ��ʾ��ΪUCT/GMTʱ�䣬false��ʾ��Ϊ����ʱ��
* @param      fmt           ������ο�@ref TIME_STR_FORMAT_TYPE ��
* @note       ʱ�����ӡ��ʽ˵��,TIME_STR_FORMAT_TYPE
*/
const char *timeval_to_str(const timeval *timeval,
                           char *str_date_time,
                           size_t str_len,
                           size_t &use_buf,
                           bool uct_time = false,
                           TIME_STR_FORMAT_TYPE fmt = zce::TIME_STRFMT_ISO_USEC
                          );

/*!
* @brief      ���ַ����и��ٵĵõ�tm�Ľṹ�Ľ��
* @param[in]  strtm   �ַ������ַ�������ȷ�����Լ�Ҫ��֤
* @param[in]  fmt     �ַ����ĸ�ʽ���ο�ö��ֵ @ref TIME_STR_FORMAT_TYPE
* @param[out] ptr_tm  ���ص�tm�ṹ��ָ�룬ע�⣬����ַ������󣬿��ܻᵼ��tm�����
* @param[out] usec    ���صĵ�΢���ʱ�䣬Ĭ��ΪNULL����ʾ����Ҫ���أ�
* @param[out] tz      ���صĵ�ʱ��,Ĭ��ΪNULL����ʾ����Ҫ���أ��ܶ��ָ�ʽ����û��ʱ����Ϣ
*/
void str_to_tm(const char *strtm,
               TIME_STR_FORMAT_TYPE fmt,
               tm *ptr_tm,
               time_t *usec = NULL,
               int *tz = NULL);


/*!
* @brief      ���ַ���ת���õ�����ʱ��timeval����
* @return     int == 0 ��ʾ�ɹ�
* @param[in]  strtm    �ַ�������
* @param[in]  uct_time ��strtm�ַ�����ΪUCT/GMTʱ�仹�Ǳ���ʱ��Local Time
*                      true��ʾ��ΪUCT/GMTʱ�䣬false��ʾ��Ϊ����ʱ��
* @param[in]  fmt      �ַ����ĸ�ʽ���ο�ö��ֵ @ref TIME_STR_FORMAT_TYPE,

* @param[out] tval     ���ص�ʱ��,
*/
int str_to_timeval(const char *strtm,
                   TIME_STR_FORMAT_TYPE fmt,
                   bool uct_time,
                   timeval *tval);


///����ʱ���ַ���ת��Ϊtime_t
int localtimestr_to_time_t(const char *localtime_str,
                           TIME_STR_FORMAT_TYPE fmt,
                           time_t *time_t_val);

/*!
* @brief      ����timeval�ڲ��ܼ��Ƕ��ٺ���
* @return     uint64_t
* @param      tv  Ҫ�����timeval
*/
uint64_t total_milliseconds(const timeval &tv);

/*!
* @brief      ����timeval�ڲ��ܼ��Ƕ���΢��
* @return     uint64_t
* @param      tv  Ҫ�����timeval
*/
uint64_t total_microseconds(const timeval &tv);

//��չtimeval��һЩ������û��֮�����ز������ŵ�ԭ���Ǳ�����Ⱦ�����ռ�

/*!
* @brief      ����һ��0,0�ĵ�timeval�Ľṹ��
* @return     const timeval Ҫ�����timeval
*/
const timeval timeval_zero();

/*!
* @brief      ��tv����Ϊ0
* @param      tv  ��0��timeval
*/
void timeval_clear(timeval &tv);

/*!
* @brief      �Ƚ�ʱ���Ƿ�һ��,
* @return     int     ���һ�·���0��left�󣬷���������right�󷵻ظ���,
* @param      left    ��ֵ
* @param      right   ��ֵ
* @note       ע������timercmp�ǲ�һ���Ĺ��ܣ�timercmp��Ҫ���������һ������������ɱȽ�����
*/
int timeval_compare(const timeval &left, const timeval &right);

/*!
* @brief      ������ʱ�������ӣ���������أ��Ǳ�׼����
* @return     const timeval
* @param      left          ��ֵ
* @param      right         ��ֵ
* @note
*/
const timeval timeval_add(const timeval &left, const timeval &right);

/*!
* @brief      ������ʱ������������������أ��Ǳ�׼����,safe==true���С��0������0
* @return     const timeval ����Ľ��
* @param      left          ��
* @param      right         ��
* @param      safe          �Ƿ���а�ȫ������������б��������С��0ʱ������0
* @note
*/
const timeval timeval_sub(const timeval &left, const timeval &right, bool safe = true);

/*!
* @brief      ��������õ�usec �ܳ���>1s���Ұ�����������Ǳ�׼����
* @return     void
* @param      tv      ������timeval
*/
void timeval_adjust(timeval &tv);

/*!
* @brief      ������TIMEVALUE�Ƿ���ʣ���ʱ�䣬�Ǳ�׼����
* @return     bool
* @param      tv
*/
bool timeval_havetime(const timeval &tv);

/*!
* @brief      ����timeval����ṹ
* @return     const timeval
* @param      sec           ��
* @param      usec          ΢��
*/
const timeval make_timeval(time_t sec, time_t usec);

/*!
* @brief      ������Ϊstd::clock_tֵ ת���õ�timeval����ṹ
* @return     const timeval ת�����timeval���
* @param      clock_value   ����ת���Ĳ���
*/
const timeval make_timeval(std::clock_t clock_value);

/*!
* @brief      ��timespec�ṹת���õ�timeval�ṹ
* @return     const timeval ת�����timeval���
* @param      timespec_val  ����ת���Ĳ���
*/
const timeval make_timeval(const ::timespec *timespec_val);

/*!
* @brief      ��CPP11��duration�����ݽṹת���õ�timeval�ṹ
* @return     const timeval ת�����timeval���
* @param      val  ����ת���Ĳ���
*/
const timeval make_timeval(const std::chrono::hours &val);
const timeval make_timeval(const std::chrono::minutes &val);
const timeval make_timeval(const std::chrono::seconds &val);
const timeval make_timeval(const std::chrono::milliseconds &val);
const timeval make_timeval(const std::chrono::microseconds &val);
const timeval make_timeval(const std::chrono::nanoseconds &val);

/*!
* @brief      ��CPP11��time_point�����ݽṹת���õ�timeval�ṹ
* @return     const timeval ת�����timeval���
* @param      val  ����ת���Ĳ���
*/
const timeval make_timeval(const std::chrono::system_clock::time_point &val);
const timeval make_timeval(const std::chrono::steady_clock::time_point &val);


//WINDOWS API���õļ�������
#if defined (ZCE_OS_WINDOWS)
/*!
* @brief      ��FILETIME�Ĳ�����Ϊһ��ʱ�䣨����ʱ�� ��2013-01-01 01:53:29����ת���õ�timeval
* @return     const timeval
* @param      file_time
*/
const timeval make_timeval(const FILETIME *file_time);

/*!
* @brief      ת��SYSTEMTIME��timeval
* @return     const timeval
* @param      system_time
* @note
*/
const timeval make_timeval(const SYSTEMTIME *system_time);

/*!
* @brief      ��FILETIME�Ĳ�����Ϊһ��ʱ�������ʱ�� ��25s����ת��FILETIME��timeval ��
* @return     const timeval
* @param      file_time
*/
const timeval make_timeval2(const FILETIME *file_time);

#endif

//�������timespec������̫��֧�֣�

/*!
* @brief      POSIX �µ��Ƽ�ʹ�õ�ʱ�亯����
* @return     int
* @param      clk_id  Linux ��֧�ֺܶ��������ʱ�䣬��㣬����ʱ�䣬���̣��߳�ʱ���
*                     WIN ������Ч���ǣ� CLOCK_REALTIME �߾���ʵ��ʱ�䣬CLOCK_MONOTONIC��
*                     ��������㣩�������������ĵĲ�����
* @param      ts      ���ص�ʱ��
*/
inline int clock_gettime(clockid_t clk_id, timespec *ts);

/*!
* @brief      ����timespec�ڲ��ܼ��Ƕ��ٺ���
* @return     uint64_t
* @param      ts
*/
uint64_t total_milliseconds(const timespec &ts);

/*!
* @brief      ��timespec �ṹ��ת���õ�timeval����ṹ
* @return     const ::timespec
* @param      timeval_val
*/
const ::timespec make_timespec(const timeval *timeval_val);

//-------------------------------------------------------------------------------
//��Ϣһ�£���Ϣһ��

/*!
* @brief      SLEEP ��
* @return     int     0�ɹ���-1ʧ��
* @param      seconds ��Ϣ������
* @note
*/
int sleep (uint32_t seconds);

/*!
* @brief      SLEEP timeval��ʱ��
* @return     int  0�ɹ���-1ʧ��
* @param      tv   ��Ϣ��timevalʱ�䳤�ȵ�ʱ��
* @note
*/
int sleep (const timeval &tv);

/*!
* @brief      ��Ϣ΢�룬��ʵ����Ϣʱ�䣬�϶�����������Ǻ�
* @return     int   0�ɹ���-1ʧ��
* @param      usec  ΢��ʱ�䳤��
*/
int usleep (unsigned long usec);

//-------------------------------------------------------------------------------
//���밲ȫ��ʱ��ת�������ĺ���

/*!
* @brief      �õ�����ʱ���tm�ṹ
* @return     inline struct tm*
* @param      timep
* @param      result
* @note       ���밲ȫ
*/
inline struct tm *localtime_r(const time_t *timep, struct tm *result);

/*!
* @brief      ����GMʱ�䣬�õ�tm�ṹ
* @return     inline struct tm*
* @param      timep
* @param      result
* @note
*/
inline struct tm *gmtime_r(const time_t *timep, struct tm *result);

/*!
* @brief      ��ӡTM�ڲ���ʱ����Ϣ
* @return     inline char*
* @param      tm
* @param      buf            ������ַ������ȣ��������Լ���֤����26��
* @note       ����ַ����ĸ�ʽ��Wed Jan 02 02:03:55 1980\n\0.ǿ�Ҳ�����ʹ��,
*             ������ʵ�ڲ�֪��������ʱ�������ɶ�ô�����ȫ��Ϊ�˼��ݲ�д������������
*/
inline char *asctime_r(const struct tm *tm, char *buf);

/*!
* @brief      ��ӡtime_t����ʾ��ʱ����Ϣ
* @return     inline char*  ���ص�ʱ���ַ���
* @param      timep        ʱ��
* @param      buf          ������ַ������ȣ��������Լ���֤����26��
* @note       ����ַ����ĸ�ʽ��Wed Jan 02 02:03:55 1980\n\0.ǿ�Ҳ�����ʹ��,
*             ������ʵ�ڲ�֪��������ʱ�������ɶ�ô�����ȫ��Ϊ�˼��ݲ�д������������
*/
inline char *ctime_r(const time_t *timep, char *buf);


/*!
* @brief      ��ͬ��mktime,��tm��Ϊ����ʱ�䣬ת��Ϊtime_t
* @return     time_t ת���õ���UTC����ʱ��
* @param      tm ��Ϊ����ʱ���tm
*/
inline time_t timelocal(struct tm *tm);

/*!
* @brief      ����mktime�������ǰ�tm��ΪGMTʱ�䣬ת��Ϊtime_t
*             WINDOWS���и�mkgmtime��
* @return     time_t ת���õ���UTC����ʱ��
* @param      tm ��ΪGMTʱ���tm
*/
inline time_t timegm(struct tm *tm);

/*!
* @brief      ��ȡTSC��Read TSC(Time-Stamp Counter),CPU��ʱ������
* @return     uint64_t TSCֵ
* @note       ����ڶ��ʱ���Ѿ����ô���½�����Ϊ�������ļ䲻һ�£�CPU�Ķ�̬Ƶ�ʵ�����
*/
uint64_t rdtsc();


};

//-------------------------------------------------------------------------------

//����LINUX�µ�gettimeofday
inline int zce::gettimeofday(struct timeval *tv, struct timezone *tz)
{
    //
#if defined ZCE_OS_WINDOWS
    //
    if (tv == NULL && tz == NULL)
    {
        return 0;
    }

    //���������ʱ��
    if (tv)
    {
        //�õ�ϵͳʱ��,
        //���ԱȽ�::GetSystemTime �����������ʱ������������е����
        FILETIME   tfile;
        ::GetSystemTimeAsFileTime (&tfile);

        ULARGE_INTEGER ui;
        ui.LowPart = tfile.dwLowDateTime;
        ui.HighPart = tfile.dwHighDateTime;

        //The FILETIME structure is a 64-bit value representing the number of
        //100-nanosecond intervals since January 1, 1601.

        //�õ�time_t����
        tv->tv_sec = static_cast<long>((ui.QuadPart - 116444736000000000) / 10000000);
        //�õ�΢�벿�֣�FILETIME��ŵ���100-nanosecond
        tv->tv_usec = static_cast<long>(((ui.QuadPart - 116444736000000000) % 10000000) / 10);
    }

    //�õ�ʱ���������
    if (tz)
    {
        //�õ�ʱ����Ϣ
        TIME_ZONE_INFORMATION tzone;
        DWORD  ret = ::GetTimeZoneInformation(&tzone);

        if (ret == TIME_ZONE_ID_INVALID)
        {
            return -1;
        }

        //
        tz->tz_minuteswest = tzone.Bias;
        //����ʱ����رܣ�LINUX���Ѿ����������������

    }

    return 0;
#endif //

    //LINUX�µõ�ʱ��
#if defined ZCE_OS_LINUX
    //ֱ�ӵ���ϵͳ��
    return ::gettimeofday(tv, tz);
#endif //

}


//�õ�ʱ�����ʱ��
inline int zce::clock_gettime(clockid_t clk_id, timespec *ts)
{
#if defined ZCE_OS_WINDOWS
    timeval tv;
    if (CLOCK_REALTIME  == clk_id)
    {
        zce::gettimeofday(&tv);
    }
    else if (CLOCK_MONOTONIC  == clk_id)
    {
        tv = zce::get_uptime();
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


inline const timeval zce::gettimeofday()
{
    timeval now_time;
    zce::gettimeofday(&now_time);
    return now_time;
}

//�õ�����ʱ��
inline struct tm *zce::localtime_r(const time_t *timep, struct tm *result)
{
#if defined (ZCE_OS_WINDOWS)
    //WINDOWS��ʹ��Ĭ�ϵ�_s ϵ�е�API
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

//GMʱ��
inline struct tm *zce::gmtime_r(const time_t *timep, struct tm *result)
{
#if defined (ZCE_OS_WINDOWS)
    //WINDOWS��ʹ��Ĭ�ϵ�_s ϵ�е�API
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

//��ӡTM�ڲ���ʱ����Ϣ
inline char *zce::asctime_r(const struct tm *tm_data, char *buf)
{
#if defined (ZCE_OS_WINDOWS)

    //������Ҫ��֤buf������26���ַ�,����д��ʵ�����Σ���ΪLINUX��API��û�б�֤�����İ�ȫ��ֻ�ǽ������������
    //�����еķ���WINDOWS��API����LINUX
    const size_t I_GUESS_BUF_HAVE_ROOM_FOR_AT_LEAST_26_BYTES = 26;

    //WINDOWS��ʹ��Ĭ�ϵ�_s ϵ�е�API
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

//��ӡtime_t����ʶ��ʱ����Ϣ
inline char *zce::ctime_r(const time_t *timep, char *buf)
{
#if defined (ZCE_OS_WINDOWS)

    //������Ҫ��֤buf������26���ַ�,����ΪLINUX��API��û�б�֤�����İ�ȫ��ֻ�ǽ������������
    const size_t I_GUESS_BUF_HAVE_ROOM_FOR_AT_LEAST_26_BYTES = 26;

    //WINDOWS��ʹ��Ĭ�ϵ�_s ϵ�е�API
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



//��ͬ��mktime,��tm��Ϊ����ʱ�䣬ת��Ϊ����ʱ��time_t
inline time_t zce::timelocal(struct tm *tm)
{
#if defined (ZCE_OS_WINDOWS)
    return ::mktime(tm);
#endif

#if defined (ZCE_OS_LINUX)
    return ::timelocal(tm);
#endif
}


//����mktime�������ǰ�tm��ΪGMTʱ�䣬ת��Ϊ����ʱ��time_t
inline time_t zce::timegm(struct tm *tm)
{
#if defined (ZCE_OS_WINDOWS)
    return ::_mkgmtime(tm);
#endif

#if defined (ZCE_OS_LINUX)
    return ::timegm(tm);
#endif
}

#endif //ZCE_LIB_SHARE_MEM_PRE_DEFINE_H_

