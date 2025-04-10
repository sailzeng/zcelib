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
    int ret = ::clock_gettime(CLOCK_MONOTONIC, &sp);

    if (ret == 0)
    {
        *tv = zce::make_timeval(&sp);
    }
    else
    {
        ZCE_LOG(RS_ERROR, "::clock_gettime(CLOCK_MONOTONIC, &sp) ret != 0,fail.ret = %d "
                "lasterror = %d", ret, zce::last_error());
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
    int ret = ::clock_gettime(CLOCK_MONOTONIC, &sp);
    if (ret == 0)
    {
        return (uint64_t)sp.tv_sec * SEC_PER_MSEC + (uint64_t)sp.tv_nsec / MSEC_PER_NSEC;
    }
    else
    {
        ZCE_LOG(RS_ERROR, "::clock_gettime(CLOCK_MONOTONIC, &sp) ret != 0,fail.ret = %d lasterror = %d",
                ret,
                zce::last_error());
        return 0;
    }
#endif
}

//
//得到当前的系统时间字符串输出
const char* zce::timestamp(char* str_date_time, size_t datetime_strlen)
{
    timeval now_time_val(zce::gettimeofday());
    return zce::timestamp(&now_time_val, str_date_time, datetime_strlen);
}

//将参数timeval的值作为的时间格格式化后输出打印出来
const char* zce::timestamp(const timeval* timeval, char* str_date_time, size_t datetime_strlen)
{
    ZCE_ASSERT(datetime_strlen > zce::TS_ISO_USEC_LEN);

    //转换为语句
    time_t now_time = timeval->tv_sec;
    tm tm_data;
    zce::localtime_r(&now_time, &tm_data);

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

const ::tm zce::make_tm(const zce::ztm* pztm) noexcept
{
    ::tm tmv;
    tmv.tm_year = pztm->year_;
    tmv.tm_mon = pztm->mon_;
    tmv.tm_mday = pztm->day_;
    tmv.tm_hour = pztm->hour_;
    tmv.tm_min = pztm->min_;
    tmv.tm_sec = pztm->sec_;
    tmv.tm_wday = -1;
    tmv.tm_yday = -1;
    tmv.tm_isdst = -1;
    return tmv;
}

const timeval zce::make_timeval(bool uct_time, const zce::ztm* pztm) noexcept
{
    timeval tv;
    ::tm tmp_tm = zce::make_tm(pztm);
    if (uct_time)
    {
#if defined ZCE_OS_WINDOWS
        tv.tv_sec = static_cast<long>(zce::timegm(&tmp_tm));
        tv.tv_usec = static_cast<long>(pztm->usec_);
#else
        tv.tv_sec = zce::timegm(&tm_value);
        tv.tv_usec = pztm->usec_;
#endif
    }
    else
    {
#if defined ZCE_OS_WINDOWS
        tv.tv_sec = static_cast<long>(zce::timelocal(&tmp_tm));
        tv.tv_usec = static_cast<long>(pztm->usec_);
#else
        tv.tv_sec = zce::timelocal(&tm_value);
        tv.tv_usec = pztm->usec_;
#endif
    }
    if (tv.tv_usec < 0 && tv.tv_usec > 999999)
    {
        tv.tv_usec = -1;
    }
    return tv;
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
int zce::timeval_compare(const timeval& left, const timeval& right)
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
const timeval zce::timeval_add(const timeval& left, const timeval& right)
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
const  timeval zce::timeval_sub(const timeval& left, const  timeval& right, bool safe)
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
const timeval zce::make_timeval(time_t sec, time_t usec) noexcept
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
    ::SystemTimeToFileTime(system_time, &ft);
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
        mov hiword, edx
        mov loword, eax
    }
    tsc_value = (uint64_t(hiword) << 32) + loword;
    tsc_value = __rdtsc();
#elif defined (ZCE_WIN64)

    int registers[4];
    __cpuid(registers, 0);
    tsc_value = __rdtsc();

#elif defined (ZCE_OS_LINUX)

    uint32_t hiword, loword;
    asm("cpuid");
    asm volatile("rdtsc" : "=a" (hiword), "=d" (loword));
    tsc_value = (uint64_t(hiword) << 32) + loword;
#endif

    return tsc_value;
}
