#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_flock.h"
#include "zce_os_adapt_process.h"
#include "zce_os_adapt_socket.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_dirent.h"
#include "zce_log_logging.h"
#include "zce_server_base.h"


/*********************************************************************************
class ZCE_Server_Base
*********************************************************************************/


ZCE_Server_Base *ZCE_Server_Base::base_instance_ = NULL;


// ���캯��,˽��,ʹ�õ������ʵ��,
ZCE_Server_Base::ZCE_Server_Base():
    pid_handle_(ZCE_INVALID_HANDLE),
    self_pid_(0),
    app_run_(true),
    app_reload_(false),
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

ZCE_Server_Base::~ZCE_Server_Base()
{
    // �ر��ļ�
    if (pid_handle_ != ZCE_INVALID_HANDLE)
    {
        zce::flock_unlock(&pidfile_lock_, SEEK_SET, 0, PID_FILE_LEN);
        zce::flock_destroy(&pidfile_lock_);
        zce::close(pid_handle_);
    }
}

// ��ʼ��
int ZCE_Server_Base::socket_init()
{
    int ret = 0;
    ret = zce::socket_init();

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

//��ӡ���PID File
int ZCE_Server_Base::out_pid_file(const char *pragramname)
{
    int ret = 0;

    std::string pidfile_name = pragramname;
    pidfile_name += ".pid";

    self_pid_ = zce::getpid();

    //���PID�ļ��Ƿ���ڣ���
    bool must_create_new = false;
    ret = zce::access(pidfile_name.c_str(), F_OK);
    if ( 0 != ret)
    {
        must_create_new = true;
    }

    // �����ļ���ȡ����,��ʾ�����û����Զ�ȡ��open�������Զ���æ���������ġ�
    int fileperms = 0644;

    pid_handle_ = zce::open(pidfile_name.c_str(),
                            O_RDWR | O_CREAT,
                            static_cast<mode_t>(fileperms));

    if (pid_handle_ == ZCE_INVALID_HANDLE)
    {
        ZCE_LOG(RS_ERROR, "Open pid file [%s]fail.", pidfile_name.c_str());
        return -1;
    }

    //���PID�ļ������ڣ������ļ����ȣ�(˵������)
    //����ط�û��ԭ�ӱ�������һ������,������
    if (true == must_create_new)
    {
        //������WINDOWS�µļ�¼����ģ���Linux���ƣ���Windows���ļ�����ʵû�жԽ����Ȳ�������0��
        //���������ļ��Ĺ��ܣ�����Ҫ�Ȱ��ļ����ȵ���
        zce::ftruncate(pid_handle_, PID_FILE_LEN);
    }


    zce::flock_init(&pidfile_lock_, pid_handle_);

    char tmpbuff[PID_FILE_LEN + 1];

    snprintf(tmpbuff, PID_FILE_LEN + 1, "%*.u", (int)PID_FILE_LEN * (-1), self_pid_);

    // ��������ȫ���ļ�������������ɹ�����ʾ��������������ļ�
    ret = zce::flock_trywrlock(&pidfile_lock_, SEEK_SET, 0, PID_FILE_LEN);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "Trylock pid file [%s]fail. Last error =%d",
                pidfile_name.c_str(), zce::last_error());
        return ret;
    }

    //д���ļ�����, �ض��ļ�ΪBUFFER_LEN��
    zce::ftruncate(pid_handle_, PID_FILE_LEN);
    zce::lseek(pid_handle_, 0, SEEK_SET);
    zce::write(pid_handle_, tmpbuff, PID_FILE_LEN);

    return 0;
}



// ���������̵�ϵͳ״��,ÿN��������һ�ξ�OK��
// ���Ź��õ����̵�״̬
int ZCE_Server_Base::watch_dog_status(bool first_record)
{
    int ret = 0;

    // ������ǵ�һ�μ�¼��������һ�μ�¼�Ľ��
    if (!first_record)
    {
        last_process_perf_ = now_process_perf_;
        last_system_perf_ = now_system_perf_;
    }

    ret = zce::get_self_perf(&now_process_perf_);

    if (0 != ret)
    {
        return ret;
    }

    ret = zce::get_system_perf(&now_system_perf_);

    if (0 != ret)
    {
        return ret;
    }

    cur_mem_usesize_ = now_process_perf_.vm_size_;

    // ��¼��һ�ε��ڴ�����
    if (first_record)
    {
        mem_checkpoint_size_ = now_process_perf_.vm_size_;
        return 0;
    }


    // �����ڴ�仯�����
    size_t vary_mem_size = 0;

    if (now_process_perf_.vm_size_ >= mem_checkpoint_size_)
    {
        vary_mem_size = now_process_perf_.vm_size_ - mem_checkpoint_size_;
    }
    // �ڴ��Ȼ��С�ˡ���
    else
    {
        mem_checkpoint_size_ = now_process_perf_.vm_size_;
    }

    // ����澯������ػ㱨Ҫ����һ��
    if (vary_mem_size >= MEMORY_LEAK_THRESHOLD)
    {
        ++check_leak_times_;
        ZCE_LOG(RS_ERROR, "[zcelib] [WATCHDOG][PID:%u] Monitor could memory leak,"
                "mem_checkpoint_size_ =[%u],run_mem_size_=[%u].",
                self_pid_,
                mem_checkpoint_size_,
                now_process_perf_.vm_size_);

        // ����Ѿ���������ɴ��ڴ�й©,���ټ�¼�澯
        if (check_leak_times_ > MAX_RECORD_MEMLEAK_NUMBER)
        {
            mem_checkpoint_size_ = now_process_perf_.vm_size_;
            check_leak_times_ = 0;
        }
    }

    // ��ʵ������ط��ˣ�����Ըɵ�����ܶ࣬
    // ��������ĳһ��ʱ���ڳ����CPUռ���ʹ���(TNNND,������������)
    timeval last_to_now = zce::timeval_sub(now_system_perf_.up_time_,
                                           last_system_perf_.up_time_);

    // �õ����̵�CPU������
    timeval proc_utime = zce::timeval_sub(now_process_perf_.run_utime_,
                                          last_process_perf_.run_utime_);
    timeval proc_stime = zce::timeval_sub(now_process_perf_.run_stime_,
                                          last_process_perf_.run_stime_);
    timeval proc_cpu_time = zce::timeval_add(proc_utime, proc_stime);

    // ������ʱ�䲻Ϊ0
    if (zce::total_milliseconds(last_to_now) > 0)
    {
        process_cpu_ratio_ = static_cast<uint32_t>(zce::total_milliseconds(proc_cpu_time)
                                                   * 1000 / zce::total_milliseconds(last_to_now));
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
            zce::total_milliseconds(now_process_perf_.run_utime_),
            zce::total_milliseconds(now_process_perf_.run_stime_),
            zce::total_milliseconds(last_to_now),
            zce::total_milliseconds(proc_utime),
            zce::total_milliseconds(proc_stime),
            cur_mem_usesize_,
            vary_mem_size);

    // ����ϵͳ��CPUʱ�䣬��IDLE�����ʱ�䶼������ʱ��
    timeval sys_idletime = zce::timeval_sub(now_system_perf_.idle_time_,
                                            last_system_perf_.idle_time_);
    timeval sys_cputime = zce::timeval_sub(last_to_now, sys_idletime);

    // ������ʱ�䲻Ϊ0
    if (zce::total_milliseconds(last_to_now) > 0)
    {
        system_cpu_ratio_ =
            static_cast<uint32_t>(zce::total_milliseconds(sys_cputime)
                                  * 1000 / zce::total_milliseconds(last_to_now));
    }
    else
    {
        ZCE_LOG(RS_ERROR, "system_uptime = %llu, process_start_time = %llu",
                zce::total_milliseconds(now_system_perf_.up_time_),
                zce::total_milliseconds(now_process_perf_.start_time_));
        system_cpu_ratio_ = 0;
    }

    // ϵͳ�����CPUʹ�ó�����ֵʱ�����˵�
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
                zce::total_milliseconds(now_process_perf_.run_utime_),
                zce::total_milliseconds(now_process_perf_.run_stime_),
                zce::total_milliseconds(last_to_now),
                zce::total_milliseconds(proc_utime),
                zce::total_milliseconds(proc_stime));
    }

    // �ڴ�ʹ������ļ��
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
            zce::total_milliseconds(now_system_perf_.user_time_),
            zce::total_milliseconds(now_system_perf_.nice_time_),
            zce::total_milliseconds(now_system_perf_.system_time_),
            zce::total_milliseconds(now_system_perf_.idle_time_),
            zce::total_milliseconds(now_system_perf_.iowait_time_),
            zce::total_milliseconds(now_system_perf_.hardirq_time_),
            zce::total_milliseconds(now_system_perf_.softirq_time_),
            zce::total_milliseconds(last_to_now),
            zce::total_milliseconds(sys_cputime),
            zce::total_milliseconds(sys_idletime),
            mem_use_ratio_,
            now_system_perf_.totalram_size_,
            can_use_size_,
            now_system_perf_.freeram_size_,
            now_system_perf_.bufferram_size_,
            now_system_perf_.cachedram_size_);

    return 0;
}


int ZCE_Server_Base::process_signal(void)
{
    //���Ӳ����ź�,������
    zce::signal(SIGHUP, SIG_IGN);
    zce::signal(SIGPIPE, SIG_IGN);
    zce::signal(SIGCHLD, SIG_IGN);

#ifdef ZCE_OS_WINDOWS
    //Windows�������˳���������������Ctrl + C �˳�
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)exit_signal, TRUE);
#else
    //��������źű��Ͽ�Ϊ�˳��ź�
    zce::signal(SIGINT, exit_signal);
    zce::signal(SIGQUIT, exit_signal);
    zce::signal(SIGTERM, exit_signal);

    //���¼��ز�������,����SIGUSR1 kill -10
    zce::signal(SIGUSR1, reload_cfg_signal);
#endif

    //SIGUSR1,SIGUSR2����������ɵ��Լ��Ļ�,
    return 0;
}

int ZCE_Server_Base::daemon_init()
{
    //Daemon �������,�����Ҳ�����Ŀ¼·��,

#if defined (ZCE_OS_LINUX)

    pid_t pid = zce::fork();

    if (pid < 0)
    {
        return -1;
    }
    else if (pid > 0)
    {
        ::exit(0);
    }

#endif

    zce::setsid();
    zce::umask(0);

#if defined (ZCE_OS_WINDOWS)
    //����Console�ı�����Ϣ
    std::string out_str = get_app_basename();
    out_str += " ";
    out_str += app_author_;
    ::SetConsoleTitle(out_str.c_str());
#endif

    return 0;
}


//ͨ����������0���õ�app_base_name_��app_run_name_
int ZCE_Server_Base::create_app_name(const char *argv_0)
{
    app_run_name_ = argv_0;
    // ȡ��base name
    char str_base_name[PATH_MAX + 1];
    str_base_name[PATH_MAX] = '\0';
    zce::basename(argv_0, str_base_name, PATH_MAX);

#if defined ZCE_OS_WINDOWS

    //Windows��Ҫȥ��,EXE��׺
    const size_t WIN_EXE_SUFFIX_LEN = 4;
    size_t name_len = strlen(str_base_name);

    if (name_len <= WIN_EXE_SUFFIX_LEN)
    {
        ZCE_LOG(RS_ERROR, "[framework] Exe file name is not expect?Path name[%s].", argv_0);
        return -1;
    }

    //����к�׺��ȡ����û�оͷ�Ѽ��
    if (strcasecmp(str_base_name + name_len - WIN_EXE_SUFFIX_LEN, ".EXE") == 0)
    {
        str_base_name[name_len - WIN_EXE_SUFFIX_LEN] = '\0';
    }

#endif

    //����ǵ��԰汾��ȥ����׺����_d
#if defined (DEBUG) || defined (_DEBUG)

    //����ǵ��԰汾��ȥ����׺����_d
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

//windows�����÷�����Ϣ
void ZCE_Server_Base::set_service_info(const char *svc_name,
                                       const char *svc_desc)
{
    if (svc_name != NULL)
    {
        service_name_ = svc_name;
    }

    if (svc_desc != NULL)
    {
        service_desc_ = svc_desc;
    }
}



//�õ�������Ϣ�����ܰ���·����Ϣ
const char *ZCE_Server_Base::get_app_runname()
{
    return app_run_name_.c_str();
}

//�õ�����������ƣ���ȥ����·����WINDOWS��ȥ���˺�׺
const char *ZCE_Server_Base::get_app_basename()
{
    return app_base_name_.c_str();
}



//���ý����Ƿ����еı�־
void ZCE_Server_Base::set_run_sign(bool app_run)
{
    app_run_ = app_run;
}

//����reload��־
void ZCE_Server_Base::set_reload_sign(bool app_reload)
{
    app_reload_ = app_reload;
}


//�źŴ�����룬
#ifdef ZCE_OS_WINDOWS

BOOL ZCE_Server_Base::exit_signal(DWORD)
{
    base_instance_->set_run_sign(false);
    return TRUE;
}

#else

void ZCE_Server_Base::exit_signal(int)
{
    base_instance_->set_run_sign(false);
    return;
}

// USER1�źŴ�����
void ZCE_Server_Base::reload_cfg_signal(int)
{
    // �źŴ������в�����IO�Ȳ�������Ĳ�����������������
    base_instance_->set_reload_sign(true);
    return;
}

#endif




#if defined ZCE_OS_WINDOWS

//���з���
int ZCE_Server_Base::win_services_run()
{
    char service_name[PATH_MAX + 1];
    service_name[PATH_MAX] = '\0';
    strncpy(service_name, app_base_name_.c_str(), PATH_MAX);

    SERVICE_TABLE_ENTRY st[] =
    {
        { service_name, (LPSERVICE_MAIN_FUNCTION)win_service_main },
        { NULL, NULL }
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

//��װ����
int ZCE_Server_Base::win_services_install()
{

    if (win_services_isinstalled())
    {
        printf("install service fail. service %s already exist", app_base_name_.c_str());
        return 0;
    }

    //�򿪷�����ƹ�����
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

    //��������
    SC_HANDLE handle_services = ::CreateService(
                                    handle_scm,
                                    app_base_name_.c_str(),
                                    service_name_.c_str(),
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

    // �޸�����
    SC_LOCK lock = LockServiceDatabase(handle_scm);

    if (lock != NULL)
    {
        SERVICE_DESCRIPTION desc;
        desc.lpDescription = (LPSTR)service_desc_.c_str();

        ChangeServiceConfig2(handle_services, SERVICE_CONFIG_DESCRIPTION, &desc);
        UnlockServiceDatabase(handle_scm);
    }

    ::CloseServiceHandle(handle_services);
    ::CloseServiceHandle(handle_scm);
    printf("install service %s succ.\n", app_base_name_.c_str());

    return 0;
}

//ж�ط���
int ZCE_Server_Base::win_services_uninstall()
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

    //ɾ������
    BOOL bDelete = ::DeleteService(handle_services);
    ::CloseServiceHandle(handle_services);
    ::CloseServiceHandle(handle_scm);

    if (bDelete)
    {
        printf("uninstall service %s succ.\n", app_base_name_.c_str());
        return 0;
    }

    printf("uninstall service %s fail.\n", app_base_name_.c_str());
    //LogEvent(_T("Service could not be deleted"));
    return -1;

}

//�������Ƿ�װ
bool ZCE_Server_Base::win_services_isinstalled()
{
    bool b_result = false;

    //�򿪷�����ƹ�����
    SC_HANDLE handle_scm = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (handle_scm != NULL)
    {
        //�򿪷���
        SC_HANDLE handle_service = ::OpenService(handle_scm, app_base_name_.c_str(), SERVICE_QUERY_CONFIG);

        if (handle_service != NULL)
        {
            b_result = true;
            ::CloseServiceHandle(handle_service);
        }

        ::CloseServiceHandle(handle_scm);
    }

    return b_result;
}

//�������к���
void WINAPI ZCE_Server_Base::win_service_main()
{
    //WIN�����õ�״̬
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

    //ע��������
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

//�������̨����Ҫ�Ŀ��ƺ���
void WINAPI ZCE_Server_Base::win_services_ctrl(DWORD op_code)
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
            //LogEvent(_T("Bad service request"));
            break;
    }
}

#endif //#if defined ZCE_OS_WINDOWS