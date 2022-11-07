/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_log_loggging.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
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

#pragma once

//==========================================================================================================

#include "zce/config.h"
#include "zce/os_adapt/error.h"

#if defined ZCE_OS_WINDOWS
#define ZNO_LOG             __noop
#else
#define ZNO_LOG(...)        (void(0))
#endif

#if defined ZCE_NO_LOG  && ZCE_NO_LOG == 1

#ifndef ZLOG_ENABLE
#define ZLOG_ENABLE           ZNO_LOG
#endif
#ifndef ZLOG_DISABLE
#define ZLOG_DISABLE          ZNO_LOG
#endif
#ifndef ZLOG_SET_OUTLEVEL
#define ZLOG_SET_OUTLEVEL(x)  ZNO_LOG
#endif
#ifndef ZCE_LOG
#define ZCE_LOG               ZNO_LOG
#endif
#ifndef ZPP_LOG
#define ZPP_LOG               ZNO_LOG
#endif

#ifndef ZLOG_TRACE
#define ZLOG_TRACE(...)       ZNO_LOG
#endif
#ifndef ZLOG_DEBUG
#define ZLOG_DEBUG(...)       ZNO_LOG
#endif
#ifndef ZLOG_INFO
#define ZLOG_INFO(...)        ZNO_LOG
#endif
#ifndef ZLOG_ERROR
#define ZLOG_ERROR(...)       ZNO_LOG
#endif
#ifndef ZLOG_ALERT
#define ZLOG_ALERT(...)       ZNO_LOG
#endif
#ifndef ZLOG_FATAL
#define ZLOG_FATAL(...)       ZNO_LOG
#endif

#ifndef ZCE_ASSERT_ALL
#define ZCE_ASSERT_ALL(expr) assert(expr)
#endif

#ifndef ZCE_ASSERT_ALL_EX
#define ZCE_ASSERT_ALL_EX(expr,str) assert(expr)
#endif

#endif //#if defined ZCE_NO_LOG  && ZCE_NO_LOG == 1

//定义日志输出,则实用内部的函数作为输出定义，如果关闭日志，只printf输出日志内容
#if defined ZCE_USE_LOGMSG  && ZCE_USE_LOGMSG == 1

#include "zce/logger/log_comm.h"
#include "zce/logger/log_file.h"
#include "zce/logger/log_msg.h"

//打开输出 和 关闭输出
#ifndef ZLOG_ENABLE
#define ZLOG_ENABLE           zce::log_msg::instance()->enable_output(true)
#endif
#ifndef ZLOG_DISABLE
#define ZLOG_DISABLE          zce::log_msg::instance()->enable_output(false)
#endif
//输出MASK级别,小于这个级别的日志信息不予输出
#ifndef ZLOG_SET_OUTLEVEL
#define ZLOG_SET_OUTLEVEL(x)  zce::log_msg::instance()->set_log_priority(x)
#endif
//当年还用过一套为GCC2.9定义的双括号的红，土死了，后来不打算兼容那么多版本，我懒

//日志输出宏，ZCE_LOG为C格式输出，ZPP_LOG是C++ {}格式
#ifndef ZCE_LOG
#define ZCE_LOG               zce::log_msg::write_logmsg
#endif
#ifndef ZPP_LOG
#define ZPP_LOG               zce::log_msg::write_logplus
#endif
#ifndef Z20_LOG
#define Z20_LOG               zce::log_msg::write_logcpp20
#endif
#ifndef ZCE_TRACE
#define ZCE_TRACE(...)        zce::log_msg::write_logmsg(RS_TRACE,__VA_ARGS__)
#endif
#ifndef ZCE_DEBUG
#define ZCE_DEBUG(...)        zce::log_msg::write_logmsg(RS_DEBUG,__VA_ARGS__)
#endif
#ifndef ZCE_INFO
#define ZCE_INFO(...)         zce::log_msg::write_logmsg(RS_INFO,__VA_ARGS__)
#endif
#ifndef ZCE_ERROR
#define ZCE_ERROR(...)        zce::log_msg::write_logmsg(RS_ERROR,__VA_ARGS__)
#endif
#ifndef ZCE_ALERT
#define ZCE_ALERT(...)        zce::log_msg::write_logmsg(RS_ALERT,__VA_ARGS__)
#endif
#ifndef ZCE_FATAL
#define ZCE_FATAL(...)        zce::log_msg::write_logmsg(RS_FATAL,__VA_ARGS__)
#endif

#ifndef ZPP_TRACE
#define ZPP_TRACE(...)        zce::log_msg::write_logplus(RS_TRACE,__VA_ARGS__)
#endif
#ifndef ZPP_DEBUG
#define ZPP_DEBUG(...)        zce::log_msg::write_logplus(RS_DEBUG,__VA_ARGS__)
#endif
#ifndef ZPP_INFO
#define ZPP_INFO(...)         zce::log_msg::write_logplus(RS_INFO,__VA_ARGS__)
#endif
#ifndef ZPP_ERROR
#define ZPP_ERROR(...)        zce::log_msg::write_logplus(RS_ERROR,__VA_ARGS__)
#endif
#ifndef ZPP_ALERT
#define ZPP_ALERT(...)        zce::log_msg::write_logplus(RS_ALERT,__VA_ARGS__)
#endif
#ifndef ZPP_FATAL
#define ZPP_FATAL(...)        zce::log_msg::write_logplus(RS_FATAL,__VA_ARGS__)
#endif

#ifndef Z20_TRACE
#define Z20_TRACE(...)        zce::log_msg::write_logcpp20(RS_TRACE,__VA_ARGS__)
#endif
#ifndef Z20_DEBUG
#define Z20_DEBUG(...)        zce::log_msg::write_logcpp20(RS_DEBUG,__VA_ARGS__)
#endif
#ifndef Z20_INFO
#define Z20_INFO(...)         zce::log_msg::write_logcpp20(RS_INFO,__VA_ARGS__)
#endif
#ifndef Z20_ERROR
#define Z20_ERROR(...)        zce::log_msg::write_logcpp20(RS_ERROR,__VA_ARGS__)
#endif
#ifndef Z20_ALERT
#define Z20_ALERT(...)        zce::log_msg::write_logcpp20(RS_ALERT,__VA_ARGS__)
#endif
#ifndef Z20_FATAL
#define Z20_FATAL(...)        zce::log_msg::write_logcpp20(RS_FATAL,__VA_ARGS__)
#endif

//----------------------------------------------------------------------------------
//无论，DEBUG版本，REALSE版本也起作用的一些断言，这些宏在所有版本都起作用，用于一些在运行时期也要判断的东东
//ALL，任何时候，

//因为Window和LINUX断言中实际使用的函数，其实在非DEBUG版本其实都没有暴漏出来，所以必现有下面这些定义才能使用。

#if defined (ZCE_OS_WINDOWS)

_ACRTIMP int __cdecl _CrtDbgReport(
    _In_       int         _ReportType,
    _In_opt_z_ char const* _FileName,
    _In_       int         _Linenumber,
    _In_opt_z_ char const* _ModuleName,
    _In_opt_z_ char const* _Format,
    ...);
#endif

#if defined (ZCE_OS_LINUX)
//__assert_fail这个函数是extern的，必须在这儿extern引用，linux自己只在非NDEBUG版本下才有extern引用
extern "C"  void __assert_fail(__const char* __assertion, __const char* __file,
                               unsigned int __line, __const char* __function)
    __THROW __attribute__((__noreturn__));
#endif //#if defined (ZCE_OS_LINUX)

#ifndef ZCE_ASSERT_ALL
#if defined (ZCE_OS_WINDOWS)
#define ZCE_ASSERT_ALL(expr) \
    (void) ((!!(expr)) || \
            (zce::log_msg::debug_assert(__FILE__,__LINE__,__ZCE_FUNC__,#expr),0) || \
            (_CrtDbgBreak(), 0))
#else
#define ZCE_ASSERT_ALL(expr) \
    (void) ((!!(expr)) || \
            (zce::log_msg::debug_assert(__FILE__,__LINE__,__ZCE_FUNC__,#expr),0) || \
            (__assert_fail (#expr, __FILE__, __LINE__, __ZCE_FUNC__),0))
#endif
#endif  //#ifndef ZCE_ASSERT_ALL

#ifndef ZCE_ASSERT_ALL_EX
#if defined (ZCE_OS_WINDOWS)
#define ZCE_ASSERT_ALL_EX(expr,str) \
    (void) ((!!(expr)) || \
            (zce::log_msg::debug_assert_ex(__FILE__,__LINE__,__ZCE_FUNC__,#expr,str),0) || \
            (_CrtDbgBreak(), 0))
#else
#define ZCE_ASSERT_ALL_EX(expr,str) \
    (void) ((!!(expr)) || \
            (zce::log_msg::debug_assert_ex(__FILE__,__LINE__,__ZCE_FUNC__,#expr,str),0) || \
            (__assert_fail (#expr, __FILE__, __LINE__, __ZCE_FUNC__),0))
#endif
#endif  //#ifndef ZCE_ASSERT_ALL_EX

#endif //#if defined ZCE_USE_LOGMSG  && ZCE_USE_LOGMSG == 1

//如果没有定义使用ZCE内部的日志输出，使用printf作为输出方法，
#if defined ZCE_USE_LOG_PRINT  && ZCE_USE_LOG_PRINT == 1

#include "zce/logger/log_print.h"

#ifndef ZLOG_ENABLE
#define ZLOG_ENABLE           zce::log_printf::instance()->enable_output(true)
#endif
#ifndef ZLOG_DISABLE
#define ZLOG_DISABLE          zce::log_printf::instance()->enable_output(false)
#endif
#ifndef ZLOG_SET_OUTLEVEL
#define ZLOG_SET_OUTLEVEL(x)  zce::log_printf::instance()->set_log_priority(x)
#endif
#ifndef ZCE_LOG
#define ZCE_LOG               zce::log_printf::write_logmsg
#endif
#ifndef ZPP_LOG
#define ZPP_LOG               #error "[Error]log printno ZPP_LOG."
#endif

#ifndef ZLOG_TRACE
#define ZLOG_TRACE(...)       zce::log_printf::write_logmsg(RS_TRACE,__VA_ARGS__)
#endif
#ifndef ZLOG_DEBUG
#define ZLOG_DEBUG(...)       zce::log_printf::write_logmsg(RS_DEBUG,__VA_ARGS__)
#endif
#ifndef ZLOG_INFO
#define ZLOG_INFO(...)        zce::log_printf::write_logmsg(RS_INFO,__VA_ARGS__)
#endif
#ifndef ZLOG_ERROR
#define ZLOG_ERROR(...)       zce::log_printf::write_logmsg(RS_ERROR,__VA_ARGS__)
#endif
#ifndef ZLOG_ALERT
#define ZLOG_ALERT(...)       zce::log_printf::write_logmsg(RS_ALERT,__VA_ARGS__)
#endif
#ifndef ZLOG_FATAL
#define ZLOG_FATAL(...)       zce::log_printf::write_logmsg(RS_FATAL,__VA_ARGS__)
#endif

#ifndef ZCE_ASSERT_ALL
#define ZCE_ASSERT_ALL(expr) assert(expr)
#endif

#ifndef ZCE_ASSERT_ALL_EX
#define ZCE_ASSERT_ALL_EX(expr,str) assert(expr)
#endif

#endif

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
namespace zce
{
class trace_function
{
public:
    //!函数名称
    const char* func_name_;
    //!文件名称
    const char* codefile_name_;
    //!文件的行号，行号是函数体内部的位置，不是函数声明的起始位置，但这又何妨
    int                code_line_;
    //!输出的日志级别
    zce::LOG_PRIORITY   log_priority_;

    //!如果需要跟踪返回值，把返回值的变量的指针作为一个参数
    int* ret_ptr_ = nullptr;

public:
    //利用构造函数显示进入函数的输出
    trace_function(const char* func_name,
                   const char* file_name,
                   int file_line,
                   zce::LOG_PRIORITY   log_priority) :
        func_name_(func_name),
        codefile_name_(file_name),
        code_line_(file_line),
        log_priority_(log_priority),
        ret_ptr_(nullptr)
    {
        ZCE_LOG(log_priority_, "[zcelib] [FUNCTION TRACE]%s entry,File %s|%u ", func_name_, codefile_name_, code_line_);
    }

    //利用构造函数显示进入函数的输出
    trace_function(const char* func_name,
                   const char* file_name,
                   int file_line,
                   zce::LOG_PRIORITY   log_priority,
                   int* ret_ptr) :
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
    ~trace_function()
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
}

//ZCE_FUNCTION_TRACE(RS_DEBUG)宏用于跟踪函数的进出
//请在函数的开始使用ZCE_FUNCTION_TRACE(RS_DEBUG)这个宏，后面必须加分号
#ifndef ZCE_TRACE_FUNCTION
#define ZCE_TRACE_FUNCTION(x) zce::trace_function  ____tmp_func_trace_(__ZCE_FUNC__,__FILE__,__LINE__,(x))
#endif

#ifndef ZCE_TRACE_FUNC_RETURN
#define ZCE_TRACE_FUNC_RETURN(x,y) zce::trace_function  ____tmp_func_trace_(__ZCE_FUNC__,__FILE__,__LINE__,(x),(y))
#endif

//!用于程序运行到的地方。
#ifndef ZCE_TRACE_FILELINE
#define ZCE_TRACE_FILELINE(log_priority)   ZCE_LOG((log_priority),"[FILELINE TRACE]goto File %s|%d,function:%s.",\
                                                   __FILE__,__LINE__,__ZCE_FUNC__)
#endif

//!
#ifndef ZCE_TRACE_FAIL_RETURN
#define ZCE_TRACE_FAIL_RETURN(log_priority,fail_str,ret_int) ZCE_LOG((log_priority),"[FAIL RETRUN]Fail in file [%s|%d],function:%s," \
                                                                     "api fail info:%s,return %d,last error %d.",__FILE__,__LINE__,__ZCE_FUNC__,(fail_str),(ret_int),zce::last_error())
#endif

#ifndef ZCE_TRACE_FAIL_INFO
#define ZCE_TRACE_FAIL_INFO(log_priority,fail_str)     ZCE_LOG((log_priority),\
                                                               "[API FAIL ]API fail in file [%s|%d],function:%s,fail api:%s,last error %d.",\
                                                               __FILE__,__LINE__,__ZCE_FUNC__,(fail_str),zce::last_error())
#endif

//日志打印堆栈信息的宏，x是日志的输出级别
#ifndef ZCE_BACKTRACE_STACK
#define ZCE_BACKTRACE_STACK(x) zce::backtrace_stack(x,20,__ZCE_FUNC__)
#endif

//日志打印堆栈信息的宏，x是日志的输出级别,y是指针，z是指针长度
#ifndef ZCE_TRACE_POINTER_DATA
#define ZCE_TRACE_POINTER_DATA(x,y,z)     zce::memory_debug((x),__ZCE_FUNC__,(y),(z))
#endif

//-----------------------------------------------------------------------------------------------
//DEBUG 版本特有的一些宏
//ZCE_ASSERT原来的写法是 while(!(f)){ do something,后来发现WINDOWS下会对这个进行告警，被迫改了

#if defined DEBUG || defined _DEBUG

#ifndef ZCE_LOG_DEBUG
#define ZCE_LOG_DEBUG        ZCE_LOG
#endif

#ifndef ZPP_LOG_DEBUG
#define ZPP_LOG_DEBUG        ZPP_LOG
#endif

#ifndef Z20_LOG_DEBUG
#define Z20_LOG_DEBUG        Z20_LOG
#endif

#ifndef ZCE_ASSERT_DEBUG
#define ZCE_ASSERT_DEBUG     ZCE_ASSERT
#endif

#ifndef ZCE_FUNCTION_TRACE_DEBUG
#define ZCE_FUNCTION_TRACE_DEBUG   ZCE_TRACE_FUNCTION
#endif

#ifndef ZCE_FILELINE_TRACE_DEBUG
#define ZCE_FILELINE_TRACE_DEBUG   ZCE_TRACE_FILELINE
#endif

#ifndef ZCE_BACKTRACE_STACK_DEBUG
#define ZCE_BACKTRACE_STACK_DEBUG  ZCE_BACKTRACE_STACK
#endif

//非调试版本的宏定义，
#else  //#if defined DEBUG || defined _DEBUG

//如果不是调试编译,将这些宏置为空

#ifndef ZCE_LOG_DEBUG
#if defined ZCE_OS_WINDOWS
#define ZCE_LOG_DEBUG             __noop
#else
#define ZCE_LOG_DEBUG(...)          do  {} while (0)
#endif
#endif

#ifndef ZPP_LOG_DEBUG
#if defined ZCE_OS_WINDOWS
#define ZPP_LOG_DEBUG             __noop
#else
#define ZPP_LOG_DEBUG(...)          do  {} while (0)
#endif
#endif

#ifndef ZCE_ASSERT_DEBUG
#define ZCE_ASSERT_DEBUG(...)       ((void)0)
#endif

#ifndef ZCE_FUNCTION_TRACE_DEBUG
#define ZCE_FUNCTION_TRACE_DEBUG(...) ((void)0)
#endif

#ifndef ZCE_FILELINE_TRACE_DEBUG
#define ZCE_FILELINE_TRACE_DEBUG(...) ((void)0)
#endif

#ifndef ZCE_BACKTRACE_STACK_DEBUG
#define ZCE_BACKTRACE_STACK_DEBUG(...) ((void)0)
#endif

#endif //#if defined DEBUG || defined _DEBUG

//-----------------------------------------------------------------------------------------------
