
#pragma once

#include "zce/logger/priority.h"

namespace zce
{
//==========================================================================================================
class Log_Printf
{
public:

    //输出va_list的参数信息
    void vwrite_logmsg(const char* str_format,
                       va_list args);
    //写日志
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char* str_format, ...);

    //打开日志输出开关
    void enable_output(bool enable_out);

    //设置日志输出Level
    zce::LOG_PRIORITY set_log_priority(zce::LOG_PRIORITY outlevel);


    //
    static Log_Printf* instance();
protected:

    //!输出日志信息的Mask值,小于这个信息的信息不予以输出
    zce::LOG_PRIORITY      permit_outlevel_ = RS_DEBUG;

    //!是否输出日志信息,可以用于暂时屏蔽
    bool                  is_output_log_ = true;
};
}

#define ZPRINT_ENABLE           zce::Log_Printf::instance()->enable_output(true)
#define ZPRINT_DISABLE          zce::Log_Printf::instance()->enable_output(false)
#define ZPRINT_SET_OUTLEVEL     zce::Log_Printf::instance()->set_log_priority
#define ZPRINT                  zce::Log_Printf::instance()->write_logmsg