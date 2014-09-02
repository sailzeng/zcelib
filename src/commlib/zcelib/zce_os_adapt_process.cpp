
#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_string.h"
#include "zce_os_adapt_time.h"
#include "zce_trace_log_debug.h"
#include "zce_os_adapt_process.h"

//得到当前的进程ID
pid_t ZCE_LIB::getpid()
{
#if defined (ZCE_OS_WINDOWS)
    return ::GetCurrentProcessId ();
#elif defined (ZCE_OS_LINUX)
    return ::getpid ();
#endif
}

//得到当前的进程的父进程ID
pid_t ZCE_LIB::getppid (void)
{

#if defined (ZCE_OS_WINDOWS)

    //其实微软没有明确说明有这个方法，这段代码完全改于网上
    //参考链接 http://hi.baidu.com/woaiwlaopo/blog/item/ecaf5760b42495db8cb10d4b.html

    typedef struct
    {
        DWORD ExitStatus;
        DWORD PebBaseAddress;
        DWORD AffinityMask;
        DWORD BasePriority;
        ULONG UniqueProcessId;
        ULONG InheritedFromUniqueProcessId;
    }   PROCESS_BASIC_INFORMATION;

    typedef LONG (WINAPI * PROCNTQSIP)(HANDLE, UINT, PVOID, ULONG, PULONG);

    DWORD dwId = ::GetCurrentProcessId ();

    LONG                      status;
    DWORD                     dwParentPID = (DWORD)(-1);
    HANDLE                    hProcess;
    PROCESS_BASIC_INFORMATION pbi;

    PROCNTQSIP NtQueryInformationProcess;

    //这个函数是不公开的
    NtQueryInformationProcess = (PROCNTQSIP)GetProcAddress(
                                    GetModuleHandleA("ntdll"),
                                    "NtQueryInformationProcess"
                                );

    if (NtQueryInformationProcess)
    {
        return DWORD (-1);
    }

    // Get process handle
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwId);

    if (!hProcess)
    {
        return (DWORD) - 1;
    }

    // Retrieve information
    status = NtQueryInformationProcess( hProcess,
                                        0,
                                        (PVOID)&pbi,
                                        sizeof(PROCESS_BASIC_INFORMATION),
                                        NULL
                                      );

    // Copy parent Id on success
    if (!status)
    {
        dwParentPID = pbi.InheritedFromUniqueProcessId;
    }

    CloseHandle (hProcess);

    return dwParentPID;

#elif defined (ZCE_OS_LINUX)
    return ::getppid ();
#endif
}

//fork
pid_t ZCE_LIB::fork (void)
{
#if defined (ZCE_OS_WINDOWS)
    //不支持了，呵呵，好像ACE有段时间蹩脚的支持过，后来也不搞了，
    //的确没有法子支持的比较好。
    return (pid_t (-1));
#elif defined (ZCE_OS_LINUX)
    return ::fork ();
#endif
}

//
pid_t ZCE_LIB::setsid (void)
{
#if defined (ZCE_OS_WINDOWS)
    //不支持了，
    return (pid_t (-1));
#elif defined (ZCE_OS_LINUX)
    return ::setsid ();
#endif
}

//取得某个环境变量
char *ZCE_LIB::getenv(const char *name)
{
    //两个环境都有，这个封装有点多余
    return ::getenv(name);
}

sighandler_t ZCE_LIB::signal (int signum,
                             sighandler_t  func )
{

    //如果这个信号，在这个平台不支持，就放弃把
    if (signum == 0)
    {
        return 0;
    }

#if defined (ZCE_OS_WINDOWS)

    return ::signal(signum, func);

#elif defined (ZCE_OS_LINUX)

    return ::signal(signum, func);
#endif
}

#if defined ZCE_OS_LINUX

namespace ZCE_LIB
{

//
static int read_proc_get_processstat(pid_t read_pid, ZCE_PROCESS_PERFORM *prc_perf_info)
{
    //得到进程的文件名称
    const char *PROC_PROCESS_STAT = "/proc/%u/stat";
    char file_name[PATH_MAX + 1];
    file_name[PATH_MAX] = '\0';
    snprintf(file_name, PATH_MAX, PROC_PROCESS_STAT, read_pid);

    char buffer[4096];
    size_t read_len = 0;
    uint64_t read_data = 0;
    //读取/proc下的文件
    int ret = ZCE_LIB::read_file_data(file_name, buffer, sizeof(buffer), &read_len);

    if ( 0 != ret )
    {
        return ret;
    }

    const char *in_para = buffer;
    char *out_para = NULL;

    in_para = ZCE_LIB::skip_token(in_para);              /* skip  pid*/
    in_para = ZCE_LIB::skip_token(in_para);              /* skip  name*/
    in_para = ZCE_LIB::skip_token(in_para);              /* skip  state*/
    in_para = ZCE_LIB::skip_token(in_para);              /* skip ppid */
    in_para = ZCE_LIB::skip_token(in_para);              /* skip pgrp */
    in_para = ZCE_LIB::skip_token(in_para);              /* skip session */
    in_para = ZCE_LIB::skip_token(in_para);              /* skip tty */
    in_para = ZCE_LIB::skip_token(in_para);              /* skip tty pgrp */
    in_para = ZCE_LIB::skip_token(in_para);              /* skip flags */
    in_para = ZCE_LIB::skip_token(in_para);              /* skip min flt */
    in_para = ZCE_LIB::skip_token(in_para);              /* skip cmin flt */
    in_para = ZCE_LIB::skip_token(in_para);              /* skip maj flt */
    in_para = ZCE_LIB::skip_token(in_para);              /* skip cmaj flt */

#if defined ZCE_OS_LINUX
    // tlinux64(2.6.32.43)下这个值是100
    long cpu_tick_precision = ::sysconf(_SC_CLK_TCK);
#else
    long cpu_tick_precision = 1000;
#endif

    const uint64_t SEC_PER_USEC = 1000000LL;

    // utime
    read_data = ::strtoull(in_para, &out_para, 10);
    in_para = out_para;
    prc_perf_info->run_utime_.tv_sec =  static_cast<time_t>(  read_data / cpu_tick_precision);
    prc_perf_info->run_utime_.tv_usec =  static_cast<time_t>((read_data % cpu_tick_precision) * (SEC_PER_USEC / cpu_tick_precision));

    // stime
    read_data = ::strtoull(in_para, &out_para, 10);
    in_para = out_para;
    prc_perf_info->run_stime_.tv_sec =  static_cast<time_t>(  read_data / cpu_tick_precision);
    prc_perf_info->run_stime_.tv_usec =  static_cast<time_t>((read_data % cpu_tick_precision) * (SEC_PER_USEC / cpu_tick_precision));

    in_para = skip_token(in_para);              /* skip cutime */
    in_para = skip_token(in_para);              /* skip cstime */

    //* priority
    prc_perf_info->priority_ = ::strtol(in_para, &out_para, 10);
    in_para = out_para;
    //* nice
    prc_perf_info->nice_ = ::strtol(in_para, &out_para, 10);
    in_para = out_para;

    in_para = skip_token(in_para);              /* skip number_threads */
    in_para = skip_token(in_para);              /* skip it_real_value */

    // 获取进程的启动时间start_time(unit：jiffies)
    read_data = ::strtoull(in_para, &out_para, 10);
    in_para = out_para;
    prc_perf_info->start_time_.tv_sec =  static_cast<time_t>( read_data / cpu_tick_precision);
    prc_perf_info->start_time_.tv_usec =  static_cast<time_t>((read_data % cpu_tick_precision) * (SEC_PER_USEC / cpu_tick_precision));

    // 计算running_time，uptime可以读取/proc/uptime得到, 这里使用系统zcelib自带的ZCE_LIB::get_uptime()
    prc_perf_info->running_time_ = ZCE_LIB::timeval_sub(ZCE_LIB::get_uptime(), prc_perf_info->start_time_);

    //内存信息我从另外一个文件读取

    return 0;
}

//
static int read_proc_get_processmem(pid_t read_pid, ZCE_PROCESS_PERFORM *prc_perf_info)
{
    const char *PROC_PROCESS_MEM = "/proc/%u/statm";
    char file_name[PATH_MAX + 1];
    file_name[PATH_MAX] = '\0';
    snprintf(file_name, PATH_MAX, PROC_PROCESS_MEM, read_pid);

    char buffer[4096];
    size_t read_len = 0;
    //读取/proc下的文件
    int ret = ZCE_LIB::read_file_data(file_name, buffer, sizeof(buffer), &read_len);

    if ( 0 != ret )
    {
        return ret;
    }

    const char *in_para = buffer;
    char *out_para = NULL;

    //64位下应该是8096
    long page_size = 4096;
#if defined ZCE_OS_LINUX
    page_size = ::sysconf(_SC_PAGESIZE);
#endif

    //total program size
    prc_perf_info->vm_size_ = static_cast<size_t>( ::strtol(in_para, &out_para, 10) * page_size );
    in_para = out_para;
    //resident set size
    prc_perf_info->resident_set_ = static_cast<size_t>( ::strtol(in_para, &out_para, 10) * page_size );
    in_para = out_para;
    //shared pages (from shared mappings)
    prc_perf_info->shared_size_ = static_cast<size_t>( ::strtol(in_para, &out_para, 10) * page_size );
    in_para = out_para;
    //text (code)
    prc_perf_info->text_size_ = static_cast<size_t>( ::strtol(in_para, &out_para, 10) * page_size );
    in_para = out_para;
    // skip library (unused in Linux 2.6)
    in_para = skip_token(in_para);
    //data + stack
    prc_perf_info->data_size_ = static_cast<size_t>( ::strtol(in_para, &out_para, 10) * page_size );
    in_para = out_para;

    return 0;
}

}

#endif //#if defined ZCE_OS_LINUX

//得到进程允许的性能信息，包括CPU时间，内存使用情况等信息
int ZCE_LIB::get_self_perf(ZCE_PROCESS_PERFORM *prc_perf_info)
{
    pid_t process_id = ZCE_LIB::getpid();

    return ZCE_LIB::get_process_perf(process_id, prc_perf_info);
}

//取得谋个进程的性能信息
int ZCE_LIB::get_process_perf(pid_t process_id, ZCE_PROCESS_PERFORM *prc_perf_info)
{

    prc_perf_info->process_id_ = process_id;

#if defined (ZCE_OS_WINDOWS)

    //得到自己的进程句柄，
    //HANDLE process_handle = ::GetCurrentProcess();

    //根据进程ID得到进程句柄
    HANDLE process_handle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                          false,
                                          process_id);

    if (ZCE_INVALID_HANDLE == process_handle)
    {
        ZLOG_ERROR("[zcelib] OpenProcess error, reson is:%u.", ::GetLastError());
        return -1;
    }

    FILETIME ft_start_time;
    FILETIME ft_end_time;
    FILETIME ru_stime;
    FILETIME ru_utime;

    BOOL bret =::GetProcessTimes (process_handle,
                                  &ft_start_time,   // start
                                  &ft_end_time,     // exited,如果进程没有结束，结果未定义
                                  &ru_stime,
                                  &ru_utime);

    //如果返回成功
    if (bret == FALSE)
    {
        ZLOG_ERROR("[zcelib] GetProcessTimes error, reson is:%u.", ::GetLastError());
        return -1;
    }

    timeval start_time = ZCE_LIB::make_timeval(&ft_start_time);
    timeval now_time = ZCE_LIB::gettimeofday();

    prc_perf_info->running_time_ = ZCE_LIB::timeval_sub(now_time, start_time);
    prc_perf_info->run_stime_ = ZCE_LIB::make_timeval2(&ru_stime);
    prc_perf_info->run_utime_ = ZCE_LIB::make_timeval2(&ru_utime);

    //WiN32下的得到内存信息的方法
    PROCESS_MEMORY_COUNTERS psmem_counters;
    //psmem_counters.cb = sizeof(PROCESS_MEMORY_COUNTERS);
    bret = ::GetProcessMemoryInfo(process_handle,
                                  &psmem_counters,
                                  sizeof(PROCESS_MEMORY_COUNTERS)
                                 );

    if (bret == FALSE)
    {
        ZLOG_ERROR("[zcelib] GetProcessMemoryInfo error, reson is:%u, %s.",
                   ::GetLastError(),
                   ::strerror(::GetLastError()));
        return -1;
    }

    DWORD priority = ::GetPriorityClass(process_handle);

    if (0 == priority)
    {
        ZLOG_ERROR("[zcelib] GetPriorityClass error, reson is:%u, %s.",
                   ::GetLastError(),
                   ::strerror(::GetLastError()));
        return -1;
    }

    prc_perf_info->priority_ = static_cast<int>( priority );
    prc_perf_info->nice_ = 0;

    prc_perf_info->vm_size_ = psmem_counters.WorkingSetSize;
    //这个值其实不准确
    prc_perf_info->resident_set_ =  psmem_counters.WorkingSetSize;
    //这些值Windows平台不太准确
    prc_perf_info->shared_size_ = 0;
    prc_perf_info->text_size_ = 0;
    prc_perf_info->data_size_ = 0;

    return 0;

#elif defined (ZCE_OS_LINUX)

    int ret = 0;
    ret = read_proc_get_processstat(process_id, prc_perf_info);

    if ( 0 != ret )
    {
        return ret;
    }

    ret =  read_proc_get_processmem(process_id, prc_perf_info);

    if ( 0 != ret )
    {
        return ret;
    }

    return 0;

#endif
}
