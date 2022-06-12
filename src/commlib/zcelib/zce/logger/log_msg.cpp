#include "zce/predefine.h"
#include "zce/os_adapt/time.h"
#include "zce/logger/log_basic.h"
#include "zce/logger/log_msg.h"

//ZengXing 22503

namespace zce
{
LogMsg* LogMsg::log_instance_ = NULL;

/******************************************************************************************
class LogMsg
******************************************************************************************/
//析构函数
LogMsg::LogMsg()
{
}

//析构函数
LogMsg::~LogMsg()
{
}

//输出va_list的参数信息
void LogMsg::vwrite_logmsg(LOG_PRIORITY outlevel,
                           const char* str_format,
                           va_list args) noexcept
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

    //得到当前时间
    timeval now_time_val(gettimeofday());

    //我要保留一个位置放'\0'，以及一个\n
    //用static 变量，保证只初始化一次， 用thread_local 保证每个线程一个
    static thread_local char* log_buffer = new char[SIZE_OF_LOG_BUFFER + 2];
    log_buffer[SIZE_OF_LOG_BUFFER + 1] = '\0';

    //还是为\n考虑留一个空间
    size_t sz_buf_len = SIZE_OF_LOG_BUFFER;
    size_t sz_use_len = 0;

    //输出头部信息
    stringbuf_loghead(outlevel,
                      now_time_val,
                      log_buffer,
                      sz_buf_len,
                      sz_use_len);
    sz_buf_len -= sz_use_len;

    //得到打印信息,_vsnprintf为特殊函数
    int len_of_out = vsnprintf(log_buffer + sz_use_len, sz_buf_len, str_format, args);

    //如果输出的字符串比想想的长
    if (len_of_out >= static_cast<int>(sz_buf_len) || len_of_out < 0)
    {
        sz_use_len = SIZE_OF_LOG_BUFFER;
        sz_buf_len = 0;
    }
    else
    {
        sz_use_len += len_of_out;
        sz_buf_len -= len_of_out;
    }

    //如果要自动增加换行符号，
    if (auto_new_line_)
    {
        log_buffer[sz_use_len] = '\n';
        ++sz_use_len;

        //注意sz_buf_len在这儿没有调整，因为'\n'的位置我前面为了安全扣除了
        //也不能直接用--sz_buf_len;因为sz_buf_len可能==0
    }

    output_log_info(now_time_val,
                    log_buffer,
                    sz_use_len);
}

//ZASSERT的扩展定义，
void LogMsg::debug_assert(const char* file_name,
                          const int file_line,
                          const char* function_name,
                          const char* expression_name) noexcept
{
    write_logmsg(RS_FATAL, "Assertion failed: FILENAME:[%s],LINENO:[%d],FUN:[%s],EXPRESSION:[%s].",
                 file_name,
                 file_line,
                 function_name,
                 expression_name);
}

//Aseert调试,增强版本函数
void LogMsg::debug_assert_ex(const char* file_name,
                             const int file_line,
                             const char* function_name,
                             const char* expression_name,
                             const char* out_string) noexcept
{
    write_logmsg(RS_FATAL,
                 "Assertion failed: FILENAME:[%s],LINENO:[%d],FUN:[%s],"
                 "EXPRESSION:[%s] OutString[%s].",
                 file_name,
                 file_line,
                 function_name,
                 expression_name,
                 out_string);
}

//调用vwrite_logmsg完成实际输出
void LogMsg::write_logmsg(LOG_PRIORITY dbglevel,
                          const char* str_format, ...) noexcept
{
    va_list args;

    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(dbglevel, str_format, args);
    }

    va_end(args);
}

//得到唯一的单子实例
LogMsg* LogMsg::instance()
{
    if (log_instance_ == NULL)
    {
        log_instance_ = new LogMsg();
    }

    return log_instance_;
}

//赋值唯一的单子实例
void LogMsg::instance(LogMsg* instatnce)
{
    clean_instance();
    log_instance_ = instatnce;
    return;
}

//清除单子实例
void LogMsg::clean_instance()
{
    if (log_instance_)
    {
        delete log_instance_;
    }

    log_instance_ = NULL;
    return;
}
}