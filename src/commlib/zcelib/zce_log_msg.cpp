#include "zce_predefine.h"
#include "zce_os_adapt_time.h"
#include "zce_log_basic.h"
#include "zce_log_msg.h"

//ZengXing 22503

//
ZCE_Trace_LogMsg *ZCE_Trace_LogMsg::log_instance_ = NULL;

/******************************************************************************************
class ZCE_Trace_LogMsg
******************************************************************************************/
//析构函数
ZCE_Trace_LogMsg::ZCE_Trace_LogMsg():
    multiline_buf_(NULL)
{
}

//析构函数
ZCE_Trace_LogMsg::~ZCE_Trace_LogMsg()
{
    if (multiline_buf_)
    {
        delete [] multiline_buf_;
    }
}

//输出va_list的参数信息
void ZCE_Trace_LogMsg::vwrite_logmsg(zce::LOG_PRIORITY outlevel,
                                     const char *str_format,
                                     va_list args)
{
    //如果日志输出开关关闭
    if (if_output_log_ == false)
    {
        return ;
    }

    //如果输出的日志级别低于Mask值
    if (permit_outlevel_ > outlevel )
    {
        return;
    }

    //得到当前时间
    timeval now_time_val (zce::gettimeofday());

    //我要保留一个位置放'\0'
    char log_tmp_buffer[LOG_TMP_BUFFER_SIZE + 1];
    log_tmp_buffer[LOG_TMP_BUFFER_SIZE] = '\0';

    //还是为\n考虑留一个空间
    size_t sz_buf_len = LOG_TMP_BUFFER_SIZE ;
    size_t sz_use_len = 0;

    //输出头部信息
    stringbuf_loghead(outlevel,
                      now_time_val,
                      log_tmp_buffer,
                      sz_buf_len,
                      sz_use_len);
    sz_buf_len -= sz_use_len;

    //得到打印信息,_vsnprintf为特殊函数
    int len_of_out = vsnprintf(log_tmp_buffer + sz_use_len, sz_buf_len, str_format, args);

    //如果输出的字符串比想想的长
    if (len_of_out >= static_cast<int>( sz_buf_len) || len_of_out < 0)
    {
        sz_use_len = LOG_TMP_BUFFER_SIZE;
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
        log_tmp_buffer[sz_use_len] = '\n';
        ++sz_use_len;

        //注意sz_buf_len在这儿没有调整，因为'\n'的位置我前面为了安全扣除了
        //也不能直接用--sz_buf_len;因为sz_buf_len可能==0
    }

    output_log_info(now_time_val,
                    log_tmp_buffer,
                    sz_use_len);

}

//写日志
void ZCE_Trace_LogMsg::write_logmsg(zce::LOG_PRIORITY outlevel, const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);
    vwrite_logmsg(outlevel, str_format, args);
    va_end(args);

}

//ZASSERT的扩展定义，
void ZCE_Trace_LogMsg::debug_assert(const char *file_name,
                                    const int file_line,
                                    const char *function_name,
                                    const char *expression_name)
{
    debug_output(RS_FATAL, "Assertion failed: FILENAME:[%s],LINENO:[%d],FUN:[%s],EXPRESSION:[%s].",
                 file_name,
                 file_line,
                 function_name,
                 expression_name);
}

//Aseert调试,增强版本函数
void ZCE_Trace_LogMsg::debug_assert_ex(const char *file_name,
                                       const int file_line,
                                       const char *function_name,
                                       const char *expression_name,
                                       const char *out_string)
{
    debug_output(RS_FATAL, "Assertion failed: FILENAME:[%s],LINENO:[%d],FUN:[%s],EXPRESSION:[%s] OutString[%s].",
                 file_name,
                 file_line,
                 function_name,
                 expression_name,
                 out_string);
}

//调用vwrite_logmsg完成实际输出
void ZCE_Trace_LogMsg::debug_output(zce::LOG_PRIORITY dbglevel,
                                    const char *str_format, ... )
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
ZCE_Trace_LogMsg *ZCE_Trace_LogMsg::instance()
{
    if (log_instance_ == NULL)
    {
        log_instance_ = new ZCE_Trace_LogMsg();
    }

    return log_instance_;
}

//赋值唯一的单子实例
void ZCE_Trace_LogMsg::instance(ZCE_Trace_LogMsg *instatnce)
{
    clean_instance();
    log_instance_ = instatnce;
    return;
}

//清除单子实例
void ZCE_Trace_LogMsg::clean_instance()
{
    if (log_instance_)
    {
        delete log_instance_;
    }

    log_instance_ = NULL;
    return;
}

//为什么要这样麻烦,问题在于不能用宏(__VA_ARGS__)解决问题(部分编译器不支持,VS2003),
#if _MSC_VER <= 1300

//用不用的日志级别输出
void ZCE_Trace_LogMsg::debug_traceex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_TRACE, str_format, args);
    }

    va_end(args);
}

void ZCE_Trace_LogMsg::debug_debugex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_DEBUG, str_format, args);
    }

    va_end(args);
}

void ZCE_Trace_LogMsg::debug_infoex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_INFO, str_format, args);
    }

    va_end(args);
}

void ZCE_Trace_LogMsg::debug_errorex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_ERROR, str_format, args);
    }

    va_end(args);
}

void ZCE_Trace_LogMsg::debug_alertex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_ALERT, str_format, args);
    }

    va_end(args);
}

void ZCE_Trace_LogMsg::debug_fatalex(const char *str_format, ... )
{
    va_list args;
    va_start(args, str_format);

    if (log_instance_)
    {
        log_instance_->vwrite_logmsg(RS_FATAL, str_format, args);
    }

    va_end(args);
}

#endif //#if _MSC_VER <= 1300

