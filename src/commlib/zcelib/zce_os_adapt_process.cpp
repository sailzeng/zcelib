
#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_string.h"
#include "zce_os_adapt_time.h"
#include "zce_log_logging.h"
#include "zce_os_adapt_process.h"

//�õ���ǰ�Ľ���ID
pid_t zce::getpid()
{
#if defined (ZCE_OS_WINDOWS)
    return ::GetCurrentProcessId ();
#elif defined (ZCE_OS_LINUX)
    return ::getpid ();
#endif
}

//�õ���ǰ�Ľ��̵ĸ�����ID
pid_t zce::getppid (void)
{

#if defined (ZCE_OS_WINDOWS)

    //��ʵ΢��û����ȷ˵���������������δ�����ȫ��������
    //�ο����� http://hi.baidu.com/woaiwlaopo/blog/item/ecaf5760b42495db8cb10d4b.html

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

    HMODULE module_hdl = GetModuleHandleA("ntdll");
    if (!module_hdl)
    {
        return DWORD(-1);
    }
    //��������ǲ�������
    NtQueryInformationProcess = (PROCNTQSIP)GetProcAddress(
                                    module_hdl,
                                    "NtQueryInformationProcess");

    if (!NtQueryInformationProcess)
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
pid_t zce::fork (void)
{
#if defined (ZCE_OS_WINDOWS)
    //��֧���ˣ��Ǻǣ�����ACE�ж�ʱ�����ŵ�֧�ֹ�������Ҳ�����ˣ�
    //��ȷû�з���֧�ֵıȽϺá�
    return (pid_t (-1));
#elif defined (ZCE_OS_LINUX)
    return ::fork ();
#endif
}

//
pid_t zce::setsid (void)
{
#if defined (ZCE_OS_WINDOWS)
    //��֧���ˣ�
    return (pid_t (-1));
#elif defined (ZCE_OS_LINUX)
    return ::setsid ();
#endif
}

//ȡ��ĳ����������
char *zce::getenv(const char *name)
{
    //�����������У������װ�е����
    return ::getenv(name);
}

sighandler_t zce::signal (int signum,
                          sighandler_t  func )
{

    //�������źţ������ƽ̨��֧�֣��ͷ�����
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

namespace zce
{

//
static int read_proc_get_processstat(pid_t read_pid, ZCE_PROCESS_PERFORM *prc_perf_info)
{
    //�õ����̵��ļ�����
    const char *PROC_PROCESS_STAT = "/proc/%u/stat";
    char file_name[PATH_MAX + 1];
    file_name[PATH_MAX] = '\0';
    snprintf(file_name, PATH_MAX, PROC_PROCESS_STAT, read_pid);
    
    //��ȡ/proc�µ��ļ�
    size_t read_len=0;
    auto pair = zce::read_file_all(file_name, &read_len);
    if ( 0 !=pair.first)
    {
        return pair.first;
    }

    const char *in_para =pair.second.get();
    char *out_para = NULL;
    in_para = zce::skip_token(in_para);              /* skip  pid*/
    in_para = zce::skip_token(in_para);              /* skip  name*/
    in_para = zce::skip_token(in_para);              /* skip  state*/
    in_para = zce::skip_token(in_para);              /* skip ppid */
    in_para = zce::skip_token(in_para);              /* skip pgrp */
    in_para = zce::skip_token(in_para);              /* skip session */
    in_para = zce::skip_token(in_para);              /* skip tty */
    in_para = zce::skip_token(in_para);              /* skip tty pgrp */
    in_para = zce::skip_token(in_para);              /* skip flags */
    in_para = zce::skip_token(in_para);              /* skip min flt */
    in_para = zce::skip_token(in_para);              /* skip cmin flt */
    in_para = zce::skip_token(in_para);              /* skip maj flt */
    in_para = zce::skip_token(in_para);              /* skip cmaj flt */

#if defined ZCE_OS_LINUX
    // tlinux64(2.6.32.43)�����ֵ��100
    long cpu_tick_precision = ::sysconf(_SC_CLK_TCK);
#else
    long cpu_tick_precision = 1000;
#endif
    uint64_t read_data=0;
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

    // ��ȡ���̵�����ʱ��start_time(unit��jiffies)
    read_data = ::strtoull(in_para, &out_para, 10);
    in_para = out_para;
    prc_perf_info->start_time_.tv_sec =  static_cast<time_t>( read_data / cpu_tick_precision);
    prc_perf_info->start_time_.tv_usec =  static_cast<time_t>((read_data % cpu_tick_precision) * (SEC_PER_USEC / cpu_tick_precision));

    // ����running_time��uptime���Զ�ȡ/proc/uptime�õ�, ����ʹ��ϵͳzcelib�Դ���zce::get_uptime()
    prc_perf_info->running_time_ = zce::timeval_sub(zce::get_uptime(), prc_perf_info->start_time_);

    //�ڴ���Ϣ�Ҵ�����һ���ļ���ȡ

    return 0;
}

//
static int read_proc_get_processmem(pid_t read_pid, ZCE_PROCESS_PERFORM *prc_perf_info)
{
    const char *PROC_PROCESS_MEM = "/proc/%u/statm";
    char file_name[PATH_MAX + 1];
    file_name[PATH_MAX] = '\0';
    snprintf(file_name, PATH_MAX, PROC_PROCESS_MEM, read_pid);
    size_t read_len=0;
    auto pair=zce::read_file_all(file_name,&read_len);
    if(0!=pair.first)
    {
        return pair.first;
    }

    const char* in_para=pair.second.get();
    char *out_para = NULL;

    //64λ��Ӧ����8096
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

//�õ����������������Ϣ������CPUʱ�䣬�ڴ�ʹ���������Ϣ
int zce::get_self_perf(ZCE_PROCESS_PERFORM *prc_perf_info)
{
    pid_t process_id = zce::getpid();

    return zce::get_process_perf(process_id, prc_perf_info);
}

//ȡ��ı�����̵�������Ϣ
int zce::get_process_perf(pid_t process_id, ZCE_PROCESS_PERFORM *prc_perf_info)
{

    prc_perf_info->process_id_ = process_id;

#if defined (ZCE_OS_WINDOWS)

    //�õ��Լ��Ľ��̾����
    //HANDLE process_handle = ::GetCurrentProcess();

    //���ݽ���ID�õ����̾��
    HANDLE process_handle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                          false,
                                          process_id);

    if (ZCE_INVALID_HANDLE == process_handle)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] OpenProcess error, reson is:%u.", ::GetLastError());
        return -1;
    }

    FILETIME ft_start_time;
    FILETIME ft_end_time;
    FILETIME ru_stime;
    FILETIME ru_utime;

    BOOL bret =::GetProcessTimes (process_handle,
                                  &ft_start_time,   // start
                                  &ft_end_time,     // exited,�������û�н��������δ����
                                  &ru_stime,
                                  &ru_utime);

    //������سɹ�
    if (bret == FALSE)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] GetProcessTimes error, reson is:%u.", ::GetLastError());
        return -1;
    }

    timeval start_time = zce::make_timeval(&ft_start_time);
    timeval now_time = zce::gettimeofday();

    prc_perf_info->running_time_ = zce::timeval_sub(now_time, start_time);
    prc_perf_info->run_stime_ = zce::make_timeval2(&ru_stime);
    prc_perf_info->run_utime_ = zce::make_timeval2(&ru_utime);

    //WiN32�µĵõ��ڴ���Ϣ�ķ���
    PROCESS_MEMORY_COUNTERS psmem_counters;
    //psmem_counters.cb = sizeof(PROCESS_MEMORY_COUNTERS);
    bret = ::GetProcessMemoryInfo(process_handle,
                                  &psmem_counters,
                                  sizeof(PROCESS_MEMORY_COUNTERS)
                                 );

    if (bret == FALSE)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] GetProcessMemoryInfo error, reson is:%u, %s.",
                ::GetLastError(),
                ::strerror(::GetLastError()));
        return -1;
    }

    DWORD priority = ::GetPriorityClass(process_handle);

    if (0 == priority)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] GetPriorityClass error, reson is:%u, %s.",
                ::GetLastError(),
                ::strerror(::GetLastError()));
        return -1;
    }

    prc_perf_info->priority_ = static_cast<int>( priority );
    prc_perf_info->nice_ = 0;

    prc_perf_info->vm_size_ = psmem_counters.WorkingSetSize;
    //���ֵ��ʵ��׼ȷ
    prc_perf_info->resident_set_ =  psmem_counters.WorkingSetSize;
    //��ЩֵWindowsƽ̨��̫׼ȷ
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
