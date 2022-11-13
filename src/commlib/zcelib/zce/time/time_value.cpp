#include "zce/predefine.h"
#include "zce/time/time_value.h"

namespace zce
{
//Time_Value默认的zero值
const time_value time_value::ZERO_TIME_VALUE(0, 0);
//
const time_value time_value::MAX_TIME_VALUE(0x7FFFFFFF, 0x7FFFFFFF);
//
const std::chrono::seconds time_value::ZERO_DURATION_VALUE(0);

//默认构造函数
time_value::time_value()
{
    zce_time_value_.tv_sec = 0;
    zce_time_value_.tv_usec = 0;
}
//析构函数
time_value::~time_value()
{
}

//构造函数，用timeval
time_value::time_value(const timeval& time_data)
{
    zce_time_value_ = time_data;
}

//构造函数，用::timespec
time_value::time_value(const ::timespec& timespec_val)
{
    zce_time_value_ = zce::make_timeval(&timespec_val);
}

//构造函数，几个时间数据数据
time_value::time_value(time_t sec, time_t usec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(sec);
    zce_time_value_.tv_usec = static_cast<long>(usec);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = sec;
    zce_time_value_.tv_usec = usec;
#endif
}

//构造函数，用time_t, usec被置为0
time_value::time_value(time_t sec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(sec);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = sec;
#endif

    zce_time_value_.tv_usec = 0;
}

time_value::time_value(const std::chrono::system_clock::time_point& val) :
    zce_time_value_(zce::make_timeval(val))
{
}
time_value::time_value(const std::chrono::steady_clock::time_point& val) :
    zce_time_value_(zce::make_timeval(val))
{
}

#ifdef ZCE_OS_WINDOWS
//构造函数，用LPFILETIME,FILETIME
time_value::time_value(LPFILETIME file_time) :
    zce_time_value_(zce::make_timeval(file_time))
{
}

//构造函数，用LPSYSTEMTIME,SYSTEMTIME
time_value::time_value(LPSYSTEMTIME system_time) :
    zce_time_value_(zce::make_timeval(system_time))
{
}

#endif

//
void time_value::set(const timeval& time_data)
{
    zce_time_value_ = time_data;
}
//
void time_value::set(const ::timespec& timespec_val)
{
    zce_time_value_ = zce::make_timeval(&timespec_val);
}
//
void time_value::set(time_t sec, time_t usec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(sec);
    zce_time_value_.tv_usec = static_cast<long>(usec);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = sec;
    zce_time_value_.tv_usec = usec;
#endif
}

//
void time_value::set(time_t sec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(sec);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = sec;
#endif
    zce_time_value_.tv_usec = 0;
}

#ifdef ZCE_OS_WINDOWS
//设置Time_Value, 用FILETIME
void time_value::set(LPFILETIME file_time)
{
    zce_time_value_ = zce::make_timeval(file_time);
}
//设置Time_Value, 用LPSYSTEMTIME
void time_value::set(LPSYSTEMTIME system_time)
{
    zce_time_value_ = zce::make_timeval(system_time);
}
#endif

//用clock_t 设置时间，被迫用这个函数名字，避免和其他函数冲突
void time_value::set_by_clock_t(clock_t time)
{
    zce_time_value_ = zce::make_timeval(time);
}

uint64_t time_value::total_sec() const
{
    return static_cast<uint64_t>(this->zce_time_value_.tv_sec) +
        this->zce_time_value_.tv_usec / zce::SEC_PER_USEC;
}
//得到总共多少毫秒
uint64_t time_value::total_msec() const
{
    return static_cast<uint64_t>(this->zce_time_value_.tv_sec) * zce::SEC_PER_MSEC
        + this->zce_time_value_.tv_usec / zce::MSEC_PER_USEC;
}

//四舍五入得到总共多少毫秒，其实不是真正的四舍五入，而是如果微秒有数据，就返回1毫秒，
uint64_t time_value::total_msec_round() const
{
    uint64_t ret_msec = total_msec();
    //可能微秒数据上有数据，但<1000,
    if (0 == ret_msec && this->zce_time_value_.tv_usec > 0)
    {
        ret_msec = 1;
    }
    return ret_msec;
}

//用毫秒作为单位设置Time_Value
void time_value::total_msec(uint64_t set_msec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(set_msec / zce::SEC_PER_MSEC);
    zce_time_value_.tv_usec = static_cast<long>((set_msec % zce::SEC_PER_MSEC) * zce::MSEC_PER_USEC);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = static_cast<time_t>(set_msec / zce::SEC_PER_MSEC);
    zce_time_value_.tv_usec = static_cast<time_t>((set_msec % zce::SEC_PER_MSEC) * zce::MSEC_PER_USEC);
#endif
}

//得到总共多少微秒
uint64_t time_value::total_usec() const
{
    return static_cast<uint64_t>(zce_time_value_.tv_sec) * zce::SEC_PER_USEC + zce_time_value_.tv_usec;
}

//用微秒作为单位，设置Time_Value，注意这个函数和usec函数的区别，usec函数是设置timeval的usec部分，
void time_value::total_usec(uint64_t set_usec)
{
    const int SEC_PER_UESC = 1000000;
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(set_usec / SEC_PER_UESC);
    zce_time_value_.tv_usec = static_cast<long>(set_usec % SEC_PER_UESC);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = static_cast<time_t>(set_usec / SEC_PER_UESC);
    zce_time_value_.tv_usec = static_cast<time_t>(set_usec % SEC_PER_UESC);
#endif
}

//得到Timevalue的秒部分
time_t time_value::sec(void) const
{
    return zce_time_value_.tv_sec;
}

// 设置Timevalue的秒部分
void time_value::sec(time_t sec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(sec);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = sec;
#endif
}

//得到Timevalue的微秒部分
time_t time_value::usec(void) const
{
    return zce_time_value_.tv_usec;
}

//设置Timevalue的微秒部分
void time_value::usec(time_t usec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_usec = static_cast<long>(usec);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_usec = usec;
#endif
}

//获取当前时间
void time_value::gettimeofday()
{
    zce_time_value_ = zce::gettimeofday();
}

//
time_value& time_value::operator += (const time_value& tv)
{
    zce_time_value_ = zce::timeval_add(zce_time_value_, tv.zce_time_value_);
    return *this;
}

// Subtract @a tv to this.
time_value& time_value::operator -= (const time_value& tv)
{
    //��֤����ֵ�����ʱ��>=0
    zce_time_value_ = zce::timeval_sub(zce_time_value_, tv.zce_time_value_, true);
    return *this;
}

// 两个时间进行比较 <
bool time_value::operator < (const time_value& tv)
{
    //�ȱȽ��룬
    if (zce_time_value_.tv_sec < tv.zce_time_value_.tv_sec)
    {
        return true;
    }

    //�ٱȽ�΢��
    if (zce_time_value_.tv_sec == tv.zce_time_value_.tv_sec &&
        zce_time_value_.tv_usec < tv.zce_time_value_.tv_usec)
    {
        return true;
    }

    return false;
}

// 两个时间进行比较 >
bool time_value::operator > (const time_value& tv)
{
    //�ȱȽ��룬
    if (zce_time_value_.tv_sec > tv.zce_time_value_.tv_sec)
    {
        return true;
    }

    //�ٱȽ�΢��
    if (zce_time_value_.tv_sec == tv.zce_time_value_.tv_sec &&
        zce_time_value_.tv_usec > tv.zce_time_value_.tv_usec)
    {
        return true;
    }

    return false;
}

//
bool time_value::operator <= (const time_value& tv)
{
    //�ȱȽ��룬
    if (zce_time_value_.tv_sec <= tv.zce_time_value_.tv_sec)
    {
        return true;
    }

    //�ٱȽ�΢��
    if (zce_time_value_.tv_sec == tv.zce_time_value_.tv_sec &&
        zce_time_value_.tv_usec <= tv.zce_time_value_.tv_usec)
    {
        return true;
    }

    return false;
}

/// True if @a tv1 >= @a tv2.
bool time_value::operator >= (const time_value& tv)
{
    //�ȱȽ��룬
    if (zce_time_value_.tv_sec > tv.zce_time_value_.tv_sec)
    {
        return true;
    }

    //�ٱȽ�΢��
    if (zce_time_value_.tv_sec == tv.zce_time_value_.tv_sec &&
        zce_time_value_.tv_usec >= tv.zce_time_value_.tv_usec)
    {
        return true;
    }

    return false;
}

/// True if @a tv1 == @a tv2.
bool time_value::operator == (const time_value& tv)
{
    if (zce_time_value_.tv_sec == tv.zce_time_value_.tv_sec &&
        zce_time_value_.tv_usec == tv.zce_time_value_.tv_usec)
    {
        return true;
    }

    return false;
}

/// True if @a tv1 != @a tv2.
bool time_value::operator != (const time_value& tv)
{
    return !(*this == tv);
}

/// Adds two time_value objects together, returns the sum.
time_value operator + (const time_value& tv1,
                       const time_value& tv2)
{
    return zce::timeval_add(tv1.zce_time_value_, tv2.zce_time_value_);
}

/// Subtracts two time_value objects, returns the difference.
time_value operator - (const time_value& tv1,
                       const time_value& tv2)
{
    //��֤����ֵ�����ʱ��>=0
    return zce::timeval_sub(tv1.zce_time_value_, tv2.zce_time_value_, true);
}

/// Returns the value of the object as a timeval.
time_value::operator timeval () const
{
    return zce_time_value_;
}

//
time_value::operator const timeval* () const
{
    return &zce_time_value_;
}

//
time_value::operator timeval* ()
{
    return &zce_time_value_;
}

//将时间打印出来
const char* time_value::to_string(char* str_date_time,
                                  size_t datetime_strlen,
                                  size_t& use_buf,
                                  bool utc_time,
                                  zce::TIME_STR_FORMAT fmt) const
{
    return zce::timeval_to_str(&(this->zce_time_value_),
                               str_date_time,
                               datetime_strlen,
                               use_buf,
                               utc_time,
                               fmt);
}

//从字符串中得到时间
int time_value::from_string(const char* strtm,
                            bool uct_time,
                            zce::TIME_STR_FORMAT fmt)
{
    return zce::str_to_timeval(strtm,
                               fmt,
                               uct_time,
                               &zce_time_value_
    );
}

const char* time_value::timestamp(char* str_date_time,
                                  size_t datetime_strlen
) const
{
    return zce::timestamp(&(this->zce_time_value_),
                          str_date_time,
                          datetime_strlen);
}
}