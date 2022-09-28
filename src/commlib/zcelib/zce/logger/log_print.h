#pragma once

#include "zce/logger/log_comm.h"

namespace zce
{
//==========================================================================================================
class log_printf
{
public:
    log_printf() = default;
    ~log_printf();

    log_printf(const log_printf &) = delete;
    log_printf& operator=(const log_printf&) = delete;

    //输出va_list的参数信息
    void vwrite_logmsg(const char* str_format,
                       va_list args);
    //写日志
    void write_logmsg(zce::LOG_PRIORITY outlevel,
                      const char* str_format, ...);

    //!打开或者关闭文件输出
    void enable_fileout(bool enable_out);

    //打开日志输出开关
    void enable_output(bool enable_out);

    //设置日志输出Level
    zce::LOG_PRIORITY set_log_priority(zce::LOG_PRIORITY outlevel);

    //!
    static log_printf* instance();
    //!
    void clear_inst();

protected:
    //!实例指针
    static log_printf* instance_;

protected:
    //!打印输出的日志
    FILE * print_fp_ = nullptr;

    //!输出日志信息的Mask值,小于这个信息的信息不予以输出
    zce::LOG_PRIORITY      permit_outlevel_ = RS_DEBUG;

    //!是否输出日志信息,可以用于暂时屏蔽
    bool                  is_output_log_ = true;
};
}

#define ZPRINT_ENABLE           zce::log_printf::instance()->enable_output(true)
#define ZPRINT_DISABLE          zce::log_printf::instance()->enable_output(false)
#define ZPRINT_ENABLE_FILEOUT   zce::log_printf::instance()->enable_fileout(true)
#define ZPRINT_DISABLE_FILEOUT  zce::log_printf::instance()->enable_fileout(false)
#define ZPRINT_SET_OUTLEVEL     zce::log_printf::instance()->set_log_priority
#define ZPRINT                  zce::log_printf::instance()->write_logmsg