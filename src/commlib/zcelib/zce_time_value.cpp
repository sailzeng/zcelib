#include "zce_predefine.h"
#include "zce_time_value.h"

//ZCE_Time_ValueĬ�ϵ�zeroֵ
const ZCE_Time_Value ZCE_Time_Value::ZERO_TIME_VALUE(0, 0);
//
const ZCE_Time_Value ZCE_Time_Value::MAX_TIME_VALUE(0x7FFFFFFF, 0x7FFFFFFF);

//Ĭ�Ϲ��캯��
ZCE_Time_Value::ZCE_Time_Value()
{
    zce_time_value_.tv_sec = 0;
    zce_time_value_.tv_usec = 0;
}
//��������
ZCE_Time_Value::~ZCE_Time_Value()
{
}

//���캯������timeval
ZCE_Time_Value::ZCE_Time_Value(const timeval &time_data)
{
    zce_time_value_ = time_data;
}

//���캯������::timespec
ZCE_Time_Value::ZCE_Time_Value(const ::timespec &timespec_val)
{
    zce_time_value_ = zce::make_timeval(&timespec_val);
}

//���캯��������ʱ����������
ZCE_Time_Value::ZCE_Time_Value(time_t sec, time_t usec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(sec);
    zce_time_value_.tv_usec = static_cast<long>(usec);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = sec;
    zce_time_value_.tv_usec = usec;
#endif

}

//���캯������time_t, usec����Ϊ0
ZCE_Time_Value::ZCE_Time_Value(time_t sec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(sec);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = sec;
#endif

    zce_time_value_.tv_usec = 0;
}

ZCE_Time_Value::ZCE_Time_Value(const std::chrono::hours &val):
    zce_time_value_(zce::make_timeval(val))
{
}
ZCE_Time_Value::ZCE_Time_Value(const std::chrono::minutes &val):
    zce_time_value_(zce::make_timeval(val))
{
}
ZCE_Time_Value::ZCE_Time_Value(const std::chrono::seconds &val):
    zce_time_value_(zce::make_timeval(val))
{
}
ZCE_Time_Value::ZCE_Time_Value(const std::chrono::milliseconds &val):
    zce_time_value_(zce::make_timeval(val))
{

}
ZCE_Time_Value::ZCE_Time_Value(const std::chrono::microseconds &val):
    zce_time_value_(zce::make_timeval(val))
{

}
ZCE_Time_Value::ZCE_Time_Value(const std::chrono::nanoseconds &val):
    zce_time_value_(zce::make_timeval(val))
{
}

ZCE_Time_Value::ZCE_Time_Value(const std::chrono::system_clock::time_point &val) :
    zce_time_value_(zce::make_timeval(val))
{
}
ZCE_Time_Value::ZCE_Time_Value(const std::chrono::steady_clock::time_point &val) :
    zce_time_value_(zce::make_timeval(val))
{
}

#ifdef ZCE_OS_WINDOWS
//���캯������LPFILETIME,FILETIME
ZCE_Time_Value::ZCE_Time_Value(LPFILETIME file_time) :
    zce_time_value_(zce::make_timeval(file_time))
{
}

//���캯������LPSYSTEMTIME,SYSTEMTIME
ZCE_Time_Value::ZCE_Time_Value(LPSYSTEMTIME system_time) :
    zce_time_value_(zce::make_timeval(system_time))
{
}

#endif

//
void ZCE_Time_Value::set(const timeval &time_data)
{
    zce_time_value_ = time_data;
}
//
void ZCE_Time_Value::set(const ::timespec &timespec_val)
{
    zce_time_value_ = zce::make_timeval(&timespec_val);
}
//
void ZCE_Time_Value::set(time_t sec, time_t usec)
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
void ZCE_Time_Value::set(time_t sec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(sec);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = sec;
#endif
    zce_time_value_.tv_usec = 0;
}

void ZCE_Time_Value::set(const std::chrono::hours &val)
{
    zce_time_value_ = zce::make_timeval(val);
}
void ZCE_Time_Value::set(const std::chrono::minutes &val)
{
    zce_time_value_ = zce::make_timeval(val);
}
void ZCE_Time_Value::set(const std::chrono::seconds &val)
{
    zce_time_value_ = zce::make_timeval(val);
}
void ZCE_Time_Value::set(const std::chrono::milliseconds &val)
{
    zce_time_value_ = zce::make_timeval(val);
}
void ZCE_Time_Value::set(const std::chrono::microseconds &val)
{
    zce_time_value_ = zce::make_timeval(val);
}
void ZCE_Time_Value::set(const std::chrono::nanoseconds &val)
{
    zce_time_value_ = zce::make_timeval(val);
}


void ZCE_Time_Value::set(const std::chrono::system_clock::time_point &val)
{
    zce_time_value_ = zce::make_timeval(val);
}
void ZCE_Time_Value::set(const std::chrono::steady_clock::time_point &val)
{
    zce_time_value_ = zce::make_timeval(val);
}

#ifdef ZCE_OS_WINDOWS
//����ZCE_Time_Value, ��FILETIME
void ZCE_Time_Value::set(LPFILETIME file_time)
{
    zce_time_value_ = zce::make_timeval(file_time);
}
//����ZCE_Time_Value, ��LPSYSTEMTIME
void ZCE_Time_Value::set(LPSYSTEMTIME system_time)
{
    zce_time_value_ = zce::make_timeval(system_time);
}

#endif

//��clock_t ����ʱ�䣬����������������֣����������������ͻ
void ZCE_Time_Value::set_by_clock_t(clock_t time)
{
    zce_time_value_ = zce::make_timeval(time);
}

//�õ��ܹ����ٺ���
uint64_t ZCE_Time_Value::total_msec() const
{
    return static_cast<uint64_t>(this->zce_time_value_.tv_sec) * zce::SEC_PER_MSEC
           + this->zce_time_value_.tv_usec / zce::MSEC_PER_USEC;
}

//��������õ��ܹ����ٺ��룬��ʵ�����������������룬�������΢�������ݣ��ͷ���1���룬
uint64_t ZCE_Time_Value::total_msec_round() const
{
    uint64_t ret_msec = total_msec();
    //����΢�������������ݣ���<1000,
    if ( 0 == ret_msec  && this->zce_time_value_.tv_usec > 0 )
    {
        ret_msec = 1;
    }
    return ret_msec;
}

//�ú�����Ϊ��λ����TimeValue
void ZCE_Time_Value::total_msec(uint64_t set_msec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(set_msec / zce::SEC_PER_MSEC);
    zce_time_value_.tv_usec = static_cast<long>((set_msec % zce::SEC_PER_MSEC) * zce::MSEC_PER_USEC);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = static_cast<time_t>(set_msec / zce::SEC_PER_MSEC);
    zce_time_value_.tv_usec = static_cast<time_t>((set_msec % zce::SEC_PER_MSEC) * zce::MSEC_PER_USEC);
#endif
}

//�õ��ܹ�����΢��
uint64_t ZCE_Time_Value::total_usec() const
{
    return static_cast<uint64_t>(zce_time_value_.tv_sec) * zce::SEC_PER_USEC + zce_time_value_.tv_usec;
}

//��΢����Ϊ��λ������TimeValue��ע�����������usec����������usec����������timeval��usec���֣�
void ZCE_Time_Value::total_usec(uint64_t set_usec)
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

//�õ�Timevalue���벿��
time_t ZCE_Time_Value::sec (void) const
{
    return zce_time_value_.tv_sec;
}

// ����Timevalue���벿��
void ZCE_Time_Value::sec (time_t sec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_sec = static_cast<long>(sec);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_sec = sec;
#endif
}

//�õ�Timevalue��΢�벿��
time_t ZCE_Time_Value::usec (void) const
{
    return zce_time_value_.tv_usec;
}

//����Timevalue��΢�벿��
void ZCE_Time_Value::usec (time_t usec)
{
#if defined ZCE_OS_WINDOWS
    zce_time_value_.tv_usec = static_cast<long>(usec);
#elif defined ZCE_OS_LINUX
    zce_time_value_.tv_usec = usec;
#endif
}

//��ȡ��ǰʱ��
void ZCE_Time_Value::gettimeofday()
{
    zce_time_value_ = zce::gettimeofday();
}

//
ZCE_Time_Value &ZCE_Time_Value::operator += (const ZCE_Time_Value &tv)
{
    zce_time_value_ = zce::timeval_add(zce_time_value_, tv.zce_time_value_);
    return *this;
}
// Assign @ tv to this
ZCE_Time_Value &ZCE_Time_Value::operator = (const ZCE_Time_Value &tv)
{
    zce_time_value_ = tv.zce_time_value_;
    return *this;
}

// Subtract @a tv to this.
ZCE_Time_Value &ZCE_Time_Value::operator -= (const ZCE_Time_Value &tv)
{
    //��֤����ֵ�����ʱ��>=0
    zce_time_value_ = zce::timeval_sub(zce_time_value_, tv.zce_time_value_, true);
    return *this;
}

// ����ʱ����бȽ� <
bool ZCE_Time_Value::operator < (const ZCE_Time_Value &tv)
{
    //�ȱȽ��룬
    if (zce_time_value_.tv_sec < tv.zce_time_value_.tv_sec)
    {
        return true;
    }

    //�ٱȽ�΢��
    if (zce_time_value_.tv_sec == tv.zce_time_value_.tv_sec &&
        zce_time_value_.tv_usec < tv.zce_time_value_.tv_usec )
    {
        return true;
    }

    return false;
}

// ����ʱ����бȽ� >
bool ZCE_Time_Value::operator > (const ZCE_Time_Value &tv)
{
    //�ȱȽ��룬
    if (zce_time_value_.tv_sec > tv.zce_time_value_.tv_sec)
    {
        return true;
    }

    //�ٱȽ�΢��
    if (zce_time_value_.tv_sec == tv.zce_time_value_.tv_sec &&
        zce_time_value_.tv_usec > tv.zce_time_value_.tv_usec )
    {
        return true;
    }

    return false;
}

//
bool ZCE_Time_Value::operator <= (const ZCE_Time_Value &tv)
{
    //�ȱȽ��룬
    if (zce_time_value_.tv_sec <= tv.zce_time_value_.tv_sec)
    {
        return true;
    }

    //�ٱȽ�΢��
    if (zce_time_value_.tv_sec == tv.zce_time_value_.tv_sec &&
        zce_time_value_.tv_usec <= tv.zce_time_value_.tv_usec )
    {
        return true;
    }

    return false;
}

/// True if @a tv1 >= @a tv2.
bool ZCE_Time_Value::operator >= (const ZCE_Time_Value &tv)
{
    //�ȱȽ��룬
    if (zce_time_value_.tv_sec > tv.zce_time_value_.tv_sec)
    {
        return true;
    }

    //�ٱȽ�΢��
    if (zce_time_value_.tv_sec == tv.zce_time_value_.tv_sec &&
        zce_time_value_.tv_usec >= tv.zce_time_value_.tv_usec )
    {
        return true;
    }

    return false;
}

/// True if @a tv1 == @a tv2.
bool ZCE_Time_Value::operator == (const ZCE_Time_Value &tv)
{
    if (zce_time_value_.tv_sec == tv.zce_time_value_.tv_sec &&
        zce_time_value_.tv_usec == tv.zce_time_value_.tv_usec )
    {
        return true;
    }

    return false;
}

/// True if @a tv1 != @a tv2.
bool ZCE_Time_Value::operator != (const ZCE_Time_Value &tv)
{
    return !(*this == tv);
}

/// Adds two ZCE_Time_Value objects together, returns the sum.
ZCE_Time_Value operator + (const ZCE_Time_Value &tv1,
                           const ZCE_Time_Value &tv2)
{
    return zce::timeval_add(tv1.zce_time_value_, tv2.zce_time_value_);
}

/// Subtracts two ZCE_Time_Value objects, returns the difference.
ZCE_Time_Value operator - (const ZCE_Time_Value &tv1,
                           const ZCE_Time_Value &tv2)
{
    //��֤����ֵ�����ʱ��>=0
    return zce::timeval_sub(tv1.zce_time_value_, tv2.zce_time_value_, true);
}

/// Returns the value of the object as a timeval.
ZCE_Time_Value::operator timeval () const
{
    return zce_time_value_;
}

//
ZCE_Time_Value::operator const timeval *() const
{
    return &zce_time_value_;
}

//
ZCE_Time_Value::operator timeval *()
{
    return &zce_time_value_;
}

//��ʱ���ӡ����
const char *ZCE_Time_Value::to_string(char *str_date_time,
                                      size_t datetime_strlen,
                                      size_t &use_buf,
                                      bool utc_time,
                                      zce::TIME_STR_FORMAT_TYPE fmt) const
{
    return zce::timeval_to_str(&(this->zce_time_value_),
                               str_date_time,
                               datetime_strlen,
                               use_buf,
                               utc_time,
                               fmt);
}


//���ַ����еõ�ʱ��
int ZCE_Time_Value::from_string(const char *strtm,
                                bool uct_time,
                                zce::TIME_STR_FORMAT_TYPE fmt)
{
    return zce::str_to_timeval(strtm,
                               fmt,
                               uct_time,
                               &zce_time_value_
                              );
}

const char *ZCE_Time_Value::timestamp(char *str_date_time,
                                      size_t datetime_strlen
                                     ) const
{
    return zce::timestamp(&(this->zce_time_value_),
                          str_date_time,
                          datetime_strlen);
}