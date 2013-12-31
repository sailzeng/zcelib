/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_trace_log_debug.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version    
* @date       2004年7月11日
* @brief      跟踪调试信息函数类ZCE_Trace_LogMsg描述
*             调试管理工具，可以在STDOUT,STDERR,Windos的调试窗口输出，
*             调试文件输出，根据不同宏定义保证跨平台允许，
* @details    
*             
*             
*             
* @note       
*             
*/


#ifndef ZCE_LIB_TRACE_LOGMSG_H_
#define ZCE_LIB_TRACE_LOGMSG_H_


#include "zce_trace_log_msg.h"



//==========================================================================================================



//打开输出
#define ZLOG_ENABLE           ZCE_Trace_LogMsg::enable_output_logmsg
//关闭输出
#define ZLOG_DISABLE          ZCE_Trace_LogMsg::disable_output_logmsg
//输出MASK级别,小于这个级别的日志信息不予输出
#define ZLOG_OUTLEVEL_MASK    ZCE_Trace_LogMsg::set_log_priority

//ZCE_LOGMSG为什么要你用双括号，GCC2.9惹得祸,土死了，但是我无能为力,
//我不打算兼容那么多版本，我懒

//使用调试级别输出日志
#define ZCE_LOGMSG            ZCE_Trace_LogMsg::debug_output

#if _MSC_VER <= 1300

//提供一些简写的方式，虽然我也觉得不是特别好
#define ZLOG_TRACE            ZCE_Trace_LogMsg::debug_traceex
#define ZLOG_DEBUG            ZCE_Trace_LogMsg::debug_debugEx
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

//==========================================================================================================
//利用一个结构的构造和析构函数进行函数跟踪
class ZCE_Trace_Function
{
public:
    //函数名称
    const char        *func_name_;
    //文件名称
    const char        *file_name_;
    //文件的行号，行号是函数体内部的位置，不是函数声明的起始位置，但这又何妨
    int                file_line_;
    //
    ZCE_LOG_PRIORITY   log_priority_;

public:
    //利用构造函数显示进入函数的输出
    ZCE_Trace_Function(const char *func_name,
                       const char *file_name,
                       int file_line,
                       ZCE_LOG_PRIORITY   log_priority)
        : func_name_(func_name)
        , file_name_(file_name)
        , file_line_(file_line)
        , log_priority_(log_priority)
    {
        ZCE_LOGMSG(log_priority_, "[zcelib] [FUNCTION TRACE]%s entry,File %s|%u ", func_name_, file_name_, file_line_);
    }
    //利用析构函数显示进入函数的输出
    ~ZCE_Trace_Function()
    {
        ZCE_LOGMSG(log_priority_, "[zcelib] [FUNCTION TRACE]%s exit,File %s|%u .", func_name_, file_name_, file_line_);
    }

};

//ZCE_FUNCTION_TRACE(RS_DEBUG)宏用于跟踪函数的进出
//请在函数的开始使用ZCE_FUNCTION_TRACE(RS_DEBUG)这个宏，后面必须加分号
#ifndef ZCE_TRACE_FUNCTION
#define ZCE_TRACE_FUNCTION(x)        ZCE_Trace_Function  ____tmp_func_trace_(__ZCE_FUNCTION__,__FILE__,__LINE__,(x))
#endif

//==========================================================================================================




//==========================================================================================================

///辅助打印一个指针内部数据的函数，用16进制的方式打印
namespace ZCE_LIB
{

    ///输出代码的堆栈信息，用于调试
    int backtrace_stack(ZCE_LOG_PRIORITY dbg_lvl,
        const char *dbg_info);

    ///日志打印堆栈信息
    ///调试打印的指针
    ///指针数据的长度
    void debug_pointer(ZCE_LOG_PRIORITY dbg_lvl,
        const char *dbg_info,
        const unsigned char  *debug_ptr,
        size_t data_len);
    
};

//日志打印堆栈信息的宏，x是日志的输出级别
#ifndef ZCE_BACKTRACE_STACK
#define ZCE_BACKTRACE_STACK(x)            ZCE_LIB::backtrace_stack(x,__ZCE_FUNCTION__)
#endif

//日志打印堆栈信息的宏，x是日志的输出级别,y是指针，z是指针长度
#ifndef ZCE_TRACE_POINTER_DATA
#define ZCE_TRACE_POINTER_DATA(x,y,z)     ZCE_LIB::debug_pointer((x),__ZCE_FUNCTION__,(y),(z))
#endif

//==========================================================================================================



///用于程序运行到的地方。
#ifndef ZCE_TRACE_FILELINE
#define ZCE_TRACE_FILELINE(x)        ZCE_LOGMSG(x,"[FILELINE TRACE]goto File %s|%d,function:%s.",__FILE__,__LINE__,__ZCE_FUNCTION__)
#endif

///
#ifndef ZCE_TRACE_FAIL_RETURN
#define ZCE_TRACE_FAIL_RETURN(x,y,z) ZCE_LOGMSG(x,"[FAIL RETRUN]Fail in file [%s|%d],function:%s,fail info:%s,return %d,last error %d.",__FILE__,__LINE__,__ZCE_FUNCTION__,y,z,ZCE_OS::last_error())
#endif

#ifndef ZCE_RECORD_FAIL_API
#define ZCE_RECORD_FAIL_API(x,y)     ZCE_LOGMSG(x,"[API FAIL ]API fail in file [%s|%d],function:%s,fail api:%s,last error %d.",__FILE__,__LINE__,__ZCE_FUNCTION__,y,ZCE_OS::last_error())
#endif


//TRACE调试部分的宏定义

//----------------------------------------------------------------------------------
//DEBUG 版本特有的一些宏
//ZCE_ASSERT原来的写法是 while(!(f)){ do something,后来发现WINDOWS下会对这个进行告警，被迫改了

#if defined DEBUG || defined _DEBUG

#ifndef ZCE_LOGMSG_DBG
#define ZCE_LOGMSG_DBG        ZCE_LOGMSG
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

#ifndef ZCE_STACK_BACKTRACE_DBG
#define ZCE_STACK_BACKTRACE_DBG  ZCE_STACK_BACKTRACE
#endif



//非调试版本的宏定义，
#else //DEBUG

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

#ifndef ZCE_STACK_BACKTRACE_DBG
#define ZCE_STACK_BACKTRACE_DBG(...) ((void)0)
#endif

#endif //DEBUG

//------------------------------------------------------------------------------------------------------

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
extern "C"  void __assert_fail (__const char *__assertion, __const char *__file,
                                unsigned int __line, __const char *__function)
__THROW __attribute__ ((__noreturn__));
#endif //#if defined (ZCE_OS_LINUX)

//非REALSE版本的一些宏
#if !defined NDEBUG

#ifndef ZCE_ASSERT
#if defined (ZCE_OS_WINDOWS)
#define ZCE_ASSERT(expr) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert(__FILE__,__LINE__,__ZCE_FUNCTION__,#expr),0) || \
            (1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, __ZCE_FUNCTION__, #expr)) || \
            (_CrtDbgBreak(), 0))
#else
#define ZCE_ASSERT(expr) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert(__FILE__,__LINE__,__ZCE_FUNCTION__,#expr),0) || \
            (__assert_fail (#expr, __FILE__, __LINE__, __ZCE_FUNCTION__),0))
#endif
#endif

#ifndef ZCE_ASSERT_EX
#if defined (ZCE_OS_WINDOWS)
#define ZCE_ASSERT_EX(expr,str) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert_ex(__FILE__,__LINE__,__ZCE_FUNCTION__,#expr,str),0) || \
            (1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, __ZCE_FUNCTION__, #expr)) || \
            (_CrtDbgBreak(), 0))
#else
#define ZCE_ASSERT_EX(expr,str) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert_ex(__FILE__,__LINE__,__ZCE_FUNCTION__,#expr,str),0) || \
            (__assert_fail (#expr, __FILE__, __LINE__, __ZCE_FUNCTION__),0))
#endif
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
#define ZCE_VERIFY(expr)             ((void)(expr))
#endif

#endif


//----------------------------------------------------------------------------------
//无论，DEBUG版本，REALSE版本也起作用的一些断言，这些宏在所有版本都起作用，用于一些在运行时期也要判断的东东
//ALL，任何时候，

#ifndef ZCE_ASSERT_ALL
#if defined (ZCE_OS_WINDOWS)
#define ZCE_ASSERT_ALL(expr) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert(__FILE__,__LINE__,__ZCE_FUNCTION__,#expr),0) || \
            (1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, __ZCE_FUNCTION__, #expr)) || \
            (_CrtDbgBreak(), 0))
#else
#define ZCE_ASSERT_ALL(expr) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert(__FILE__,__LINE__,__ZCE_FUNCTION__,#expr),0) || \
            (__assert_fail (#expr, __FILE__, __LINE__, __ZCE_FUNCTION__),0))
#endif
#endif


#ifndef ZCE_ASSERT_ALL_EX
#if defined (ZCE_OS_WINDOWS)
#define ZCE_ASSERT_ALL_EX(expr,str) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert_ex(__FILE__,__LINE__,__ZCE_FUNCTION__,#expr,str),0) || \
            (1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, __ZCE_FUNCTION__, #expr)) || \
            (_CrtDbgBreak(), 0))
#else
#define ZCE_ASSERT_ALL_EX(expr,str) \
    (void) ((!!(expr)) || \
            (ZCE_Trace_LogMsg::debug_assert_ex(__FILE__,__LINE__,__ZCE_FUNCTION__,#expr,str),0) || \
            (__assert_fail (#expr, __FILE__, __LINE__, __ZCE_FUNCTION__),0))
#endif
#endif


#endif //ZCE_LIB_TRACE_LOGMSG_H_

