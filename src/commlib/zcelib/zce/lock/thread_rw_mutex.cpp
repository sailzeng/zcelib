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

    ret = zce::pthread_rwlock_initex(&rw_lock_, false);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_init", ret);
        return;
    }
}

thread_rw_mutex::~thread_rw_mutex()
{
    int ret = 0;
    ret = zce::pthread_rwlock_destroy(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_destroy", ret);
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
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_rdlock", ret);
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
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_timedrdlock", ret);
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
    ret = zce::pthread_rwlock_unlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_unlock", ret);
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
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_wrlock", ret);
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
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_timedlock", ret);
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
    ret = zce::pthread_rwlock_unlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_unlock", ret);
        return;
    }
}

//取出内部的锁的指针
pthread_rwlock_t* thread_rw_mutex::get_lock()
{
    return &rw_lock_;
}

/************************************************************************************************************
Class           : thread_win_rw_mutex 轻量级的读写锁，不提供超时等函数
************************************************************************************************************/

#if defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008

//如果用WIN自带的读写锁

//构造函数
thread_win_rw_mutex::thread_win_rw_mutex()
{
    ::InitializeSRWLock(&(this->rwlock_slim_));
}

thread_win_rw_mutex::~thread_win_rw_mutex()
{
}

//读取锁
void thread_win_rw_mutex::lock_shared() noexcept
{
    ::AcquireSRWLockShared(&(this->rwlock_slim_));
    return;
}

//尝试读取锁
bool thread_win_rw_mutex::try_lock_shared() noexcept
{
    //如果用WIN自带的读写锁
    BOOL bret = ::TryAcquireSRWLockShared(&(this->rwlock_slim_));
    if (FALSE == bret)
    {
        errno = EBUSY;
        return false;
    }
    return true;
}

//解锁,如果是读写锁也只需要这一个函数
void thread_win_rw_mutex::unlock_shared() noexcept
{
    ::ReleaseSRWLockShared(&(this->rwlock_slim_));
}

//写锁定
void thread_win_rw_mutex::lock() noexcept
{
    ::AcquireSRWLockExclusive(&(this->rwlock_slim_));
    return;
}

//尝试读取锁
bool thread_win_rw_mutex::try_lock() noexcept
{
    //如果用WIN自带的读写锁
    BOOL bret = ::TryAcquireSRWLockExclusive(&(this->rwlock_slim_));
    if (FALSE == bret)
    {
        errno = EBUSY;
        return false;
    }
    return true;
}

//这肯定是VS2019的一个BUG。理论上不应该有这个告警
#pragma warning (disable:26110)

void thread_win_rw_mutex::unlock() noexcept
{
    ::ReleaseSRWLockExclusive(&(this->rwlock_slim_));
}

#pragma warning (default:26110)

//取出内部的锁的指针
SRWLOCK* thread_win_rw_mutex::get_lock()
{
    return &rwlock_slim_;
}

#endif
}