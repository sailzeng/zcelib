/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_time_value.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年10月6日
* @brief      本来希望自己所有的代码里面的通用时间对象就是timeval的，
*             但写着写着，发现还是要有一个时间的类，否则很多地方冗余
*             代码会非常多，于是就有了这个东东，
*             在ZCELIB里面，他和timeval的作用一样，有时候标识时间点，
*             有时候表示一段时间。
*
* @details    内部封装的还是timeval,这点和ACE一样，我曾经记得ACE曾经
*             封装过double作为时间值，但无法确认了。
*
*/
#ifndef ZCE_LIB_TIME_VALUE_H_
#define ZCE_LIB_TIME_VALUE_H_

#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_time.h"

/*!
* @brief      ZCELIB 里面的时间值，时间对象，
*
*/
class ZCE_Time_Value
{
public:

    /*!
    * @brief      默认构造函数
    */
    ZCE_Time_Value();

    /*!
    * @brief      析构函数
    */
    ~ZCE_Time_Value();

    ZCE_Time_Value(const ZCE_Time_Value&) = default;
    ZCE_Time_Value(ZCE_Time_Value&&) = default;


    /*!
    * @brief      构造函数，用timeval
    * @param      time_data
    */
    ZCE_Time_Value(const timeval &time_data);

    /*!
    * @brief      构造函数，用::timespec
    * @param      timespec_val timespec表示时间
    */
    ZCE_Time_Value(const ::timespec &timespec_val);

    /*!
    * @brief      构造函数，几个时间数据数据
    * @param      sec   timeval 中秒数据
    * @param      usec  timeval中的微秒数据
    */
    ZCE_Time_Value(time_t sec, time_t usec);

    /*!
    * @brief      构造函数，用time_t, usec被置为0
    * @param      sec
    */
    ZCE_Time_Value(time_t sec);

    /*!
    * @brief      构造函数，用CPP 11的一些duration的值
    * @param      val CPP11的duration时间
    */
    ZCE_Time_Value(const std::chrono::hours &val);
    ZCE_Time_Value(const std::chrono::minutes &val);
    ZCE_Time_Value(const std::chrono::seconds &val);
    ZCE_Time_Value(const std::chrono::milliseconds &val);
    ZCE_Time_Value(const std::chrono::microseconds &val);
    ZCE_Time_Value(const std::chrono::nanoseconds &val);

    /*!
    * @brief      构造函数，用CPP 11的一些time_point的值
    * @param      val CPP11的duration时间
    */
    ZCE_Time_Value(const std::chrono::system_clock::time_point &val);
    ZCE_Time_Value(const std::chrono::steady_clock::time_point &val);

#ifdef ZCE_OS_WINDOWS

    /*!
    * @brief      构造函数，用LPFILETIME,FILETIME
    * @param      file_time
    */
    ZCE_Time_Value(LPFILETIME file_time);

    /*!
    * @brief      构造函数，用LPSYSTEMTIME,SYSTEMTIME
    * @param      system_time
    */
    ZCE_Time_Value(LPSYSTEMTIME system_time);

#endif

public:
    /*!
    * @brief      设置ZCE_Time_Value,用timeval
    * @param      time_data
    */
    void set(const timeval &time_data);

    /*!
    * @brief      设置ZCE_Time_Value,用::timespec
    * @param      timespec_val
    */
    void set(const ::timespec &timespec_val);

    /*!
    * @brief      设置ZCE_Time_Value,几个时间数据数据
    * @param      sec
    * @param      usec
    */
    void set(time_t sec, time_t usec);

    /*!
    * @brief      设置ZCE_Time_Value,用time_t, usec被置为0
    * @param      sec
    */
    void set(time_t sec);

    /*!
    * @brief      用clock_t 设置时间，因为都是整数，为了避免函数冲突，所以这个函数没有set的名字
    * @param      time 设置的clock_t
    */
    void set_by_clock_t(clock_t time);


    void set(const std::chrono::hours &val);
    void set(const std::chrono::minutes &val);
    void set(const std::chrono::seconds &val);
    void set(const std::chrono::milliseconds &val);
    void set(const std::chrono::microseconds &val);
    void set(const std::chrono::nanoseconds &val);


    void set(const std::chrono::system_clock::time_point &val);
    void set(const std::chrono::steady_clock::time_point &val);

#ifdef ZCE_OS_WINDOWS

    /*!
    * @brief      设置ZCE_Time_Value, 用FILETIME
    * @param      file_time
    */
    void set(LPFILETIME file_time);

    /*!
    * @brief      设置ZCE_Time_Value, 用LPSYSTEMTIME
    * @param      system_time  WINDOWS的SYSTEMTIME时间
    */
    void set(LPSYSTEMTIME system_time);

#endif

    /*!
    * @brief      得到Timevalue的秒部分
    * @return     time_t
    */
    time_t sec (void) const;
    /*!
    * @brief      设置Timevalue的秒部分
    * @param      set_sec  设置的，timeval的秒部分的数值
    */
    void sec (time_t set_sec);

    /*!
    * @brief      得到Timevalue的微秒部分，注意是得到微秒部分，不是总计值呀。（已经看见一个人调用错了）
    * @return     time_t ZCE_Time_Value微秒部分的数据
    */
    time_t usec (void) const;
    /*!
    * @brief      设置Timevalue的微秒部分
    * @param      set_usec 设置的，timeval的微秒部分的数值，注意他和total_usec的区别。
    */
    void usec(time_t set_usec);

    /*!
    * @brief      得到总共多少毫秒
    * @return     uint64_t 返回毫秒总数
    */
    uint64_t total_msec() const;

    /*!
    * @brief      四舍五入得到总共多少毫秒，其实不是真正的四舍五入，而是如果微秒有数据，就返回1毫秒，
    *             为什么要这样呢，因为EPOLL等函数是使用毫秒数值的，但有的地方我们写的等待时间是小于
    *             1毫秒的，这时会让epoll_wait进入一个高速循环状态。
    * @return     uint64_t 返回毫秒总数
    */
    uint64_t total_msec_round() const;

    /*!
    * @brief      用毫秒作为单位设置TimeValue
    * @param      set_msec
    */
    void total_msec(uint64_t set_msec);

    /*!
    * @brief      得到总共多少微秒
    * @return     uint64_t
    */
    uint64_t total_usec() const;

    /*!
    * @brief      用微秒作为单位，设置TimeValue，注意这个函数和usec函数的区别，usec函数是设置timeval的usec部分，
    * @param[in]  set_usec 总共的微秒时间，将被换算为timeval
    */
    void total_usec(uint64_t set_usec);



    /*!
    * @brief
    * @return     const char*
    * @param      str_date_time
    * @param      datetime_strlen
    */
    const char *timestamp(char *str_date_time,
                          size_t datetime_strlen) const;

    /*!
    * @brief      根据你的格式化要求，将时间戳打印出来
    * @return     const char*     打印的字符串，方便你处理
    * @param[out] str_date_time   要输出的字符串数组
    * @param[in]  datetime_strlen 字符串的长度
    * @param[in]   fromat_type    时间格式，参考@ref TIME_STR_FORMAT_TYPE 枚举，默认输出ISO格式的本地时间，精度到USEC。
    */
    const char *to_string(char *str_date_time,
                          size_t datetime_strlen,
                          size_t &use_buf,
                          bool uct_time = false,
                          zce::TIME_STR_FORMAT fromat_type = zce::TIME_STR_FORMAT::ISO_USEC
                         )  const;

    /*!
    * @brief      根据你的格式化要求,将字符串，转换为事件
    * @return     int ==0
    * @param[in]  strtm       时间字符串
    * @param[in]  uct_time    是否转换为UTC时间
    * @param[in]  fromat_type 时间格式，默认为ISO的usec
    */
    int from_string(const char *strtm,
                    bool uct_time = false,
                    zce::TIME_STR_FORMAT fromat_type = zce::TIME_STR_FORMAT::ISO_USEC);

    ///返回一个timespec的对象
    //operator ::timespec () const;

    ///返回一个timeval的对象，Returns the value of the object as a timeval.
    operator timeval () const;

    ///返回一个const 的timeval，指针，
    operator const timeval *() const;

    ///返回timeval的指针，你可以通过这个方法直接修改这个对象内部的数据，在某些函数有用，
    operator timeval *();

    ///获取当前时间
    void gettimeofday();

    /// 加上 @a tv 的时间
    ZCE_Time_Value &operator += (const ZCE_Time_Value &tv);

    /// 赋值 @a tv 的时间
    ZCE_Time_Value &operator = (const ZCE_Time_Value &tv);

    /// 减去 @a tv的时间
    ZCE_Time_Value &operator -= (const ZCE_Time_Value &tv);

    /// 返回true，如果 < @a tv 的时间
    bool operator < (const ZCE_Time_Value &tv);
    /// 返回true，如果 > @a tv 的时间
    bool operator > (const ZCE_Time_Value &tv);

    /// 返回true，如果 <= @a tv 的时间
    bool operator <= (const ZCE_Time_Value &tv);

    /// 返回true，如果 >= @a tv 的时间
    bool operator >= (const ZCE_Time_Value &tv);

    /// 返回true，如果 == @a tv 的时间
    bool operator == (const ZCE_Time_Value &tv);

    /// 返回true，如果 != @a tv 的时间
    bool operator != (const ZCE_Time_Value &tv);

    /// 将两个 ZCE_Time_Value 对象 @a tv1 和 @a tv2 加起来，返回结果，
    friend ZCE_Time_Value operator + (const ZCE_Time_Value &tv1,
                                      const ZCE_Time_Value &tv2);

    /// 将两个 ZCE_Time_Value 对象 @a tv1 和 @a tv2 相减，返回结果，
    friend ZCE_Time_Value operator - (const ZCE_Time_Value &tv1,
                                      const ZCE_Time_Value &tv2);

protected:

    ///底层采用timeval记录时间，可能会有一些进度误差，但我实在想不出来，服务器为什么要精细超过usec
    timeval                        zce_time_value_;

public:
    ///为0的时间
    static const ZCE_Time_Value    ZERO_TIME_VALUE;
    ///最大能表示的时间
    static const ZCE_Time_Value    MAX_TIME_VALUE;
};

#endif //# ZCE_LIB_TIME_VALUE_H_

