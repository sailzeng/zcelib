
#include "zce_predefine.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_string.h"
#include "zce_os_adapt_file.h"

#include "zce_os_adapt_sysinfo.h"


//----------------------------------------------------------------------------------------
//������Щ�����Ǵ�top�Ĵ����ܽ�ժ���޸Ĺ��������ģ�
//��ο� http://www.cppblog.com/dyj057/archive/2007/01/31/18221.aspx

#if defined ZCE_OS_LINUX

namespace zce
{

//
int read_proc_get_cpuhz (struct ZCE_SYSTEM_INFO *info)
{
    //
    const char *PROC_FILENAME_CPUHZ = "/proc/cpuinfo";
    size_t read_len=0;
    auto pair=zce::read_file_all(PROC_FILENAME_CPUHZ,&read_len);
    if(0!=pair.first)
    {
        return pair.first;
    }

    const char* in_para=pair.second.get();
    char *out_para = NULL;
    in_para = zce::skip_line(in_para);
    in_para = zce::skip_line(in_para);
    in_para = zce::skip_line(in_para);
    in_para = zce::skip_line(in_para);
    in_para = zce::skip_line(in_para);
    in_para = zce::skip_line(in_para);

    info->cpu_hz_ = static_cast<uint64_t>(::strtod(in_para, &out_para) * 1024 * 1024);
    in_para = out_para;

    return 0;
}

// get load averages ,�õ�CPU���ص�
static int read_proc_get_loadavg (struct ZCE_SYSTEM_PERFORMANCE *info)
{
    //
    const char *PROC_FILENAME_LOADAVG = "/proc/loadavg";
    size_t read_len=0;
    auto pair=zce::read_file_all(PROC_FILENAME_LOADAVG,&read_len);
    if(0!=pair.first)
    {
        return pair.first;
    }

    const char* in_para=pair.second.get();
    char *out_para = NULL;

    //1,5,10 min load averages
    info->sys_loads_[0] = ::strtod(in_para, &out_para);
    in_para = out_para;
    info->sys_loads_[1] = ::strtod(in_para, &out_para);
    in_para = out_para;
    info->sys_loads_[2] = ::strtod(in_para, &out_para);
    in_para = out_para;

    //running/tasks
    info->running_num_ = ::strtoul(in_para, &out_para, 0);
    in_para = out_para;
    in_para = zce::skip_separator(in_para, '/');
    info->processes_num_ = ::strtoul(in_para, &out_para, 0);
    in_para = out_para;

    return 0;
}

//��ȡ/proc/meminfo�ļ���ȡ���ڴ����Ϣ
int read_proc_get_meminfo(struct ZCE_SYSTEM_PERFORMANCE *info)
{
    const char *PROC_FILENAME_MEMINFO = "/proc/meminfo";
    size_t read_len=0;
    auto pair=zce::read_file_all(PROC_FILENAME_MEMINFO,&read_len);
    if(0!=pair.first)
    {
        return pair.first;
    }

    const char* in_para=pair.second.get();
    char *out_para = NULL;

    uint64_t mem_data = 0;

    // be prepared for extra columns to appear be seeking to ends of lines
    // total memory ���� MemTotal:        3584536 kB
    in_para = zce::skip_token(in_para);
    mem_data = strtoull(in_para, &out_para, 10);
    info->totalram_size_ = mem_data * 1024;
    in_para = zce::skip_line(in_para);

    // free memory ���� MemFree:           22532 kB
    in_para = zce::skip_token(in_para);
    mem_data = strtoull(in_para, &out_para, 10);
    info->freeram_size_ = mem_data * 1024;
    in_para = zce::skip_line(in_para);


    // buffer memory ���� Buffers:          443268 kB
    in_para = zce::skip_token(in_para);
    mem_data = strtoull(in_para, &out_para, 10);
    info->bufferram_size_ = mem_data * 1024;
    in_para = zce::skip_line(in_para);

    // cached memory ���� Cached:          2580892 kB
    in_para = zce::skip_token(in_para);
    mem_data = strtoull(in_para, &out_para, 10);
    info->cachedram_size_ = mem_data * 1024;
    in_para = zce::skip_line(in_para);

    // cached swip
    in_para = zce::skip_token(in_para);
    mem_data = strtoull(in_para, &out_para, 10);
    info->swapcached_size_ = mem_data * 1024;
    in_para = zce::skip_line(in_para);

    //����8�У�
    for (size_t i = 0; i < 8 ; i++)
    {
        in_para = zce::skip_line(in_para);
    }

    /* total swap */
    in_para = zce::skip_token(in_para);
    mem_data = strtoull(in_para, &out_para, 10);
    info->totalswap_size_ = mem_data * 1024;
    in_para = zce::skip_line(in_para);

    /* free swap */
    in_para = zce::skip_token(in_para);
    mem_data = strtoull(in_para, &out_para, 10);
    info->freeswap_size_ = mem_data * 1024;
    in_para = zce::skip_line(in_para);

    return 0;
}


// get the cpu time info ������cpu  34151643 618 25185530 4126447092 5407693 433 2641178 1187028 0
//user: normal processes executing in user mode
//nice: niced processes executing in user mode
//system: processes executing in kernel mode
//idle: twiddling thumbs
//iowait: waiting for I/O to complete
//irq: servicing interrupts
//softirq: servicing softirqs
int read_proc_get_stat(struct ZCE_SYSTEM_PERFORMANCE *info)
{
    const char *PROC_FILENAME_STAT = "/proc/stat";
    size_t read_len=0;
    auto pair=zce::read_file_all(PROC_FILENAME_STAT,&read_len);
    if(0!=pair.first)
    {
        return pair.first;
    }

    const char* in_para=pair.second.get();
    char *out_para = NULL;

    // "cpu"
    in_para = zce::skip_token(in_para);

    long cpu_tick_precision = 1000;
    long cpu_config_num = 0;
#if defined ZCE_OS_LINUX
    // tlinux64(2.6.32.43)�����ֵ��100
    cpu_tick_precision = ::sysconf(_SC_CLK_TCK);

    // ��ȡCPU���ĸ���
    cpu_config_num = ::sysconf(_SC_NPROCESSORS_CONF);
#endif

    //user
    uint64_t time_data  = 0;
    time_data = ::strtoull(in_para, &out_para, 10);
    in_para = out_para;
    info->user_time_.tv_sec =  static_cast<time_t>(  time_data / cpu_tick_precision);
    info->user_time_.tv_usec =  static_cast<time_t>((time_data % cpu_tick_precision) * (SEC_PER_USEC / cpu_tick_precision));

    //nice
    time_data = ::strtoull(in_para, &out_para, 10);
    in_para = out_para;
    info->nice_time_.tv_sec = static_cast<time_t>(time_data / cpu_tick_precision);
    info->nice_time_.tv_usec = static_cast<time_t>((time_data % cpu_tick_precision) * (SEC_PER_USEC / cpu_tick_precision));

    //system
    time_data = ::strtoull(in_para, &out_para, 10);
    in_para = out_para;
    info->system_time_.tv_sec = static_cast<time_t>(time_data / cpu_tick_precision);
    info->system_time_.tv_usec = static_cast<time_t>((time_data % cpu_tick_precision) * (SEC_PER_USEC / cpu_tick_precision));

    //idle
    time_data = ::strtoull(in_para, &out_para, 10);
    in_para = out_para;
    info->idle_time_.tv_sec = static_cast<time_t>(time_data / cpu_tick_precision / cpu_config_num );
    info->idle_time_.tv_usec = static_cast<time_t>((time_data % cpu_tick_precision) * (SEC_PER_USEC / cpu_tick_precision ) / cpu_config_num);

    //iowait
    time_data = ::strtoull(in_para, &out_para, 10);
    in_para = out_para;
    info->iowait_time_.tv_sec = static_cast<time_t>(time_data / cpu_tick_precision);
    info->iowait_time_.tv_usec = static_cast<time_t>((time_data % cpu_tick_precision) * (SEC_PER_USEC / cpu_tick_precision));

    //hard irq
    time_data = ::strtoull(in_para, &out_para, 10);
    in_para = out_para;
    info->hardirq_time_.tv_sec = static_cast<time_t>(time_data / cpu_tick_precision);
    info->hardirq_time_.tv_usec = static_cast<time_t>((time_data % cpu_tick_precision) * (SEC_PER_USEC / cpu_tick_precision));

    //soft irq
    time_data = ::strtoull(in_para, &out_para, 10);
    in_para = out_para;
    info->softirq_time_.tv_sec = static_cast<time_t>(time_data / cpu_tick_precision);
    info->softirq_time_.tv_usec = static_cast<time_t>((time_data % cpu_tick_precision) * (SEC_PER_USEC / cpu_tick_precision));

    // ���ϵͳ��uptime
    info->up_time_ = zce::get_uptime();

    //���ֲ����������ǣ���ʵ��7����ֵ�����������е�CPUʱ�䣨���Ƿǳ�����ںˣ������滹��������ֵ������������
    //����ʵ��Ӧ����user+nice+system+idle+iowait+irq+softirq+( stealstolen  +  guest)
    //stealstolen 2.6.11
    //guest       2.6.24

    return 0;
}



//ȡ��UPTIME  ���� 10665138.96 41312475.36
int read_proc_get_uptime(struct ZCE_SYSTEM_PERFORMANCE *info)
{
    const char *PROC_FILENAME_UPTIME = "/proc/uptime";
    size_t read_len=0;
    auto pair=zce::read_file_all(PROC_FILENAME_UPTIME,&read_len);
    if(0!=pair.first)
    {
        return pair.first;
    }

    const char* in_para=pair.second.get();
    char *out_para = NULL;

    //
    double uptime = ::strtod(in_para, &out_para);
    in_para = out_para;

    info->up_time_.tv_sec = static_cast<time_t>(::floor(uptime));
    info->up_time_.tv_usec = static_cast<time_t>((uptime - ::floor(uptime)) * SEC_PER_USEC);

    return 0;
}



};



//����PROC�ļ����ļ��õ���ǰ��ϵͳ����
int  zce::read_proc_get_systemperf(struct ZCE_SYSTEM_PERFORMANCE *info)
{
    int ret = 0;
    ret = zce::read_proc_get_loadavg(info);

    if ( 0 != ret )
    {
        return ret;
    }

    ret = zce::read_proc_get_meminfo(info);

    if ( 0 != ret )
    {
        return ret;
    }

    ret = zce::read_proc_get_stat(info);

    if ( 0 != ret )
    {
        return ret;
    }


    return 0;
}



//sysinfo�����м�ĺܶ���ֵ��/procĿ¼�Ķ��岢��һ�£��ҽ�������
int zce::read_fun_get_systemperf(struct ZCE_SYSTEM_PERFORMANCE *zce_system_perf)
{
    struct sysinfo info;
    //��ʵͨ��::sysconfҲ���Եõ��������
    int ret = ::sysinfo(&info);

    if (ret == -1)
    {
        return -1;
    }

    ret = zce::read_proc_get_stat(zce_system_perf);

    if (ret == -1)
    {
        return -1;
    }

    //������һ��������Ҫ���⴦��һ��
    zce_system_perf->sys_loads_[0] = double(info.loads[0]) / (1 << SI_LOAD_SHIFT);
    zce_system_perf->sys_loads_[1] = double(info.loads[1]) / (1 << SI_LOAD_SHIFT);
    zce_system_perf->sys_loads_[2] = double(info.loads[2]) / (1 << SI_LOAD_SHIFT);

    zce_system_perf->processes_num_ = info.procs;
    zce_system_perf->running_num_ = 0;

    zce_system_perf->totalram_size_ = info.totalram * info.mem_unit;
    zce_system_perf->freeram_size_ = info.freeram * info.mem_unit;

    //bufferr �� cache ��sysinfoֻ��һ��buffer�ĸ���ҹ������bufferӦ����meminfo��buf��cache�ܺ�
    zce_system_perf->shared_size_ = info.sharedram * info.mem_unit;
    zce_system_perf->bufferram_size_ = info.bufferram * info.mem_unit;
    zce_system_perf->cachedram_size_ = 0;

    zce_system_perf->totalswap_size_ = info.totalswap * info.mem_unit;
    zce_system_perf->freeswap_size_ =  info.freeswap * info.mem_unit;
    zce_system_perf->swapcached_size_ = 0;

    //uptime���Զ�ȡ/proc/uptime�õ�,�������ʱ����
    zce_system_perf->up_time_ = zce::get_uptime();

    return 0;
}

#endif //end of #if define ZCE_OS_LINUX

//----------------------------------------------------------------------------------------


int zce::get_system_info(ZCE_SYSTEM_INFO *zce_system_info)
{
#if defined ZCE_OS_WINDOWS

    //
    SYSTEM_INFO system_info;
    ::GetSystemInfo(&system_info);
    //
    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(MEMORYSTATUSEX);
    BOOL bool_ret = FALSE;

    //�õ��ڴ洦�����
    bool_ret = ::GlobalMemoryStatusEx(&mem_status);

    if (!bool_ret)
    {
        return -1;
    }

    //�õ�CPU�ĸ���
    zce_system_info->nprocs_conf_ = system_info.dwNumberOfProcessors;
    zce_system_info->nprocs_av_ = system_info.dwNumberOfProcessors;

    //�õ��ڴ�״��
    zce_system_info->totalram_size_ = mem_status.ullTotalPhys;
    zce_system_info->freeram_size_ = mem_status.ullAvailPhys;

    //�⼸�����ݶ���WINDOWSû���κ��ô�,
    zce_system_info->shared_size_ = 0;
    zce_system_info->bufferram_size_ = 0;

    //����ط��Ұ�MS��pagesfile�ĳߴ磬��ȥ  �����ڴ�ߴ� �õ��Ľ��������ĳߴ磨���߸������ƣ�
    zce_system_info->totalswap_size_ = mem_status.ullTotalPageFile - mem_status.ullTotalPhys;

    if (zce_system_info->totalswap_size_ < 0)
    {
        zce_system_info->totalswap_size_ = 0;
    }

    zce_system_info->freeswap_size_ =  mem_status.ullAvailPageFile - mem_status.ullAvailPhys;

    if (zce_system_info->freeswap_size_ < 0)
    {
        zce_system_info->freeswap_size_ = 0;
    }

    //��ע����ȡCPU����Ƶ

    //ʹ��һ��Ĭ��ֵ
    zce_system_info->cpu_hz_ = 1024 * 1024 * 1024;
    DWORD dw_mhz;
    DWORD dw_size = sizeof(DWORD);
    HKEY  hdl_key;
    LONG  l_return = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                                    0,
                                    KEY_READ,
                                    &hdl_key);
    if (l_return != ERROR_SUCCESS)
    {
        return -1;
    }
    l_return = ::RegQueryValueExA(hdl_key, "~MHz", NULL, NULL, (LPBYTE)&dw_mhz, &dw_size);
    if (l_return != ERROR_SUCCESS)
    {
        return -1;
    }
    ::RegCloseKey(hdl_key);
    zce_system_info->cpu_hz_ = dw_mhz * 1024ll * 1024;

    return 0;

#elif defined ZCE_OS_LINUX

    struct sysinfo info;
    //��ʵͨ��::sysconfҲ���Եõ��������
    int ret = ::sysinfo(&info);

    if (ret == -1)
    {
        return -1;
    }

    zce_system_info->nprocs_conf_ = ::sysconf(_SC_NPROCESSORS_CONF);
    zce_system_info->nprocs_av_  = ::sysconf(_SC_NPROCESSORS_ONLN);

    zce_system_info->totalram_size_ = info.totalram * info.mem_unit;
    zce_system_info->freeram_size_ = info.freeram * info.mem_unit;

    //bufferr �� cache ��sysinfoֻ��һ��buffer�ĸ���ҹ������bufferӦ����meminfo��buf��cache�ܺ�
    zce_system_info->shared_size_ = info.sharedram * info.mem_unit;
    zce_system_info->bufferram_size_ = info.bufferram * info.mem_unit;

    zce_system_info->totalswap_size_ = info.totalswap * info.mem_unit;
    zce_system_info->freeswap_size_ =  info.freeswap * info.mem_unit;

    ret = zce::read_proc_get_cpuhz(zce_system_info);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
#endif
}



//�õ�����ϵͳ��������Ϣ
int zce::get_system_perf(ZCE_SYSTEM_PERFORMANCE *zce_system_perf)
{
#if defined ZCE_OS_WINDOWS

    BOOL bool_ret = FALSE;

    //�õ���������

    //�õ��ڴ�ʹ��״��
    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(MEMORYSTATUSEX);
    bool_ret = ::GlobalMemoryStatusEx(&mem_status);

    //�õ�CPU��ʹ��״��
    FILETIME idle_time, kernel_time, user_time;
    bool_ret = ::GetSystemTimes(&idle_time,
                                &kernel_time,
                                &user_time);

    if (!bool_ret)
    {
        return -1;
    }

    //
    PERFORMANCE_INFORMATION performance_info;
    performance_info.cb = sizeof(PERFORMANCE_INFORMATION);
    bool_ret = ::GetPerformanceInfo(&performance_info,
                                    sizeof(PERFORMANCE_INFORMATION)
                                   );

    if (!bool_ret)
    {
        return -1;
    }

    //�õ��ڴ�״��
    zce_system_perf->totalram_size_ = mem_status.ullTotalPhys;
    zce_system_perf->freeram_size_ = mem_status.ullAvailPhys;

    //���������ݶ���WINDOWSû���κ��ô�
    zce_system_perf->shared_size_ = 0;
    zce_system_perf->bufferram_size_ = 0;
    zce_system_perf->cachedram_size_ = 0;

    //����ط��Ұ�MS��pagesfile�ĳߴ磬��ȥ  �����ڴ�ߴ� �õ��� �����ڴ�ĳߴ�
    zce_system_perf->totalswap_size_ = mem_status.ullTotalPageFile - mem_status.ullTotalPhys;

    if (zce_system_perf->totalswap_size_ < 0)
    {
        zce_system_perf->totalswap_size_ = 0;
    }

    zce_system_perf->freeswap_size_ =  mem_status.ullAvailPageFile - mem_status.ullAvailPhys;

    if (zce_system_perf->freeswap_size_ < 0)
    {
        zce_system_perf->freeswap_size_ = 0;
    }

    zce_system_perf->swapcached_size_ = 0;

    //�õ�ϵͳ������ʱ��
    zce_system_perf->up_time_ = zce::get_uptime();

    zce_system_perf->idle_time_ = zce::make_timeval(&idle_time);
    zce_system_perf->system_time_ = zce::make_timeval(&kernel_time);
    zce_system_perf->user_time_ = zce::make_timeval(&user_time);

    //WINDOWSû�и��������
    zce_system_perf->nice_time_.tv_sec = 0;
    zce_system_perf->nice_time_.tv_usec = 0;
    zce_system_perf->iowait_time_.tv_sec = 0;
    zce_system_perf->iowait_time_.tv_usec = 0;
    zce_system_perf->softirq_time_.tv_sec = 0;
    zce_system_perf->softirq_time_.tv_usec = 0;
    zce_system_perf->hardirq_time_.tv_sec = 0;
    zce_system_perf->hardirq_time_.tv_usec = 0;


    //WINDOWSƽ̨û��������û��
    zce_system_perf->sys_loads_[0] = 0.0;
    zce_system_perf->sys_loads_[1] = 0.0;
    zce_system_perf->sys_loads_[2] = 0.0;


    zce_system_perf->processes_num_ = performance_info.ProcessCount;
    //WINDOWҲû���������
    zce_system_perf->running_num_ = 0;

    return 0;

#elif defined ZCE_OS_LINUX

    return read_proc_get_systemperf(zce_system_perf);
#endif
}





