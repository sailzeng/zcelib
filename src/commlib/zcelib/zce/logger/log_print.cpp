#include "zce/predefine.h"
#include "zce/os_adapt/dirent.h"
#include "zce/logger/log_print.h"


//==========================================================================================================
zce::Log_Printf *zce::Log_Printf::instance_ = nullptr;


zce::Log_Printf::~Log_Printf()
{
    if (print_fp_)
    {
        ::fclose(print_fp_);
        print_fp_ = nullptr;
    }
}

//输出va_list的参数信息
void zce::Log_Printf::vwrite_logmsg(const char* str_format,
                                    va_list args)
{
    //得到打印信息,_vsnprintf为特殊函数
    vfprintf(stderr, str_format, args);
    fprintf(stderr, "\n");
    if (print_fp_)
    {
        vfprintf(print_fp_, str_format, args);
        fprintf(print_fp_, "\n");
    }
}

//写日志
void zce::Log_Printf::write_logmsg(zce::LOG_PRIORITY outlevel,
                                   const char* str_format, ...)
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
    va_list args;
    va_start(args, str_format);
    vwrite_logmsg(str_format, args);
    va_end(args);
}

void zce::Log_Printf::enable_fileout(bool enable_out)
{
    if (enable_out)
    {
        zce::mkdir("./log");
        if (!print_fp_)
        {
            print_fp_ = ::fopen("./log/log_print.log", "a+");
        }
    }
    else
    {
        if (print_fp_)
        {
            ::fclose(print_fp_);
            print_fp_ = nullptr;
        }
    }
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
    if (instance_ == nullptr)
    {
        instance_ = new zce::Log_Printf();
    }
    return instance_;
}

void zce::Log_Printf::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = nullptr;
    }
}
