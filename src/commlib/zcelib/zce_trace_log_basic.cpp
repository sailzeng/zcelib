

#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_dirent.h"
#include "zce_os_adapt_thread.h"
#include "zce_os_adapt_process.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_string.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_file.h"
#include "zce_trace_log_basic.h"

const char ZCE_LogTrace_Basic::STR_LOG_POSTFIX[LEN_LOG_POSTFIX + 1] = ".log";

//构造函数
ZCE_LogTrace_Basic::ZCE_LogTrace_Basic():
    div_log_file_(LOGDEVIDE_NONE),
    output_way_(LOG_OUTPUT_FILE | LOG_OUTPUT_ERROUT),
    if_thread_synchro_(false),
    auto_new_line_(true),
    max_size_log_file_(DEFAULT_LOG_SIZE),
    reserve_file_num_(DEFAULT_RESERVE_FILENUM),
    record_info_(LOG_HEAD_RECORD_CURRENTTIME | LOG_HEAD_RECORD_LOGLEVEL),
    current_click_(1),
    out_level_mask_(RS_TRACE),
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
int ZCE_LogTrace_Basic::init_time_log(ZCE_LOGFILE_DEVIDE div_log_file,
                                      const char *log_file_prefix,
                                      bool if_thread_synchro,
                                      bool auto_new_line,
                                      size_t reserve_file_num,
                                      unsigned int output_way,
                                      unsigned int head_record)
{
    assert (LOGDEVIDE_NONE <= div_log_file  && LOGDEVIDE_BY_YEAR >= div_log_file  );
    return initialize( div_log_file,
                       log_file_prefix,
                       if_thread_synchro,
                       auto_new_line,
                       0,
                       reserve_file_num,
                       output_way,
                       head_record);
}

//初始化函数,用于尺寸分割日志的构造 ZCE_LOGFILE_DEVIDE = LOGDEVIDE_BY_SIZE
int ZCE_LogTrace_Basic::init_size_log(
    const char *log_file_prefix,
    bool if_thread_synchro,
    bool auto_new_line,
    size_t max_size_log_file,
    unsigned int reserve_file_num,
    unsigned int output_way,
    unsigned int head_record)
{

    ZCE_LOGFILE_DEVIDE div_log_file = LOGDEVIDE_BY_SIZE;

    //如果不标识文件分割大小
    if ( 0 == max_size_log_file )
    {
        div_log_file = LOGDEVIDE_NONE;
    }

    return initialize( div_log_file,
                       log_file_prefix,
                       if_thread_synchro,
                       auto_new_line,
                       max_size_log_file,
                       reserve_file_num,
                       output_way,
                       head_record);
}

//初始化函数，用于标准输出
int ZCE_LogTrace_Basic::init_stdout(bool if_thread_synchro ,
                                    bool use_err_out,
                                    bool auto_new_line,
                                    unsigned int head_record)
{
    unsigned int output_way = 0;

    if (use_err_out)
    {
        output_way |= LOG_OUTPUT_ERROUT;
    }
    else
    {
        output_way |= LOG_OUTPUT_STDOUT;
    }

    return initialize( LOGDEVIDE_NONE,
                       "",
                       if_thread_synchro,
                       auto_new_line,
                       0,
                       0,
                       output_way,
                       head_record);
}

//初始化函数,参数最齐全的一个
int ZCE_LogTrace_Basic::initialize(ZCE_LOGFILE_DEVIDE div_log_file,
                                   const char *log_file_prefix,
                                   bool if_thread_synchro,
                                   bool auto_new_line,
                                   size_t max_size_log_file,
                                   size_t reserve_file_num,
                                   unsigned int output_way,
                                   unsigned int head_record)
{
    div_log_file_ = div_log_file;
    output_way_ = output_way;

    if_thread_synchro_  = if_thread_synchro;
    auto_new_line_ = auto_new_line;

    max_size_log_file_ = max_size_log_file;

    reserve_file_num_ = reserve_file_num;
    record_info_ = head_record;
    current_click_ = 1;
    out_level_mask_ = RS_TRACE;
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
        assert(0 == ( output_way_ & (LOG_OUTPUT_FILE)) );
    }

    //
    make_configure();

    set_reserve_file_num((unsigned int)reserve_file_num);
    //如果需要日志文件输出，输出一个文件
    if (output_way_ & LOG_OUTPUT_FILE )
    {
        make_new_logfile(time(NULL), true);
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
    out_level_mask_ = RS_TRACE;

    div_log_file_ = LOGDEVIDE_NONE;
    output_way_ = LOG_OUTPUT_FILE | LOG_OUTPUT_ERROUT;

    size_log_file_ = 0;
    if_output_log_ = true;
}

//配置日志文件
void ZCE_LogTrace_Basic::make_configure(void)
{
    int ret = 0;

    //检查max_size_log_file_的大小范围
    if (max_size_log_file_ < MIN_LOG_SIZE)
    {
        max_size_log_file_ = MIN_LOG_SIZE;
    }

    if (max_size_log_file_ > MAX_LOG_SIZE)
    {
        max_size_log_file_ = MAX_LOG_SIZE;
    }

    //得到目录的名称
    char dir_name[PATH_MAX + 16];
    dir_name[PATH_MAX] = '\0';

    ZCE_OS::dirname(log_file_prefix_.c_str(), dir_name, PATH_MAX + 1);
    log_file_dir_ = dir_name;

    time_t cur_time = 0;

    // 如果目录不存在，则创建
    if (ZCE_OS::mkdir_recurse(log_file_dir_.c_str()) != 0)
    {
        // 创建失败，
        printf("mkdir %s fail. err=%s\n", log_file_dir_.c_str(), strerror(errno));
    }

    //根据分割文件的方法进行预处理
    switch (div_log_file_)
    {
            //
        case LOGDEVIDE_NONE:
            log_file_name_ = log_file_prefix_ + STR_LOG_POSTFIX;
            break;

            //得到日期和月份分割的文件名称
        case LOGDEVIDE_BY_FIVEMINUTE:
        case LOGDEVIDE_BY_QUARTER:
        case LOGDEVIDE_BY_HOUR:
        case LOGDEVIDE_BY_SIXHOUR:
        case LOGDEVIDE_BY_DATE:
        case LOGDEVIDE_BY_MONTH:
        case LOGDEVIDE_BY_YEAR:
            cur_time = time(NULL);
            current_click_ = cur_time / ONE_HOUR_SECONDS;
            create_time_logname(cur_time, log_file_name_);
            break;

            //使用ID区分分隔日志有一个问题.就是每次初始化时要得到ID,否则要覆盖原有
            //日志文件,所以要查询原有最后一个日志文件,还要得到其的记录个数以及文件尺寸
            //确定日志文件的序号已经到了几
        case LOGDEVIDE_BY_SIZE:

            //记录日志文件的文件名称
            create_id_logname(0, log_file_name_);

            //如果要按照SIZE划分日志,得到记录日志文件的尺寸
            if (LOGDEVIDE_BY_SIZE == div_log_file_)
            {
                ret = ZCE_OS::filesize(log_file_name_.c_str(), &size_log_file_);

                if ( 0 != ret  )
                {
                    size_log_file_ = 0;
                }
            }

            break;

        default:
            break;
    }
}

//打开日志输出开关
void ZCE_LogTrace_Basic::enable_output_logmsg(void)
{
    if_output_log_ = true;
}
//关闭日志输出开关
void ZCE_LogTrace_Basic::disable_output_logmsg(void)
{
    if_output_log_ = false;
}

//设置日志输出Mask
ZCE_LOG_PRIORITY ZCE_LogTrace_Basic::set_log_priority( ZCE_LOG_PRIORITY outlevel )
{
    ZCE_LOG_PRIORITY oldlevel = out_level_mask_;
    out_level_mask_ = outlevel;
    return oldlevel;
}
//取得输出Mask
ZCE_LOG_PRIORITY ZCE_LogTrace_Basic::get_log_priority(void )
{
    return out_level_mask_;
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
void ZCE_LogTrace_Basic::make_new_logfile(time_t cur_time, bool init)
{
    //是否要生成新的文件名称
    bool to_new_file = false;

    time_t cur_click = 0;

    switch (div_log_file_)
    {
        case LOGDEVIDE_NONE:
            log_file_name_ = log_file_prefix_ + STR_LOG_POSTFIX;
            break;

        case LOGDEVIDE_BY_FIVEMINUTE:
            cur_click = cur_time / FIVE_MINUTE_SECONDS;
            break;

        case LOGDEVIDE_BY_QUARTER:
            cur_click = cur_time / ONE_QUARTER_SECONDS;
            break;

        case LOGDEVIDE_BY_HOUR:
        case LOGDEVIDE_BY_SIXHOUR:
        case LOGDEVIDE_BY_DATE:
        case LOGDEVIDE_BY_MONTH:
        case LOGDEVIDE_BY_YEAR:
            //得到小时精度的时间记录,
            cur_click = cur_time / ONE_HOUR_SECONDS;

            //每小时检查一次,对按年月日分割好像不公平,但这样可以节省大量代码,而且也没有什么效率问题
            //另外这样可以避免时区的判断
            if (current_click_ !=  cur_click)
            {
                current_click_ = cur_click;
                std::string new_file_name;
                new_file_name.reserve(512);
                create_time_logname(cur_time, new_file_name);

                //如果日志文件名称已经更新,表示要产生一个新文件,
                //String的比较是比较耗时的,但前面的限定保证1天最多比较24次,小case
                if (log_file_name_ != new_file_name)
                {
                    to_new_file = true;
                    log_file_name_ = new_file_name;
                }
            }

            break;

        case LOGDEVIDE_BY_SIZE:

            //如果日志文件的尺寸已经超出
            if (size_log_file_ > max_size_log_file_)
            {
                to_new_file = true;
            }

            break;

        default:
            break;
    }

    //如果文件是打开状态,但文件句柄不正常，重新打开一个
    if (log_file_handle_.is_open())
    {
        if (log_file_handle_.good() == false)
        {
            to_new_file = true;
        }
    }
    //如果文件没有打开
    else
    {
        //不是新打开文件，打开旧有的文件，保持尺寸
        if (to_new_file == false)
        {
            log_file_handle_.clear();
            log_file_handle_.open(log_file_name_.c_str(), std::ios::out | std::ios::app);
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

        //如果保留所有日志，或者分割日志的时间为 月 或者 年
        del_old_logfile(cur_time, init);

        log_file_handle_.clear();
        //如果是新文件,打开之,并清零
        log_file_handle_.open(log_file_name_.c_str(), std::ios::out);

        size_log_file_ = 0;
    }
}

//处理超期的时间分割日志文件,可能会删除,
//初始化的时候会过滤日志目录，删除错误文件
void ZCE_LogTrace_Basic::del_old_time_logfile(time_t cur_time, bool init)
{
    //如果确定了只保留一定数量的日志文件,而且文件过多,删除掉多余的文件

    //
    std::string del_logfile_name;
    del_logfile_name.reserve(PATH_MAX + 16);

    time_t del_file_time = 0;

    switch (div_log_file_)
    {
        case LOGDEVIDE_NONE:
            log_file_name_ = log_file_prefix_ + STR_LOG_POSTFIX;
            break;

        case LOGDEVIDE_BY_FIVEMINUTE:
            del_file_time = cur_time - reserve_file_num_ * FIVE_MINUTE_SECONDS;
            break;

        case LOGDEVIDE_BY_QUARTER:
            del_file_time = cur_time - reserve_file_num_ * ONE_QUARTER_SECONDS;
            break;

        case LOGDEVIDE_BY_HOUR:
            del_file_time = cur_time - reserve_file_num_ * ONE_HOUR_SECONDS;
            break;

        case LOGDEVIDE_BY_SIXHOUR:
            del_file_time = cur_time - reserve_file_num_ * 6 * ONE_HOUR_SECONDS;
            break;

        case LOGDEVIDE_BY_DATE:
            del_file_time = cur_time - reserve_file_num_ * 24 * ONE_HOUR_SECONDS;
            break;

            //使用最大一个月的时间进行判断，
        case LOGDEVIDE_BY_MONTH:
            del_file_time = cur_time - reserve_file_num_ * 31 * ONE_DAY_SECONDS;
            break;

            //使用最大一年闰年的时间进行判断
        case LOGDEVIDE_BY_YEAR:
            del_file_time = cur_time - reserve_file_num_ * 366 * ONE_DAY_SECONDS;
            break;

        default:
            assert(false);
            break;
    }

    create_time_logname(del_file_time, del_logfile_name);

    //如果不是初始化，只尝试删除一个过期的文件
    if (!init)
    {
        //尝试删除这个文件，
        ::remove(del_logfile_name.c_str());
    }
    //如果是初始化，扫描日志目录，检查所有文件
    else
    {
        //过滤目录
        DIR *dir_handle = ZCE_OS::opendir(log_file_dir_.c_str());

        if (NULL == dir_handle)
        {
            return ;
        }

        dirent *find_file = NULL;

        for ( find_file = ZCE_OS::readdir(dir_handle);
              find_file != NULL;
              find_file = ZCE_OS::readdir(dir_handle))
        {
            int ret_cmp = 0;
            std::string process_file_name(log_file_dir_);
            process_file_name += find_file->d_name;

            //检查前缀
            ret_cmp = strncmp(process_file_name.c_str(),
                              log_file_prefix_.c_str(),
                              log_file_prefix_.length());

            if (ret_cmp != 0)
            {
                continue;
            }

            size_t filename_len = strlen(find_file->d_name);

            if ( filename_len <= LEN_LOG_POSTFIX)
            {
                continue;
            }

            //前面的判断保证了这儿不会溢出
            //检查日志后缀结尾
            if (0 != strcmp(find_file->d_name + filename_len - LEN_LOG_POSTFIX,
                            STR_LOG_POSTFIX))
            {
                continue;
            }

            //这个判断有点简单粗暴，但是因为已经检查了后缀，前缀，问题不大
            if (strcmp(process_file_name.c_str(), del_logfile_name.c_str()) <= 0)
            {
                ::remove(process_file_name.c_str());
            }
        }

        ZCE_OS::closedir(dir_handle);

    }
}



//处理超期的ID日志文件
void ZCE_LogTrace_Basic::del_old_id_logfile()
{

    std::string dellogfname;
    dellogfname.reserve(PATH_MAX + 16);
    create_id_logname(reserve_file_num_ - 1, dellogfname);
    ::remove(dellogfname.c_str());

    std::string oldlogfilename , renamefilename;
    oldlogfilename.reserve(512);
    renamefilename.reserve(512);

    // 重命名需要从后面开始往前重命名
    for (size_t i = reserve_file_num_ - 1; i > 0; -- i)
    {
        create_id_logname(i - 1, oldlogfilename);
        create_id_logname(i, renamefilename);
        ::rename(oldlogfilename.c_str(), renamefilename.c_str());
    }
}




//根据日期得到文件名称
void ZCE_LogTrace_Basic::create_time_logname(time_t tmt, std::string &logfilename)
{
    tm curtm = *localtime(&tmt);
    char tmpbuf[65] = {0};
    size_t buflen = sizeof(tmpbuf) - 1;

    switch (div_log_file_)
    {
            //以分钟为单位得到文件名称
        case LOGDEVIDE_BY_FIVEMINUTE:
        case LOGDEVIDE_BY_QUARTER:
        {
            strftime( tmpbuf, buflen, "_%Y%m%d_%H%M", &curtm);
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;
        }

        //以小时为单位得到文件名称
        case LOGDEVIDE_BY_HOUR:
        case LOGDEVIDE_BY_SIXHOUR:
        {
            strftime( tmpbuf, buflen, "_%Y%m%d_%H", &curtm);
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;
        }

        //以天为单位
        case LOGDEVIDE_BY_DATE:
            strftime( tmpbuf, buflen, "_%Y%m%d", &curtm);
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;

            //
        case LOGDEVIDE_BY_MONTH:
            strftime( tmpbuf, buflen, "_%Y%m", &curtm);
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;

        case LOGDEVIDE_BY_YEAR:
            strftime( tmpbuf, buflen, "_%Y", &curtm);
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
void ZCE_LogTrace_Basic::create_id_logname(size_t logfileid, std::string &idlogfilename)
{
    char tmpbuf[32];

    if (logfileid == 0)
    {
        snprintf(tmpbuf, sizeof(tmpbuf) - 1, "%s",
                 STR_LOG_POSTFIX);
    }
    else
    {
        snprintf(tmpbuf, sizeof(tmpbuf) - 1, "%s.%u",
                 STR_LOG_POSTFIX,
                 (unsigned int)logfileid);
    }

    idlogfilename = log_file_prefix_;
    idlogfilename += tmpbuf;
}

//将日志的头部信息输出到一个Stringbuf中
void ZCE_LogTrace_Basic::stringbuf_loghead(ZCE_LOG_PRIORITY outlevel,
                                           const timeval &now_time,
                                           char *log_tmp_buffer,
                                           size_t sz_buf_len,
                                           size_t &sz_use_len)
{

    sz_use_len = 0;

    //如果纪录时间
    if (record_info_ & LOG_HEAD_RECORD_CURRENTTIME)
    {
        //转换为语句
        ZCE_OS::timestamp(&now_time, log_tmp_buffer + sz_use_len, sz_buf_len);

        //别计算了，快点
        sz_use_len = ZCE_OS::LEN_OF_TIME_STAMP;

        sz_buf_len -= sz_use_len;
    }

    //如果记录日志级别
    if (record_info_ & LOG_HEAD_RECORD_LOGLEVEL)
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
    if (record_info_ & LOG_HEAD_RECORD_PROCESSID)
    {
        sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "[PID:%u]", static_cast<unsigned int>(ZCE_OS::getpid()));
        sz_buf_len -= sz_use_len;
    }

    if (record_info_ & LOG_HEAD_RECORD_THREADID)
    {
        sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "[TID:%u]", static_cast<unsigned int>(ZCE_OS::pthread_self()));
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
    if ((output_way_ & LOG_OUTPUT_FILE) )
    {
        //得到新的文件名字
        make_new_logfile(now_time.tv_sec);

        //如果文件状态OK
        if (log_file_handle_)
        {
            log_file_handle_.write(log_tmp_buffer, static_cast<std::streamsize>(sz_use_len));

            //必须调用flush进行输出,因为如果有缓冲你就不能立即看到日志输出了，
            //这儿必须明白，不实用缓冲会让日志的速度下降很多很多,很多很多,
            //是否可以优化呢，这是一个两难问题
            log_file_handle_.flush();

            //size_log_file_ = static_cast<unsigned long>( log_file_handle_.tellp());
            size_log_file_ += sz_use_len;
        }
    }

    //如果有同步要求输出的地方
    if (output_way_ & LOG_OUTPUT_STDOUT)
    {
        //cout是行缓冲
        std::cout.write(log_tmp_buffer, static_cast<std::streamsize>(sz_use_len));
    }

    if (output_way_ & LOG_OUTPUT_ERROUT)
    {
        //cerr没有缓冲，云飞说的
        std::cerr.write(log_tmp_buffer, static_cast<std::streamsize>( sz_use_len));
    }

    //WIN32 下的调试输出,向调试窗口输出
#ifdef ZCE_OS_WINDOWS

    if (output_way_ & LOG_OUTPUT_WINDBG)
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
ZCE_LOG_PRIORITY ZCE_LogTrace_Basic::log_priorities(const char *str_priority)
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

    //Never Goto Here.
    //return RS_DEBUG;
}

size_t  ZCE_LogTrace_Basic::set_reserve_file_num(size_t file_num)
{
    size_t old_reserve = reserve_file_num_;
    reserve_file_num_ = file_num;
    // 重新计算需要清除的旧文件
    del_old_logfile(time(NULL), true);
    return old_reserve;
}

unsigned int ZCE_LogTrace_Basic::get_reserve_file_num() const
{
    return (unsigned int)reserve_file_num_;
}

void ZCE_LogTrace_Basic::del_old_logfile(time_t cur_time, bool init)
{
    //如果保留所有日志，或者分割日志的时间为 月 或者 年
    if ( reserve_file_num_ > 0 )
    {
        //如果是按照时间进行分割文件的
        if ( div_log_file_ > LOGDEVIDE_BY_TIMEBEGIN && div_log_file_ < LOGDEVIDE_BY_TIMEEND)
        {
            del_old_time_logfile(cur_time, init);
        }

        //如果是安装尺寸风格文件的
        if (LOGDEVIDE_BY_SIZE == div_log_file_ )
        {
            del_old_id_logfile();
        }

        ZCE_OS::clear_last_error();
    }
}
