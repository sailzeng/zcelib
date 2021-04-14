/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/sysinfo.h
* @author     Pascalshen <pascalshen@tencent.com>  Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年11月1日
* @brief      得到操作系统的各种信息，包括，内存大小，CPU个数
*             在LINUX下，使用/proc文件系统，和sysconf，systinfo 这些函数，
*             在WINDOWS，使用GlobalMemoryStatusEx，GetSystemInfo，函数等
*             为什么不直接模拟sysconf 和sysinfo函数,为啥要这样呢，也是倒霉催的，
*             因为sysinfo的结构其实也是一个升级的结果，
*             原来2.3.23前面的版本以前的版本内存尺寸不支持
*
* @details    对于函数接口，我总是犹豫来犹豫去，一开始，我模拟sysconf，systinfo，
*             结果发现sysinfo在内存处理上比较变态，我又倾向了单独接口，结果写了一会，
*             发现冗余的变态，又犹豫是否写回sysconf，systinfo,但仔细看看systinfo的结构又TMD烦恼。
*             算了，算了，我自己给个结构吧。让TMD兼容去死。
*
* @note       另外：Windows和LINUX在获取系统信息，进程信息这些事情上基本都是一群打酱油的主，
*             大家对到底什么都是是系统的，什么东西是进程的都比较含混
*             LINUX下有函数getrlimit,(ulimit已经不用了),sysconf，sysinfo，等，
*             Windows下有函数GlobalMemoryStatusEx，GetSystemInfo，
*             而且WINDOWS对于限制这个东西，很多时候采取的是不太限制的态度，比如他没有最大线程
*             个数限制，没有最大进程个数限制，等等，他的目标就是用完为止
*
*             听了一首有趣的歌，《流川枫和苍井空》黑撒的，莫名的想抽烟，。
*/

#ifndef ZCE_LIB_OS_ADAPT_SYSINFO_H_
#define ZCE_LIB_OS_ADAPT_SYSINFO_H_

#include "zce/os_adapt/common.h"

//TODO:还可以增加一些IO处理，NET的系统数据数据。但WINDOWS下得到这些数据就有点麻烦了，要依靠注册表，TNNND

/// @brief 系统的信息数据，包括CPU个数，内存大小等
struct ZCE_SYSTEM_INFO
{
    ///CPU数量
    int       nprocs_conf_;
    ///有效的CPU数量
    int       nprocs_av_;

    ///处理器的主频
    int64_t   cpu_hz_;

    ///总共的内存大小，单位字节
    int64_t  totalram_size_;
    ///空闲内存的大小，单位字节,其实在LINUX系统中，可以使用的内存是bufferram_size_ +  freeram_size_
    int64_t  freeram_size_;

    ///共享内存区的大小，WINDOWS下这个数值没有用
    int64_t   shared_size_;
    ///用于IO缓冲的buffer的大小，WINDOWS下这个数值没有用，恒定为0
    int64_t   bufferram_size_;

    ///总计的交互分区大小 Total swap space size
    int64_t   totalswap_size_;
    ///有效的交换分区大小 swap space still available
    int64_t   freeswap_size_;

};

///@brief 系统的性能信息数据，包括CPU个数，内存大小等
struct ZCE_SYSTEM_PERFORMANCE
{

    ///总共的内存大小，单位字节
    int64_t   totalram_size_;
    ///空闲内存的大小，单位字节,其实在LINUX系统中，可以使用的内存是bufferram_size_ + cachedram_size_ + freeram_size_
    int64_t   freeram_size_;

    ///共享内存区的大小，WINDOWS下这个数值没有用
    int64_t   shared_size_;
    ///用于IO缓冲的buffer的大小(用于写)，WINDOWS下这个数值没有用，恒定为0
    int64_t   bufferram_size_;
    ///用于IO缓冲的cached的大小(用于读取)，WINDOWS下这个数值没有用，恒定为0
    int64_t   cachedram_size_;

    ///总计的交互分区大小 Total swap space size
    int64_t   totalswap_size_;
    ///有效的交换分区大小 swap space still available
    int64_t   freeswap_size_;
    ///交互分区的尺寸
    int64_t   swapcached_size_;

    ///服务器启动时间
    timeval   up_time_;

    ///用户态时间
    timeval   user_time_;
    ///NICE为负数的进程所使用的CPU时间,其实这不应该叫NICE TIME，应该是NO NICETime，WINDOWS下这个数值没有用，恒定为0
    ///NICE为负是指只有root才能设定的一种优先级，（好像LINUX会用于实时操作系统的某些特质）
    timeval   nice_time_;
    ///系统态时间
    timeval   system_time_;
    ///空闲时间
    timeval   idle_time_;
    ///iowait的时间，WINDOWS下无效，恒定为0
    timeval   iowait_time_;
    ///中断的时间，WINDOWS下无效，恒定为0
    timeval   hardirq_time_;
    ///软中断的时间，WINDOWS下无效，恒定为0
    timeval   softirq_time_;

    ///系统的1分钟，5分钟15分钟的平均负载，WINDOWS下这个数值没有用,恒定为0.0
    double    sys_loads_[3];

    ///当前的进程数量Number of current processes
    int       processes_num_;
    ///正在运行的进程任务数量，WINDOWS下这个数值没有用，恒定为0
    int       running_num_;

};

namespace zce
{

//----------------------------------------------------------------------------------------------

/*!
* @brief      得到内存使用情况,
* @return     int 0成功，-1失败
* @param      zce_system_info  返回的系统信息 @ref ZCE_SYSTEM_INFO
*/
int get_system_info(ZCE_SYSTEM_INFO *zce_system_info);

//
/*!
* @brief      得到操作系统的性能信息
* @return     int  0成功，-1失败
* @param      zce_system_perf  返回的系统性能信息  @ref ZCE_SYSTEM_PERFORMANCE
*/
int get_system_perf(ZCE_SYSTEM_PERFORMANCE *zce_system_perf);

#if defined ZCE_OS_LINUX

///LINUX下读取proc目录得到系统的性能信息
int read_proc_get_systemperf(struct ZCE_SYSTEM_PERFORMANCE *info);

///通过系统函数得到系统的性能信息，内部没有使用这个函数，写它主要是为了熟悉相关知识。
int read_fun_get_systemperf(struct ZCE_SYSTEM_PERFORMANCE *info);

#endif //end of #if define ZCE_OS_LINUX

};

#endif //ZCE_LIB_OS_ADAPT_SYSCONF_H_

