/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_log_plus.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2004年8月1日
* @brief
*
* @details    ow，ow，又可以卖老来讲一点点历史了，一点点，
*             最开始写这个类的时候，我刚刚对C++的stream有一些了解。正如一切土孩子
*             看见新东东一样，你会以为新东东是完美的，在第一版我用stream的方式实现了
*             日志输出，当然不得不说，也还是挺酷的，
*             但说回来C++ stream真算不上一个设计完美的东东
*             1.格式化控制痛苦，必须承认在格式化控制上他远远不如printf这种函数，
*             2.性能弱，大约比snprintf这类函数的慢50%.(GCC7.1最新版本有惊喜)
*             3.多线程情况并不好控制，特别是多线程环境下，你基本无法加锁，
*             所以C++ stream空有一身靓丽的外表，却很少人真正使用。
*
*             所以当要彻底重构的时候，我决定测定抛弃steam的代码，特别是当我发现
*             fast format这些函数库在字符串格式化上已经超越C的snprintf，无疑
*             极大的振奋了我。我决定自己实现一下，当然C的log函数迅速整理完毕，
*             C++的版本却仍然等待了很长一段时间，即使我的zce::foo_snprintf
*             已经完成后，因为我发现，如果没有变餐的模版，每一次用模版实现字符串格式
*             化都是一次煎熬，你只能用宏完成代码替换。
*
*             所以C++ 20，快点到来把。期待C++ format的实现。
*
* @note
*
*/

#ifndef ZCE_LIB_TRACE_LOG_PLUS_H_
#define ZCE_LIB_TRACE_LOG_PLUS_H_

#include "zce_string_extend.h"
#include "zce_log_basic.h"



/******************************************************************************************
class ZCE_LogTrace_Plus 只是为C++爱好者准备的封装,
******************************************************************************************/
class ZCE_LogTrace_Plus : public ZCE_LogTrace_Basic
{

public:

    ZCE_LogTrace_Plus();
    //析构函数
    ~ZCE_LogTrace_Plus();


#if ZCE_SUPPORT_CPP11 == 1

    template <typename... out_type >
    void foo_write_logmsg(zce::LOG_PRIORITY outlevel,
                          const char *str_format,
                          const out_type &...out_data)
    {
        //如果日志输出开关关闭
        if (if_output_log_ == false)
        {
            return;
        }

        //如果输出的日志级别低于Mask值
        if (permit_outlevel_ > outlevel)
        {
            return;
        }

        //得到当前时间
        timeval now_time_val(zce::gettimeofday());

        //我要保留一个位置放'\0'
        char log_tmp_buffer[LOG_TMP_BUFFER_SIZE + 1];
        log_tmp_buffer[LOG_TMP_BUFFER_SIZE] = '\0';

        //还是为\n考虑留一个空间
        size_t sz_buf_len = LOG_TMP_BUFFER_SIZE;
        size_t sz_use_len = 0;

        stringbuf_loghead(outlevel,
                          now_time_val,
                          log_tmp_buffer,
                          sz_buf_len,
                          sz_use_len);

        sz_buf_len -= sz_use_len;

        //得到打印信息,foo_snprintf 为自己内部的函数，str_format使用%?作为输出控制符
        size_t sprt_use_len = 0;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data...);
        sz_use_len += sprt_use_len;
        sz_buf_len -= sprt_use_len;

        //如果要自动增加换行符号，
        if (auto_new_line_)
        {
            log_tmp_buffer[sz_use_len] = '\n';
            ++sz_use_len;

            //注意sz_buf_len在这儿没有调整，因为'\n'的位置我前面为了安全扣除了
            //也不能直接用--sz_buf_len;因为sz_buf_len可能==0
        }

        output_log_info(now_time_val,
                        log_tmp_buffer,
                        sz_use_len);
    }

#else

#define __ZCE_LOGPP_WRITE_BEGIN        if (if_output_log_ == false)\
    { \
        return ;\
    }\
    if (permit_outlevel_ > outlevel ) \
    { \
        return; \
    } \
    timeval now_time_val (zce::gettimeofday()); \
    char log_tmp_buffer[LOG_TMP_BUFFER_SIZE +1 ]; \
    log_tmp_buffer[LOG_TMP_BUFFER_SIZE ] = '\0'; \
    size_t sz_buf_len = LOG_TMP_BUFFER_SIZE; \
    size_t sz_use_len = 0; \
    stringbuf_loghead(outlevel,now_time_val,log_tmp_buffer,sz_buf_len,sz_use_len); \
    sz_buf_len -= sz_use_len; \
    size_t sprt_use_len = 0

#define __ZCE_LOGPP_WRITE_END        if (if_output_log_ == false) \
        sz_use_len += sprt_use_len; \
    sz_buf_len -= sprt_use_len; \
    if (auto_new_line_) \
    { \
        log_tmp_buffer[sz_use_len] = '\n'; \
        ++sz_use_len; \
    } \
    output_log_info(now_time_val,log_tmp_buffer,sz_use_len)

    //
    template <class T1, class T2>
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2)
    {
        //如果日志输出开关关闭
        if (if_output_log_ == false)
        {
            return ;
        }

        //如果输出的日志级别低于Mask值
        if (permit_outlevel_ > outlevel )
        {
            return;
        }

        //得到当前时间
        timeval now_time_val (zce::gettimeofday());

        //我要保留一个位置放'\0'
        char log_tmp_buffer[LOG_TMP_BUFFER_SIZE ];
        log_tmp_buffer[LOG_TMP_BUFFER_SIZE - 1] = '\0';

        //还是为\n考虑留一个空间
        size_t sz_buf_len = LOG_TMP_BUFFER_SIZE - 2;
        size_t sz_use_len = 0;

        stringbuf_loghead(outlevel,
                          now_time_val,
                          log_tmp_buffer,
                          sz_buf_len,
                          sz_use_len);

        sz_buf_len -= sz_use_len;

        //得到打印信息,_vsnprintf为特殊函数
        size_t sprt_use_len = 0;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2);
        sz_use_len += sprt_use_len;
        sz_buf_len -= sprt_use_len;

        //如果要自动增加换行符号，
        if (auto_new_line_)
        {
            log_tmp_buffer[sz_use_len] = '\n';
            ++sz_use_len;

            //注意sz_buf_len在这儿没有调整，因为'\n'的位置我前面为了安全扣除了
            //也不能直接用--sz_buf_len;因为sz_buf_len可能==0
        }

        output_log_info(now_time_val,
                        log_tmp_buffer,
                        sz_use_len);

    }

    template < class T1 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21,
                      const T22 &out_data22 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21,
                      const T22 &out_data22,
                      const T23 &out_data23 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21,
                      const T22 &out_data22,
                      const T23 &out_data23,
                      const T24 &out_data24 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21,
                      const T22 &out_data22,
                      const T23 &out_data23,
                      const T24 &out_data24,
                      const T25 &out_data25 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21,
                      const T22 &out_data22,
                      const T23 &out_data23,
                      const T24 &out_data24,
                      const T25 &out_data25,
                      const T26 &out_data26 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21,
                      const T22 &out_data22,
                      const T23 &out_data23,
                      const T24 &out_data24,
                      const T25 &out_data25,
                      const T26 &out_data26,
                      const T27 &out_data27 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21,
                      const T22 &out_data22,
                      const T23 &out_data23,
                      const T24 &out_data24,
                      const T25 &out_data25,
                      const T26 &out_data26,
                      const T27 &out_data27,
                      const T28 &out_data28 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27, out_data28);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21,
                      const T22 &out_data22,
                      const T23 &out_data23,
                      const T24 &out_data24,
                      const T25 &out_data25,
                      const T26 &out_data26,
                      const T27 &out_data27,
                      const T28 &out_data28,
                      const T29 &out_data29 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27, out_data28, out_data29);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21,
                      const T22 &out_data22,
                      const T23 &out_data23,
                      const T24 &out_data24,
                      const T25 &out_data25,
                      const T26 &out_data26,
                      const T27 &out_data27,
                      const T28 &out_data28,
                      const T29 &out_data29,
                      const T30 &out_data30 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27, out_data28, out_data29, out_data30);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30, class T31 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21,
                      const T22 &out_data22,
                      const T23 &out_data23,
                      const T24 &out_data24,
                      const T25 &out_data25,
                      const T26 &out_data26,
                      const T27 &out_data27,
                      const T28 &out_data28,
                      const T29 &out_data29,
                      const T30 &out_data30,
                      const T31 &out_data31 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27, out_data28, out_data29, out_data30, out_data31);
        __ZCE_LOGPP_WRITE_END;
    }

    template < class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20, class T21, class T22, class T23, class T24, class T25, class T26, class T27, class T28, class T29, class T30, class T31, class T32 >
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char *str_format,
                      const T1 &out_data1,
                      const T2 &out_data2,
                      const T3 &out_data3,
                      const T4 &out_data4,
                      const T5 &out_data5,
                      const T6 &out_data6,
                      const T7 &out_data7,
                      const T8 &out_data8,
                      const T9 &out_data9,
                      const T10 &out_data10,
                      const T11 &out_data11,
                      const T12 &out_data12,
                      const T13 &out_data13,
                      const T14 &out_data14,
                      const T15 &out_data15,
                      const T16 &out_data16,
                      const T17 &out_data17,
                      const T18 &out_data18,
                      const T19 &out_data19,
                      const T20 &out_data20,
                      const T21 &out_data21,
                      const T22 &out_data22,
                      const T23 &out_data23,
                      const T24 &out_data24,
                      const T25 &out_data25,
                      const T26 &out_data26,
                      const T27 &out_data27,
                      const T28 &out_data28,
                      const T29 &out_data29,
                      const T30 &out_data30,
                      const T31 &out_data31,
                      const T32 &out_data32 )
    {
        __ZCE_LOGPP_WRITE_BEGIN;
        zce::foo_snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, sprt_use_len, str_format, out_data1, out_data2, out_data3, out_data4, out_data5, out_data6, out_data7, out_data8, out_data9, out_data10, out_data11, out_data12, out_data13, out_data14, out_data15, out_data16, out_data17, out_data18, out_data19, out_data20, out_data21, out_data22, out_data23, out_data24, out_data25, out_data26, out_data27, out_data28, out_data29, out_data30, out_data31, out_data32);
        __ZCE_LOGPP_WRITE_END;
    }

#endif //

public:

    //实例的赋值
    static void instance(ZCE_LogTrace_Plus *);
    //实例的获得
    static ZCE_LogTrace_Plus *instance();
    //清除实例
    static void clean_instance();

protected:
    //
    static ZCE_LogTrace_Plus *lpp_instance_;

};

#endif //ZCE_LIB_TRACE_LOG_PLUS_H_

