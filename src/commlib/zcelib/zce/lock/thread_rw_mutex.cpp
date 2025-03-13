//线程的互斥量，有轻量的锁，递归锁，非递归锁，读写锁

#include "zce/predefine.h"

#include "zce/os_adapt/mutex.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/rwlock.h"
#include "zce/os_adapt/semaphore.h"

#include "zce/logger/logging.h"
#include "zce/lock/thread_rw_mutex.h"

namespace zce
{
/************************************************************************************************************
Class           : thread_rw_mutex
************************************************************************************************************/
//构造函数
thread_rw_mutex::thread_rw_mutex()
{
    //pthread_rwlockattr_t属性的初始化
    int ret = 0;
    pthread_rwlockattr_t attr;
    ret = zce::pthread_rwlock_init(&rw_lock_,&attr);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR,"zce::pthread_mutex_init",ret);
        return;
    }
}

thread_rw_mutex::~thread_rw_mutex()
{
    int ret = 0;
    ret = zce::pthread_rwlock_destroy(&rw_lock_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR,"zce::pthread_rwlock_destroy",ret);
        return;
    }
}

//读取锁
void thread_rw_mutex::lock_shared() noexcept
{
    int ret = 0;
    ret = zce::pthread_rwlock_rdlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR,"zce::pthread_rwlock_rdlock",ret);
        return;
    }
}

//尝试读取锁
bool thread_rw_mutex::try_lock_shared() noexcept
{
    int ret = 0;
    ret = zce::pthread_rwlock_trywrlock(&rw_lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//绝对时间
bool thread_rw_mutex::try_lock_shared_until(const zce::time_value& abs_time) noexcept
{
    int ret = 0;

    ret = zce::pthread_rwlock_timedrdlock(&rw_lock_,
                                          abs_time);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR,"zce::pthread_rwlock_timedrdlock",ret);
        return false;
    }

    return true;
}
//相对时间
bool thread_rw_mutex::try_lock_shared_for(const zce::time_value& relative_time) noexcept
{
    zce::time_value abs_time(zce::gettimeofday());
    abs_time += relative_time;
    return try_lock_shared_until(abs_time);
}

//解读锁
void thread_rw_mutex::unlock_shared() noexcept
{
    //解锁
    int ret = 0;
    ret = zce::pthread_rwlock_rdunlock(&rw_lock_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR,"zce::pthread_rwlock_unlock",ret);
        return;
    }
}

//写锁定
void thread_rw_mutex::lock() noexcept
{
    int ret = 0;
    ret = zce::pthread_rwlock_wrlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR,"zce::pthread_rwlock_wrlock",ret);
        return;
    }
}

//尝试读取锁
bool thread_rw_mutex::try_lock() noexcept
{
    int ret = 0;
    ret = zce::pthread_rwlock_trywrlock(&rw_lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//写锁定超时，绝对时间
bool thread_rw_mutex::try_lock_until(const zce::time_value& abs_time) noexcept
{
    int ret = 0;

    ret = zce::pthread_rwlock_timedwrlock(&rw_lock_,
                                          abs_time);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR,"zce::pthread_mutex_timedlock",ret);
        return false;
    }

    return true;
}

//写锁定超时，相对时间
bool thread_rw_mutex::try_lock_for(const zce::time_value& relative_time) noexcept
{
    zce::time_value abs_time(zce::gettimeofday());
    abs_time += relative_time;
    return try_lock_until(abs_time);
}

//解写锁
void thread_rw_mutex::unlock() noexcept
{
    //解锁
    int ret = 0;
    ret = zce::pthread_rwlock_wrunlock(&rw_lock_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR,"zce::pthread_rwlock_unlock",ret);
        return;
    }
}

//取出内部的锁的指针
pthread_rwlock_t* thread_rw_mutex::get_lock()
{
    return &rw_lock_;
}
}