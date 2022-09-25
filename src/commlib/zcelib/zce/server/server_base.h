/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/server/base.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
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
* 笑看风云
* 作词：黄 霑
* 作曲：徐嘉良
*
* 谁没有一些刻骨铭心事
* 谁能预计后果
* 谁没有一些旧恨心魔
* 一点点无心错
*
* 谁没有一些得不到的梦
* 谁人负你负我多
* 谁愿意解释为了什么
* 一笑已经风云过
*
* 活得开心心不记恨
* 为今天欢笑唱首歌
* 任胸襟吸收新的快乐
* 在晚风中敞开心锁
*
* 谁愿记沧桑匆匆往事
* 谁人是对是错
* 从没有解释为了什么
* 一笑看风云过
*
* 谁没有一些刻骨铭心事
* 谁能预计后果
* 谁没有一些旧恨心魔
* 一点点无心错
*
* 谁没有一些得不到的梦
* 谁人负你负我多
* 谁愿意解释为了什么
* 一笑已经风云过
*
* 活得开心心不记恨
* 为今天欢笑唱首歌
* 任胸襟吸收新的快乐
* 在晚风中敞开心锁
*
* 谁愿记沧桑匆匆往事
* 谁人是对是错
* 从没有解释为了什么
* 一笑看风云过
*
* 活得开心心不记恨
* 为今天欢笑唱首歌
* 任胸襟吸收新的快乐
* 在晚风中敞开心锁
*
* 谁愿记沧桑匆匆往事
* 谁人是对是错
* 从没有解释为了什么
* 一笑看风云过
*/

#pragma once

#include "zce/os_adapt/process.h"
#include "zce/os_adapt/sysinfo.h"
#include "zce/os_adapt/flock.h"
#include "zce/util/non_copyable.h"

namespace zce
{
/*********************************************************************************
class Server_Base
*********************************************************************************/
class server_base : public zce::non_copyable
{
protected:
    //构造函数,私有,使用单子类的实例,
    server_base();
    ~server_base();

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
    int out_pid_file(const char* pragramname);

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
    int create_app_name(const char* argv_0);

    ///得到运行信息，可能包括路径信息
    const char* get_app_runname();

    ///得到程序进程名称，WINDOWS下去掉了后缀
    const char* get_app_basename();

    /*!
    * @brief      windows下设置服务信息
    * @param      svc_name 服务名称
    * @param      svc_desc 服务描述
    */
    void set_service_info(const char* svc_name,
                          const char* svc_desc);

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

    int log_event(const char* format_str, ...);

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
    static const size_t   MEMORY_LEAK_THRESHOLD = 128 * 1024 * 1024;

    //进程CPU比率过高,其实我们一般的单线程CPU达不到这个值
    static const size_t   PROCESS_CPU_RATIO_THRESHOLD = 600;
    //系统CPU比率过高
    static const size_t   SYSTEM_CPU_RATIO_THRESHOLD = 750;

protected:

    //单子实例
    static zce::server_base* base_instance_;

protected:

    ///PID 文件句柄
    ZCE_HANDLE            pid_handle_ = ZCE_INVALID_HANDLE;
    ///
    zce::file_lock_t      pidfile_lock_;

    ///self的PID
    pid_t                 self_pid_ = 0;

    ///运行状态,是否继续运行
    bool                  app_run_ = true;

    ///是否加重新加载配置
    bool                  app_reload_ = false;

    ///进程名字，抛开运行目录，文件后缀的名字，在Windows 的服务模式，也用于services name
    std::string           app_base_name_;
    ///程序运行名称,如果包含路径运行，会有路径信息
    std::string           app_run_name_;

    ///作者名称
    std::string           app_author_;

#ifdef ZCE_OS_WINDOWS

    ///服务显示名称，仅仅在Windows 服务模式下使用
    std::string           display_name_;
    ///服务描述，仅仅在Windows 服务模式下使用
    std::string           service_desc_;

#endif

public:

    ///已经检查到的内存泄漏的次数，只记录5次
    int                    check_leak_times_ = 0;

    ///开始的时候（或者检查点的时候）内存的尺寸
    size_t                 mem_checkpoint_size_ = 0;
    ///当前内存使用
    size_t                 cur_mem_usesize_ = 0;

    ///进程的CPU利用率,千分率
    uint32_t               process_cpu_ratio_ = 0;
    ///系统的CPU利用率,千分率
    uint32_t               system_cpu_ratio_ = 0;

    ///真正可以利用的内存的内存大小
    uint64_t               can_use_size_ = 0;
    ///系统的内存使用率,千分率
    uint32_t               mem_use_ratio_ = 0;

    ///上一次的进程性能数据
    ZCE_PROCESS_PERFORM    last_process_perf_;
    ///当前这一次的进程性能数据
    ZCE_PROCESS_PERFORM    now_process_perf_;

    ///上一次的系统的性能数据
    ZCE_SYSTEM_PERFORMANCE last_system_perf_;
    ///当前这一次的系统性能数据
    ZCE_SYSTEM_PERFORMANCE now_system_perf_;
};
}


