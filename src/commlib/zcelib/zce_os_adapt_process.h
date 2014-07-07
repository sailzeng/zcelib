/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_process.h
* @author     Sailzeng <sailerzeng@gmail.com>  pascalshen <pascalshen@tencent.com>
* @version
* @date       2013年1月3日
* @brief      进程相关的
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_OS_ADAPT_PROCESS_H_
#define ZCE_LIB_OS_ADAPT_PROCESS_H_

#include "zce_os_adapt_predefine.h"

///@brief      进程下的性能数据信息，
struct ZCE_PROCESS_PERFORM
{
    //进程ID
    pid_t         process_id_;

    //进程运行的时间
    timeval       running_time_;
    // 进程启动时间
    timeval       start_time_;
    //运行时的系统时间要求
    timeval       run_stime_;
    //运行时间的用户时间
    timeval       run_utime_;

    //优先级
    int           priority_;
    //NICE数据
    int           nice_;

    //占用的虚拟内存大小，也就是总内存大小
    size_t        vm_size_;
    //占用物理内存的大小，Resident Set Size: number of pages the process has in real    memory.,WINDOWS下这个值不可靠，我找了半天也没有找到合适的值
    size_t        resident_set_;

    //共享的内存的大小,WINDOWS下没有这个值无效
    size_t        shared_size_;
    //code size 代码的大小,WINDOWS下没有这个值无效
    size_t        text_size_;
    //data + stack，data + 堆栈的大小,WINDOWS下没有这个值无效
    size_t        data_size_;

};

namespace ZCE_OS
{

///得到当前的进程ID
pid_t getpid();

///得到当前的进程的父进程ID
pid_t getppid (void);

///fork
pid_t fork (void);

///取得某个环境变量
char *getenv(const char *name);

///
pid_t setsid (void);

///signal信号处理函数，放在这儿其实有点异类，呵呵
sighandler_t signal (int signum,
                     sighandler_t);

//--------------------------------------------------------
//非标准函数，得到进程的各种性能数据

///得到进程允许的性能信息，包括CPU时间，内存使用情况等信息
int get_self_perf(ZCE_PROCESS_PERFORM *prc_perf_info);

///取得谋个进程的性能信息
int get_process_perf(pid_t process_id, ZCE_PROCESS_PERFORM *prc_perf_info);

};

#endif //ZCE_LIB_OS_ADAPT_TIME_H_

