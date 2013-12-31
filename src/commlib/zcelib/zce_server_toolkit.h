/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_server_toolkit.h
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



/*********************************************************************************
class ZCE_Server_Toolkit
*********************************************************************************/
class ZCELIB_EXPORT ZCE_Server_Toolkit
{
protected:
    //构造函数,私有,使用单子类的实例,
    ZCE_Server_Toolkit();
    ~ZCE_Server_Toolkit();

public:

    //初始化
    int socket_init();

    //打印输出PID File
    int out_pid_file(const char *pragramname,
                     bool lock_pid);

    //监控进程的运行状态
    int watch_dog_status(bool first_record);

    // 获取运行状态统计
    uint32_t get_sys_cpu_ratio(void) const;
    uint32_t get_app_cpu_ratio(void) const;
    uint32_t get_sys_mem_ratio(void) const;
    uint64_t get_can_use_mem_size(void) const;


protected:
    //每次记录内存泄漏的次数
    static const int      MAX_RECORD_MEMORY_NUMBER = 5;

    //内存泄漏的阈值
    static const size_t   MEMORY_LEAK_THRESHOLD  = 300 * 1024 * 1024;

    //进程CPU比率过高,其实我们一般的单线程CPU达不到这个值
    static const size_t   PROCESS_CPU_RATIO_THRESHOLD = 600;
    //系统CPU比率过高
    static const size_t   SYSTEM_CPU_RATIO_THRESHOLD = 750;

protected:

    //PID 文件句柄
    ZCE_HANDLE             pid_handle_;

    //self的PID
    pid_t                  self_pid_;


    //已经检查到的内存泄漏的次数，只记录5次
    int                    check_leak_times_;

    //开始的时候（或者检查点的时候）内存的尺寸
    size_t                 mem_checkpoint_size_;
    //当前内存使用
    size_t                 cur_mem_usesize_;

    //进程的CPU利用率,千分率
    uint32_t               process_cpu_ratio_;
    //系统的CPU利用率,千分率
    uint32_t               system_cpu_ratio_;

    //真正可以利用的内存的内存大小
    uint64_t               can_use_size_;
    //系统的内存使用率,千分率
    uint32_t               mem_use_ratio_;

    //上一次的进程性能数据
    ZCE_PROCESS_PERFORM    last_process_perf_;
    //当前这一次的进程性能数据
    ZCE_PROCESS_PERFORM    now_process_perf_;

    //上一次的系统的性能数据
    ZCE_SYSTEM_PERFORMANCE last_system_perf_;
    //当前这一次的系统性能数据
    ZCE_SYSTEM_PERFORMANCE now_system_perf_;


};










#endif //_ZCE_LIB_SERVER_TOOLKIT_H_


