/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_time_value.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��10��6��
* @brief      ����ϣ���Լ����еĴ��������ͨ��ʱ��������timeval�ģ�
*             ��д��д�ţ����ֻ���Ҫ��һ��ʱ����࣬����ܶ�ط�����
*             �����ǳ��࣬���Ǿ��������������
*             ��ZCELIB���棬����timeval������һ������ʱ���ʶʱ��㣬
*             ��ʱ���ʾһ��ʱ�䡣
*
* @details    �ڲ���װ�Ļ���timeval,����ACEһ�����������ǵ�ACE����
*             ��װ��double��Ϊʱ��ֵ�����޷�ȷ���ˡ�
*
*/
#ifndef ZCE_LIB_TIME_VALUE_H_
#define ZCE_LIB_TIME_VALUE_H_

#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_time.h"

/*!
* @brief      ZCELIB �����ʱ��ֵ��ʱ�����
*
*/
class ZCE_Time_Value
{
public:

    /*!
    * @brief      Ĭ�Ϲ��캯��
    */
    ZCE_Time_Value();
    /*!
    * @brief      ��������
    */
    ~ZCE_Time_Value();

    /*!
    * @brief      ���캯������timeval
    * @param      time_data
    */
    ZCE_Time_Value(const timeval &time_data);

    /*!
    * @brief      ���캯������::timespec
    * @param      timespec_val timespec��ʾʱ��
    */
    ZCE_Time_Value(const ::timespec &timespec_val);

    /*!
    * @brief      ���캯��������ʱ����������
    * @param      sec   timeval ��������
    * @param      usec  timeval�е�΢������
    */
    ZCE_Time_Value(time_t sec, time_t usec);

    /*!
    * @brief      ���캯������time_t, usec����Ϊ0
    * @param      sec
    */
    ZCE_Time_Value(time_t sec);

    /*!
    * @brief      ���캯������CPP 11��һЩduration��ֵ
    * @param      val CPP11��durationʱ��
    */
    ZCE_Time_Value(const std::chrono::hours &val);
    ZCE_Time_Value(const std::chrono::minutes &val);
    ZCE_Time_Value(const std::chrono::seconds &val);
    ZCE_Time_Value(const std::chrono::milliseconds &val);
    ZCE_Time_Value(const std::chrono::microseconds &val);
    ZCE_Time_Value(const std::chrono::nanoseconds &val);

    /*!
    * @brief      ���캯������CPP 11��һЩtime_point��ֵ
    * @param      val CPP11��durationʱ��
    */
    ZCE_Time_Value(const std::chrono::system_clock::time_point &val);
    ZCE_Time_Value(const std::chrono::steady_clock::time_point &val);

#ifdef ZCE_OS_WINDOWS

    /*!
    * @brief      ���캯������LPFILETIME,FILETIME
    * @param      file_time
    */
    ZCE_Time_Value(LPFILETIME file_time);

    /*!
    * @brief      ���캯������LPSYSTEMTIME,SYSTEMTIME
    * @param      system_time
    */
    ZCE_Time_Value(LPSYSTEMTIME system_time);

#endif

public:
    /*!
    * @brief      ����ZCE_Time_Value,��timeval
    * @param      time_data
    */
    void set(const timeval &time_data);

    /*!
    * @brief      ����ZCE_Time_Value,��::timespec
    * @param      timespec_val
    */
    void set(const ::timespec &timespec_val);

    /*!
    * @brief      ����ZCE_Time_Value,����ʱ����������
    * @param      sec
    * @param      usec
    */
    void set(time_t sec, time_t usec);

    /*!
    * @brief      ����ZCE_Time_Value,��time_t, usec����Ϊ0
    * @param      sec
    */
    void set(time_t sec);

    /*!
    * @brief      ��clock_t ����ʱ�䣬��Ϊ����������Ϊ�˱��⺯����ͻ�������������û��set������
    * @param      time ���õ�clock_t
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
    * @brief      ����ZCE_Time_Value, ��FILETIME
    * @param      file_time
    */
    void set(LPFILETIME file_time);

    /*!
    * @brief      ����ZCE_Time_Value, ��LPSYSTEMTIME
    * @param      system_time  WINDOWS��SYSTEMTIMEʱ��
    */
    void set(LPSYSTEMTIME system_time);

#endif

    /*!
    * @brief      �õ�Timevalue���벿��
    * @return     time_t
    */
    time_t sec (void) const;
    /*!
    * @brief      ����Timevalue���벿��
    * @param      set_sec  ���õģ�timeval���벿�ֵ���ֵ
    */
    void sec (time_t set_sec);

    /*!
    * @brief      �õ�Timevalue��΢�벿�֣�ע���ǵõ�΢�벿�֣������ܼ�ֵѽ�����Ѿ�����һ���˵��ô��ˣ�
    * @return     time_t ZCE_Time_Value΢�벿�ֵ�����
    */
    time_t usec (void) const;
    /*!
    * @brief      ����Timevalue��΢�벿��
    * @param      set_usec ���õģ�timeval��΢�벿�ֵ���ֵ��ע������total_usec������
    */
    void usec(time_t set_usec);

    /*!
    * @brief      �õ��ܹ����ٺ���
    * @return     uint64_t ���غ�������
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
    * @brief      �ú�����Ϊ��λ����TimeValue
    * @param      set_msec
    */
    void total_msec(uint64_t set_msec);

    /*!
    * @brief      �õ��ܹ�����΢��
    * @return     uint64_t
    */
    uint64_t total_usec() const;

    /*!
    * @brief      ��΢����Ϊ��λ������TimeValue��ע�����������usec����������usec����������timeval��usec���֣�
    * @param[in]  set_usec �ܹ���΢��ʱ�䣬��������Ϊtimeval
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
    * @brief      ������ĸ�ʽ��Ҫ�󣬽�ʱ�����ӡ����
    * @return     const char*     ��ӡ���ַ����������㴦��
    * @param[out] str_date_time   Ҫ������ַ�������
    * @param[in]  datetime_strlen �ַ����ĳ���
    * @param[in]   fromat_type    ʱ���ʽ���ο�@ref TIME_STR_FORMAT_TYPE ö�٣�Ĭ�����ISO��ʽ�ı���ʱ�䣬���ȵ�USEC��
    */
    const char *to_string(char *str_date_time,
                          size_t datetime_strlen,
                          size_t &use_buf,
                          bool uct_time = false,
                          zce::TIME_STR_FORMAT_TYPE fromat_type = zce::TIME_STRFMT_ISO_USEC
                         )  const;

    /*!
    * @brief      ������ĸ�ʽ��Ҫ��,���ַ�����ת��Ϊ�¼�
    * @return     int ==0
    * @param[in]  strtm       ʱ���ַ���
    * @param[in]  uct_time    �Ƿ�ת��ΪUTCʱ��
    * @param[in]  fromat_type ʱ���ʽ��Ĭ��ΪISO��usec
    */
    int from_string(const char *strtm,
                    bool uct_time = false,
                    zce::TIME_STR_FORMAT_TYPE fromat_type = zce::TIME_STRFMT_ISO_USEC);

    ///����һ��timespec�Ķ���
    //operator ::timespec () const;

    ///����һ��timeval�Ķ���Returns the value of the object as a timeval.
    operator timeval () const;

    ///����һ��const ��timeval��ָ�룬
    operator const timeval *() const;

    ///����timeval��ָ�룬�����ͨ���������ֱ���޸���������ڲ������ݣ���ĳЩ�������ã�
    operator timeval *();

    ///��ȡ��ǰʱ��
    void gettimeofday();

    /// ���� @a tv ��ʱ��
    ZCE_Time_Value &operator += (const ZCE_Time_Value &tv);

    /// ��ֵ @a tv ��ʱ��
    ZCE_Time_Value &operator = (const ZCE_Time_Value &tv);

    /// ��ȥ @a tv��ʱ��
    ZCE_Time_Value &operator -= (const ZCE_Time_Value &tv);

    /// ����true����� < @a tv ��ʱ��
    bool operator < (const ZCE_Time_Value &tv);
    /// ����true����� > @a tv ��ʱ��
    bool operator > (const ZCE_Time_Value &tv);

    /// ����true����� <= @a tv ��ʱ��
    bool operator <= (const ZCE_Time_Value &tv);

    /// ����true����� >= @a tv ��ʱ��
    bool operator >= (const ZCE_Time_Value &tv);

    /// ����true����� == @a tv ��ʱ��
    bool operator == (const ZCE_Time_Value &tv);

    /// ����true����� != @a tv ��ʱ��
    bool operator != (const ZCE_Time_Value &tv);

    /// ������ ZCE_Time_Value ���� @a tv1 �� @a tv2 �����������ؽ����
    friend ZCE_Time_Value operator + (const ZCE_Time_Value &tv1,
                                      const ZCE_Time_Value &tv2);

    /// ������ ZCE_Time_Value ���� @a tv1 �� @a tv2 ��������ؽ����
    friend ZCE_Time_Value operator - (const ZCE_Time_Value &tv1,
                                      const ZCE_Time_Value &tv2);

protected:

    ///�ײ����timeval��¼ʱ�䣬���ܻ���һЩ����������ʵ���벻������������ΪʲôҪ��ϸ����usec
    timeval                        zce_time_value_;

public:
    ///Ϊ0��ʱ��
    static const ZCE_Time_Value    ZERO_TIME_VALUE;
    ///����ܱ�ʾ��ʱ��
    static const ZCE_Time_Value    MAX_TIME_VALUE;
};

#endif //# ZCE_LIB_TIME_VALUE_H_

