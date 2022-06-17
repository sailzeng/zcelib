/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_log_msg.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       Monday, December 30, 2013
* @brief
*
*
* @details    支持传统的printf格式的日志输出，
*             也支持{}的format方式的日志输出，
*             ow，ow，又可以卖老来讲一点点历史了，一点点，
*             最开始写这个类的时候，我刚刚对C++的stream有一些了解。正如一切土孩子
*             看见新东东一样，你会以为新东东是完美的，在第一版我用stream的方式实现了
*             日志输出，当然不得不说，光看着也还是挺酷的，
*             但说回来C++ stream真算不上一个设计完美的东东
*             1.格式化控制痛苦，必须承认在格式化控制上他远远不如printf这种函数，
*             2.性能弱，大约比snprintf这类函数的慢50%.(GCC7.1最新版本有惊喜)
*             3.多线程情况并不好控制，特别是多线程环境下，你基本无法加锁，
*             所以C++ stream空有一身靓丽的外表，却很少人真正使用。
*
*             所以当要彻底重构的时候，我决定测定抛弃steam的代码，特别是当我发现
*             fast format这些函数库在字符串格式化上已经超越C的snprintf，无疑
*             极大的振奋了我。我决定自己实现一下，当然C的log函数迅速整理完毕，
*             C++的版本却仍然等待了很长一段时间，即使我的str_nprintf
*             已经完成后，因为我发现，如果没有变餐的模版，每一次用模版实现字符串格式
*             化都是一次煎熬，你只能用宏完成代码替换。好在C++11目前以及普及，
*             大约在15年，C++的版本也完成了。
*
*             最后C++ 20，快点到来把。期待C++ format的实现。
*
*
*
* @note
*
*/

#pragma once

#include "zce/string/extend.h"
#include "zce/logger/log_file.h"

///默认记录的数据,在每条日志的头部
enum class LOG_HEAD
{
    ///什么都不纪录
    NONE = 0,
    ///纪录当前的时间
    CURRENTTIME = (0x1 << 0),
    ///纪录日志的级别信息
    LOGLEVEL = (0x1 << 1),
    ///纪录进程ID
    PROCESS_ID = (0x1 << 2),
    ///纪录线程ID
    THREAD_ID = (0x1 << 3),
};

///选择输出的方式
enum class LOG_OUTPUT
{
    ///不向任何地方输出
    NONE = 0,
    ///同步不向其他地方输出,默认
    LOGFILE = (0x1 << 0),
    ///同步向标准输出输出.如果你的程序是CGI程序,慎用
    STDOUT = (0x1 << 1),
    ///同步向标准错误输出.
    ERROUT = (0x1 << 2),
    ///向共享内存文件里面输出
    MMAP_FILE = (0x1 << 3),
    ///同步向WINDOWS的调试窗口输出,仅仅在WIN32环境起作用
    WINDBG = (0x1 << 4)
};

namespace zce
{
class Log_Msg
{
public:

    ///构造函数
    Log_Msg();
    virtual ~Log_Msg();

    /*!
    * @brief      初始化函数,用于时间分割日志的构造
    * @return     int                返回0标识初始化成功
    * @param[in]  div_log_file       分割日志的方式，时间分割有多种方式
    * @param[in]  log_file_prefix    日志的前缀，包括目录信息
    * @param[in]  reserve_file_num   保留的日志文件数量，超过这个数量的日志将被删除
    * @param[in]  multithread_log    是否是多线程进行日志输出，如果是进行线程同步，
    * @param[in]  thread_output_file 是否使用独立的线程进行日志文件的输出，避免主线程的等待日志输出
    * @param[in]  trunc_old          是否删除原有的（覆盖原有老文件）日志文件的信息，
    * @param[in]  auto_new_line      日志记录的末尾是否自动的换行，new一行
    * @param[in]  output_way         日志输出的方式,可以多种方式并存，参考 @ref LOG_OUTPUT
    * @param[in]  head_record        日志头部包含的信息包括，参考 @ref LOG_HEAD_RECORD_INFO
    */
    int init_time_log(LOGFILE_DEVIDE div_log_file,
                      const char* log_file_prefix,
                      size_t reserve_file_num = Log_File::DEFAULT_RESERVE_FILENUM,
                      bool multithread_log = false,
                      bool thread_output_file = false,
                      bool auto_new_line = true,
                      bool trunc_old = false,
                      int output_way = DEFUALT_LOG_OUTPUT,
                      int head_record = (int)LOG_HEAD::CURRENTTIME | (int)LOG_HEAD::LOGLEVEL) noexcept;

    /*!
    @brief      初始化函数,用于尺寸分割日志的构造 内部的 ZCE_LOGFILE_DEVIDE_NAME = LOGDEVIDE_BY_SIZE
    @return     int                返回0标识初始化成功
    @param[in]  log_file_prefix    日志的前缀，包括目录信息
    @param[in]  max_size_log_file  日志文件的最大尺寸，目前最大尺寸内部用的4G
    @param[in]  reserve_file_num   保留的日志文件数量，超过这个数量的日志将被删除
    @param[in]  is_thread_synchro  是否进行线程同步
    @param[in]  thread_output_file 使用线程输出文件
    @param[in]  auto_new_line      日志记录的末尾是否自动的换行，new一行
    @param[in]  trunc_old          是否截断原有的日志文件的信息，
    @param[in]  output_way         日志输出的方式，参考 @ref LOG_OUTPUT
    @param[in]  head_record        日志头部包含的信息包括，参考 @ref LOG_HEAD_RECORD_INFO
    */
    int init_size_log(const char* log_file_prefix,
                      size_t max_size_log_file = Log_File::DEFAULT_LOG_SIZE,
                      size_t reserve_file_num = Log_File::DEFAULT_RESERVE_FILENUM,
                      bool multithread_log = false,
                      bool thread_output_file = false,
                      bool auto_new_line = true,
                      bool trunc_old = false,
                      int output_way = DEFUALT_LOG_OUTPUT,
                      int head_record = (int)LOG_HEAD::CURRENTTIME | (int)LOG_HEAD::LOGLEVEL) noexcept;

    /*!
    @brief      初始化函数，用于标准输出
    @return     int                返回0标识初始化成功
    @param[in]  use_err_out        是否是使用错误输出进行输出，因为标准输出的麻烦，一般用错误输出
    @param[in]  if_thread_synchro  是否进行线程同步
    @param[in]  auto_new_line      日志记录的末尾是否自动的换行，new一行
    @param[in]  head_record        日志头部包含的信息包括，参考 @ref LOG_HEAD_RECORD_INFO
    */
    int init_stdout(bool use_err_out = true,
                    bool auto_new_line = true,
                    bool multithread_log = false,
                    int head_record = (int)LOG_HEAD::CURRENTTIME | (int)LOG_HEAD::LOGLEVEL) noexcept;


    /*!
    @brief      关闭日志，注意关闭后，必须重新初始化
    */
    void terminate();

    /*!
    @brief      设置默认输出的信息类型
    @return     unsigned int
    @param      recdinfo
    */
    unsigned int set_log_head(int recdinfo);
    /*!
    @brief      取得默认输出的信息类型
    @return     unsigned int
    */
    unsigned int get_log_head(void);

    /*!
    @brief      设置日志输出级别的Level
    @return     zce::LOG_PRIORITY  旧的日志输出级别
    @param[in]  outmask           设置的日志输出级别
    */
    zce::LOG_PRIORITY set_log_priority(zce::LOG_PRIORITY out_level);
    /*!
    @brief      取得输出Level
    @return     zce::LOG_PRIORITY
    */
    zce::LOG_PRIORITY get_log_priority(void);

    /*!
    @brief      设置同步输出的位置
    @return     unsigned int
    @param      output_way
    */
    unsigned int set_output_way(int output_way);
    /*!
    @brief      设置默认输出的位置
    @return     unsigned int
    */
    unsigned int get_output_way(void);

    /*!
    @brief      实际写入日志的函数
    @param      outlevel    输出日志级别
    @param      lpszFormat  日志的格式化字符串
    @param      args        动态的日志参数列表组成的va_list
    */
    void vwrite_logmsg(LOG_PRIORITY outlevel,
                       const char* str_format,
                       va_list args) noexcept;

    /*!
    @brief      将日志的头部信息输出到一个Stringbuf中
    @param[in]  outlevel       输出的日志级别
    @param[in]  now_time       当前时间
    @param[out] log_tmp_buffer 头部信息输出的buffer
    @param[in]  sz_buf_len     buffer的长度
    @param[out] sz_use_len     使用了buffer的长度
    */
    void stringbuf_loghead(zce::LOG_PRIORITY outlevel,
                           const timeval& now_time,
                           char* log_tmp_buffer,
                           size_t sz_buf_len,
                           size_t& sz_use_len) noexcept;

    /*!
    @brief      设置是否线程同步
    @return     bool              旧（原）有的是否多线程同步值，
    @param      if_thread_synchro 是否进行多线程同步保护
    */
    bool set_thread_synchro(bool if_thread_synchro);
    /*!
    @brief      取得是否进行线程同步
    @return     bool   当前的是否多线程同步值
    */
    bool get_thread_synchro(void);

    /*!
    @brief      打开/关闭日志输出开关
    */
    void enable_output(bool enable_out);

    /*!
    @brief      输出文件日志信息
    @param      now_time         当前的时间
    @param      log_tmp_buffer   输出的buffer，
    @param      sz_use_len       输出的buffer长度
    */
    void output_log_info(const timeval& now_time,
                         char* log_tmp_buffer,
                         size_t sz_use_len) noexcept;

    template <typename... out_type >
    void foo_write_logmsg(LOG_PRIORITY outlevel,
                          const char* str_format,
                          const out_type &...out_data)
    {
        //如果日志输出开关关闭
        if (allow_output_log_ == false)
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

        //我要保留一个位置放'\0',还为\n考虑留一个空间,注意thread_local
        static thread_local char \
            log_tmp_buffer[Log_File::SIZE_OF_LOG_BUFFER];
        log_tmp_buffer[Log_File::SIZE_OF_LOG_BUFFER - 1] = '\0';

        size_t sz_buf_len = Log_File::SIZE_OF_LOG_BUFFER - 2;
        size_t sz_use_len = 0;

        stringbuf_loghead(outlevel,
                          now_time_val,
                          log_tmp_buffer,
                          sz_buf_len,
                          sz_use_len);

        sz_buf_len -= sz_use_len;

        //得到打印信息,str_nprintf 为自己内部的函数，str_format使用{}作为输出控制符
        size_t sprt_use_len = 0;
        str_nprintf(log_tmp_buffer + sz_use_len,
                    sz_buf_len,
                    sprt_use_len,
                    str_format,
                    out_data...);
        sz_use_len += sprt_use_len;
        sz_buf_len -= sprt_use_len;

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
public:

    //实例的赋值
    static void instance(Log_Msg*);
    //实例的获得
    static Log_Msg* instance();
    //清除实例
    static void clean_instance();

    //利用单子对象，Aseert调试
    static void debug_assert(const char* file_name,
                             const int file_line,
                             const char* fuction_name,
                             const char* expression_name) noexcept;

    ///利用单子对象，输出Aseert调试信息,增强版本函数
    static void debug_assert_ex(const char* file_name,
                                const int file_line,
                                const char* fuction_name,
                                const char* expression_name,
                                const char* out_string) noexcept;

    ///利用单子对象，打印日志信息
    static void write_logmsg(LOG_PRIORITY dbglevel,
                             const char* str_format,
                             ...) noexcept;

    template <typename... out_type >
    static void write_logplus(LOG_PRIORITY outlevel,
                              const char* str_format,
                              const out_type &...out_data) noexcept
    {
        log_instance_->foo_write_logmsg(outlevel,
                                        str_format,
                                        out_data...);
    }

public:

    //!根据字符串，得到日志级别
    static zce::LOG_PRIORITY log_priorities(const char* str_priority);

    //!根据字符串,得到日志分割方式的枚举
    static LOGFILE_DEVIDE log_file_devide(const char* str_devide);

    //!
#if defined DEBUG || defined _DEBUG
    static const int DEFUALT_LOG_OUTPUT = ((int)LOG_OUTPUT::LOGFILE | (int)LOG_OUTPUT::ERROUT);
#else
    static const int DEFUALT_LOG_OUTPUT = ((int)LOG_OUTPUT::LOGFILE;
#endif

protected:

    //!是否进行多线程的同步
    bool multithread_log_ = false;

    //!由于我内部还是使用的C++的ofstream 作为输出对象，所以我在多线程下还是使用了锁。
    std::mutex protect_lock_;

    ///输出的方式，LOG_OUTPUT的枚举值组合 @ref LOG_OUTPUT
    int output_way_ = (int)LOG_OUTPUT::LOGFILE | (int)LOG_OUTPUT::ERROUT;

    ///是否进行自动换行
    bool auto_new_line_ = true;

    //!默认记录的数据,按照和LOG_HEAD_RECORD_INFO 异或
    int head_record_ = (int)LOG_HEAD::CURRENTTIME | (int)LOG_HEAD::LOGLEVEL;

    //!输出日志信息的Mask值,小于这个信息的信息不予以输出
    zce::LOG_PRIORITY      permit_outlevel_ = RS_DEBUG;

    //!是否输出日志信息,可以用于暂时屏蔽
    bool allow_output_log_ = true;

    //!日志文件
    Log_File log_file_;

protected:

    ///单子实例指针
    static Log_Msg * log_instance_;
};
}