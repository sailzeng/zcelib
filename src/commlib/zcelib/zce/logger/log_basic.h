/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_log_basic.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2004年7月11日
* @brief
*
*
* @details    关于日志的优化，有2个方向也许是可以考虑的，
*             1.将锁的粒度降低，写日志的锁，一般在2个地方有用，日志文件文件名称变化，
*             日志要写入另外一个文件时，需要加锁。这种情况，几乎无法优化
*             第二个地方是写入文件地方，如果我们用了缓冲的写入方式，那么必须加锁，否
*             则输出的信息将乱掉。而这个是可能可以优化掉的，因为write函数在写入BUFFER
*             在4K以下的情况时，其实是不需要加锁的，（但这个特性在LINUX下是正确的，在其
*             他平台呢？并不好好证明。），如果这个地方直接用wirte函数，那么可以缩小加锁
*             的范围，
*             2.第二个优化的方向就是缓冲的问题，日志的输出一般是不能用缓冲的，这是日志的
*             特点决定了的，但文件输出使用缓冲和不实用缓冲，速度那个差别是相当相当大喔。
*             所以能否有一种方式，让用户明确自己调用缓冲？比如自己在定时器中调用缓冲，
*             当然这个方案最大的问题是如果发生崩溃……，会不会丢日志呢、（当然可以截获
*             信号等，但这种方式如何通用呢）
*
*
* @note
*
*/

#pragma once

#include "zce/util/non_copyable.h"
#include "zce/lock/thread_mutex.h"
#include "zce/logger/priority.h"

///日志文件的分割方法,以及对应的名称关系
///默认的分割方法是按照时间.分割就是按照每天一个文件,文件名称中记录时间
///如果按照文件SIZE,或者日志的行数分割文件,用一个ID标识文件日志文件
///使用ID区分分隔日志有一个问题.就是每次初始化时要得到ID,否则要覆盖原有
///日志文件,所以要查询原有最后一个日志文件,还要得到其的记录个数以及文件尺寸
enum class LOGFILE_DEVIDE
{
    ///记录单个个日志文件,不分割
    NONE = 1,

    ///按照尺寸记录分割日志文件,用ID标识,这个ID的意思是滚动处理的ID，
    ///当前的日志文件是.log这个文件，其他的就有日志文件名称是1-N，当前处理的文件
    ///写满后，删除第N个文件，其他文件依次改名，当前文件改名为log.1,然后新生成一个
    ///文件作为当前写入的日志文件。这种方式好处是大小绝对固定，好处是文件名称总在变
    ///化，并不利于维护和管理
    ///在32位的机器，分割日志的大小不要超过4G，64位理论无限制,代码内部限制是16G
    BY_SIZE_NAME_ID = 101,

    ///按照小时分割日志,
    BY_TIME_HOUR = 201,
    ///按照6个小时分割日志,1天分割为4个
    BY_TIME_SIX_HOUR = 202,
    ///按照日期分割日志,文件按天记录
    BY_TIME_DAY = 203,
    ///按照月份分割日志,
    BY_TIME_MONTH = 204,
    ///按照时间分割日志,文件按年记录
    BY_TIME_YEAR = 205,

    //文件名称按毫秒记录,日志按大小分割，
    BY_TIME_NAME_MILLISECOND = 301,
};
//本来有意图实现这个类别，但是感觉总不完善，放弃，设计就是一种取舍
//LOGDEVIDE_BY_RECORD                按照记录的个数(不是行数)目分割日志文件,用ID标识,不推荐使用

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

/*!
@brief      日志输出的基本功能都在这个类里面，包括线程同步，文件名称，文件大小控制，
            日志的输出，各种初始化，控制函数，
            他的扩张类都是都是在函数接口上进行了封装，
            LOG_MSG  C printf的字符串格式化输出，和我们常用的方式一样，基本上
            内部和其他封装都是依据这个类来的，
            LOG_PLUS CPP ，一种依靠C++模版的输出方式，某种程度上提高了字符串格
            式化的性能，（注意性能瓶颈大头在文件输出那块），内部用的函数
            zce::foo_snprintf，

*/
namespace zce
{
class LogTrace_Base : public zce::NON_Copyable
{
protected:

    //time(NULL)函数是一个比较耗时的操作,如果你对性能有强烈的癖好,可以使用时间戳提高效率
    //你自己使用定时器每秒改写定时器,然后所有的日志系统时间将使用这个时间戳
    //对于这方面ACE也没有用考虑,

public:

    ///构造函数,
    LogTrace_Base();
    ///析构函数
    virtual ~LogTrace_Base();

    /*!
    @brief      初始化函数,用于时间分割日志的构造
    @return     int               返回0标识初始化成功
    @param[in]  div_log_file      分割日志的方式
    @param[in]  log_file_prefix   日志的前缀
    @param[in]  reserve_file_num  保留的日志文件数量，超过这个数量的日志将被删除
    @param[in]  trunc_old         是否截断原有的日志文件的信息，
    @param[in]  is_thread_synchro 是否进行线程同步，
    @param[in]  auto_new_line     日志记录的末尾是否自动的换行，new一行
    @param[in]  output_way        日志输出的方式,可以多种方式并存，参考 @ref LOG_OUTPUT
    @param[in]  head_record       日志头部包含的信息包括，参考 @ref LOG_HEAD_RECORD_INFO

    */
    int init_time_log(LOGFILE_DEVIDE div_log_file,
                      const char* log_file_prefix,
                      size_t reserve_file_num = DEFAULT_RESERVE_FILENUM,
                      bool trunc_old = false,
                      bool is_thread_synchro = false,
                      bool auto_new_line = true,
                      int output_way = (int)LOG_OUTPUT::LOGFILE | (int)LOG_OUTPUT::ERROUT,
                      int head_record = (int)LOG_HEAD::CURRENTTIME | (int)LOG_HEAD::LOGLEVEL);

    /*!
    @brief      初始化函数,用于尺寸分割日志的构造 内部的 ZCE_LOGFILE_DEVIDE_NAME = LOGDEVIDE_BY_SIZE
    @return     int                返回0标识初始化成功
    @param[in]  log_file_prefix    日志的前缀
    @param[in]  max_size_log_file  日志文件的最大尺寸，目前最大尺寸内部用的4G
    @param[in]  trunc_old          是否截断原有的日志文件的信息，
    @param[in]  is_thread_synchro  是否进行线程同步
    @param[in]  auto_new_line      日志记录的末尾是否自动的换行，new一行
    @param[in]  reserve_file_num   保留的日志文件数量，超过这个数量的日志将被删除
    @param[in]  output_way         日志输出的方式，参考 @ref LOG_OUTPUT
    @param[in]  head_record        日志头部包含的信息包括，参考 @ref LOG_HEAD_RECORD_INFO
    */
    int init_size_log(const char* log_file_prefix,
                      size_t max_size_log_file = DEFAULT_LOG_SIZE,
                      unsigned int reserve_file_num = DEFAULT_RESERVE_FILENUM,
                      bool trunc_old = false,
                      bool is_thread_synchro = false,
                      bool auto_new_line = true,
                      int output_way = (int)LOG_OUTPUT::LOGFILE | (int)LOG_OUTPUT::ERROUT,
                      int head_record = (int)LOG_HEAD::CURRENTTIME | (int)LOG_HEAD::LOGLEVEL);

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
                    bool is_thread_synchro = false,
                    int head_record = (int)LOG_HEAD::CURRENTTIME | (int)LOG_HEAD::LOGLEVEL);

    /*!
    @brief      初始化函数，超级大集合型号,根据各种参数组合选择,
    @return     int                返回0标识初始化成功
    @param[in]  output_way        日志输出的方式,可以多种方式并存，参考 @ref LOG_OUTPUT
    @param[in]  div_log_file
    @param[in]  log_file_prefix
    @param[in]  is_thread_synchro
    @param[in]  auto_new_line
    @param[in]  max_size_log_file 日志文件的最大尺寸
    @param[in]  reserve_file_num  保留的日志文件数量，超过这个数量的日志将被删除
    @param[in]  head_record       日志头部包含的信息包括，参考 @ref LOG_HEAD_RECORD_INFO
    */
    int initialize(unsigned int output_way,
                   LOGFILE_DEVIDE div_log_file,
                   const char* log_file_prefix,
                   bool trunc_old,
                   bool is_thread_synchro,
                   bool auto_new_line,
                   size_t max_size_log_file,
                   size_t reserve_file_num,
                   unsigned int head_record);

    /*!
    @brief      关闭日志，注意关闭后，必须重新初始化
    */
    void terminate();

    /*!
    @brief      打开/关闭日志输出开关
    */
    void enable_output(bool enable_out);

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
    @brief      设置默认输出的信息类型
    @return     unsigned int
    @param      recdinfo
    */
    unsigned int set_log_head(unsigned int recdinfo);
    /*!
    @brief      取得默认输出的信息类型
    @return     unsigned int
    */
    unsigned int get_log_head(void);

    /*!
    @brief      设置同步输出的位置
    @return     unsigned int
    @param      output_way
    */
    unsigned int set_output_way(unsigned int output_way);
    /*!
    @brief      设置默认输出的位置
    @return     unsigned int
    */
    unsigned int get_output_way(void);

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
    @brief      输出文件日志信息
    @param      now_time         当前的时间
    @param      log_tmp_buffer   输出的buffer，
    @param      sz_use_len       输出的buffer长度
    */
    void output_log_info(const timeval& now_time,
                         char* log_tmp_buffer,
                         size_t sz_use_len);

protected:

    /*!
    @brief      根据ID得到文件名称
    @param      logfileid     日志的ID
    @param      idlogfilename 生成的日志文件名称
    */
    void create_id_logname(size_t logfileid,
                           std::string& idlogfilename);

    /*!
    @brief      根据日期得到文件名称
    @param      tmt         用于生成日志文件名称的时间戳
    @param      logfilename 生成的日志文件名称
    */
    void create_time_logname(const timeval& cur_time,
                             std::string& logfilename);

    /*!
    @brief      处理超期的日志文件，
    */
    void del_old_logfile();

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
                           size_t& sz_use_len);

    /*!
    @brief      生成配置信息,修改错误配置,
    */
    void make_configure(void);

    /*!
    @brief      （如果有必要，）得到新的日志文件，
    @param      init     是否是初始化阶段
    @param      current_time  当前时间
    */
    void open_new_logfile(bool initiate, const timeval& current_time);

public:

    ///根据字符串，得到日志级别
    static zce::LOG_PRIORITY log_priorities(const char* str_priority);

    ///根据字符串,得到日志分割方式的枚举
    static LOGFILE_DEVIDE log_file_devide(const char* str_devide);

protected:

    ///默认的保留的文件的数量
    static const size_t DEFAULT_RESERVE_FILENUM = 100;
    ///最大的保留的文件的数量
    static const size_t MAX_RESERVE_FILENUM = 9999;
    ///最小的保留的文件的数量
    static const size_t MIN_RESERVE_FILENUM = 3;

    ///日志文件的最小允许尺寸是8M
    static const size_t MIN_LOG_SIZE = 4096000UL;
    ///日志文件的最大允许尺寸是4G
    static const size_t MAX_LOG_SIZE = 4096000000UL;
    ///日志文件的默认允许尺寸是40M
    static const size_t DEFAULT_LOG_SIZE = 40960000UL;

    ///日志的统一后缀长度
    static const size_t LEN_LOG_POSTFIX = 4;
    ///日志后缀的4个字母.log，什么，你想用.tlog？BS
    static const char STR_LOG_POSTFIX[LEN_LOG_POSTFIX + 1];

protected:

    ///日志分片的处理方式
    LOGFILE_DEVIDE div_log_file_ = LOGFILE_DEVIDE::NONE;

    ///日志文件名的前缀,包括路径
    std::string log_file_prefix_;

    ///日志的文件名称,当前是输出的日志文件名称
    std::string log_file_name_;

    ///日志文件的当前目录信息
    std::string log_file_dir_;

    ///输出的方式，LOG_OUTPUT的枚举值组合 @ref LOG_OUTPUT
    int output_way_ = (int)LOG_OUTPUT::LOGFILE | (int)LOG_OUTPUT::ERROUT;

    ///对于线程安全,我的考虑如下,多进程模型,无需加锁,只用对多线程模型加锁,
    //多进程模型共用一个文件描述符,而且不共享文件缓冲区,所以不用考虑同步,
    //多线程模型,共享文件缓冲区(完全用堆栈可以解决),IO本身的缓冲也可能共享,所以必须同步。
    //而同步的点应该有两个，1.文件的更换，这个要避免几个人同时重入，2.向缓冲区写入的时候，
    //对于2，其实由于我写入的数据区长度最大只有4K，所以其实理论上可以逃避这个问题，当然这样不能使用带有缓冲的输出,只能用write

    ///是否进行多线程的同步
    bool is_thread_synchro_ = false;

    ///同步锁
    Thread_Light_Mutex protect_lock_;

    ///是否进行自动换行
    bool                   auto_new_line_ = true;

    bool                   trunc_old_ = false;

    ///文件的最大尺寸
    size_t                 max_size_log_file_ = DEFAULT_LOG_SIZE;

    ///保留文件的个数,如果有太多文件要删除,为0表示不删除
    size_t                 reserve_file_num_ = DEFAULT_RESERVE_FILENUM;

    ///默认记录的数据,按照和LOG_HEAD_RECORD_INFO 异或
    int                    record_info_ = (int)LOG_HEAD::CURRENTTIME | (int)LOG_HEAD::LOGLEVEL;

    ///当前的大概时间,按小时记录,避免进行过多的时间判断
    time_t                 current_click_ = 1;

    ///输出日志信息的Mask值,小于这个信息的信息不予以输出
    zce::LOG_PRIORITY      permit_outlevel_ = RS_DEBUG;

    ///日志文件的尺寸
    size_t                 size_log_file_ = 0;

    ///是否输出日志信息,可以用于暂时屏蔽
    bool                   is_output_log_ = true;

    ///日志的文件句柄
    std::ofstream         log_file_handle_;

    //r
    std::list<std::string> time_logfile_list_;
};
}