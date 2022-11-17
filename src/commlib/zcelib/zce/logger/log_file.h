/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_log_file.h
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

#include "zce/logger/log_comm.h"

//!日志文件的分割方法,以及对应的名称关系
//!默认的分割方法是按照时间.分割就是按照每天一个文件,文件名称中记录时间
//!如果按照文件SIZE,或者日志的行数分割文件,用一个ID标识文件日志文件
//!使用ID区分分隔日志有一个问题.就是每次初始化时要得到ID,否则要覆盖原有
//!日志文件,所以要查询原有最后一个日志文件,还要得到其的记录个数以及文件尺寸
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
class queue_buffer;

template<typename L, typename B> class buffer_pool;
typedef buffer_pool<std::recursive_mutex, queue_buffer> queue_buffer_pool_s;

struct LOG_RECORD
{
    //!
    timeval rec_time_;
    //!
    queue_buffer* rec_buf_ = nullptr;
};
//目标的前置声明，真尼玛麻烦。
template<typename T> class msgring_condi;

class log_file
{
protected:

    //time(nullptr)函数是一个比较耗时的操作,如果你对性能有强烈的癖好,可以使用时间戳提高效率
    //你自己使用定时器每秒改写定时器,然后所有的日志系统时间将使用这个时间戳
    //对于这方面ACE也没有用考虑,

public:

    ///构造函数,
    log_file();
    ///析构函数
    virtual ~log_file();

    ///
    log_file(const log_file &) = delete;
    log_file& operator=(const log_file&) = delete;

    /*!
    @brief      初始化函数，超级大集合型号,根据各种参数组合选择,
    @return     int                返回0标识初始化成功
    @param[in]  div_log_file
    @param[in]  log_file_prefix
    @param[in]  trunc_old
    @param[in]  thread_output_file 使用独立的线程打印
    @param[in]  max_size_log_file  日志文件的最大尺寸
    @param[in]  reserve_file_num   保留的日志文件数量，超过这个数量的日志将被删除
    */
    int open(int output_way,
             LOGFILE_DEVIDE div_log_file,
             const char* log_file_prefix,
             bool trunc_old,
             bool thread_output_file,
             size_t max_size_log_file,
             size_t reserve_file_num) noexcept;

    //!日志销毁
    void close();

    void fileout_log_info(const timeval& now_time,
                          char* log_tmp_buffer,
                          size_t sz_use_len) noexcept;
protected:

    /*!
    @brief      根据ID得到文件名称
    @param      logfileid     日志的ID
    @param      idlogfilename 生成的日志文件名称
    */
    void create_id_logname(size_t logfileid,
                           std::string& idlogfilename) noexcept;

    /*!
    @brief      根据日期得到文件名称
    @param      tmt         用于生成日志文件名称的时间戳
    @param      logfilename 生成的日志文件名称
    */
    void create_time_logname(const timeval& cur_time,
                             std::string& logfilename) noexcept;

    /*!
    @brief      处理超期的日志文件，
    */
    void del_old_logfile() noexcept;

    /*!
    @brief      生成配置信息,修改错误配置,
    */
    void make_configure(void) noexcept;

    /*!
    @brief      （如果有必要，）得到新的日志文件，
    @param      current_time  当前时间
    */
    void open_new_logfile(const timeval& current_time) noexcept;

    void thread_work();
public:

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

#if defined LARGE_LOG && LARGE_LOG == 1
    //!巨型日志
    static const size_t SIZE_OF_LOG_BUFFER = 512 * 1024;
#else
    //!日志的缓冲区的尺寸,这儿用了8K，很长了，末尾有是因为\0 和\n
    //!但如果直接用write 函数写，4096(-1)是一个更合适的值，
    static const size_t  SIZE_OF_LOG_BUFFER = 8192;
#endif
    //!
    static const size_t  SIZE_OF_BUCKET_ARY = 4;
    //!
    static const size_t  BUCKET_SIZE_ARY[SIZE_OF_BUCKET_ARY];
    //!
    static const size_t  POOL_INIT = 8;
    //!
    static const size_t  POOL_ONCE_EXTEND = 16;
    //!
    static const size_t  MAX_LEN_MSG_QUEUE = 4096;

protected:
    //!
    bool vaild_ = false;

    //日志分片的处理方式
    LOGFILE_DEVIDE div_log_file_ = LOGFILE_DEVIDE::NONE;

    ///日志文件名的前缀,包括路径
    std::string log_file_prefix_;

    ///日志的文件名称,当前是输出的日志文件名称
    std::string log_file_name_;

    ///日志文件的当前目录信息
    std::string log_file_dir_;

    ///对于线程安全,我的考虑如下,多进程模型,无需加锁,只用对多线程模型加锁,
    //多进程模型共用一个文件描述符,而且不共享文件缓冲区,所以不用考虑同步,
    //多线程模型,共享文件缓冲区(完全用堆栈可以解决),IO本身的缓冲也可能共享,所以必须同步。
    //而同步的点应该有两个，1.文件的更换，这个要避免几个人同时重入，2.向缓冲区写入的时候，
    //对于2，其实由于我写入的数据区长度最大只有4K，所以其实理论上可以逃避这个问题，
    //当然这样不能使用带有缓冲的输出,只能用write

    //!删除日志文件原有的信息
    bool trunc_old_ = false;

    //!线程输出日志
    bool thread_outfile_ = false;

    //!文件的最大尺寸
    size_t max_size_log_file_ = DEFAULT_LOG_SIZE;

    //!保留文件的个数,如果有太多文件要删除,为0表示不删除
    size_t reserve_file_num_ = DEFAULT_RESERVE_FILENUM;

    ///当前的大概时间,按小时记录,避免进行过多的时间判断
    time_t current_click_ = 1;

    //!日志文件的尺寸
    size_t size_log_file_ = 0;

    //!日志的文件句柄
    std::ofstream         log_file_handle_;

    //!时间日志文件列表，
    std::list<std::string> time_logfile_list_;
    //!日志输出线程是否处于运行状态
    bool out_thread_run_ = true;
    //!输出到文件的现场
    std::thread thread_outlog_;
    //!日志记录缓存的池子，
    zce::queue_buffer_pool_s *buf_pool_ = nullptr;

    //!日志记录的消息队列
    zce::msgring_condi<LOG_RECORD> *msg_queue_ = nullptr;
};
}