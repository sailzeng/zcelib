/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_trace_loggging.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2010年7月11日
* @brief      定义各种调试的日志，断言，宏，用于调试日志输出
*             
* @details
*
*
*
* @note
*
*/

#ifndef ZCE_LIB_LOGGING_H_
#define ZCE_LIB_LOGGING_H_

//==========================================================================================================

#include "zce_config.h"

//定义日志输出,则实用内部的函数作为输出定义
#if defined ZCE_USE_LOGMSG  && ZCE_USE_LOGMSG == 1

#include "zce_trace_log_msg.h"

//打开输出
#define ZLOG_ENABLE           ZCE_Trace_LogMsg::instance()->enable_output(true)
//关闭输出
#define ZLOG_DISABLE          ZCE_Trace_LogMsg::instance()->enable_output(false)
//输出MASK级别,小于这个级别的日志信息不予输出
#define ZLOG_SET_OUTLEVEL     ZCE_Trace_LogMsg::instance()->set_log_priority


//当年还用过一套为GCC2.9定义的双括号的红，土死了，后来不打算兼容那么多版本，我懒

//使用调试级别输出日志
#define ZCE_LOG               ZCE_Trace_LogMsg::debug_output

#if _MSC_VER <= 1300

//提供一些简写的方式，虽然我也觉得不是特别好
#define ZLOG_TRACE            ZCE_Trace_LogMsg::debug_traceex
#define ZLOG_DEBUG            ZCE_Trace_LogMsg::debug_debugex
#define ZLOG_INFO             ZCE_Trace_LogMsg::debug_infoex
#define ZLOG_ERROR            ZCE_Trace_LogMsg::debug_errorex
#define ZLOG_ALERT            ZCE_Trace_LogMsg::debug_alertex
#define ZLOG_FATAL            ZCE_Trace_LogMsg::debug_fatalex

#else

#define ZLOG_TRACE(...)       ZCE_Trace_LogMsg::debug_output(RS_TRACE,__VA_ARGS__)
#define ZLOG_DEBUG(...)       ZCE_Trace_LogMsg::debug_output(RS_DEBUG,__VA_ARGS__)
#define ZLOG_INFO(...)        ZCE_Trace_LogMsg::debug_output(RS_INFO,__VA_ARGS__)
#define ZLOG_ERROR(...)       ZCE_Trace_LogMsg::debug_output(RS_ERROR,__VA_ARGS__)
#define ZLOG_ALERT(...)       ZCE_Trace_LogMsg::debug_output(RS_ALERT,__VA_ARGS__)
#define ZLOG_FATAL(...)       ZCE_Trace_LogMsg::debug_output(RS_FATAL,__VA_ARGS__)

#endif

//----------------------------------------------------------------------------------
//无论，DEBUG版本，REALSE版本也起作用的一些断言，这些宏在所有版本都起作用，用于一些在运行时期也要判断的东东
//ALL，任何时候，

//因为Window和LINUX断言中实际使用的函数，其实在非DEBUG版本其实都没有暴漏出来，所以必现有下面这些定义才能使用。

#if defined (ZCE_OS_WINDOWS)
_CRTIMP int __cdecl _CrtDbgReport(
    int _ReportType,
    const char *_Filename,
    int _Linenumber,
    const char *_ModuleName,
    const char *_Format,
    ...);
#endif

#if defined (ZCE_OS_LINUX)
//__assert_fail这个函数是extern的，必须在这儿extern引用，linux自己只在非NDEBUG版本下才有extern引用
extern "C"  void __assert_fail(__const char *__assertion, __const char *__file,
    unsigned int __line, __const char *__function)
    __THROW __attribute__((__noreturn__));
#endif //#if defined (ZCE_OS_LINUX)


#ifndef ZCE_ASSERT_ALL
#if defined (ZCE_OS_WINDOWS)
#define ZCE_ASSERT_ALL(expr) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert(__FILE__,__LINE__,__ZCE_FUNC__,#expr),0) || \
            (_CrtDbgBreak(), 0))
#else
#define ZCE_ASSERT_ALL(expr) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert(__FILE__,__LINE__,__ZCE_FUNC__,#expr),0) || \
            (__assert_fail (#expr, __FILE__, __LINE__, __ZCE_FUNC__),0))
#endif
#endif  //#ifndef ZCE_ASSERT_ALL


#ifndef ZCE_ASSERT_ALL_EX
#if defined (ZCE_OS_WINDOWS)
#define ZCE_ASSERT_ALL_EX(expr,str) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert_ex(__FILE__,__LINE__,__ZCE_FUNC__,#expr,str),0) || \
            (_CrtDbgBreak(), 0))
#else
#define ZCE_ASSERT_ALL_EX(expr,str) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert_ex(__FILE__,__LINE__,__ZCE_FUNC__,#expr,str),0) || \
            (__assert_fail (#expr, __FILE__, __LINE__, __ZCE_FUNC__),0))
#endif
#endif  //#ifndef ZCE_ASSERT_ALL_EX


//如果没有定义实用ZCE内部的日志输出，使用printf作为输出方法，
#else

#include "zce_trace_log_priority.h"

//==========================================================================================================
class ZCE_Trace_Printf
{
public:

    //输出va_list的参数信息
    void vwrite_logmsg(ZCE_LOG_PRIORITY outlevel,
        const char *str_format,
        va_list args)
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

        //得到打印信息,_vsnprintf为特殊函数
        vfprintf(stderr, str_format, args);
		fprintf(stderr, "\n");
    }

    //写日志
    void write_logmsg(ZCE_LOG_PRIORITY outlevel, const char *str_format, ...)
    {
        va_list args;
        va_start(args, str_format);
        vwrite_logmsg(outlevel, str_format, args);
        va_end(args);

    }

    //打开日志输出开关
    void enable_output(bool enable_out)
    {
        if_output_log_ = enable_out;
    }

    //设置日志输出Level
    ZCE_LOG_PRIORITY set_log_priority(ZCE_LOG_PRIORITY outlevel)
    {
        ZCE_LOG_PRIORITY oldlevel = permit_outlevel_;
        permit_outlevel_ = outlevel;
        return oldlevel;
    }

    //!利用单子对象，打印日志信息
    static void debug_output(ZCE_LOG_PRIORITY dbglevel,
        const char *str_format,
        ...)
    {
        va_list args;
        va_start(args, str_format);
        
		ZCE_Trace_Printf::instance()->vwrite_logmsg(dbglevel, str_format, args);
        
        va_end(args);
    }

    //实例的获得
    static ZCE_Trace_Printf *instance()
    {
		static ZCE_Trace_Printf log_instance;
        return &log_instance;
    }

    //
protected:

    //!输出日志信息的Mask值,小于这个信息的信息不予以输出
    ZCE_LOG_PRIORITY      permit_outlevel_ = RS_DEBUG;

    //!是否输出日志信息,可以用于暂时屏蔽
    bool                  if_output_log_ = true;

};

#define ZLOG_ENABLE           ZCE_Trace_Printf::instance()->enable_output(true)
#define ZLOG_DISABLE          ZCE_Trace_Printf::instance()->enable_output(false)
#define ZLOG_SET_OUTLEVEL     ZCE_Trace_Printf::instance()->set_log_priority
#define ZCE_LOG               ZCE_Trace_Printf::debug_output

#define ZLOG_TRACE(...)       ZCE_Trace_Printf::debug_output(RS_TRACE,__VA_ARGS__)
#define ZLOG_DEBUG(...)       ZCE_Trace_Printf::debug_output(RS_DEBUG,__VA_ARGS__)
#define ZLOG_INFO(...)        ZCE_Trace_Printf::debug_output(RS_INFO,__VA_ARGS__)
#define ZLOG_ERROR(...)       ZCE_Trace_Printf::debug_output(RS_ERROR,__VA_ARGS__)
#define ZLOG_ALERT(...)       ZCE_Trace_Printf::debug_output(RS_ALERT,__VA_ARGS__)
#define ZLOG_FATAL(...)       ZCE_Trace_Printf::debug_output(RS_FATAL,__VA_ARGS__)

#ifndef ZCE_ASSERT_ALL
#define ZCE_ASSERT_ALL(expr) assert(expr)
#endif 

#ifndef ZCE_ASSERT_ALL_EX
#define ZCE_ASSERT_ALL_EX(expr,str) assert(expr)
#endif 

#endif //#if !defined ZCE_USE_LOGMSG 


//==========================================================================================================
//断言的宏的定义

//非REALSE版本的一些宏
#if !defined NDEBUG

#ifndef ZCE_ASSERT
#define ZCE_ASSERT(expr)   ZCE_ASSERT_ALL(expr)
#endif

#ifndef ZCE_ASSERT_EX
#define ZCE_ASSERT_EX(expr,str)   ZCE_ASSERT_ALL_EX(expr,str) 
#endif

#ifndef ZCE_VERIFY
#define ZCE_VERIFY(expr)       ZCE_ASSERT(expr)
#endif

//#if !defined NDEBUG
#else

//注意区别ZCE_ASSERT和VERIFYZCE_VERIFY的区别,VERIFY内部的表达式式仍然保留的
#ifndef ZCE_ASSERT
#define ZCE_ASSERT(...)             ((void)0)
#endif

#ifndef ZCE_ASSERT_EX
#define ZCE_ASSERT_EX(...)          ((void)0)
#endif


#ifndef ZCE_VERIFY
#define ZCE_VERIFY(expr)            ((void)(expr))
#endif

#endif


//==========================================================================================================
//利用一个结构的构造和析构函数进行函数跟踪
class ZCE_Trace_Function
{
public:
    //!函数名称
    const char        *func_name_;
    //!文件名称
    const char        *codefile_name_;
    //!文件的行号，行号是函数体内部的位置，不是函数声明的起始位置，但这又何妨
    int                code_line_;
    //!输出的日志级别
    ZCE_LOG_PRIORITY   log_priority_;

    //!如果需要跟踪返回值，把返回值的变量的指针作为一个参数
    int               *ret_ptr_ = NULL;

public:
    //利用构造函数显示进入函数的输出
    ZCE_Trace_Function(const char *func_name,
        const char *file_name,
        int file_line,
        ZCE_LOG_PRIORITY   log_priority) :
        func_name_(func_name),
        codefile_name_(file_name),
        code_line_(file_line),
        log_priority_(log_priority),
        ret_ptr_(NULL)
    {
        ZCE_LOG(log_priority_, "[zcelib] [FUNCTION TRACE]%s entry,File %s|%u ", func_name_, codefile_name_, code_line_);
    }

    //利用构造函数显示进入函数的输出
    ZCE_Trace_Function(const char *func_name,
        const char *file_name,
        int file_line,
        ZCE_LOG_PRIORITY   log_priority,
        int *ret_ptr) :
        func_name_(func_name),
        codefile_name_(file_name),
        code_line_(file_line),
        log_priority_(log_priority),
        ret_ptr_(ret_ptr)
    {
        ZCE_LOG(log_priority_, "[zcelib] [FUNCTION TRACE][%s] entry,code file [%s|%u] default ret = [%d].",
            func_name_,
            codefile_name_,
            code_line_,
            *ret_ptr_);
    }

    //利用析构函数显示进入函数的输出
    ~ZCE_Trace_Function()
    {
        //根据是否关注返回值进行不同的输出
        if (ret_ptr_)
        {
            //这个地方输出的成功失败文字只有相对参考意义。
            ZCE_LOG(log_priority_, "[zcelib] [FUNCTION TRACE][%s] exit,code file [%s|%u] "
                "ret = [%d],return %s.",
                func_name_,
                codefile_name_,
                code_line_,
                *ret_ptr_,
                (*ret_ptr_ == 0) ? "success" : "fail");
        }
        else
        {
            ZCE_LOG(log_priority_, "[zcelib] [FUNCTION TRACE][%s] exit,code file [%s|%u] .",
                func_name_,
                codefile_name_,
                code_line_);
        }
    }

};

//ZCE_FUNCTION_TRACE(RS_DEBUG)宏用于跟踪函数的进出
//请在函数的开始使用ZCE_FUNCTION_TRACE(RS_DEBUG)这个宏，后面必须加分号
#ifndef ZCE_TRACE_FUNCTION
#define ZCE_TRACE_FUNCTION(x) ZCE_Trace_Function  ____tmp_func_trace_(__ZCE_FUNC__,__FILE__,__LINE__,(x))
#endif


#ifndef ZCE_TRACE_FUNC_RETURN
#define ZCE_TRACE_FUNC_RETURN(x,y) ZCE_Trace_Function  ____tmp_func_trace_(__ZCE_FUNC__,__FILE__,__LINE__,(x),(y))
#endif


//!用于程序运行到的地方。
#ifndef ZCE_TRACE_FILELINE
#define ZCE_TRACE_FILELINE(log_priority)        ZCE_LOG((log_priority),"[FILELINE TRACE]goto File %s|%d,function:%s.",\
    __FILE__,__LINE__,__ZCE_FUNC__)
#endif

//!
#ifndef ZCE_TRACE_FAIL_RETURN
#define ZCE_TRACE_FAIL_RETURN(log_priority,fail_str,ret_int) ZCE_LOG((log_priority),"[FAIL RETRUN]Fail in file [%s|%d],function:%s," \
    "fail info:%s,return %d,last error %d.",__FILE__,__LINE__,__ZCE_FUNC__,(fail_str),(ret_int),ZCE_LIB::last_error())
#endif

#ifndef ZCE_RECORD_FAIL_API
#define ZCE_RECORD_FAIL_API(log_priority,fail_str)     ZCE_LOG((log_priority),"[API FAIL ]API fail in file [%s|%d],function:%s,fail api:%s,last error %d.",__FILE__,__LINE__,__ZCE_FUNC__,(fail_str),ZCE_LIB::last_error())
#endif


//==========================================================================================================
//DEBUG 版本特有的一些宏
//ZCE_ASSERT原来的写法是 while(!(f)){ do something,后来发现WINDOWS下会对这个进行告警，被迫改了

#if defined DEBUG || defined _DEBUG

#ifndef ZCE_LOGMSG_DBG
#define ZCE_LOGMSG_DBG        ZCE_LOG
#endif

#ifndef ZCE_ASSERT_DGB
#define ZCE_ASSERT_DGB         ZCE_ASSERT
#endif

#ifndef ZCE_FUNCTION_TRACE_DBG
#define ZCE_FUNCTION_TRACE_DBG   ZCE_TRACE_FUNCTION
#endif


#ifndef ZCE_FILELINE_TRACE_DBG
#define ZCE_FILELINE_TRACE_DBG   ZCE_TRACE_FILELINE
#endif


//非调试版本的宏定义，
#else  //#if defined DEBUG || defined _DEBUG

//如果不是调试编译,将这些宏置为空


#ifndef ZCE_LOGMSG_DBG
#if defined ZCE_OS_WINDOWS
#define ZCE_LOGMSG_DBG             __noop
#else
#define ZCE_LOGMSG_DBG(...)          do  {} while (0)
#endif
#endif

#ifndef ZCE_ASSERT_DGB
#define ZCE_ASSERT_DGB(...)       ((void)0)
#endif

#ifndef ZCE_FUNCTION_TRACE_DBG
#define ZCE_FUNCTION_TRACE_DBG(...) ((void)0)
#endif


#ifndef ZCE_FILELINE_TRACE_DBG
#define ZCE_FILELINE_TRACE_DBG(...) ((void)0)
#endif


#endif //#if defined DEBUG || defined _DEBUG




#endif //ZCE_LIB_LOGGING_H_