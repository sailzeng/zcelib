/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/time/time_value.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011å¹´10æœˆ6æ—¥
* @brief      æœ¬æ¥å¸Œæœ›è‡ªå·±æ‰€æœ‰çš„ä»£ç é‡Œé¢çš„é€šç”¨æ—¶é—´å¯¹è±¡å°±æ˜¯timevalçš„ï¼Œ
*             ä½†å†™ç€å†™ç€ï¼Œå‘ç°è¿˜æ˜¯è¦æœ‰ä¸€ä¸ªæ—¶é—´çš„ç±»ï¼Œå¦åˆ™å¾ˆå¤šåœ°æ–¹å†—ä½™
*             ä»£ç ä¼šéå¸¸å¤šï¼Œäºæ˜¯å°±æœ‰äº†è¿™ä¸ªä¸œä¸œï¼Œ
*             åœ¨ZCELIBé‡Œé¢ï¼Œä»–å’Œtimevalçš„ä½œç”¨ä¸€æ ·ï¼Œæœ‰æ—¶å€™æ ‡è¯†æ—¶é—´ç‚¹ï¼Œ
*             æœ‰æ—¶å€™è¡¨ç¤ºä¸€æ®µæ—¶é—´ã€‚
*             åœ¨C++ 11åï¼Œä»ç„¶ä¿å­˜è¿™ä¸ªä¸œè¥¿çš„æ„ä¹‰åœ¨äºæˆ‘ä»¬å¤§é‡çš„ä»£ç 
*             éœ€è¦å’ŒCçš„ä»£ç æ··ç”¨ï¼Œchronoå…¶å®å¹¶ä¸æ–¹ä¾¿
* @details    å†…éƒ¨å°è£…çš„è¿˜æ˜¯timeval,è¿™ç‚¹å’ŒACEä¸€æ ·ï¼Œæˆ‘æ›¾ç»è®°å¾—ACEæ›¾ç»
*             å°è£…è¿‡doubleä½œä¸ºæ—¶é—´å€¼ï¼Œä½†æ— æ³•ç¡®è®¤äº†ã€‚
*
*/
#pragma once

#include "zce/os_adapt/define.h"
#include "zce/os_adapt/time.h"

namespace zce
{
/*!
* @brief      ZCELIB ÀïÃæµÄÊ±¼äÖµ£¬Ê±¼ä¶ÔÏó£¬
*
*/
class time_value
{
public:

    /*!
    * @brief      Ä¬ÈÏ¹¹Ôìº¯Êı
    */
    time_value();

    time_value(const time_value&) = default;
    time_value(time_value&&) = default;
    time_value& operator=(const time_value&) = default;
    time_value& operator=(time_value&&) = default;

    /*!
    * @brief      ¿½±´¹¹Ôìº¯Êı
    */
    ZCE_Time_Value(const ZCE_Time_Value &val);
    /*!
    * @brief      Îö¹¹º¯Êı
    */
    ~time_value();

    /*!
    * @brief      ¹¹Ôìº¯Êı£¬ÓÃtimeval
    * @param      time_data
    */
    time_value(const timeval& time_data);

    /*!
    * @brief      ¹¹Ôìº¯Êı£¬ÓÃ::timespec
    * @param      timespec_val timespec±íÊ¾Ê±¼ä
    */
    time_value(const ::timespec& timespec_val);

    /*!
    * @brief      ¹¹Ôìº¯Êı£¬¼¸¸öÊ±¼äÊı¾İÊı¾İ
    * @param      sec   timeval ÖĞÃëÊı¾İ
    * @param      usec  timevalÖĞµÄÎ¢ÃëÊı¾İ
    */
    time_value(time_t sec, time_t usec);

    /*!
    * @brief      ¹¹Ôìº¯Êı£¬ÓÃtime_t, usec±»ÖÃÎª0
    * @param      sec
    */
    time_value(time_t sec);

    /*!
    * @brief      å°†CPP11çš„durationçš„æ•°æ®ç»“æ„è½¬æ¢å¾—åˆ°timevalç»“æ„
    * @param      val  è¿›è¡Œè½¬æ¢çš„å‚æ•°
    */
    template<class Rep, class Period>
    time_value(const std::chrono::duration<Rep, Period>& val)
    {
        set(val);
    }

    /*!
    * @brief      ¹¹Ôìº¯Êı£¬ÓÃCPP 11µÄÒ»Ğ©time_pointµÄÖµ
    * @param      val CPP11µÄdurationÊ±¼ä
    */
    time_value(const std::chrono::system_clock::time_point& val);
    time_value(const std::chrono::steady_clock::time_point& val);

#ifdef ZCE_OS_WINDOWS

    /*!
    * @brief      ¹¹Ôìº¯Êı£¬ÓÃLPFILETIME,FILETIME
    * @param      file_time
    */
    time_value(LPFILETIME file_time);

    /*!
    * @brief      ¹¹Ôìº¯Êı£¬ÓÃLPSYSTEMTIME,SYSTEMTIME
    * @param      system_time
    */
    time_value(LPSYSTEMTIME system_time);

#endif

public:
    /*!
    * @brief      è®¾ç½®Time_Value,ç”¨timeval
    * @param      time_data
    */
    void set(const timeval& time_data);

    /*!
    * @brief      è®¾ç½®Time_Value,ç”¨::timespec
    * @param      timespec_val
    */
    void set(const ::timespec& timespec_val);

    /*!
    * @brief      è®¾ç½®Time_Value,å‡ ä¸ªæ—¶é—´æ•°æ®æ•°æ®
    * @param      sec
    * @param      usec
    */
    void set(time_t sec, time_t usec);

    /*!
    * @brief      è®¾ç½®Time_Value,ç”¨time_t, usecè¢«ç½®ä¸º0
    * @param      sec
    */
    void set(time_t sec);

    /*!
    * @brief      ÓÃclock_t ÉèÖÃÊ±¼ä£¬ÒòÎª¶¼ÊÇÕûÊı£¬ÎªÁË±ÜÃâº¯Êı³åÍ»£¬ËùÒÔÕâ¸öº¯ÊıÃ»ÓĞsetµÄÃû×Ö
    * @param      time ÉèÖÃµÄclock_t
    */
    void set_by_clock_t(clock_t time);

    /*!
    * @brief      å°†CPP11çš„durationçš„æ•°æ®ç»“æ„è½¬æ¢å¾—åˆ°time_valueç»“æ„
    * @param      val  è¿›è¡Œè½¬æ¢çš„å‚æ•°
    * @note       val å¯ä»¥æ˜¯ std::chrono::durationçš„å„ç§å˜ç§,æ¯”å¦‚ï¼š
    *             std::chrono::hours,std::chrono::minutes,std::chrono::seconds
    *             std::chrono::milliseconds,std::chrono::microseconds,
    *             std::chrono::nanoseconds,ç­‰
    *             ä¹Ÿå¯ä»¥æ˜¯ std::literals::chrono_literals::operator""ms
    *             è¿™ç±»æ ‡è¯†
    */
    template<class Rep, class Period>
    void set(const std::chrono::duration<Rep, Period>& val)
    {
        zce_time_value_ = zce::make_timeval(val);
        return;
    }

    /*!
    * @brief      å°†CPP11çš„time_pointçš„æ•°æ®ç»“æ„è½¬æ¢å¾—åˆ°time_valueç»“æ„
    * @return     const timeval è½¬æ¢åçš„timevalç»“æœ
    * @param      val  è¿›è¡Œè½¬æ¢çš„å‚æ•°,å¯ä»¥æ˜¯
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
    * @brief      è®¾ç½®Time_Value, ç”¨FILETIME
    * @param      file_time
    */
    void set(LPFILETIME file_time);

    /*!
    * @brief      è®¾ç½®Time_Value, ç”¨LPSYSTEMTIME
    * @param      system_time  WINDOWSçš„SYSTEMTIMEæ—¶é—´
    */
    void set(LPSYSTEMTIME system_time);

#endif

    /*!
    * @brief      µÃµ½TimevalueµÄÃë²¿·Ö
    * @return     time_t
    */
    time_t sec(void) const;
    /*!
    * @brief      ÉèÖÃTimevalueµÄÃë²¿·Ö
    * @param      set_sec  ÉèÖÃµÄ£¬timevalµÄÃë²¿·ÖµÄÊıÖµ
    */
    void sec(time_t set_sec);

    /*!
    * @brief      å¾—åˆ°Timevalueçš„å¾®ç§’éƒ¨åˆ†ï¼Œæ³¨æ„æ˜¯å¾—åˆ°å¾®ç§’éƒ¨åˆ†ï¼Œä¸æ˜¯æ€»è®¡å€¼å‘€ã€‚ï¼ˆå·²ç»çœ‹è§ä¸€ä¸ªäººè°ƒç”¨é”™äº†ï¼‰
    * @return     time_t Time_Valueå¾®ç§’éƒ¨åˆ†çš„æ•°æ®
    */
    time_t usec(void) const;
    /*!
    * @brief      ÉèÖÃTimevalueµÄÎ¢Ãë²¿·Ö
    * @param      set_usec ÉèÖÃµÄ£¬timevalµÄÎ¢Ãë²¿·ÖµÄÊıÖµ£¬×¢ÒâËûºÍtotal_usecµÄÇø±ğ¡£
    */
    void usec(time_t set_usec);

    /*!
    * @brief      å¾—åˆ°æ€»å…±å¤šå°‘ç§’
    * @return     uint64_t è¿”å›ç§’æ€»æ•°
    */
    uint64_t total_sec() const;

    /*!
    * @brief      å¾—åˆ°æ€»å…±å¤šå°‘æ¯«ç§’
    * @return     uint64_t è¿”å›æ¯«ç§’æ€»æ•°
    */
    uint64_t total_msec() const;

    /*!
    * @brief      ËÄÉáÎåÈëµÃµ½×Ü¹²¶àÉÙºÁÃë£¬ÆäÊµ²»ÊÇÕæÕıµÄËÄÉáÎåÈë£¬¶øÊÇÈç¹ûÎ¢ÃëÓĞÊı¾İ£¬¾Í·µ»Ø1ºÁÃë£¬
    *             ÎªÊ²Ã´ÒªÕâÑùÄØ£¬ÒòÎªEPOLLµÈº¯ÊıÊÇÊ¹ÓÃºÁÃëÊıÖµµÄ£¬µ«ÓĞµÄµØ·½ÎÒÃÇĞ´µÄµÈ´ıÊ±¼äÊÇĞ¡ÓÚ
    *             1ºÁÃëµÄ£¬ÕâÊ±»áÈÃepoll_wait½øÈëÒ»¸ö¸ßËÙÑ­»·×´Ì¬¡£
    * @return     uint64_t ·µ»ØºÁÃë×ÜÊı
    */
    uint64_t total_msec_round() const;

    /*!
    * @brief      ç”¨æ¯«ç§’ä½œä¸ºå•ä½è®¾ç½®Time_Value
    * @param      set_msec
    */
    void total_msec(uint64_t set_msec);

    /*!
    * @brief      µÃµ½×Ü¹²¶àÉÙÎ¢Ãë
    * @return     uint64_t
    */
    uint64_t total_usec() const;

    /*!
    * @brief      ç”¨å¾®ç§’ä½œä¸ºå•ä½ï¼Œè®¾ç½®Time_Valueï¼Œæ³¨æ„è¿™ä¸ªå‡½æ•°å’Œusecå‡½æ•°çš„åŒºåˆ«ï¼Œusecå‡½æ•°æ˜¯è®¾ç½®timevalçš„usecéƒ¨åˆ†ï¼Œ
    * @param[in]  set_usec æ€»å…±çš„å¾®ç§’æ—¶é—´ï¼Œå°†è¢«æ¢ç®—ä¸ºtimeval
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
    * @brief      æ ¹æ®ä½ çš„æ ¼å¼åŒ–è¦æ±‚ï¼Œå°†æ—¶é—´æˆ³æ‰“å°å‡ºæ¥
    * @return     const char*     æ‰“å°çš„å­—ç¬¦ä¸²ï¼Œæ–¹ä¾¿ä½ å¤„ç†
    * @param[out] str_date_time   è¦è¾“å‡ºçš„å­—ç¬¦ä¸²æ•°ç»„
    * @param[in]  datetime_strlen å­—ç¬¦ä¸²çš„é•¿åº¦
    * @param[in]   fromat_type    æ—¶é—´æ ¼å¼ï¼Œå‚è€ƒ@ref TIME_STR_FORMAT æšä¸¾ï¼Œé»˜è®¤è¾“å‡ºISOæ ¼å¼çš„æœ¬åœ°æ—¶é—´ï¼Œç²¾åº¦åˆ°USECã€‚
    */
    const char* to_string(char* str_date_time,
                          size_t datetime_strlen,
                          size_t& use_buf,
                          bool uct_time = false,
                          zce::TIME_STR_FORMAT fromat_type = zce::TIME_STR_FORMAT::ISO_USEC
    )  const;

    /*!
    * @brief      ¸ù¾İÄãµÄ¸ñÊ½»¯ÒªÇó,½«×Ö·û´®£¬×ª»»ÎªÊÂ¼ş
    * @return     int ==0
    * @param[in]  strtm       Ê±¼ä×Ö·û´®
    * @param[in]  uct_time    ÊÇ·ñ×ª»»ÎªUTCÊ±¼ä
    * @param[in]  fromat_type Ê±¼ä¸ñÊ½£¬Ä¬ÈÏÎªISOµÄusec
    */
    int from_string(const char* strtm,
                    bool uct_time = false,
                    zce::TIME_STR_FORMAT fromat_type = zce::TIME_STR_FORMAT::ISO_USEC);

    ///·µ»ØÒ»¸ötimespecµÄ¶ÔÏó
    //operator ::timespec () const;

    ///·µ»ØÒ»¸ötimevalµÄ¶ÔÏó£¬Returns the value of the object as a timeval.
    operator timeval () const;

    ///è¿”å›ä¸€ä¸ªconst çš„timevalï¼ŒæŒ‡é’ˆï¼Œ
    operator const timeval* () const;

    ///è¿”å›timevalçš„æŒ‡é’ˆï¼Œä½ å¯ä»¥é€šè¿‡è¿™ä¸ªæ–¹æ³•ç›´æ¥ä¿®æ”¹è¿™ä¸ªå¯¹è±¡å†…éƒ¨çš„æ•°æ®ï¼Œåœ¨æŸäº›å‡½æ•°æœ‰ç”¨ï¼Œ
    operator timeval* ();

    ///»ñÈ¡µ±Ç°Ê±¼ä
    void gettimeofday();

    /// åŠ ä¸Š @a tv çš„æ—¶é—´
    time_value& operator += (const time_value& tv);

    /// å‡å» @a tvçš„æ—¶é—´
    time_value& operator -= (const time_value& tv);

    /// è¿”å›trueï¼Œå¦‚æœ < @a tv çš„æ—¶é—´
    bool operator < (const time_value& tv);
    /// è¿”å›trueï¼Œå¦‚æœ > @a tv çš„æ—¶é—´
    bool operator > (const time_value& tv);

    /// è¿”å›trueï¼Œå¦‚æœ <= @a tv çš„æ—¶é—´
    bool operator <= (const time_value& tv);

    /// è¿”å›trueï¼Œå¦‚æœ >= @a tv çš„æ—¶é—´
    bool operator >= (const time_value& tv);

    /// è¿”å›trueï¼Œå¦‚æœ == @a tv çš„æ—¶é—´
    bool operator == (const time_value& tv);

    /// è¿”å›trueï¼Œå¦‚æœ != @a tv çš„æ—¶é—´
    bool operator != (const time_value& tv);

    /// å°†ä¸¤ä¸ª time_value å¯¹è±¡ @a tv1 å’Œ @a tv2 åŠ èµ·æ¥ï¼Œè¿”å›ç»“æœï¼Œ
    friend time_value operator + (const time_value& tv1,
                                  const time_value& tv2);

    /// å°†ä¸¤ä¸ª time_value å¯¹è±¡ @a tv1 å’Œ @a tv2 ç›¸å‡ï¼Œè¿”å›ç»“æœï¼Œ
    friend time_value operator - (const time_value& tv1,
                                  const time_value& tv2);

public:
    ///ä¸º0çš„æ—¶é—´
    static const time_value    ZERO_TIME_VALUE;
    ///
    static const std::chrono::seconds ZERO_DURATION_VALUE;
    ///æœ€å¤§èƒ½è¡¨ç¤ºçš„æ—¶é—´
    static const time_value    MAX_TIME_VALUE;

protected:
    ///åº•å±‚é‡‡ç”¨timevalè®°å½•æ—¶é—´ï¼Œå¯èƒ½ä¼šæœ‰ä¸€äº›è¿›åº¦è¯¯å·®ï¼Œä½†æˆ‘å®åœ¨æƒ³ä¸å‡ºæ¥ï¼ŒæœåŠ¡å™¨ä¸ºä»€ä¹ˆè¦ç²¾ç»†è¶…è¿‡usec
    timeval                    zce_time_value_;
};
}
