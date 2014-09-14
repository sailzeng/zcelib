/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_server_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2006年3月7日
* @brief      服务器的小工具集合,作为服务器基类使用
*             功能包括：
*             1.输出PID
              2.监控服务器的内存，CPU占用率等
* @details
*
*
*
* @note
*
*/

#ifndef ZCE_LIB_SERVER_TOOLKIT_H_
#define ZCE_LIB_SERVER_TOOLKIT_H_

#include "zce_os_adapt_process.h"
#include "zce_os_adapt_sysinfo.h"
#include "zce_os_adapt_flock.h"



/*********************************************************************************
class ZCE_Server_Toolkit
*********************************************************************************/
class ZCELIB_EXPORT ZCE_Server_Base :public ZCE_NON_Copyable
{
protected:
    //构造函数,私有,使用单子类的实例,
    ZCE_Server_Base();
    ~ZCE_Server_Base();

public:

    //初始化socket，Windows下的麻烦
    int socket_init();

    /*!
    * @brief      打印输出PID File
    * @return     int
    * @param      pragramname 进程的名字，可以将argv[0]传递给我,内部会处理,
    * @param      lock_pid    是否对PID文件进行加锁处理，
    * @note       
    */
    int out_pid_file(const char *pragramname);

    ///监控进程的运行状态
    int watch_dog_status(bool first_record);

    ///处理信号灯，根据平台有一些特殊处理
    int process_signal(void);

    ///精灵化，
    int daemon_init();


    ///设置进程是否运行的标志
    void set_run_sign(bool app_run);

    /// 设置reload标志
    void set_reload_sign(bool app_reload);

    ///通过启动参数0,得到app_base_name_，app_run_name_
    int create_app_name(const char *argv_0);


    ///得到运行信息，可能包括路径信息
    const char *get_app_runname();

    ///得到程序进程名称，WINDOWS下去掉了后缀
    const char *get_app_basename();


    /*!
    * @brief      windows下设置服务信息
    * @param      svc_name 服务名称
    * @param      svc_desc 服务描述
    */
    void set_service_info(const char *svc_name, const char *svc_desc);

    //信号处理代码，
#ifdef ZCE_OS_WINDOWS

    ///WIN 下面的退出处理
    static BOOL exit_signal(DWORD);

#else

    ///退出信号
    static void exit_signal(int);

    ///重新加载配置
    static void reload_cfg_signal(int);
#endif

    //WIN 服务的代码，用于服务器的注册注销等
#ifdef ZCE_OS_WINDOWS
    ///运行服务
    int win_services_run();

    ///安装服务
    int win_services_install();
    ///卸载服务
    int win_services_uninstall();
    ///检查服务是否安装
    bool win_services_isinstalled();

    ///服务运行函数
    static void WINAPI win_service_main();
    ///服务控制台所需要的控制函数
    static void WINAPI win_services_ctrl(DWORD op_code);

#endif

protected:
    
    //PID文件长度，用一个最大长度，避免一些长度变化的麻烦，
    static const size_t   PID_FILE_LEN = 16;

    //最大的记录内存泄漏的次数
    static const int      MAX_RECORD_MEMLEAK_NUMBER = 5;

    //内存泄漏的阈值
    static const size_t   MEMORY_LEAK_THRESHOLD  = 128 * 1024 * 1024;

    //进程CPU比率过高,其实我们一般的单线程CPU达不到这个值
    static const size_t   PROCESS_CPU_RATIO_THRESHOLD = 600;
    //系统CPU比率过高
    static const size_t   SYSTEM_CPU_RATIO_THRESHOLD = 750;


protected:

    //单子实例
    static ZCE_Server_Base *base_instance_;

protected:

    ///PID 文件句柄
    ZCE_HANDLE            pid_handle_;
    ///
    zce_flock_t           pidfile_lock_;

    ///self的PID
    pid_t                 self_pid_;


    ///运行状态,是否继续运行
    bool                  app_run_;

    ///是否加重新加载配置
    bool                  app_reload_;


    ///进程名字，抛开运行目录，文件后缀的名字，
    std::string           app_base_name_;
    ///程序运行名称,如果包含路径运行，会有路径信息
    std::string           app_run_name_;

    ///作者名称
    std::string           app_author_;



    ///服务名称
    std::string           service_name_;

    ///服务描述
    std::string           service_desc_;

public:

    ///已经检查到的内存泄漏的次数，只记录5次
    int                    check_leak_times_;


    ///开始的时候（或者检查点的时候）内存的尺寸
    size_t                 mem_checkpoint_size_;
    ///当前内存使用
    size_t                 cur_mem_usesize_;

    ///进程的CPU利用率,千分率
    uint32_t               process_cpu_ratio_;
    ///系统的CPU利用率,千分率
    uint32_t               system_cpu_ratio_;

    ///真正可以利用的内存的内存大小
    uint64_t               can_use_size_;
    ///系统的内存使用率,千分率
    uint32_t               mem_use_ratio_;

    ///上一次的进程性能数据
    ZCE_PROCESS_PERFORM    last_process_perf_;
    ///当前这一次的进程性能数据
    ZCE_PROCESS_PERFORM    now_process_perf_;

    ///上一次的系统的性能数据
    ZCE_SYSTEM_PERFORMANCE last_system_perf_;
    ///当前这一次的系统性能数据
    ZCE_SYSTEM_PERFORMANCE now_system_perf_;


};










#endif //_ZCE_LIB_SERVER_TOOLKIT_H_


