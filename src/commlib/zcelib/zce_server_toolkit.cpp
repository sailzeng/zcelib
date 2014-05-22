#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_flock.h"
#include "zce_os_adapt_process.h"
#include "zce_os_adapt_socket.h"
#include "zce_os_adapt_time.h"
#include "zce_trace_log_debug.h"
#include "zce_server_toolkit.h"

/*********************************************************************************
class ZCE_Server_Toolkit
*********************************************************************************/

// 构造函数,私有,使用单子类的实例,
ZCE_Server_Toolkit::ZCE_Server_Toolkit():
    pid_handle_(ZCE_INVALID_HANDLE),
    self_pid_(0),
    check_leak_times_(0),
    mem_checkpoint_size_(0),
    cur_mem_usesize_(0),
    process_cpu_ratio_(0),
    system_cpu_ratio_(0),
    mem_use_ratio_(0)
{
    memset(&last_process_perf_, 0, sizeof(last_process_perf_));
    memset(&now_process_perf_, 0, sizeof(now_process_perf_));
    memset(&last_system_perf_, 0, sizeof(last_system_perf_));
    memset(&now_system_perf_, 0, sizeof(now_system_perf_));

}

ZCE_Server_Toolkit::~ZCE_Server_Toolkit()
{
    // 关闭文件
    if (pid_handle_ != ZCE_INVALID_HANDLE)
    {
        ZCE_OS::flock(pid_handle_, LOCK_UN);
        ZCE_OS::close(pid_handle_);
    }
}

// 初始化
int ZCE_Server_Toolkit::socket_init()
{
    int ret = 0;
    ret = ZCE_OS::socket_init();

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}


/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2007年8月28日
Function        : ZCE_Server_Toolkit::out_pid_file
Return          : int ==0 表示成功
Parameter List  :
Param1: const char* pragramname  进程的名字，可以将argv[0]传递给我
Param2: bool lock_pid            是否对PID文件进行加锁处理，
Description     : 打印输出PID File
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int ZCE_Server_Toolkit::out_pid_file(const char *pragramname,
                                     bool lock_pid)
{
    int ret = 0;

    // std::string filename =  ACE::basename(pragramname);
    std::string filename = pragramname;
    filename += ".pid";

    // 设置文件读取参数,表示其他用户可以读取，open函数会自动帮忙调整参数的。
    int fileperms = 0644;

    pid_handle_ = ZCE_OS::open(filename.c_str(),
                               O_RDWR | O_CREAT,
                               static_cast<mode_t>(fileperms));

    if (pid_handle_ == ZCE_INVALID_HANDLE)
    {
        return -1;
    }

    // 写入文件内容
    const size_t BUFFER_LEN = 128;
    char tmpbuff[BUFFER_LEN + 1];

    // 这儿不对buffer做判断的原因是，肯定够长
    self_pid_ = ZCE_OS::getpid();
    int len = snprintf(tmpbuff, BUFFER_LEN, "%u", self_pid_ );

    // 截断文件为len，这点很重要,用了自己的OS 匹配层
    // 我是用WINDOWS下的记录锁模拟的文件锁，所以要先把文件长度调整对
    ZCE_OS::ftruncate(pid_handle_, len);

    // 如果PID文件要加锁
    if (lock_pid)
    {
        // 尝试锁定全部文件，为啥要try，见下面,这儿系统调用的是F_SETLK,
        ret = ZCE_OS::flock(pid_handle_, LOCK_EX | LOCK_NB);

        if (ret != 0)
        {
            return ret;
        }

        // 原来的代码有下面一行代码，但在WINDOWS下这个地方会阻塞，我疑惑和很久，
        // 为此代码还分了WINDOWS进行特殊处理
        // 后来发现其实是自己考虑错了，这根本就是不是一个double check的地方，try就会进行加锁。二了。
        // 锁定全部文件，不知道为何，，,这儿系统调用的是，F_SETLKW
        // ret = ZCE_OS::flock(pid_handle_,LOCK_EX);
    }

    ZCE_OS::write(pid_handle_, tmpbuff, static_cast<unsigned int>(len));

    // ZCE_OS::close(pid_handle_);

    return 0;
}



// 监测这个进程的系统状况,每N分钟运行一次就OK了
// 看门狗得到进程的状态
int ZCE_Server_Toolkit::watch_dog_status(bool first_record)
{
    int ret = 0;

    // 如果不是第一次记录，保存上一次记录的结果
    if (!first_record)
    {
        last_process_perf_ = now_process_perf_;
        last_system_perf_ = now_system_perf_;
    }

    ret = ZCE_OS::get_self_perf(&now_process_perf_);

    if (0 != ret)
    {
        return ret;
    }

    ret = ZCE_OS::get_system_perf(&now_system_perf_);

    if (0 != ret)
    {
        return ret;
    }

    cur_mem_usesize_ = now_process_perf_.vm_size_;

    // 记录第一次的内存数据
    if (first_record)
    {
        mem_checkpoint_size_ = now_process_perf_.vm_size_;
        return 0;
    }


    // 处理内存变化的情况
    size_t vary_mem_size = 0;

    if (now_process_perf_.vm_size_ >= mem_checkpoint_size_)
    {
        vary_mem_size = now_process_perf_.vm_size_ - mem_checkpoint_size_;
    }
    // 内存居然缩小了……
    else
    {
        mem_checkpoint_size_ = now_process_perf_.vm_size_;
    }

    // 这个告警如何向监控汇报要考虑一下
    if (vary_mem_size >= MEMORY_LEAK_THRESHOLD)
    {
        ++check_leak_times_;
        ZLOG_ERROR("[zcelib] [WATCHDOG][PID:%u] Monitor could memory leak,"
                   "mem_checkpoint_size_ =[%u],run_mem_size_=[%u].",
                   self_pid_,
                   mem_checkpoint_size_,
                   now_process_perf_.vm_size_);

        // 如果已经监测了若干次内存泄漏,则不再记录告警
        if (check_leak_times_ > MAX_RECORD_MEMORY_NUMBER)
        {
            mem_checkpoint_size_ = now_process_perf_.vm_size_;
            check_leak_times_ = 0;
        }
    }

    // 其实到这个地方了，你可以干的事情很多，
    // 甚至计算某一段时间内程序的CPU占用率过高(TNNND,后来我真做了)
    timeval last_to_now = ZCE_OS::timeval_sub(now_system_perf_.up_time_,
                                              last_system_perf_.up_time_);

    // 得到进程的CPU利用率
    timeval proc_utime = ZCE_OS::timeval_sub(now_process_perf_.run_utime_,
                                             last_process_perf_.run_utime_);
    timeval proc_stime = ZCE_OS::timeval_sub(now_process_perf_.run_stime_,
                                             last_process_perf_.run_stime_);
    timeval proc_cpu_time = ZCE_OS::timeval_add(proc_utime, proc_stime);

    // 如果间隔时间不为0
    if (ZCE_OS::total_milliseconds(last_to_now) > 0)
    {
        process_cpu_ratio_ = static_cast<uint32_t>(ZCE_OS::total_milliseconds(proc_cpu_time)
                                                   * 1000 / ZCE_OS::total_milliseconds(last_to_now));
    }
    else
    {
        process_cpu_ratio_ = 0;
    }

    ZCE_LOGMSG(RS_INFO,"[zcelib] [WATCHDOG][PID:%u] cpu ratio[%u] "
              "totoal process user/sys[%lld/%lld] milliseconds "
              "leave last point all/usr/sys[%lld/%lld/%lld] milliseconds "
              "memory use//add [%ld/%ld].",
              self_pid_,
              process_cpu_ratio_,
              ZCE_OS::total_milliseconds(now_process_perf_.run_utime_),
              ZCE_OS::total_milliseconds(now_process_perf_.run_stime_),
              ZCE_OS::total_milliseconds(last_to_now),
              ZCE_OS::total_milliseconds(proc_utime),
              ZCE_OS::total_milliseconds(proc_stime),
              cur_mem_usesize_,
              vary_mem_size);

    // 计算系统的CPU时间，非IDLE以外的时间都是消耗时间
    timeval sys_idletime = ZCE_OS::timeval_sub(now_system_perf_.idle_time_,
                                               last_system_perf_.idle_time_);
    timeval sys_cputime = ZCE_OS::timeval_sub(last_to_now, sys_idletime);

    // 如果间隔时间不为0
    if (ZCE_OS::total_milliseconds(last_to_now) > 0)
    {
        system_cpu_ratio_ =
            static_cast<uint32_t>(ZCE_OS::total_milliseconds(sys_cputime)
                                  * 1000 / ZCE_OS::total_milliseconds(last_to_now));
    }
    else
    {
        ZLOG_ERROR("system_uptime = %llu, process_start_time = %llu",
                   ZCE_OS::total_milliseconds(now_system_perf_.up_time_),
                   ZCE_OS::total_milliseconds(now_process_perf_.start_time_));
        system_cpu_ratio_ = 0;
    }

    // 系统或进程CPU使用超过阈值时记条账单
    if (process_cpu_ratio_ >= PROCESS_CPU_RATIO_THRESHOLD ||
        system_cpu_ratio_ >= SYSTEM_CPU_RATIO_THRESHOLD)
    {
        ZLOG_ERROR("[zcelib] [WATCHDOG][PID:%u] point[%u] vm_size[%u] "
                   "process cpu ratio [%f] threshold [%f], system cpu ratio[%f] threshold[%f] "
                   "totoal process user/sys[%lld/%lld] milliseconds "
                   "leave last point all/usr/sys[%lld/%lld/%lld] milliseconds.",
                   self_pid_,
                   mem_checkpoint_size_,
                   now_process_perf_.vm_size_,
                   double(process_cpu_ratio_) / 10,
                   double(PROCESS_CPU_RATIO_THRESHOLD) / 10,
                   double(system_cpu_ratio_) / 10,
                   double(SYSTEM_CPU_RATIO_THRESHOLD) / 10,
                   ZCE_OS::total_milliseconds(now_process_perf_.run_utime_),
                   ZCE_OS::total_milliseconds(now_process_perf_.run_stime_),
                   ZCE_OS::total_milliseconds(last_to_now),
                   ZCE_OS::total_milliseconds(proc_utime),
                   ZCE_OS::total_milliseconds(proc_stime));
    }

    // 内存使用情况的监控
    can_use_size_ = now_system_perf_.freeram_size_ +
                    now_system_perf_.cachedram_size_ +
                    now_system_perf_.bufferram_size_;

    if (now_system_perf_.totalram_size_ > 0)
    {
        mem_use_ratio_ = static_cast<uint32_t>((now_system_perf_.totalram_size_
                                                - can_use_size_) * 1000 / now_system_perf_.totalram_size_);
    }
    else
    {
        mem_use_ratio_ = 0;
    }

    ZCE_LOGMSG(RS_INFO,
              "[zcelib] [WATCHDOG][SYSTEM] cpu radio [%u] "
              "totoal usr/nice/sys/idle/iowait/hardirq/softirq "
              "[%lld/%lld/%lld/%lld/%lld/%lld/%lld] milliseconds"
              "leave last point all/use/idle[%lld/%lld/%lld] milliseconds "
              "mem ratio[%u] [totoal/can use/free/buffer/cache] "
              "[%lld/%lld/%lld/%lld/%lld] bytes",
              system_cpu_ratio_,
              ZCE_OS::total_milliseconds(now_system_perf_.user_time_),
              ZCE_OS::total_milliseconds(now_system_perf_.nice_time_),
              ZCE_OS::total_milliseconds(now_system_perf_.system_time_),
              ZCE_OS::total_milliseconds(now_system_perf_.idle_time_),
              ZCE_OS::total_milliseconds(now_system_perf_.iowait_time_),
              ZCE_OS::total_milliseconds(now_system_perf_.hardirq_time_),
              ZCE_OS::total_milliseconds(now_system_perf_.softirq_time_),
              ZCE_OS::total_milliseconds(last_to_now),
              ZCE_OS::total_milliseconds(sys_cputime),
              ZCE_OS::total_milliseconds(sys_idletime),
              mem_use_ratio_,
              now_system_perf_.totalram_size_,
              can_use_size_,
              now_system_perf_.freeram_size_,
              now_system_perf_.bufferram_size_,
              now_system_perf_.cachedram_size_);

    return 0;
}


// 获取运行状态统计
uint32_t ZCE_Server_Toolkit::get_sys_cpu_ratio(void) const
{
    return system_cpu_ratio_;
}

uint32_t ZCE_Server_Toolkit::get_app_cpu_ratio(void) const
{
    return process_cpu_ratio_;
}

uint32_t ZCE_Server_Toolkit::get_sys_mem_ratio(void) const
{
    return mem_use_ratio_;
}

uint64_t ZCE_Server_Toolkit::get_can_use_mem_size(void) const
{
    return can_use_size_;
}

