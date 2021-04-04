

#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_dirent.h"
#include "zce_os_adapt_thread.h"
#include "zce_os_adapt_process.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_string.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_file.h"
#include "zce_log_basic.h"

const char ZCE_LogTrace_Basic::STR_LOG_POSTFIX[LEN_LOG_POSTFIX + 1] = ".log";

//构造函数
ZCE_LogTrace_Basic::ZCE_LogTrace_Basic():
    div_log_file_(LOGFILE_DEVIDE::NONE),
    output_way_(static_cast<int>(LOG_OUTPUT::FILE) | static_cast<int>(LOG_OUTPUT::ERROUT)),
    if_thread_synchro_(false),
    auto_new_line_(true),
    max_size_log_file_(DEFAULT_LOG_SIZE),
    reserve_file_num_(DEFAULT_RESERVE_FILENUM),
    record_info_(static_cast<int>(LOG_HEAD::CURRENTTIME) | static_cast<int>(LOG_HEAD::LOGLEVEL)),
    current_click_(1),
    permit_outlevel_(RS_TRACE),
    size_log_file_(0),
    if_output_log_(true)
{
    //预先分配空间
    log_file_name_.reserve(PATH_MAX + 32);
    log_file_dir_.reserve(PATH_MAX + 32);
    log_file_prefix_.reserve(PATH_MAX + 32);
}


//
ZCE_LogTrace_Basic::~ZCE_LogTrace_Basic()
{
    //注销
    finalize();
}


//初始化函数,用于时间分割日志的构造
int ZCE_LogTrace_Basic::init_time_log(LOGFILE_DEVIDE div_log_file,
                                      const char *log_file_prefix,
                                      bool if_thread_synchro,
                                      bool auto_new_line,
                                      size_t reserve_file_num,
                                      unsigned int output_way,
                                      unsigned int head_record)
{
    assert (LOGFILE_DEVIDE::BY_TIME_HOUR <= div_log_file  && LOGFILE_DEVIDE::BY_TIME_YEAR >= div_log_file  );
    return initialize(output_way,
                      div_log_file,
                      log_file_prefix,
                      if_thread_synchro,
                      auto_new_line,
                      0,
                      reserve_file_num,
                      head_record);
}

//初始化函数,用于尺寸分割日志的构造 ZCE_LOGFILE_DEVIDE_NAME = LOGDEVIDE_BY_SIZE
int ZCE_LogTrace_Basic::init_size_log(
    const char *log_file_prefix,
    bool if_thread_synchro,
    bool auto_new_line,
    size_t max_size_log_file,
    unsigned int reserve_file_num,
    unsigned int output_way,
    unsigned int head_record)
{

    LOGFILE_DEVIDE div_log_file = LOGFILE_DEVIDE::BY_SIZE_NAME_ID;

    //如果不标识文件分割大小
    if ( 0 == max_size_log_file )
    {
        div_log_file = LOGFILE_DEVIDE::NONE;
    }

    return initialize(output_way,
                      div_log_file,
                      log_file_prefix,
                      if_thread_synchro,
                      auto_new_line,
                      max_size_log_file,
                      reserve_file_num,
                      head_record);
}

//初始化函数，用于标准输出
int ZCE_LogTrace_Basic::init_stdout(bool if_thread_synchro,
                                    bool use_err_out,
                                    bool auto_new_line,
                                    unsigned int head_record)
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

    return initialize(output_way,
                      LOGFILE_DEVIDE::NONE,
                      "",
                      if_thread_synchro,
                      auto_new_line,
                      0,
                      0,
                      head_record);
}

//初始化函数,参数最齐全的一个
int ZCE_LogTrace_Basic::initialize(unsigned int output_way,
                                   LOGFILE_DEVIDE div_log_file,
                                   const char *log_file_prefix,
                                   bool if_thread_synchro,
                                   bool auto_new_line,
                                   size_t max_size_log_file,
                                   size_t reserve_file_num,
                                   unsigned int head_record)
{
    output_way_ = output_way;
    div_log_file_ = div_log_file;

    if_thread_synchro_  = if_thread_synchro;
    auto_new_line_ = auto_new_line;

    max_size_log_file_ = max_size_log_file;
    reserve_file_num_ = reserve_file_num;
    record_info_ = head_record;
    current_click_ = 1;
    permit_outlevel_ = RS_TRACE;
    size_log_file_ = 0;

    if_output_log_ = true;

    //断言检查输入参数
    if ( log_file_prefix != NULL)
    {
        log_file_prefix_ = log_file_prefix;
    }
    //如果文件参数不齐全
    else
    {
        assert(0 == ( output_way_ & static_cast<int>(LOG_OUTPUT::FILE)) );
    }

    //
    make_configure();
    //如果需要日志文件输出，输出一个文件
    if (output_way_ & static_cast<int>(LOG_OUTPUT::FILE) )
    {
        timeval now_time(zce::gettimeofday());
        open_new_logfile(true, now_time);
    }

    return 0;
}

//关闭日志，注意关闭后，必须重新初始化
void ZCE_LogTrace_Basic::finalize()
{
    if (log_file_handle_.is_open())
    {
        log_file_handle_.close();
    }

    current_click_ = 1;
    permit_outlevel_ = RS_TRACE;

    div_log_file_ = LOGFILE_DEVIDE::NONE;
    output_way_ = static_cast<int>(LOG_OUTPUT::FILE) | static_cast<int>(LOG_OUTPUT::ERROUT);

    size_log_file_ = 0;
    if_output_log_ = true;
}

//配置日志文件
void ZCE_LogTrace_Basic::make_configure(void)
{
    //检查max_size_log_file_等参数的大小范围
    if (max_size_log_file_ < MIN_LOG_SIZE)
    {
        max_size_log_file_ = MIN_LOG_SIZE;
    }

    if (max_size_log_file_ > MAX_LOG_SIZE)
    {
        max_size_log_file_ = MAX_LOG_SIZE;
    }
    //
    if (reserve_file_num_ < MIN_RESERVE_FILENUM)
    {
        reserve_file_num_ = MIN_RESERVE_FILENUM;
    }
    if (reserve_file_num_ > MAX_RESERVE_FILENUM)
    {
        reserve_file_num_ = MAX_RESERVE_FILENUM;
    }

    //得到目录的名称
    char dir_name[PATH_MAX + 16];
    dir_name[PATH_MAX] = '\0';

    zce::dirname(log_file_prefix_.c_str(), dir_name, PATH_MAX + 1);
    log_file_dir_ = dir_name;

    // 如果目录不存在，则创建
    if (zce::mkdir_recurse(log_file_dir_.c_str()) != 0)
    {
        // 创建失败，
        fprintf(stderr, "mkdir %s fail. err=%d|%s\n",
                log_file_dir_.c_str(),
                errno,
                strerror(errno));
    }
}

//打开日志输出开关
void ZCE_LogTrace_Basic::enable_output(bool enable_out)
{
    if_output_log_ = enable_out;
}


//设置日志输出Level
zce::LOG_PRIORITY ZCE_LogTrace_Basic::set_log_priority( zce::LOG_PRIORITY outlevel )
{
    zce::LOG_PRIORITY oldlevel = permit_outlevel_;
    permit_outlevel_ = outlevel;
    return oldlevel;
}
//取得输出Level
zce::LOG_PRIORITY ZCE_LogTrace_Basic::get_log_priority(void )
{
    return permit_outlevel_;
}

//设置默认输出的信息类型
unsigned int ZCE_LogTrace_Basic::set_log_head(unsigned int recdinfo)
{
    unsigned int tmprecdinfo = recdinfo;
    record_info_ = recdinfo;
    return tmprecdinfo;
}
//取得默认输出的信息类型
unsigned int ZCE_LogTrace_Basic::get_log_head(void)
{
    return record_info_;
}

//设置同步输出的标示
//如果开始没有设置文件同步输出,后面不调整.
unsigned int ZCE_LogTrace_Basic::set_output_way(unsigned int output_way)
{
    //
    unsigned int tmpsynchr = output_way_;
    output_way_ = output_way;

    return tmpsynchr;
}

//取得同步输出的标示
unsigned int ZCE_LogTrace_Basic::get_output_way(void)
{
    return output_way_;
}

//设置是否线程同步
bool ZCE_LogTrace_Basic::set_thread_synchro(bool if_thread_synchro)
{
    bool old_synchro = if_thread_synchro_;
    if_thread_synchro_ = if_thread_synchro;
    return old_synchro;

}
//取得是否进行线程同步
bool ZCE_LogTrace_Basic::get_thread_synchro(void)
{
    return if_thread_synchro_;
}

//得到新的日志文件文件名称
void ZCE_LogTrace_Basic::open_new_logfile(bool initiate, const timeval &current_time)
{
    //是否要生成新的文件名称
    bool to_new_file = false;
    if (initiate)
    {
        to_new_file = true;

        //把时间日志的旧日志都扫描出来，便于删除处理
        if (LOGFILE_DEVIDE::BY_TIME_HOUR == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_SIX_HOUR == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_DAY == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_MONTH == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_YEAR == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_NAME_MILLISECOND == div_log_file_)
        {
            int ret = 0;
            std::vector<std::string> file_name_ary;
            ret = zce::readdir_nameary(log_file_dir_.c_str(),
                                       log_file_prefix_.c_str(),
                                       STR_LOG_POSTFIX,
                                       false,
                                       true,
                                       file_name_ary);
            if (ret != 0)
            {
                fprintf(stderr, "readdir %s | %s fail. err=%d|%s\n",
                        log_file_dir_.c_str(),
                        log_file_prefix_.c_str(),
                        errno,
                        strerror(errno));
            }

            std::sort(file_name_ary.begin(), file_name_ary.end());

            for (auto file_name : file_name_ary)
            {
                std::string log_file = log_file_dir_ + ZCE_DIRECTORY_SEPARATOR_STR + file_name;
                time_logfile_list_.push_back(file_name);
            }
        }
    }

    time_t cur_click = 0;

    if (LOGFILE_DEVIDE::BY_TIME_HOUR == div_log_file_ ||
        LOGFILE_DEVIDE::BY_TIME_SIX_HOUR == div_log_file_ ||
        LOGFILE_DEVIDE::BY_TIME_DAY == div_log_file_ ||
        LOGFILE_DEVIDE::BY_TIME_MONTH == div_log_file_ ||
        LOGFILE_DEVIDE::BY_TIME_YEAR == div_log_file_)
    {
        cur_click = current_time.tv_sec / zce::ONE_HOUR_SECONDS;

        //降低比较频率
        if (current_click_ != cur_click)
        {
            current_click_ = cur_click;
            std::string new_file_name;
            new_file_name.reserve(PATH_MAX + 32);
            create_time_logname(current_time, new_file_name);

            //如果日志文件名称已经更新,表示要产生一个新文件,
            //String的比较是比较耗时的,但前面的限定保证1天最多比较24次,小case
            if (log_file_name_ != new_file_name)
            {
                to_new_file = true;
                log_file_name_ = new_file_name;
            }
        }
    }
    else if (LOGFILE_DEVIDE::BY_SIZE_NAME_ID == div_log_file_)
    {
        //如果日志文件的尺寸已经超出
        if (size_log_file_ > max_size_log_file_)
        {
            to_new_file = true;
        }
    }
    else if (LOGFILE_DEVIDE::NONE == div_log_file_)
    {
        log_file_name_ = log_file_prefix_ + STR_LOG_POSTFIX;
    }
    else if (LOGFILE_DEVIDE::BY_TIME_NAME_MILLISECOND == div_log_file_)
    {
        std::string new_file_name;
        new_file_name.reserve(PATH_MAX + 32);
        //如果日志文件的尺寸已经超出
        if (size_log_file_ > max_size_log_file_)
        {
            to_new_file = true;
        }
        create_time_logname(current_time, new_file_name);
        log_file_name_ = new_file_name;
    }
    else
    {
    }

    //如果文件是打开状态,但文件句柄不正常，重新打开一个
    if (log_file_handle_.is_open())
    {
        if (log_file_handle_.good() == false)
        {
            to_new_file = true;
        }
    }

    //如果生成了一个新的文件名称
    if (to_new_file == true )
    {
        //关闭原有的文件.
        if (log_file_handle_.is_open())
        {
            log_file_handle_.close();
        }

        del_old_logfile();

        log_file_handle_.clear();
        //打开之,
        log_file_handle_.open(log_file_name_.c_str(), std::ios::out | std::ios::app);

        size_log_file_ = static_cast<size_t>(log_file_handle_.tellp());

        if (LOGFILE_DEVIDE::BY_TIME_HOUR == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_SIX_HOUR == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_DAY == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_MONTH == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_YEAR == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_NAME_MILLISECOND == div_log_file_)
        {
            time_logfile_list_.push_back(log_file_name_);
        }

    }
}

void ZCE_LogTrace_Basic::del_old_logfile()
{
    //如果保留所有日志，或者分割日志的时间为 月 或者 年
    if (reserve_file_num_ > 0)
    {
        //如果是按照时间进行分割文件的
        if (LOGFILE_DEVIDE::BY_TIME_HOUR == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_SIX_HOUR == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_DAY == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_MONTH == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_YEAR == div_log_file_ ||
            LOGFILE_DEVIDE::BY_TIME_NAME_MILLISECOND == div_log_file_ )
        {
            //如果确定了只保留一定数量的日志文件,而且文件过多,删除掉多余的文件
            if (time_logfile_list_.size() > reserve_file_num_)
            {
                auto dlenum = time_logfile_list_.size() - reserve_file_num_;
                for (size_t i = 0; i < dlenum; ++i)
                {
                    auto del_log = *(time_logfile_list_.begin());
                    time_logfile_list_.pop_front();
                    ::remove(del_log.c_str());
                }
            }
        }

        //如果是安装尺寸风格文件的
        else if (LOGFILE_DEVIDE::BY_SIZE_NAME_ID == div_log_file_)
        {
            std::string dellogfname;
            dellogfname.reserve(PATH_MAX + 16);
            create_id_logname(reserve_file_num_, dellogfname);
            ::remove(dellogfname.c_str());

            std::string oldlogfilename, renamefilename;
            oldlogfilename.reserve(MAX_PATH + 16);
            renamefilename.reserve(MAX_PATH + 16);

            // 重命名需要从后面开始往前重命名
            for (size_t i = reserve_file_num_ ; i > 0; --i)
            {
                create_id_logname(i - 1, oldlogfilename);
                create_id_logname(i, renamefilename);
                int ret = ::rename(oldlogfilename.c_str(), renamefilename.c_str());
                if (ret != 0)
                {
                    fprintf(stderr, "Log file rename fail,errno = %d. old file[%s] new file [%s] ",
                            errno,
                            oldlogfilename.c_str(),
                            renamefilename.c_str());
                }
            }
        }

        zce::clear_last_error();
    }
}


//根据日期得到文件名称
void ZCE_LogTrace_Basic::create_time_logname(const timeval &cur_time,
                                             std::string &logfilename)
{
    time_t cur_t = cur_time.tv_sec;
    tm curtm = *localtime(&(cur_t));
    char tmpbuf[64] = {0};
    size_t buflen = sizeof(tmpbuf) - 1;

    switch (div_log_file_)
    {
        //以小时为单位得到文件名称
        case LOGFILE_DEVIDE::BY_TIME_HOUR:
        case LOGFILE_DEVIDE::BY_TIME_SIX_HOUR:
        {
            strftime( tmpbuf, buflen, "_%Y%m%d_%H", &curtm);
#if defined ZCE_LOG_TEST && ZCE_LOG_TEST== 1
            strftime(tmpbuf, buflen, "_%Y%m%d_%H%M", &curtm);
#endif
            ::strcat(tmpbuf, STR_LOG_POSTFIX);

            break;
        }

        case LOGFILE_DEVIDE::BY_TIME_DAY:
            ::strftime( tmpbuf, buflen, "_%Y%m%d", &curtm);
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;

        //
        case LOGFILE_DEVIDE::BY_TIME_MONTH:
            ::strftime( tmpbuf, buflen, "_%Y%m", &curtm);
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;

        case LOGFILE_DEVIDE::BY_TIME_YEAR:
            ::strftime( tmpbuf, buflen, "_%Y", &curtm);
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;

        case LOGFILE_DEVIDE::BY_TIME_NAME_MILLISECOND:
            char mill_sec_str[16];
            ::strftime(tmpbuf, buflen, "_%Y%m%d_%H%M%s_", &curtm);
            snprintf(mill_sec_str, 15, "%03d", static_cast<int>(cur_time.tv_usec / 1000));
            ::strcat(tmpbuf, mill_sec_str);
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;
        //Never goto here.
        default:
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;
    }

    logfilename = log_file_prefix_;
    logfilename += tmpbuf;
}



//根据ID得到文件名称f
void ZCE_LogTrace_Basic::create_id_logname(size_t logfileid, std::string &log_filename)
{
    char tmpbuf[32];

    if (logfileid == 0)
    {
        snprintf(tmpbuf, sizeof(tmpbuf) - 1, "%s",
                 STR_LOG_POSTFIX);
    }
    else
    {
        snprintf(tmpbuf, sizeof(tmpbuf) - 1, "%s.%05u",
                 STR_LOG_POSTFIX,
                 (unsigned int)logfileid);
    }

    log_filename = log_file_prefix_;
    log_filename += tmpbuf;
}

//将日志的头部信息输出到一个Stringbuf中
void ZCE_LogTrace_Basic::stringbuf_loghead(zce::LOG_PRIORITY outlevel,
                                           const timeval &now_time,
                                           char *log_tmp_buffer,
                                           size_t sz_buf_len,
                                           size_t &sz_use_len)
{

    sz_use_len = 0;

    //如果纪录时间
    if (ZCE_U32_BIT_IS_SET(record_info_ ,LOG_HEAD::CURRENTTIME))
    {
        //转换为语句
        zce::timestamp(&now_time, log_tmp_buffer + sz_use_len, sz_buf_len);

        //别计算了，快点
        sz_use_len = zce::TIMESTR_ISO_USEC_LEN;

        sz_buf_len -= sz_use_len;
    }

    //如果记录日志级别
    if (ZCE_U32_BIT_IS_SET(record_info_ ,LOG_HEAD::LOGLEVEL))
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
    if (ZCE_U32_BIT_IS_SET(record_info_ ,LOG_HEAD::PROCESS_ID))
    {
        sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "[PID:%u]", static_cast<unsigned int>(zce::getpid()));
        sz_buf_len -= sz_use_len;
    }

    if (ZCE_U32_BIT_IS_SET(record_info_ ,LOG_HEAD::THREAD_ID))
    {
        sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "[TID:%u]", static_cast<unsigned int>(zce::pthread_self()));
        sz_buf_len -= sz_use_len;
    }
}

void ZCE_LogTrace_Basic::output_log_info(const timeval &now_time,
                                         char *log_tmp_buffer,
                                         size_t sz_use_len)
{
    //如果要线程同步，在这个地方加锁，由于使用了条件判断是否加锁，而不是模版，所以这个地方没有用GRUAD，
    if (if_thread_synchro_)
    {
        protect_lock_.lock();
    }

    //记录到文件中
    if (output_way_ & static_cast<int>(LOG_OUTPUT::FILE) )
    {
        //得到新的文件名字
        open_new_logfile(false, now_time);

        //如果文件状态OK
        if (log_file_handle_)
        {
            log_file_handle_.write(log_tmp_buffer, static_cast<std::streamsize>(sz_use_len));

            //必须调用flush进行输出,因为如果有缓冲你就不能立即看到日志输出了，
            //这儿必须明白，不使用缓冲会让日志的速度下降很多很多,很多很多,
            //是否可以优化呢，这是一个两难问题
            log_file_handle_.flush();

            //size_log_file_ = static_cast<size_t>( log_file_handle_.tellp());
            size_log_file_ += sz_use_len;
        }
    }

    //如果有同步要求输出的地方
    if (output_way_ & static_cast<int>(LOG_OUTPUT::STDOUT))
    {
        //cout是行缓冲
        std::cout.write(log_tmp_buffer, static_cast<std::streamsize>(sz_use_len));
    }

    if (output_way_ & static_cast<int>(LOG_OUTPUT::ERROUT))
    {
        //cerr没有缓冲，云飞说的
        std::cerr.write(log_tmp_buffer, static_cast<std::streamsize>( sz_use_len));
    }

    //WIN32 下的调试输出,向调试窗口输出
#ifdef ZCE_OS_WINDOWS
    if (output_way_ & static_cast<int>(LOG_OUTPUT::WINDBG))
    {
        ::OutputDebugStringA(log_tmp_buffer);
    }

#endif

    //如果有线程同步，在这个地方解锁
    if (if_thread_synchro_)
    {
        protect_lock_.unlock();
    }
}

//通过字符串得到对应的日志策略,
zce::LOG_PRIORITY ZCE_LogTrace_Basic::log_priorities(const char *str_priority)
{
    if (strcasecmp (str_priority,  ("TRACE")) == 0)
    {
        return RS_TRACE;
    }
    else if (strcasecmp (str_priority,  ("DEBUG")) == 0)
    {
        return RS_DEBUG;
    }
    else if (strcasecmp (str_priority, ("INFO")) == 0)
    {
        return RS_INFO;
    }
    else if (strcasecmp (str_priority, ("ERROR")) == 0)
    {
        return RS_ERROR;
    }
    else if (strcasecmp (str_priority,  ("ALERT")) == 0)
    {
        return RS_ALERT;
    }
    else if (strcasecmp (str_priority, ("FATAL")) == 0)
    {
        return RS_FATAL;
    }
    else
    {
        return RS_DEBUG;
    }
}


//通过字符串得到对应的日志策略,
LOGFILE_DEVIDE ZCE_LogTrace_Basic::log_file_devide(const char *str_devide)
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



