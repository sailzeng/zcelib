#ifndef ZCE_LIB_TRACE_LOG_PRIORITY_H_
#define ZCE_LIB_TRACE_LOG_PRIORITY_H_

///RS的缩写是用于纪念Richard steven
///日志输出级别,日志记录时使用的是级别,
enum ZCE_LOG_PRIORITY
{
    ///跟踪信息,默认Mask输出级别,默认全部输出
    RS_TRACE   = 1,
    ///调试信息
    RS_DEBUG   = 2,
    ///普通信息
    RS_INFO    = 3,
    ///错误信息
    RS_ERROR   = 4,
    ///告警类型的错误
    RS_ALERT   = 5,
    ///致命错误
    RS_FATAL   = 6,
};


#endif //ZCE_LIB_TRACE_LOG_PRIORITY_H_

