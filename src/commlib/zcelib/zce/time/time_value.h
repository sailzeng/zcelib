/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/time/time_value.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年10月6日
* @brief      本来希望自己所有的代码里面的通用时间对象就是timeval的，
*             但写着写着，发现还是要有一个时间的类，否则很多地方冗余
*             代码会非常多，于是就有了这个东东，
*             在ZCELIB里面，他和timeval的作用一样，有时候标识时间点，
*             有时候表示一段时间。
*             在C++ 11后，仍然保存这个东西的意义在于我们大量的代码
*             需要和C的代码混用，chrono其实并不方便
* @details    内部封装的还是timeval,这点和ACE一样，我曾经记得ACE曾经
*             封装过double作为时间值，但无法确认了。
*
*/
#pragma once

#include "zce/os_adapt/define.h"
#include "zce/os_adapt/time.h"

namespace zce
{
/*!
* @brief      ZCELIB �����ʱ��ֵ��ʱ�����
*
*/
class time_value
{
public:

    /*!
    * @brief      Ĭ�Ϲ��캯��
    */
    time_value();

    time_value(const time_value&) = default;
    time_value(time_value&&) = default;
    time_value& operator=(const time_value&) = default;
    time_value& operator=(time_value&&) = default;

    /*!
    * @brief      �������캯��
    */
    ZCE_Time_Value(const ZCE_Time_Value &val);
    /*!
    * @brief      ��������
    */
    ~time_value();

    /*!
    * @brief      ���캯������timeval
    * @param      time_data
    */
    time_value(const timeval& time_data);

    /*!
    * @brief      ���캯������::timespec
    * @param      timespec_val timespec��ʾʱ��
    */
    time_value(const ::timespec& timespec_val);

    /*!
    * @brief      ���캯��������ʱ����������
    * @param      sec   timeval ��������
    * @param      usec  timeval�е�΢������
    */
    time_value(time_t sec, time_t usec);

    /*!
    * @brief      ���캯������time_t, usec����Ϊ0
    * @param      sec
    */
    time_value(time_t sec);

    /*!
    * @brief      将CPP11的duration的数据结构转换得到timeval结构
    * @param      val  进行转换的参数
    */
    template<class Rep, class Period>
    time_value(const std::chrono::duration<Rep, Period>& val)
    {
        set(val);
    }

    /*!
    * @brief      ���캯������CPP 11��һЩtime_point��ֵ
    * @param      val CPP11��durationʱ��
    */
    time_value(const std::chrono::system_clock::time_point& val);
    time_value(const std::chrono::steady_clock::time_point& val);

#ifdef ZCE_OS_WINDOWS

    /*!
    * @brief      ���캯������LPFILETIME,FILETIME
    * @param      file_time
    */
    time_value(LPFILETIME file_time);

    /*!
    * @brief      ���캯������LPSYSTEMTIME,SYSTEMTIME
    * @param      system_time
    */
    time_value(LPSYSTEMTIME system_time);

#endif

public:
    /*!
    * @brief      设置Time_Value,用timeval
    * @param      time_data
    */
    void set(const timeval& time_data);

    /*!
    * @brief      设置Time_Value,用::timespec
    * @param      timespec_val
    */
    void set(const ::timespec& timespec_val);

    /*!
    * @brief      设置Time_Value,几个时间数据数据
    * @param      sec
    * @param      usec
    */
    void set(time_t sec, time_t usec);

    /*!
    * @brief      设置Time_Value,用time_t, usec被置为0
    * @param      sec
    */
    void set(time_t sec);

    /*!
    * @brief      ��clock_t ����ʱ�䣬��Ϊ����������Ϊ�˱��⺯����ͻ�������������û��set������
    * @param      time ���õ�clock_t
    */
    void set_by_clock_t(clock_t time);

    /*!
    * @brief      将CPP11的duration的数据结构转换得到time_value结构
    * @param      val  进行转换的参数
    * @note       val 可以是 std::chrono::duration的各种变种,比如：
    *             std::chrono::hours,std::chrono::minutes,std::chrono::seconds
    *             std::chrono::milliseconds,std::chrono::microseconds,
    *             std::chrono::nanoseconds,等
    *             也可以是 std::literals::chrono_literals::operator""ms
    *             这类标识
    */
    template<class Rep, class Period>
    void set(const std::chrono::duration<Rep, Period>& val)
    {
        zce_time_value_ = zce::make_timeval(val);
        return;
    }

    /*!
    * @brief      将CPP11的time_point的数据结构转换得到time_value结构
    * @return     const timeval 转换后的timeval结果
    * @param      val  进行转换的参数,可以是
    *             std::chrono::system_clock::time_point
    *             std::chrono::steady_clock::time_point
    */
    template<class Clock, class Duration >
    void set(const std::chrono::time_point<Clock, Duration>& val)
    {
        zce_time_value_ = zce::make_timeval(val);
    }

    template<class Rep, class Period>
    void to(std::chrono::duration<Rep, Period>& val) const
    {
        zce::make_duration(zce_time_value_, val);
        return;
    }

    void to(std::chrono::system_clock::time_point& val) const;
    void to(std::chrono::steady_clock::time_point& val) const;

#ifdef ZCE_OS_WINDOWS

    /*!
    * @brief      设置Time_Value, 用FILETIME
    * @param      file_time
    */
    void set(LPFILETIME file_time);

    /*!
    * @brief      设置Time_Value, 用LPSYSTEMTIME
    * @param      system_time  WINDOWS的SYSTEMTIME时间
    */
    void set(LPSYSTEMTIME system_time);

#endif

    /*!
    * @brief      �õ�Timevalue���벿��
    * @return     time_t
    */
    time_t sec(void) const;
    /*!
    * @brief      ����Timevalue���벿��
    * @param      set_sec  ���õģ�timeval���벿�ֵ���ֵ
    */
    void sec(time_t set_sec);

    /*!
    * @brief      得到Timevalue的微秒部分，注意是得到微秒部分，不是总计值呀。（已经看见一个人调用错了）
    * @return     time_t Time_Value微秒部分的数据
    */
    time_t usec(void) const;
    /*!
    * @brief      ����Timevalue��΢�벿��
    * @param      set_usec ���õģ�timeval��΢�벿�ֵ���ֵ��ע������total_usec������
    */
    void usec(time_t set_usec);

    /*!
    * @brief      得到总共多少秒
    * @return     uint64_t 返回秒总数
    */
    uint64_t total_sec() const;

    /*!
    * @brief      得到总共多少毫秒
    * @return     uint64_t 返回毫秒总数
    */
    uint64_t total_msec() const;

    /*!
    * @brief      ��������õ��ܹ����ٺ��룬��ʵ�����������������룬�������΢�������ݣ��ͷ���1���룬
    *             ΪʲôҪ�����أ���ΪEPOLL�Ⱥ�����ʹ�ú�����ֵ�ģ����еĵط�����д�ĵȴ�ʱ����С��
    *             1����ģ���ʱ����epoll_wait����һ������ѭ��״̬��
    * @return     uint64_t ���غ�������
    */
    uint64_t total_msec_round() const;

    /*!
    * @brief      用毫秒作为单位设置Time_Value
    * @param      set_msec
    */
    void total_msec(uint64_t set_msec);

    /*!
    * @brief      �õ��ܹ�����΢��
    * @return     uint64_t
    */
    uint64_t total_usec() const;

    /*!
    * @brief      用微秒作为单位，设置Time_Value，注意这个函数和usec函数的区别，usec函数是设置timeval的usec部分，
    * @param[in]  set_usec 总共的微秒时间，将被换算为timeval
    */
    void total_usec(uint64_t set_usec);

    /*!
    * @brief
    * @return     const char*
    * @param      str_date_time
    * @param      datetime_strlen
    */
    const char* timestamp(char* str_date_time,
                          size_t datetime_strlen) const;

    /*!
    * @brief      根据你的格式化要求，将时间戳打印出来
    * @return     const char*     打印的字符串，方便你处理
    * @param[out] str_date_time   要输出的字符串数组
    * @param[in]  datetime_strlen 字符串的长度
    * @param[in]   fromat_type    时间格式，参考@ref TIME_STR_FORMAT 枚举，默认输出ISO格式的本地时间，精度到USEC。
    */
    const char* to_string(char* str_date_time,
                          size_t datetime_strlen,
                          size_t& use_buf,
                          bool uct_time = false,
                          zce::TIME_STR_FORMAT fromat_type = zce::TIME_STR_FORMAT::ISO_USEC
    )  const;

    /*!
    * @brief      ������ĸ�ʽ��Ҫ��,���ַ�����ת��Ϊ�¼�
    * @return     int ==0
    * @param[in]  strtm       ʱ���ַ���
    * @param[in]  uct_time    �Ƿ�ת��ΪUTCʱ��
    * @param[in]  fromat_type ʱ���ʽ��Ĭ��ΪISO��usec
    */
    int from_string(const char* strtm,
                    bool uct_time = false,
                    zce::TIME_STR_FORMAT fromat_type = zce::TIME_STR_FORMAT::ISO_USEC);

    ///����һ��timespec�Ķ���
    //operator ::timespec () const;

    ///����һ��timeval�Ķ���Returns the value of the object as a timeval.
    operator timeval () const;

    ///返回一个const 的timeval，指针，
    operator const timeval* () const;

    ///返回timeval的指针，你可以通过这个方法直接修改这个对象内部的数据，在某些函数有用，
    operator timeval* ();

    ///��ȡ��ǰʱ��
    void gettimeofday();

    /// 加上 @a tv 的时间
    time_value& operator += (const time_value& tv);

    /// 减去 @a tv的时间
    time_value& operator -= (const time_value& tv);

    /// 返回true，如果 < @a tv 的时间
    bool operator < (const time_value& tv);
    /// 返回true，如果 > @a tv 的时间
    bool operator > (const time_value& tv);

    /// 返回true，如果 <= @a tv 的时间
    bool operator <= (const time_value& tv);

    /// 返回true，如果 >= @a tv 的时间
    bool operator >= (const time_value& tv);

    /// 返回true，如果 == @a tv 的时间
    bool operator == (const time_value& tv);

    /// 返回true，如果 != @a tv 的时间
    bool operator != (const time_value& tv);

    /// 将两个 time_value 对象 @a tv1 和 @a tv2 加起来，返回结果，
    friend time_value operator + (const time_value& tv1,
                                  const time_value& tv2);

    /// 将两个 time_value 对象 @a tv1 和 @a tv2 相减，返回结果，
    friend time_value operator - (const time_value& tv1,
                                  const time_value& tv2);

public:
    ///为0的时间
    static const time_value    ZERO_TIME_VALUE;
    ///
    static const std::chrono::seconds ZERO_DURATION_VALUE;
    ///最大能表示的时间
    static const time_value    MAX_TIME_VALUE;

protected:
    ///底层采用timeval记录时间，可能会有一些进度误差，但我实在想不出来，服务器为什么要精细超过usec
    timeval                    zce_time_value_;
};
}
