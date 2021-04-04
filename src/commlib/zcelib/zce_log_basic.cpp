

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

//���캯��
ZCE_LogTrace_Basic::ZCE_LogTrace_Basic():
    div_log_file_(LOGDEVIDE_NONE),
    output_way_(LOG_OUTPUT_FILE | LOG_OUTPUT_ERROUT),
    if_thread_synchro_(false),
    auto_new_line_(true),
    max_size_log_file_(DEFAULT_LOG_SIZE),
    reserve_file_num_(DEFAULT_RESERVE_FILENUM),
    record_info_(LOG_HEAD_RECORD_CURRENTTIME | LOG_HEAD_RECORD_LOGLEVEL),
    current_click_(1),
    permit_outlevel_(RS_TRACE),
    size_log_file_(0),
    if_output_log_(true)
{
    //Ԥ�ȷ���ռ�
    log_file_name_.reserve(PATH_MAX + 32);
    log_file_dir_.reserve(PATH_MAX + 32);
    log_file_prefix_.reserve(PATH_MAX + 32);
}


//
ZCE_LogTrace_Basic::~ZCE_LogTrace_Basic()
{
    //ע��
    finalize();
}


//��ʼ������,����ʱ��ָ���־�Ĺ���
int ZCE_LogTrace_Basic::init_time_log(ZCE_LOGFILE_NAME_DEVIDE div_log_file,
                                      const char *log_file_prefix,
                                      bool if_thread_synchro,
                                      bool auto_new_line,
                                      size_t reserve_file_num,
                                      unsigned int output_way,
                                      unsigned int head_record)
{
    assert (NAME_TIME_HOUR_DEVIDE_TIME <= div_log_file  && NAME_TIME_YEAR_DEVIDE_TIME >= div_log_file  );
    return initialize(output_way,
                      div_log_file,
                      log_file_prefix,
                      if_thread_synchro,
                      auto_new_line,
                      0,
                      reserve_file_num,
                      head_record);
}

//��ʼ������,���ڳߴ�ָ���־�Ĺ��� ZCE_LOGFILE_DEVIDE_NAME = LOGDEVIDE_BY_SIZE
int ZCE_LogTrace_Basic::init_size_log(
    const char *log_file_prefix,
    bool if_thread_synchro,
    bool auto_new_line,
    size_t max_size_log_file,
    unsigned int reserve_file_num,
    unsigned int output_way,
    unsigned int head_record)
{

    ZCE_LOGFILE_NAME_DEVIDE div_log_file = NAME_ID_DEVIDE_SIZE;

    //�������ʶ�ļ��ָ��С
    if ( 0 == max_size_log_file )
    {
        div_log_file = LOGDEVIDE_NONE;
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

//��ʼ�����������ڱ�׼���
int ZCE_LogTrace_Basic::init_stdout(bool if_thread_synchro,
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

    return initialize(output_way,
                      LOGDEVIDE_NONE,
                      "",
                      if_thread_synchro,
                      auto_new_line,
                      0,
                      0,
                      head_record);
}

//��ʼ������,��������ȫ��һ��
int ZCE_LogTrace_Basic::initialize(unsigned int output_way,
                                   ZCE_LOGFILE_NAME_DEVIDE div_log_file,
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

    //���Լ���������
    if ( log_file_prefix != NULL)
    {
        log_file_prefix_ = log_file_prefix;
    }
    //����ļ���������ȫ
    else
    {
        assert(0 == ( output_way_ & (LOG_OUTPUT_FILE)) );
    }

    //
    make_configure();
    //�����Ҫ��־�ļ���������һ���ļ�
    if (output_way_ & LOG_OUTPUT_FILE )
    {
        timeval now_time(zce::gettimeofday());
        open_new_logfile(true, now_time);
    }

    return 0;
}

//�ر���־��ע��رպ󣬱������³�ʼ��
void ZCE_LogTrace_Basic::finalize()
{
    if (log_file_handle_.is_open())
    {
        log_file_handle_.close();
    }

    current_click_ = 1;
    permit_outlevel_ = RS_TRACE;

    div_log_file_ = LOGDEVIDE_NONE;
    output_way_ = LOG_OUTPUT_FILE | LOG_OUTPUT_ERROUT;

    size_log_file_ = 0;
    if_output_log_ = true;
}

//������־�ļ�
void ZCE_LogTrace_Basic::make_configure(void)
{
    //���max_size_log_file_�Ȳ����Ĵ�С��Χ
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

    //�õ�Ŀ¼������
    char dir_name[PATH_MAX + 16];
    dir_name[PATH_MAX] = '\0';

    zce::dirname(log_file_prefix_.c_str(), dir_name, PATH_MAX + 1);
    log_file_dir_ = dir_name;

    // ���Ŀ¼�����ڣ��򴴽�
    if (zce::mkdir_recurse(log_file_dir_.c_str()) != 0)
    {
        // ����ʧ�ܣ�
        fprintf(stderr, "mkdir %s fail. err=%d|%s\n",
                log_file_dir_.c_str(),
                errno,
                strerror(errno));
    }
}

//����־�������
void ZCE_LogTrace_Basic::enable_output(bool enable_out)
{
    if_output_log_ = enable_out;
}


//������־���Level
ZCE_LOG_PRIORITY ZCE_LogTrace_Basic::set_log_priority( ZCE_LOG_PRIORITY outlevel )
{
    ZCE_LOG_PRIORITY oldlevel = permit_outlevel_;
    permit_outlevel_ = outlevel;
    return oldlevel;
}
//ȡ�����Level
ZCE_LOG_PRIORITY ZCE_LogTrace_Basic::get_log_priority(void )
{
    return permit_outlevel_;
}

//����Ĭ���������Ϣ����
unsigned int ZCE_LogTrace_Basic::set_log_head(unsigned int recdinfo)
{
    unsigned int tmprecdinfo = recdinfo;
    record_info_ = recdinfo;
    return tmprecdinfo;
}
//ȡ��Ĭ���������Ϣ����
unsigned int ZCE_LogTrace_Basic::get_log_head(void)
{
    return record_info_;
}

//����ͬ������ı�ʾ
//�����ʼû�������ļ�ͬ�����,���治����.
unsigned int ZCE_LogTrace_Basic::set_output_way(unsigned int output_way)
{
    //
    unsigned int tmpsynchr = output_way_;
    output_way_ = output_way;

    return tmpsynchr;
}

//ȡ��ͬ������ı�ʾ
unsigned int ZCE_LogTrace_Basic::get_output_way(void)
{
    return output_way_;
}

//�����Ƿ��߳�ͬ��
bool ZCE_LogTrace_Basic::set_thread_synchro(bool if_thread_synchro)
{
    bool old_synchro = if_thread_synchro_;
    if_thread_synchro_ = if_thread_synchro;
    return old_synchro;

}
//ȡ���Ƿ�����߳�ͬ��
bool ZCE_LogTrace_Basic::get_thread_synchro(void)
{
    return if_thread_synchro_;
}

//�õ��µ���־�ļ��ļ�����
void ZCE_LogTrace_Basic::open_new_logfile(bool initiate, const timeval &current_time)
{
    //�Ƿ�Ҫ�����µ��ļ�����
    bool to_new_file = false;
    if (initiate)
    {
        to_new_file = true;

        //��ʱ����־�ľ���־��ɨ�����������ɾ������
        if (NAME_TIME_HOUR_DEVIDE_TIME == div_log_file_ ||
            NAME_SIXHOUR_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_DAY_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_MONTH_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_YEAR_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_MILLISECOND_DEVIDE_SIZE == div_log_file_)
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

    if (NAME_TIME_HOUR_DEVIDE_TIME == div_log_file_ ||
        NAME_SIXHOUR_DEVIDE_TIME == div_log_file_ ||
        NAME_TIME_DAY_DEVIDE_TIME == div_log_file_ ||
        NAME_TIME_MONTH_DEVIDE_TIME == div_log_file_ ||
        NAME_TIME_YEAR_DEVIDE_TIME == div_log_file_)
    {
        cur_click = current_time.tv_sec / zce::ONE_HOUR_SECONDS;

        //���ͱȽ�Ƶ��
        if (current_click_ != cur_click)
        {
            current_click_ = cur_click;
            std::string new_file_name;
            new_file_name.reserve(PATH_MAX + 32);
            create_time_logname(current_time, new_file_name);

            //�����־�ļ������Ѿ�����,��ʾҪ����һ�����ļ�,
            //String�ıȽ��ǱȽϺ�ʱ��,��ǰ����޶���֤1�����Ƚ�24��,Сcase
            if (log_file_name_ != new_file_name)
            {
                to_new_file = true;
                log_file_name_ = new_file_name;
            }
        }
    }
    else if (NAME_ID_DEVIDE_SIZE == div_log_file_)
    {
        //�����־�ļ��ĳߴ��Ѿ�����
        if (size_log_file_ > max_size_log_file_)
        {
            to_new_file = true;
        }
    }
    else if (LOGDEVIDE_NONE == div_log_file_)
    {
        log_file_name_ = log_file_prefix_ + STR_LOG_POSTFIX;
    }
    else if (NAME_TIME_MILLISECOND_DEVIDE_SIZE == div_log_file_)
    {
        std::string new_file_name;
        new_file_name.reserve(PATH_MAX + 32);
        //�����־�ļ��ĳߴ��Ѿ�����
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

    //����ļ��Ǵ�״̬,���ļ���������������´�һ��
    if (log_file_handle_.is_open())
    {
        if (log_file_handle_.good() == false)
        {
            to_new_file = true;
        }
    }

    //���������һ���µ��ļ�����
    if (to_new_file == true )
    {
        //�ر�ԭ�е��ļ�.
        if (log_file_handle_.is_open())
        {
            log_file_handle_.close();
        }

        del_old_logfile();

        log_file_handle_.clear();
        //��֮,
        log_file_handle_.open(log_file_name_.c_str(), std::ios::out | std::ios::app);

        size_log_file_ = static_cast<size_t>(log_file_handle_.tellp());

        if (NAME_TIME_HOUR_DEVIDE_TIME == div_log_file_ ||
            NAME_SIXHOUR_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_DAY_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_MONTH_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_YEAR_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_MILLISECOND_DEVIDE_SIZE == div_log_file_)
        {
            time_logfile_list_.push_back(log_file_name_);
        }

    }
}

void ZCE_LogTrace_Basic::del_old_logfile()
{
    //�������������־�����߷ָ���־��ʱ��Ϊ �� ���� ��
    if (reserve_file_num_ > 0)
    {
        //����ǰ���ʱ����зָ��ļ���
        if (NAME_TIME_HOUR_DEVIDE_TIME == div_log_file_ ||
            NAME_SIXHOUR_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_DAY_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_MONTH_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_YEAR_DEVIDE_TIME == div_log_file_ ||
            NAME_TIME_MILLISECOND_DEVIDE_SIZE == div_log_file_ )
        {
            //���ȷ����ֻ����һ����������־�ļ�,�����ļ�����,ɾ����������ļ�
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

        //����ǰ�װ�ߴ����ļ���
        else if (NAME_ID_DEVIDE_SIZE == div_log_file_)
        {
            std::string dellogfname;
            dellogfname.reserve(PATH_MAX + 16);
            create_id_logname(reserve_file_num_, dellogfname);
            ::remove(dellogfname.c_str());

            std::string oldlogfilename, renamefilename;
            oldlogfilename.reserve(MAX_PATH + 16);
            renamefilename.reserve(MAX_PATH + 16);

            // ��������Ҫ�Ӻ��濪ʼ��ǰ������
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


//�������ڵõ��ļ�����
void ZCE_LogTrace_Basic::create_time_logname(const timeval &cur_time,
                                             std::string &logfilename)
{
    time_t cur_t = cur_time.tv_sec;
    tm curtm = *localtime(&(cur_t));
    char tmpbuf[64] = {0};
    size_t buflen = sizeof(tmpbuf) - 1;

    switch (div_log_file_)
    {
        //��СʱΪ��λ�õ��ļ�����
        case NAME_TIME_HOUR_DEVIDE_TIME:
        case NAME_SIXHOUR_DEVIDE_TIME:
        {
            strftime( tmpbuf, buflen, "_%Y%m%d_%H", &curtm);
#if defined ZCE_LOG_TEST && ZCE_LOG_TEST== 1
            strftime(tmpbuf, buflen, "_%Y%m%d_%H%M", &curtm);
#endif
            ::strcat(tmpbuf, STR_LOG_POSTFIX);

            break;
        }

        case NAME_TIME_DAY_DEVIDE_TIME:
            ::strftime( tmpbuf, buflen, "_%Y%m%d", &curtm);
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;

        //
        case NAME_TIME_MONTH_DEVIDE_TIME:
            ::strftime( tmpbuf, buflen, "_%Y%m", &curtm);
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;

        case NAME_TIME_YEAR_DEVIDE_TIME:
            ::strftime( tmpbuf, buflen, "_%Y", &curtm);
            ::strcat(tmpbuf, STR_LOG_POSTFIX);
            break;

        case NAME_TIME_MILLISECOND_DEVIDE_SIZE:
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



//����ID�õ��ļ�����f
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

//����־��ͷ����Ϣ�����һ��Stringbuf��
void ZCE_LogTrace_Basic::stringbuf_loghead(ZCE_LOG_PRIORITY outlevel,
                                           const timeval &now_time,
                                           char *log_tmp_buffer,
                                           size_t sz_buf_len,
                                           size_t &sz_use_len)
{

    sz_use_len = 0;

    //�����¼ʱ��
    if (record_info_ & LOG_HEAD_RECORD_CURRENTTIME)
    {
        //ת��Ϊ���
        zce::timestamp(&now_time, log_tmp_buffer + sz_use_len, sz_buf_len);

        //������ˣ����
        sz_use_len = zce::TIMESTR_ISO_USEC_LEN;

        sz_buf_len -= sz_use_len;
    }

    //�����¼��־����
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

    //�����¼��ǰ��PID
    if (record_info_ & LOG_HEAD_RECORD_PROCESSID)
    {
        sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "[PID:%u]", static_cast<unsigned int>(zce::getpid()));
        sz_buf_len -= sz_use_len;
    }

    if (record_info_ & LOG_HEAD_RECORD_THREADID)
    {
        sz_use_len += snprintf(log_tmp_buffer + sz_use_len, sz_buf_len, "[TID:%u]", static_cast<unsigned int>(zce::pthread_self()));
        sz_buf_len -= sz_use_len;
    }
}

void ZCE_LogTrace_Basic::output_log_info(const timeval &now_time,
                                         char *log_tmp_buffer,
                                         size_t sz_use_len)
{
    //���Ҫ�߳�ͬ����������ط�����������ʹ���������ж��Ƿ������������ģ�棬��������ط�û����GRUAD��
    if (if_thread_synchro_)
    {
        protect_lock_.lock();
    }

    //��¼���ļ���
    if ((output_way_ & LOG_OUTPUT_FILE) )
    {
        //�õ��µ��ļ�����
        open_new_logfile(false, now_time);

        //����ļ�״̬OK
        if (log_file_handle_)
        {
            log_file_handle_.write(log_tmp_buffer, static_cast<std::streamsize>(sz_use_len));

            //�������flush�������,��Ϊ����л�����Ͳ�������������־����ˣ�
            //����������ף���ʹ�û��������־���ٶ��½��ܶ�ܶ�,�ܶ�ܶ�,
            //�Ƿ�����Ż��أ�����һ����������
            log_file_handle_.flush();

            //size_log_file_ = static_cast<size_t>( log_file_handle_.tellp());
            size_log_file_ += sz_use_len;
        }
    }

    //�����ͬ��Ҫ������ĵط�
    if (output_way_ & LOG_OUTPUT_STDOUT)
    {
        //cout���л���
        std::cout.write(log_tmp_buffer, static_cast<std::streamsize>(sz_use_len));
    }

    if (output_way_ & LOG_OUTPUT_ERROUT)
    {
        //cerrû�л��壬�Ʒ�˵��
        std::cerr.write(log_tmp_buffer, static_cast<std::streamsize>( sz_use_len));
    }

    //WIN32 �µĵ������,����Դ������
#ifdef ZCE_OS_WINDOWS
    if (output_way_ & LOG_OUTPUT_WINDBG)
    {
        ::OutputDebugStringA(log_tmp_buffer);
    }

#endif

    //������߳�ͬ����������ط�����
    if (if_thread_synchro_)
    {
        protect_lock_.unlock();
    }
}

//ͨ���ַ����õ���Ӧ����־����,
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
}


//ͨ���ַ����õ���Ӧ����־����,
ZCE_LOGFILE_NAME_DEVIDE ZCE_LogTrace_Basic::log_file_devide(const char *str_devide)
{
    if (strcasecmp(str_devide, ("SIZE_ID")) == 0)
    {
        return NAME_ID_DEVIDE_SIZE;
    }
    else if (strcasecmp(str_devide, ("HOUR")) == 0)
    {
        return NAME_TIME_HOUR_DEVIDE_TIME;
    }
    else if (strcasecmp(str_devide, ("SIXHOUR")) == 0)
    {
        return NAME_SIXHOUR_DEVIDE_TIME;
    }
    else if (strcasecmp(str_devide, ("DAY")) == 0)
    {
        return NAME_TIME_DAY_DEVIDE_TIME;
    }
    else if (strcasecmp(str_devide, ("MONTH")) == 0)
    {
        return NAME_TIME_MONTH_DEVIDE_TIME;
    }
    else if (strcasecmp(str_devide, ("YEAR")) == 0)
    {
        return NAME_TIME_YEAR_DEVIDE_TIME;
    }
    else if (strcasecmp(str_devide, ("SIZE_MILLSENCOND")) == 0)
    {
        return NAME_TIME_MILLISECOND_DEVIDE_SIZE;
    }
    else
    {
        return NAME_TIME_DAY_DEVIDE_TIME;
    }
}



