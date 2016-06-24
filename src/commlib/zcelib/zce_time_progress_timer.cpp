#include "zce_predefine.h"
#include "zce_trace_debugging.h"
#include "zce_os_adapt_sysinfo.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_time.h"
#include "zce_time_progress_timer.h"

/************************************************************************************************************
Class           : ZCE_Progress_Timer 用于记录一个事件用时的计时器
************************************************************************************************************/

ZCE_Progress_Timer::ZCE_Progress_Timer():
    start_time_(0),
    end_time_(0),
    addup_time_(0)
{
}

ZCE_Progress_Timer::~ZCE_Progress_Timer()
{
}

//从新开始计时
void ZCE_Progress_Timer::restart()
{
    end_time_ = 0;
    addup_time_ = 0;
    start_time_ = std::clock();
}

//结束计时
void ZCE_Progress_Timer::end()
{
    end_time_ = std::clock();
}

//累计计时开始
void ZCE_Progress_Timer::addup_start()
{
    if (end_time_ > start_time_)
    {
        addup_time_ += end_time_ - start_time_;
    }

    end_time_ = 0;
    start_time_ = std::clock();
}

//计算消耗的时间
double ZCE_Progress_Timer::elapsed_sec() const
{
    //暂时去掉这个断言，在WINDOWS平台，即使你使用正确，也可能出现这个断言,在类的说明写清楚了，自己阅读
    //ZCE_ASSERT(end_time_ > start_time_);

    ZCE_ASSERT(end_time_ > 0);
    if (end_time_ > start_time_)
    {
        return (double(end_time_) - start_time_ + addup_time_) / CLOCKS_PER_SEC;
    }
    else
    {
        return  ( addup_time_) / CLOCKS_PER_SEC;
    }

}

//返回最小的计时精度单位（s），各个平台不太一致，
double ZCE_Progress_Timer::precision()
{
    return double(1) / double(CLOCKS_PER_SEC);
}

/************************************************************************************************************
Class           : ZCE_Auto_Progress_Timer 利用析构自动停止的的计时器
************************************************************************************************************/

ZCE_Auto_Progress_Timer::ZCE_Auto_Progress_Timer()
{
}
ZCE_Auto_Progress_Timer::~ZCE_Auto_Progress_Timer()
{
    end();
    std::cout << "This operation use time " << std::setprecision(6) << elapsed_sec() << " second." << std::endl;
}

/************************************************************************************************************
Class           : ZCE_HR_Progress_Timer 高性能计时器
************************************************************************************************************/
ZCE_HR_Progress_Timer::ZCE_HR_Progress_Timer()
{

#if defined ZCE_OS_WINDOWS

    start_time_.QuadPart = 0;
    end_time_.QuadPart = 0;
    addup_time_.QuadPart = 0;
    frequency_.QuadPart = 0;
    old_affinity_mask_ = 0;
#elif defined ZCE_OS_LINUX

    start_time_.tv_sec = 0;
    start_time_.tv_nsec = 0;
    end_time_.tv_sec = 0;
    end_time_.tv_nsec = 0;
    addup_time_ = 0;

    int ret = ::clock_getres(CLOCK_MONOTONIC, &precision_);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "::clock_getres return fail. error is %d.", ZCE_LIB::last_error());
    }

#endif
}

ZCE_HR_Progress_Timer::~ZCE_HR_Progress_Timer()
{
}

//从新开始计时
void ZCE_HR_Progress_Timer::restart()
{
#if defined ZCE_OS_WINDOWS
    //如果设置不成功，会返回0
    old_affinity_mask_ = ::SetThreadAffinityMask(GetCurrentThread(), ONLY_YOU_PROCESSOR);
    //
    ::QueryPerformanceFrequency(&frequency_);
    ::QueryPerformanceCounter(&start_time_);
    end_time_.QuadPart = 0;
    addup_time_.QuadPart = 0;
#elif defined ZCE_OS_LINUX

    int ret = ::clock_gettime(CLOCK_MONOTONIC, &start_time_);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "::clock_gettime return fail. error is %d.", ZCE_LIB::last_error());
    }

    end_time_.tv_sec = 0;
    end_time_.tv_nsec = 0;
    addup_time_ = 0;

#endif
}

//累计计时开始,用于多次计时的过程，
void ZCE_HR_Progress_Timer::addup_start()
{
#if defined ZCE_OS_WINDOWS

    addup_time_.QuadPart += end_time_.QuadPart - start_time_.QuadPart;
    //如果设置不成功，会返回0
    old_affinity_mask_ = ::SetThreadAffinityMask(GetCurrentThread(), ONLY_YOU_PROCESSOR);
    //
    ::QueryPerformanceFrequency(&frequency_);
    ::QueryPerformanceCounter(&start_time_);
    end_time_.QuadPart = 0;

#elif defined ZCE_OS_LINUX

    const uint64_t NSEC_PER_SEC = 1000000000ULL;

    addup_time_ += ((end_time_.tv_sec * NSEC_PER_SEC  + end_time_.tv_nsec ) -
                    (start_time_.tv_sec * NSEC_PER_SEC + start_time_.tv_nsec ));

    int ret = ::clock_gettime(CLOCK_MONOTONIC, &start_time_);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "::clock_gettime return fail. error is %d.", ZCE_LIB::last_error());
    }

    end_time_.tv_sec = 0;
    end_time_.tv_nsec = 0;

#endif
}

//结束计时
void ZCE_HR_Progress_Timer::end()
{
#if defined ZCE_OS_WINDOWS

    ::QueryPerformanceCounter(&end_time_);
    //还原线程的处理器绑定关系
    if (old_affinity_mask_ != 0)
    {
        ::SetThreadAffinityMask(GetCurrentThread(), old_affinity_mask_);
    }

#elif defined ZCE_OS_LINUX

    int ret = ::clock_gettime(CLOCK_MONOTONIC, &end_time_);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "::clock_gettime return fail. error is %d.", ZCE_LIB::last_error());
    }
#endif
}

//计算消耗的时间(us)
double ZCE_HR_Progress_Timer::elapsed_usec() const
{

#if defined ZCE_OS_WINDOWS

    ZCE_ASSERT(end_time_.QuadPart >= start_time_.QuadPart);
    const double USEC_PER_SEC = 1000000.0;
    return double(end_time_.QuadPart - start_time_.QuadPart + addup_time_.QuadPart ) * USEC_PER_SEC / frequency_.QuadPart;
#elif defined ZCE_OS_LINUX

    const uint64_t NSEC_PER_SEC = 1000000000ULL;

    ZCE_ASSERT((end_time_.tv_sec * NSEC_PER_SEC  + end_time_.tv_nsec ) >
               (start_time_.tv_sec * NSEC_PER_SEC + start_time_.tv_nsec ));
    const double NSEC_PER_USEC = 1000.0;

    return ((end_time_.tv_sec * NSEC_PER_SEC  + end_time_.tv_nsec ) -
            (start_time_.tv_sec * NSEC_PER_SEC + start_time_.tv_nsec ) + addup_time_) / NSEC_PER_USEC ;
#endif
}

//得到计时器的精度（us微秒-6）
double ZCE_HR_Progress_Timer::precision_usec()
{
#if defined ZCE_OS_WINDOWS
    const double USEC_PER_SEC = 1000000.0;
    return USEC_PER_SEC / ((uint64_t)( frequency_.QuadPart));
#elif defined ZCE_OS_LINUX
    const double NSEC_PER_USEC = 1000.0;
    const uint64_t NSEC_PER_SEC = 1000000000LL;
    return (precision_.tv_sec * NSEC_PER_SEC  + precision_.tv_nsec ) * NSEC_PER_USEC ;
#endif
}

/************************************************************************************************************
Class           : ZCE_TSC_Progress_Timer TSC计时器，
************************************************************************************************************/
uint64_t ZCE_TSC_Progress_Timer::cpu_hz_ = 0;

//构造函数
ZCE_TSC_Progress_Timer::ZCE_TSC_Progress_Timer():
    start_time_(0),
    end_time_(0),
    addup_time_(0)
{
}

//析构函数
ZCE_TSC_Progress_Timer::~ZCE_TSC_Progress_Timer()
{
}

///从新开始计时
void ZCE_TSC_Progress_Timer::restart()
{
    end_time_ = 0;
    addup_time_ = 0;
    start_time_ = ZCE_LIB::rdtsc();
}

//结束计时
void ZCE_TSC_Progress_Timer::end()
{
    end_time_ = ZCE_LIB::rdtsc();
}

///累计计时开始,用于多次计时的过程，
void ZCE_TSC_Progress_Timer::addup_start()
{
    if (end_time_ > start_time_)
    {
        addup_time_ += end_time_ - start_time_;
    }
    //会有一些特殊情况，导致 start_time_ > end_time_,最大可能是在不同的CPU上计时了,

    end_time_ = 0;
    start_time_ = ZCE_LIB::rdtsc();
}

//计算消耗的TICK（CPU周期）数量，注意这个值，只能在自己的机器上做对比才有意义，
uint64_t ZCE_TSC_Progress_Timer::elapsed_tick() const
{
    if (end_time_ > start_time_)
    {
        return end_time_ - start_time_ + addup_time_;
    }
    return addup_time_;
}

//计算消耗的时间(us),注意这个数值不会太准确
double ZCE_TSC_Progress_Timer::elapsed_usec() const
{
    const uint64_t DEFAULT_CPU_HZ = 1024 * 1024 * 1024;
    int ret = 0;
    //如果静态变量没有初始化
    if ( 0 == cpu_hz_ )
    {
        ZCE_SYSTEM_INFO system_info;
        ret = ZCE_LIB::get_system_info(&system_info);
        if (0 == ret )
        {
            cpu_hz_ = system_info.cpu_hz_;
        }
        else
        {
            ZCE_LOG(RS_ERROR, "ZCE_LIB::get_system_info return fail. cpu use default 1G.");
            //用1G作为作为默认值
            cpu_hz_ = DEFAULT_CPU_HZ;
        }
    }
    const uint64_t USEC_PER_SEC = 1000 * 1000;
    return double (end_time_ - start_time_ + addup_time_) / double(cpu_hz_ * USEC_PER_SEC );
}

