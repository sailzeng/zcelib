#include "zce/predefine.h"
#include "zce/logger/log_print.h"


//==========================================================================================================

//输出va_list的参数信息
void zce::Log_Printf::vwrite_logmsg(zce::LOG_PRIORITY outlevel,
                                    const char* str_format,
                                    va_list args)
{
    //如果日志输出开关关闭
    if (is_output_log_ == false)
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
void zce::Log_Printf::write_logmsg(zce::LOG_PRIORITY outlevel,
                                   const char* str_format, ...)
{
    va_list args;
    va_start(args, str_format);
    vwrite_logmsg(outlevel, str_format, args);
    va_end(args);
}

//打开日志输出开关
void zce::Log_Printf::enable_output(bool enable_out)
{
    is_output_log_ = enable_out;
}

//设置日志输出Level
zce::LOG_PRIORITY zce::Log_Printf::set_log_priority(zce::LOG_PRIORITY outlevel)
{
    zce::LOG_PRIORITY oldlevel = permit_outlevel_;
    permit_outlevel_ = outlevel;
    return oldlevel;
}


//实例的获得
zce::Log_Printf* zce::Log_Printf::instance()
{
    static zce::Log_Printf log_instance;
    return &log_instance;
}

