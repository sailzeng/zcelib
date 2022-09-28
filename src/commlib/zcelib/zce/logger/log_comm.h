#pragma once

namespace zce
{
//!日志输出级别,日志记录时使用的是级别,
enum class LOG_PRIORITY
{
    //!跟踪信息,默认Mask输出级别,默认全部输出
    LP_TRACE = 1,
    //!调试信息
    LP_DEBUG = 2,
    //!普通信息
    LP_INFO = 3,
    //!错误信息
    LP_ERROR = 4,
    //!告警类型的错误
    LP_ALERT = 5,
    //!致命错误
    LP_FATAL = 6,
};

///默认记录的数据,在每条日志的头部
enum class LOG_HEAD
{
    ///什么都不纪录
    NONE = 0,
    ///纪录当前的时间
    CURRENTTIME = (0x1 << 0),
    ///纪录日志的级别信息
    LOGLEVEL = (0x1 << 1),
    ///纪录进程ID
    PROCESS_ID = (0x1 << 2),
    ///纪录线程ID
    THREAD_ID = (0x1 << 3),
};

///选择输出的方式
enum class LOG_OUTPUT
{
    ///不向任何地方输出
    NONE = 0,
    ///同步不向其他地方输出,默认
    LOGFILE = (0x1 << 0),
    ///同步向标准输出输出.如果你的程序是CGI程序,慎用
    STDOUT = (0x1 << 1),
    ///同步向标准错误输出.
    ERROUT = (0x1 << 2),
    ///向共享内存文件里面输出
    MMAP_FILE = (0x1 << 3),
    ///同步向WINDOWS的调试窗口输出,仅仅在WIN32环境起作用
    WINDBG = (0x1 << 4)
};

//!RS的缩写是用于纪念Richard steven
#ifndef RS_TRACE
#define RS_TRACE zce::LOG_PRIORITY::LP_TRACE
#endif

#ifndef RS_DEBUG
#define RS_DEBUG zce::LOG_PRIORITY::LP_DEBUG
#endif

#ifndef RS_INFO
#define RS_INFO zce::LOG_PRIORITY::LP_INFO
#endif

#ifndef RS_ERROR
#define RS_ERROR zce::LOG_PRIORITY::LP_ERROR
#endif

#ifndef RS_ALERT
#define RS_ALERT zce::LOG_PRIORITY::LP_ALERT
#endif

#ifndef RS_FATAL
#define RS_FATAL zce::LOG_PRIORITY::LP_FATAL
#endif
};
