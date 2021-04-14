/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_log_msg.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       Monday, December 30, 2013
* @brief
*
*
* @details
*
*
*
* @note
*
*/

#ifndef ZCE_LIB_LOG_MESSAGE_H_
#define ZCE_LIB_LOG_MESSAGE_H_

#include "zce/logger/log_basic.h"

/******************************************************************************************
class ZCE_Trace_LogMsg
******************************************************************************************/
class ZCE_Trace_LogMsg : public ZCE_LogTrace_Basic
{

public:

    ///构造函数
    ZCE_Trace_LogMsg();
    virtual ~ZCE_Trace_LogMsg();

    ///为C爱好者准备的封装，且记不要混用,输出一条(行)日志记录.
    void write_logmsg(zce::LOG_PRIORITY outlevel, const char *str_format, ...);

protected:

    /*!
    @brief      实际写入日志的函数
    @param      outlevel    输出日志级别
    @param      lpszFormat  日志的格式化字符串
    @param      args        动态的日志参数列表组成的va_list
    */
    void vwrite_logmsg(zce::LOG_PRIORITY outlevel, const char *str_format, va_list args);

public:

    //实例的赋值
    static void instance(ZCE_Trace_LogMsg *);
    //实例的获得
    static ZCE_Trace_LogMsg *instance();
    //清除实例
    static void clean_instance();

    //利用单子对象，Aseert调试
    static void debug_assert(const char *file_name,
                             const int file_line,
                             const char *fuction_name,
                             const char *expression_name);

    ///利用单子对象，输出Aseert调试信息,增强版本函数
    static void debug_assert_ex(const char *file_name,
                                const int file_line,
                                const char *fuction_name,
                                const char *expression_name,
                                const char *out_string);

    ///利用单子对象，打印日志信息
    static void debug_output(zce::LOG_PRIORITY dbglevel,
                             const char *str_format,
                             ... );

    //为什么要这样麻烦,问题在于不能用宏(__VA_ARGS__)解决问题(部分编译器不支持,VS2003),
#if _MSC_VER <= 1300

    //使用 RS_TRACE 调试级别输出
    static void debug_traceex(const char *, ... );
    //使用 RS_DEBUG 调试级别输出
    static void debug_debugex(const char *str_format, ... );
    //使用 RS_INFO 调试级别输出
    static void debug_infoex(const char *str_format, ... );
    //使用 RS_ERROR 错误级别输出
    static void debug_errorex(const char *str_format, ... );
    //使用 RS_ALERT 严重错误级别输出
    static void debug_alertex(const char *str_format, ... );
    //使用 RS_FATAL 致命错误级别输出
    static void debug_fatalex(const char *str_format, ... );

#endif //#if _MSC_VER <= 1300

protected:

    ///多行输出对象的最大长度
    static const size_t SIZE_OF_MULTILINE_BUF = 512 * 1024 - 1;

protected:

    ///多行术后出的
    char                    *multiline_buf_;

    ///同步锁
    ZCE_Thread_Light_Mutex   multiline_lock_;

protected:

    ///单子实例指针
    static ZCE_Trace_LogMsg *log_instance_;

};

#endif //ZCE_LIB_TRACE_LOG_MESSAGE_H_

