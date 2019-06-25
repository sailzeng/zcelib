/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_trace_aid_debug.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2004年7月11日
* @brief      跟踪调试的辅助条数函数，类等。
*
*             调试文件输出，根据不同宏定义保证跨平台允许，
* @details
*
*
*
* @note
*
*/


#pragma once


//==========================================================================================================

///辅助打印一个指针内部数据的函数，用16进制的方式打印
namespace zce
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
#define ZCE_BACKTRACE_STACK(x)            zce::backtrace_stack(x,__ZCE_FUNC__)
#endif

//日志打印堆栈信息的宏，x是日志的输出级别,y是指针，z是指针长度
#ifndef ZCE_TRACE_POINTER_DATA
#define ZCE_TRACE_POINTER_DATA(x,y,z)     zce::debug_pointer((x),__ZCE_FUNC__,(y),(z))
#endif


#if defined DEBUG || defined _DEBUG

#ifndef ZCE_BACKTRACE_STACK_DBG
#define ZCE_BACKTRACE_STACK_DBG    ZCE_BACKTRACE_STACK
#endif

#else

#ifndef ZCE_BACKTRACE_STACK_DBG
#define ZCE_BACKTRACE_STACK_DBG(...) ((void)0)
#endif

#endif

//==========================================================================================================



