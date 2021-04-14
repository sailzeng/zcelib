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

