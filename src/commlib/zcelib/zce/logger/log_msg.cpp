#include "zce/predefine.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/thread.h"
#include "zce/logger/log_file.h"
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

//初始化函数,用于时间分割日志的构造
int LogMsg::init_time_log(LOGFILE_DEVIDE div_log_file,
                          const char* log_file_prefix,
                          size_t reserve_file_num,
                          bool trunc_log,
                          bool multithread_out,
                          bool auto_new_line,
                          bool thread_output,
                          int output_way,
                          int head_record) noexcept
{
    output_way_ = output_way;
    head_record_ = head_record;
    auto_new_line_ = auto_new_line;
    multithread_out_ = multithread_out;
    permit_outlevel_ = RS_DEBUG;
    assert(LOGFILE_DEVIDE::BY_TIME_HOUR <= div_log_file &&
           LOGFILE_DEVIDE::BY_TIME_YEAR >= div_log_file);
    return log_file_.initialize(output_way,
                                div_log_file,
                                log_file_prefix,
                                trunc_log,
                                thread_output,
                                0,
                                reserve_file_num);
}

//初始化函数,用于尺寸分割日志的构造 ZCE_LOGFILE_DEVIDE_NAME = LOGDEVIDE_BY_SIZE
int LogMsg::init_size_log(const char* log_file_prefix,
                          size_t max_size_log_file,
                          unsigned int reserve_file_num,
                          bool trunc_log,
                          bool multithread_out,
                          bool auto_new_line,
                          bool thread_output,
                          int output_way,
                          int head_record) noexcept
{
    LOGFILE_DEVIDE div_log_file = LOGFILE_DEVIDE::BY_SIZE_NAME_ID;
    output_way_ = output_way;
    head_record_ = head_record;
    auto_new_line_ = auto_new_line;
    multithread_out_ = multithread_out;
    permit_outlevel_ = RS_DEBUG;
    //如果不标识文件分割大小
    if (0 == max_size_log_file)
    {
        div_log_file = LOGFILE_DEVIDE::NONE;
    }

    return log_file_.initialize(output_way,
                                div_log_file,
                                log_file_prefix,
                                trunc_log,
                                thread_output,
                                max_size_log_file,
                                reserve_file_num);
}

//初始化函数，用于标准输出
int LogMsg::init_stdout(bool use_err_out,
                        bool multithread_out,
                        bool auto_new_line,
                        int head_record) noexcept
{
    unsigned int output_way = 0;
    if (use_err_out)
    {
        output_way |= static_cast<int>(LOG_OUTPUT::ERROUT);
    }
    else
    {
        output_way |= static_cast<int>(LOG_OUTPUT::STDOUT);
    }
    output_way_ = output_way;
    head_record_ = head_record;
    auto_new_line_ = auto_new_line;
    multithread_out_ = multithread_out;
    return log_file_.initialize(output_way,
                                LOGFILE_DEVIDE::NONE,
                                "",
                                false,
                                false,
                                0,
                                0);
}

//打开日志输出开关
void LogMsg::enable_output(bool enable_out)
{
    is_output_log_ = enable_out;
}

//!设置日志输出Level
LOG_PRIORITY LogMsg::set_log_priority(LOG_PRIORITY outlevel)
{
    LOG_PRIORITY oldlevel = permit_outlevel_;
    permit_outlevel_ = outlevel;
    return oldlevel;
}
//!取得输出Level
LOG_PRIORITY LogMsg::get_log_priority(void)
{
    return permit_outlevel_;
}

//!设置默认输出的信息类型
unsigned int LogMsg::set_log_head(int recdinfo)
{
    unsigned int tmprecdinfo = recdinfo;
    head_record_ = recdinfo;
    return tmprecdinfo;
}
//!取得默认输出的信息类型
unsigned int LogMsg::get_log_head(void)
{
    return head_record_;
}

//!设置同步输出的标示
//!如果开始没有设置文件同步输出,后面不调整.
unsigned int LogMsg::set_output_way(int output_way)
{
    //
    int tmp = output_way_;
    output_way_ = output_way;
    return tmp;
}

//取得同步输出的标示
unsigned int LogMsg::get_output_way(void)
{
    return output_way_;
}

//设置是否线程同步
bool LogMsg::set_thread_synchro(bool multithread_out)
{
    bool old_multi = multithread_out_;
    multithread_out_ = multithread_out;
    return old_multi;
}
//取得是否进行线程同步
bool LogMsg::get_thread_synchro(void)
{
    return multithread_out_;
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

//将日志的头部信息输出到一个Stringbuf中
void LogMsg::stringbuf_loghead(LOG_PRIORITY outlevel,
                               const timeval& now_time,
                               char* log_tmp_buffer,
                               size_t sz_buf_len,
                               size_t& sz_use_len) noexcept
{
    sz_use_len = 0;

    //如果纪录时间
    if (ZCE_U32_BIT_IS_SET(head_record_, LOG_HEAD::CURRENTTIME))
    {
        //转换为语句
        timestamp(&now_time, log_tmp_buffer + sz_use_len, sz_buf_len);

        //别计算了，快点
        sz_use_len = TIMESTR_ISO_USEC_LEN;

        sz_buf_len -= sz_use_len;
    }

    //如果记录日志级别
    if (ZCE_U32_BIT_IS_SET(head_record_, LOG_HEAD::LOGLEVEL))
    {
        switch (outlevel)
        {
        case RS_TRACE:
            sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "%s", "[TRACE]");
            sz_buf_len -= sz_use_len;
            break;

        case RS_DEBUG:
            sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "%s", "[DEBUG]");
            sz_buf_len -= sz_use_len;
            break;

        case RS_INFO:
            sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "%s", "[INFO]");
            sz_buf_len -= sz_use_len;
            break;

        case RS_ERROR:
            sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "%s", "[ERROR]");
            sz_buf_len -= sz_use_len;
            break;

        case RS_FATAL:
            sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "%s", "[FATAL]");
            sz_buf_len -= sz_use_len;
            break;

        default:
            break;
        }
    }

    //如果纪录当前的PID
    if (ZCE_U32_BIT_IS_SET(head_record_, LOG_HEAD::PROCESS_ID))
    {
        sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "[PID:%u]",
                               static_cast<unsigned int>(getpid()));
        sz_buf_len -= sz_use_len;
    }

    if (ZCE_U32_BIT_IS_SET(head_record_, LOG_HEAD::THREAD_ID))
    {
        sz_use_len += snprintf(log_tmp_buffer + sz_use_len,
                               sz_buf_len,
                               "[TID:%u]",
                               static_cast<unsigned int>(zce::pthread_self()));
        sz_buf_len -= sz_use_len;
    }
}

void LogMsg::output_log_info(const timeval& now_time,
                             char* log_tmp_buffer,
                             size_t sz_use_len) noexcept
{
    //如果要线程同步，在这个地方加锁，由于使用了条件判断是否加锁，而不是模版，所以这个地方没有用GRUAD，
    if (multithread_out_)
    {
        protect_lock_.lock();
    }

    //记录到文件中
    if (output_way_ & static_cast<int>(LOG_OUTPUT::LOGFILE))
    {
        log_file_.fileout_log_info(now_time,
                                   log_tmp_buffer,
                                   sz_use_len);
    }

    //如果有同步要求输出的地方
    if (output_way_ & static_cast<int>(LOG_OUTPUT::STDOUT))
    {
        //cout是行缓冲
        std::cout.write(log_tmp_buffer,
                        static_cast<std::streamsize>(sz_use_len));
    }

    if (output_way_ & static_cast<int>(LOG_OUTPUT::ERROUT))
    {
        //cerr没有缓冲，云飞说的
        std::cerr.write(log_tmp_buffer,
                        static_cast<std::streamsize>(sz_use_len));
    }

    //WIN32 下的调试输出,向调试窗口输出
#ifdef ZCE_OS_WINDOWS
    if (output_way_ & static_cast<int>(LOG_OUTPUT::WINDBG))
    {
        ::OutputDebugStringA(log_tmp_buffer);
    }
#endif

    //如果有线程同步，在这个地方解锁
    if (multithread_out_)
    {
        protect_lock_.unlock();
    }
}

//通过字符串得到对应的日志策略,
LOG_PRIORITY LogMsg::log_priorities(const char* str_priority)
{
    if (strcasecmp(str_priority, ("TRACE")) == 0)
    {
        return RS_TRACE;
    }
    else if (strcasecmp(str_priority, ("DEBUG")) == 0)
    {
        return RS_DEBUG;
    }
    else if (strcasecmp(str_priority, ("INFO")) == 0)
    {
        return RS_INFO;
    }
    else if (strcasecmp(str_priority, ("ERROR")) == 0)
    {
        return RS_ERROR;
    }
    else if (strcasecmp(str_priority, ("ALERT")) == 0)
    {
        return RS_ALERT;
    }
    else if (strcasecmp(str_priority, ("FATAL")) == 0)
    {
        return RS_FATAL;
    }
    else
    {
        return RS_DEBUG;
    }
}

//通过字符串得到对应的日志策略,
LOGFILE_DEVIDE LogMsg::log_file_devide(const char* str_devide)
{
    if (strcasecmp(str_devide, ("SIZE_ID")) == 0)
    {
        return LOGFILE_DEVIDE::BY_SIZE_NAME_ID;
    }
    else if (strcasecmp(str_devide, ("HOUR")) == 0)
    {
        return LOGFILE_DEVIDE::BY_TIME_HOUR;
    }
    else if (strcasecmp(str_devide, ("SIXHOUR")) == 0)
    {
        return LOGFILE_DEVIDE::BY_TIME_SIX_HOUR;
    }
    else if (strcasecmp(str_devide, ("DAY")) == 0)
    {
        return LOGFILE_DEVIDE::BY_TIME_DAY;
    }
    else if (strcasecmp(str_devide, ("MONTH")) == 0)
    {
        return LOGFILE_DEVIDE::BY_TIME_MONTH;
    }
    else if (strcasecmp(str_devide, ("YEAR")) == 0)
    {
        return LOGFILE_DEVIDE::BY_TIME_YEAR;
    }
    else if (strcasecmp(str_devide, ("SIZE_MILLSENCOND")) == 0)
    {
        return LOGFILE_DEVIDE::BY_TIME_NAME_MILLISECOND;
    }
    else
    {
        return LOGFILE_DEVIDE::BY_TIME_DAY;
    }
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