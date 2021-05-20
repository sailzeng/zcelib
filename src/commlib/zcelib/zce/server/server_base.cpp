#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/os_adapt/file.h"
#include "zce/os_adapt/flock.h"
#include "zce/os_adapt/process.h"
#include "zce/os_adapt/socket.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/dirent.h"
#include "zce/logger/logging.h"
#include "zce/server/server_base.h"

/*********************************************************************************
class Server_Base
*********************************************************************************/
namespace zce
{
Server_Base* Server_Base::base_instance_ = NULL;

// 构造函数,私有,使用单子类的实例,
Server_Base::Server_Base()
{
    memset(&last_process_perf_, 0, sizeof(last_process_perf_));
    memset(&now_process_perf_, 0, sizeof(now_process_perf_));
    memset(&last_system_perf_, 0, sizeof(last_system_perf_));
    memset(&now_system_perf_, 0, sizeof(now_system_perf_));
}

Server_Base::~Server_Base()
{
    // 关闭文件
    if (pid_handle_ != ZCE_INVALID_HANDLE)
    {
        fcntl_unlock(&pidfile_lock_, SEEK_SET, 0, PID_FILE_LEN);
        close(pid_handle_);
    }
}

// 初始化
int Server_Base::socket_init()
{
    int ret = 0;
    ret = zce::socket_init();
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

//打印输出PID File
int Server_Base::out_pid_file(const char* pragramname)
{
    int ret = 0;

    std::string pidfile_name = pragramname;
    pidfile_name += ".pid";

    self_pid_ = getpid();

    //检查PID文件是否存在，，
    bool must_create_new = false;
    ret = access(pidfile_name.c_str(), F_OK);
    if (0 != ret)
    {
        must_create_new = true;
    }

    // 设置文件读取参数,表示其他用户可以读取，open函数会自动帮忙调整参数的。
    int fileperms = 0644;

    pid_handle_ = open(pidfile_name.c_str(),
                       O_RDWR | O_CREAT,
                       static_cast<mode_t>(fileperms));

    if (pid_handle_ == ZCE_INVALID_HANDLE)
    {
        ZCE_LOG(RS_ERROR, "Open pid file [%s]fail.", pidfile_name.c_str());
        return -1;
    }

    //如果PID文件不存在，调整文件长度，(说明见下)
    //这个地方没有原子保护，有一定风险,但……
    if (true == must_create_new)
    {
        //我是用WINDOWS下的记录锁是模拟和Linux类似，但Windows的文件锁其实没有对将长度参数设置0，
        //锁定整个文件的功能，所以要先把文件长度调整
        ftruncate(pid_handle_, PID_FILE_LEN);
    }

    file_lock_init(&pidfile_lock_, pid_handle_);

    char tmpbuff[PID_FILE_LEN + 1];

    snprintf(tmpbuff, PID_FILE_LEN + 1, "%*.u", (int)PID_FILE_LEN * (-1), self_pid_);

    // 尝试锁定全部文件，如果锁定不成功，表示有人正在用这个文件
    ret = fcntl_trywrlock(&pidfile_lock_, SEEK_SET, 0, PID_FILE_LEN);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "Trylock pid file [%s]fail. Last error =%d",
                pidfile_name.c_str(), last_error());
        return ret;
    }

    //写入文件内容, 截断文件为BUFFER_LEN，
    ftruncate(pid_handle_, PID_FILE_LEN);
    lseek(pid_handle_, 0, SEEK_SET);
    write(pid_handle_, tmpbuff, PID_FILE_LEN);

    return 0;
}

// 监测这个进程的系统状况,每N分钟运行一次就OK了
// 看门狗得到进程的状态
int Server_Base::watch_dog_status(bool first_record)
{
    int ret = 0;

    // 如果不是第一次记录，保存上一次记录的结果
    if (!first_record)
    {
        last_process_perf_ = now_process_perf_;
        last_system_perf_ = now_system_perf_;
    }

    ret = get_self_perf(&now_process_perf_);

    if (0 != ret)
    {
        return ret;
    }

    ret = get_system_perf(&now_system_perf_);

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
        ZCE_LOG(RS_ERROR, "[zcelib] [WATCHDOG][PID:%u] Monitor could memory leak,"
                "mem_checkpoint_size_ =[%u],run_mem_size_=[%u].",
                self_pid_,
                mem_checkpoint_size_,
                now_process_perf_.vm_size_);

        // 如果已经监测了若干次内存泄漏,则不再记录告警
        if (check_leak_times_ > MAX_RECORD_MEMLEAK_NUMBER)
        {
            mem_checkpoint_size_ = now_process_perf_.vm_size_;
            check_leak_times_ = 0;
        }
    }

    // 其实到这个地方了，你可以干的事情很多，
    // 甚至计算某一段时间内程序的CPU占用率过高(TNNND,后来我真做了)
    timeval last_to_now = timeval_sub(now_system_perf_.up_time_,
                                      last_system_perf_.up_time_);

    // 得到进程的CPU利用率
    timeval proc_utime = timeval_sub(now_process_perf_.run_utime_,
                                     last_process_perf_.run_utime_);
    timeval proc_stime = timeval_sub(now_process_perf_.run_stime_,
                                     last_process_perf_.run_stime_);
    timeval proc_cpu_time = timeval_add(proc_utime, proc_stime);

    // 如果间隔时间不为0
    if (total_milliseconds(last_to_now) > 0)
    {
        process_cpu_ratio_ = static_cast<uint32_t>(total_milliseconds(proc_cpu_time)
                                                   * 1000 / total_milliseconds(last_to_now));
    }
    else
    {
        process_cpu_ratio_ = 0;
    }

    ZCE_LOG(RS_INFO, "[zcelib] [WATCHDOG][PID:%u] cpu ratio[%u] "
            "totoal process user/sys[%lld/%lld] milliseconds "
            "leave last point all/usr/sys[%lld/%lld/%lld] milliseconds "
            "memory use//add [%ld/%ld].",
            self_pid_,
            process_cpu_ratio_,
            total_milliseconds(now_process_perf_.run_utime_),
            total_milliseconds(now_process_perf_.run_stime_),
            total_milliseconds(last_to_now),
            total_milliseconds(proc_utime),
            total_milliseconds(proc_stime),
            cur_mem_usesize_,
            vary_mem_size);

    // 计算系统的CPU时间，非IDLE以外的时间都是消耗时间
    timeval sys_idletime = timeval_sub(now_system_perf_.idle_time_,
                                       last_system_perf_.idle_time_);
    timeval sys_cputime = timeval_sub(last_to_now, sys_idletime);

    // 如果间隔时间不为0
    if (total_milliseconds(last_to_now) > 0)
    {
        system_cpu_ratio_ =
            static_cast<uint32_t>(total_milliseconds(sys_cputime)
                                  * 1000 / total_milliseconds(last_to_now));
    }
    else
    {
        ZCE_LOG(RS_ERROR, "system_uptime = %llu, process_start_time = %llu",
                total_milliseconds(now_system_perf_.up_time_),
                total_milliseconds(now_process_perf_.start_time_));
        system_cpu_ratio_ = 0;
    }

    // 系统或进程CPU使用超过阈值时记条账单
    if (process_cpu_ratio_ >= PROCESS_CPU_RATIO_THRESHOLD ||
        system_cpu_ratio_ >= SYSTEM_CPU_RATIO_THRESHOLD)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] [WATCHDOG][PID:%u] point[%u] vm_size[%u] "
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
                total_milliseconds(now_process_perf_.run_utime_),
                total_milliseconds(now_process_perf_.run_stime_),
                total_milliseconds(last_to_now),
                total_milliseconds(proc_utime),
                total_milliseconds(proc_stime));
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

    ZCE_LOG(RS_INFO,
            "[zcelib] [WATCHDOG][SYSTEM] cpu radio [%u] "
            "totoal usr/nice/sys/idle/iowait/hardirq/softirq "
            "[%lld/%lld/%lld/%lld/%lld/%lld/%lld] milliseconds"
            "leave last point all/use/idle[%lld/%lld/%lld] milliseconds "
            "mem ratio[%u] [totoal/can use/free/buffer/cache] "
            "[%lld/%lld/%lld/%lld/%lld] bytes",
            system_cpu_ratio_,
            total_milliseconds(now_system_perf_.user_time_),
            total_milliseconds(now_system_perf_.nice_time_),
            total_milliseconds(now_system_perf_.system_time_),
            total_milliseconds(now_system_perf_.idle_time_),
            total_milliseconds(now_system_perf_.iowait_time_),
            total_milliseconds(now_system_perf_.hardirq_time_),
            total_milliseconds(now_system_perf_.softirq_time_),
            total_milliseconds(last_to_now),
            total_milliseconds(sys_cputime),
            total_milliseconds(sys_idletime),
            mem_use_ratio_,
            now_system_perf_.totalram_size_,
            can_use_size_,
            now_system_perf_.freeram_size_,
            now_system_perf_.bufferram_size_,
            now_system_perf_.cachedram_size_);

    return 0;
}

int Server_Base::process_signal(void)
{
    //忽视部分信号,这样简单
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

#ifdef ZCE_OS_WINDOWS
    //Windows下设置退出处理函数，可以用Ctrl + C 退出
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)exit_signal, TRUE);
#else
    //这个几个信号被认可为退出信号
    signal(SIGINT, exit_signal);
    signal(SIGQUIT, exit_signal);
    signal(SIGTERM, exit_signal);

    //重新加载部分配置,用了SIGUSR1 kill -10
    signal(SIGUSR1, reload_cfg_signal);
#endif

    //SIGUSR1,SIGUSR2你可以用来干点自己的活,
    return 0;
}

int Server_Base::daemon_init()
{
    //Daemon 精灵进程,但是我不清理目录路径,

#if defined (ZCE_OS_LINUX)

    pid_t pid = fork();

    if (pid < 0)
    {
        return -1;
    }
    else if (pid > 0)
    {
        ::exit(0);
    }

#endif

    setsid();
    umask(0);

#if defined (ZCE_OS_WINDOWS)
    //设置Console的标题信息
    std::string out_str = get_app_basename();
    out_str += " ";
    out_str += app_author_;
    ::SetConsoleTitle(out_str.c_str());
#endif

    return 0;
}

//通过启动参数0，得到app_base_name_，app_run_name_
int Server_Base::create_app_name(const char* argv_0)
{
    app_run_name_ = argv_0;
    // 取得base name
    char str_base_name[PATH_MAX + 1];
    str_base_name[PATH_MAX] = '\0';
    basename(argv_0, str_base_name, PATH_MAX);

#if defined ZCE_OS_WINDOWS

    //Windows下要去掉,EXE后缀
    const size_t WIN_EXE_SUFFIX_LEN = 4;
    size_t name_len = strlen(str_base_name);

    if (name_len <= WIN_EXE_SUFFIX_LEN)
    {
        ZCE_LOG(RS_ERROR, "[framework] Exe file name is not expect?Path name[%s].", argv_0);
        return -1;
    }

    //如果有后缀才取消，没有就放鸭子
    if (strcasecmp(str_base_name + name_len - WIN_EXE_SUFFIX_LEN, ".EXE") == 0)
    {
        str_base_name[name_len - WIN_EXE_SUFFIX_LEN] = '\0';
    }

#endif

    //如果是调试版本，去掉后缀符号_d
#if defined (DEBUG) || defined (_DEBUG)

    //如果是调试版本，去掉后缀符号_d
    const size_t DEBUG_SUFFIX_LEN = 2;
    size_t debug_name_len = strlen(str_base_name);

    if (debug_name_len <= DEBUG_SUFFIX_LEN)
    {
        ZCE_LOG(RS_ERROR, "[framework] Exe file name is not debug _d suffix?str_base_name[%s].", str_base_name);
        return -1;
    }
    if (0 == strcasecmp(str_base_name + debug_name_len - DEBUG_SUFFIX_LEN, "_D"))
    {
        str_base_name[debug_name_len - DEBUG_SUFFIX_LEN] = '\0';
    }

#endif

    app_base_name_ = str_base_name;

    return 0;
}

//windows下设置服务信息
void Server_Base::set_service_info(const char* svc_name,
                                   const char* svc_desc)
{
    if (svc_name != NULL)
    {
        display_name_ = svc_name;
    }
    if (svc_desc != NULL)
    {
        service_desc_ = svc_desc;
    }
}

//得到运行信息，可能包括路径信息
const char* Server_Base::get_app_runname()
{
    return app_run_name_.c_str();
}

//得到程序进程名称，，去掉了路径，WINDOWS下去掉了后缀
const char* Server_Base::get_app_basename()
{
    return app_base_name_.c_str();
}

//设置进程是否运行的标志
void Server_Base::set_run_sign(bool app_run)
{
    app_run_ = app_run;
}

//设置reload标志
void Server_Base::set_reload_sign(bool app_reload)
{
    app_reload_ = app_reload;
}

//信号处理代码，
#ifdef ZCE_OS_WINDOWS

BOOL Server_Base::exit_signal(DWORD)
{
    base_instance_->set_run_sign(false);
    return TRUE;
}

#else

void Server_Base::exit_signal(int)
{
    base_instance_->set_run_sign(false);
    return;
}

// USER1信号处理函数
void Server_Base::reload_cfg_signal(int)
{
    // 信号处理函数中不能有IO等不可重入的操作，否则容易死锁
    base_instance_->set_reload_sign(true);
    return;
}

#endif

//在Windows 体系体系一下，可以搞成一个服务，安全性更高
#if defined ZCE_OS_WINDOWS

//运行服务
int Server_Base::win_services_run()
{
    char service_name[PATH_MAX + 1];
    service_name[PATH_MAX] = '\0';
    strncpy(service_name, app_base_name_.c_str(), PATH_MAX);

    SERVICE_TABLE_ENTRY st[] =
    {
        {service_name,(LPSERVICE_MAIN_FUNCTION)win_service_main},
        {NULL,NULL}
    };

    BOOL b_ret = ::StartServiceCtrlDispatcher(st);

    if (b_ret)
    {
        //LogEvent(_T("Register Service Main Function Success!"));
    }
    else
    {
        UINT error_info = ::GetLastError();
        ZCE_UNUSED_ARG(error_info);
        //LogEvent(_T("Register Service Main Function Error!"));
    }

    return 0;
}

//安装服务
int Server_Base::win_services_install()
{
    if (win_services_isinstalled())
    {
        printf("install service fail. service %s already exist", app_base_name_.c_str());
        return 0;
    }

    //打开服务控制管理器
    SC_HANDLE handle_scm = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (handle_scm == NULL)
    {
        //::MessageBox(NULL, _T("Couldn't open service manager"), app_base_name_.c_str(), MB_OK);
        printf("can't open service manager.\n");
        return FALSE;
    }

    // Get the executable file path
    char file_path[MAX_PATH + 1];
    file_path[MAX_PATH] = '\0';
    ::GetModuleFileName(NULL, file_path, MAX_PATH);

    //创建服务
    SC_HANDLE handle_services = ::CreateService(
        handle_scm,
        app_base_name_.c_str(),
        display_name_.c_str(),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        file_path,
        NULL,
        NULL,
        "",
        NULL,
        NULL);

    if (handle_services == NULL)
    {
        printf("install service %s fail. err=%d\n", app_base_name_.c_str(),
               GetLastError());
        ::CloseServiceHandle(handle_scm);
        //MessageBox(NULL, _T("Couldn't create service"), app_base_name_.c_str(), MB_OK);
        return -1;
    }

    // 修改描述
    SC_LOCK lock = ::LockServiceDatabase(handle_scm);

    if (lock != NULL)
    {
        SERVICE_DESCRIPTION desc;
        desc.lpDescription = (LPSTR)service_desc_.c_str();

        ::ChangeServiceConfig2(handle_services, SERVICE_CONFIG_DESCRIPTION, &desc);
        ::UnlockServiceDatabase(handle_scm);
    }

    ::CloseServiceHandle(handle_services);
    ::CloseServiceHandle(handle_scm);
    printf("install service %s success.\n", app_base_name_.c_str());

    return 0;
}

//卸载服务
int Server_Base::win_services_uninstall()
{
    if (!win_services_isinstalled())
    {
        printf("uninstall fail. service %s is not exist.\n", app_base_name_.c_str());
        return 0;
    }

    SC_HANDLE handle_scm = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (handle_scm == NULL)
    {
        //::MessageBox(NULL, _T("Couldn't open service manager"), app_base_name_.c_str(), MB_OK);
        printf("uninstall fail. can't open service manager");
        return FALSE;
    }

    SC_HANDLE handle_services = ::OpenService(handle_scm,
                                              app_base_name_.c_str(),
                                              SERVICE_STOP | DELETE);

    if (handle_services == NULL)
    {
        ::CloseServiceHandle(handle_scm);
        //::MessageBox(NULL, _T("Couldn't open service"), app_base_name_.c_str(), MB_OK);
        printf("can't open service %s\n", app_base_name_.c_str());
        return -1;
    }

    SERVICE_STATUS status;
    ::ControlService(handle_services, SERVICE_CONTROL_STOP, &status);

    //删除服务
    BOOL bDelete = ::DeleteService(handle_services);
    ::CloseServiceHandle(handle_services);
    ::CloseServiceHandle(handle_scm);
    if (bDelete)
    {
        printf("uninstall service %s success.\n", app_base_name_.c_str());
        return 0;
    }

    printf("uninstall service %s fail.\n", app_base_name_.c_str());
    //LogEvent(_T("Service could not be deleted"));
    return -1;
}

//检查服务是否安装
bool Server_Base::win_services_isinstalled()
{
    bool b_result = false;

    //打开服务控制管理器
    SC_HANDLE handle_scm = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (handle_scm != NULL)
    {
        //打开服务
        SC_HANDLE handle_service = ::OpenService(handle_scm,
                                                 app_base_name_.c_str(),
                                                 SERVICE_QUERY_CONFIG);
        if (handle_service != NULL)
        {
            b_result = true;
            ::CloseServiceHandle(handle_service);
        }
        ::CloseServiceHandle(handle_scm);
    }

    return b_result;
}

//服务运行函数
void WINAPI Server_Base::win_service_main()
{
    //WIN服务用的状态
    static SERVICE_STATUS_HANDLE handle_service_status = NULL;

    SERVICE_STATUS status;

    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status.dwCurrentState = SERVICE_STOPPED;
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    status.dwWin32ExitCode = 0;
    status.dwServiceSpecificExitCode = 0;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;

    // Register the control request handler
    status.dwCurrentState = SERVICE_START_PENDING;
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    //注册服务控制
    handle_service_status = ::RegisterServiceCtrlHandler(base_instance_->get_app_basename(),
                                                         win_services_ctrl);

    if (handle_service_status == NULL)
    {
        //LogEvent(_T("Handler not installed"));
        return;
    }

    SetServiceStatus(handle_service_status, &status);

    status.dwWin32ExitCode = S_OK;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;
    status.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(handle_service_status, &status);

    //base_instance_->do_run();

    status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(handle_service_status, &status);
    //LogEvent(_T("Service stopped"));
}

//服务控制台所需要的控制函数
void WINAPI Server_Base::win_services_ctrl(DWORD op_code)
{
    switch (op_code)
    {
    case SERVICE_CONTROL_STOP:
        //
        base_instance_->app_run_ = false;
        break;

    case SERVICE_CONTROL_PAUSE:
        break;

    case SERVICE_CONTROL_CONTINUE:
        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        break;

    default:
        base_instance_->log_event("Bad service request");
        break;
    }
}

int Server_Base::log_event(const char* format_str, ...)
{
    const size_t BUFFER_LEN = 512;
    char out_msg[BUFFER_LEN];
    char* one_string[1];
    one_string[0] = out_msg;
    HANDLE  event_source = INVALID_HANDLE_VALUE;
    va_list arg_list;

    va_start(arg_list, format_str);
    vsnprintf(out_msg, BUFFER_LEN - 1, format_str, arg_list);
    va_end(arg_list);

    event_source = ::RegisterEventSource(NULL, app_base_name_.c_str());
    if (event_source != NULL)
    {
        ::ReportEvent(event_source, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, (LPCSTR*)one_string, NULL);
        ::DeregisterEventSource(event_source);
    }
    return 0;
}
}

#endif //#if defined ZCE_OS_WINDOWS