#include "zce/predefine.h"
#include "zce/os_adapt/common.h"
#include "zce/os_adapt/dirent.h"

#include "zce/os_adapt/process.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/string.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/file.h"
#include "zce/logger/log_file.h"

namespace zce
{
const char Log_File::STR_LOG_POSTFIX[LEN_LOG_POSTFIX + 1] = ".log";

//构造函数
Log_File::Log_File()
{
    //预先分配空间
    log_file_name_.reserve(PATH_MAX + 32);
    log_file_dir_.reserve(PATH_MAX + 32);
    log_file_prefix_.reserve(PATH_MAX + 32);
}

//
Log_File::~Log_File()
{
    //注销
    terminate();
}





//初始化函数,参数最齐全的一个
int Log_File::initialize(int output_way,
                         LOGFILE_DEVIDE div_log_file,
                         const char* log_file_prefix,
                         bool trunc_old,
                         bool thread_output,
                         size_t max_size_log_file,
                         size_t reserve_file_num) noexcept
{
    div_log_file_ = div_log_file;

    trunc_old_ = trunc_old;
    max_size_log_file_ = max_size_log_file;
    reserve_file_num_ = reserve_file_num;
    current_click_ = 1;
    thread_output_ = thread_output;
    size_log_file_ = 0;


    //断言检查输入参数
    if (log_file_prefix != NULL)
    {
        log_file_prefix_ = log_file_prefix;
    }
    //如果文件参数不齐全
    else
    {
        assert(0 == (output_way & static_cast<int>(LOG_OUTPUT::LOGFILE)));
    }

    //
    make_configure();
    //如果需要日志文件输出，输出一个文件
    if (output_way & static_cast<int>(LOG_OUTPUT::LOGFILE))
    {
        timeval now_time(gettimeofday());
        open_new_logfile(true, now_time);
    }

    return 0;
}

//关闭日志，注意关闭后，必须重新初始化
void Log_File::terminate()
{
    if (log_file_handle_.is_open())
    {
        log_file_handle_.close();
    }

    current_click_ = 1;
    div_log_file_ = LOGFILE_DEVIDE::NONE;
    size_log_file_ = 0;
}

//配置日志文件
void Log_File::make_configure(void) noexcept
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

    dirname(log_file_prefix_.c_str(), dir_name, PATH_MAX + 1);
    log_file_dir_ = dir_name;

    // 如果目录不存在，则创建
    if (mkdir_recurse(log_file_dir_.c_str()) != 0)
    {
        // 创建失败，
        fprintf(stderr, "mkdir %s fail. err=%d|%s\n",
                log_file_dir_.c_str(),
                errno,
                strerror(errno));
    }
}

//得到新的日志文件文件名称
void Log_File::open_new_logfile(bool initiate,
                                const timeval& current_time) noexcept
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
            ret = readdir_nameary(log_file_dir_.c_str(),
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
        cur_click = current_time.tv_sec / ONE_HOUR_SECONDS;

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
    if (to_new_file == true)
    {
        //关闭原有的文件.
        if (log_file_handle_.is_open())
        {
            log_file_handle_.close();
        }

        del_old_logfile();

        log_file_handle_.clear();
        //打开之,
        std::ios_base::openmode mode = std::ios::out | std::ios::app;
        if (trunc_old_)
        {
            mode = std::ios::out | std::ios::trunc;
        }
        log_file_handle_.open(log_file_name_.c_str(), mode);
        if (log_file_handle_)
        {
            fprintf(stderr, "Open log file name [%s] ok.",
                    log_file_name_.c_str());
        }
        else
        {
            fprintf(stderr, "Open log file name [%s] fail. errno=%d.",
                    log_file_name_.c_str(),
                    errno);
        }
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

void Log_File::del_old_logfile() noexcept
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
            LOGFILE_DEVIDE::BY_TIME_NAME_MILLISECOND == div_log_file_)
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
            for (size_t i = reserve_file_num_; i > 0; --i)
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

        clear_last_error();
    }
}

//根据日期得到文件名称
void Log_File::create_time_logname(const timeval& cur_time,
                                   std::string& logfilename) noexcept
{
    const time_t cur_t = cur_time.tv_sec;
    tm curtm = *localtime(&(cur_t));
    char tmpbuf[64] = { 0 };
    size_t buflen = sizeof(tmpbuf) - 1;

    switch (div_log_file_)
    {
        //以小时为单位得到文件名称
    case LOGFILE_DEVIDE::BY_TIME_HOUR:
    case LOGFILE_DEVIDE::BY_TIME_SIX_HOUR:
    {
        strftime(tmpbuf, buflen, "_%Y%m%d_%H", &curtm);
#if defined ZCE_LOG_TEST && ZCE_LOG_TEST== 1
        strftime(tmpbuf, buflen, "_%Y%m%d_%H%M", &curtm);
#endif
        ::strcat(tmpbuf, STR_LOG_POSTFIX);

        break;
    }

    case LOGFILE_DEVIDE::BY_TIME_DAY:
        ::strftime(tmpbuf, buflen, "_%Y%m%d", &curtm);
        ::strcat(tmpbuf, STR_LOG_POSTFIX);
        break;

        //
    case LOGFILE_DEVIDE::BY_TIME_MONTH:
        ::strftime(tmpbuf, buflen, "_%Y%m", &curtm);
        ::strcat(tmpbuf, STR_LOG_POSTFIX);
        break;

    case LOGFILE_DEVIDE::BY_TIME_YEAR:
        ::strftime(tmpbuf, buflen, "_%Y", &curtm);
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
void Log_File::create_id_logname(size_t logfileid,
                                 std::string& log_filename) noexcept
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

void Log_File::fileout_log_info(const timeval& now_time,
                                char* log_tmp_buffer,
                                size_t sz_use_len) noexcept
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


}