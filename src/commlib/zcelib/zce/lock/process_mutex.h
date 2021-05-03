/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/process_mutex.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年1月15日
* @brief      进程的MUTEX，必须有名字，
*
*/
#ifndef ZCE_LIB_LOCK_PROCESS_MUTEX_H_
#define ZCE_LIB_LOCK_PROCESS_MUTEX_H_

#include "zce/lock/lock_base.h"
#include "zce/lock/lock_guard.h"
#include "zce/shared_mem/posix.h"

/*!
* @brief      进程Mutex锁,用于多进程间的互斥同步处理，
*             在WINDOWS下，递归锁是用MUTEX模拟的（必须有名字），非递归锁是用信号灯模拟，
*             在LINUX下，还是用pthread_mutex实现的，但是用共享内存存放，名字用于共享
*             内存名字
*/
class ZCE_Process_Mutex : public ZCE_Lock_Base
{

public:

    //进程锁的GUARD
    typedef ZCE_Lock_Guard<ZCE_Process_Mutex> LOCK_GUARD;

public:
    ///构造函数
    ZCE_Process_Mutex (const char *mutex_name, bool recursive = true);
    ///析构函数
    virtual ~ZCE_Process_Mutex (void);

    ///锁定
    virtual void lock();

    ///尝试锁定
    virtual bool try_lock();

    ///解锁,
    virtual void unlock();

    ///绝对时间超时的的锁定，超时后解锁
    virtual bool systime_lock(const zce::Time_Value &abs_time);

    ///相对时间的超时锁定，超时后，解锁
    virtual bool duration_lock(const zce::Time_Value &relative_time);

    ///取出内部的锁的指针
    pthread_mutex_t *get_lock();

protected:

    ///进程锁，注意，注意，注意，这个地方用的是一个指针，
    pthread_mutex_t    *lock_;

#if defined ZCE_OS_LINUX
    //共享内存，LINUX需要共享内存
    ZCE_ShareMem_Posix posix_sharemem_;
#endif

};

#endif //ZCE_LIB_LOCK_PROCESS_MUTEX_H_

