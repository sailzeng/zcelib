#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/define.h"
#include "zce/os_adapt/mutex.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/rwlock.h"

#if defined (ZCE_OS_WINDOWS)
intptr_t NO_LOCK = (intptr_t)0x0;
intptr_t READ_LOCK = (intptr_t)0x1;
intptr_t WRITE_LOCK = (intptr_t)0x2;
#endif

//读写锁的对象的初始化
int zce::pthread_rwlock_init(pthread_rwlock_t* rwlock,
                             const pthread_rwlockattr_t* attr)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(attr);
    //其他倒霉蛋只能模拟,Windows 2008以后的版本才有的RWLOCK,但又不支持超时，函数接口也不太一样
    ::InitializeSRWLock(&rwlock->rwlock_slim_);
    ::InitializeConditionVariable(&rwlock->cv_);
    rwlock->tls_rwlock_status_ = ::TlsAlloc();
    if (TLS_OUT_OF_INDEXES == rwlock->tls_rwlock_status_)
    {
        return ::GetLastError();
    }
    if (FALSE == ::TlsSetValue(rwlock->tls_rwlock_status_, (LPVOID)NO_LOCK))
    {
        return ::GetLastError();
    }
    return 0;

#elif defined (ZCE_OS_LINUX)

    return ::pthread_rwlock_init(rwlock, attr);
#endif
}

//读写锁的对象的销毁
int zce::pthread_rwlock_destroy(pthread_rwlock_t* rwlock)
{
#if defined (ZCE_OS_WINDOWS)
    if (::TlsFree(rwlock->tls_rwlock_status_) == FALSE)
    {
        return ::GetLastError();
    }
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_destroy(rwlock);
#endif
}

//获得读取的锁
int zce::pthread_rwlock_rdlock(pthread_rwlock_t* rwlock)
{
#if defined (ZCE_OS_WINDOWS)

    ::AcquireSRWLockShared(&rwlock->rwlock_slim_);
    ::TlsSetValue(rwlock->tls_rwlock_status_, (LPVOID)READ_LOCK);
    return  0;

#elif defined (ZCE_OS_LINUX)

    return ::pthread_rwlock_rdlock(rwlock);
#endif
}

//尝试获取读取锁
int zce::pthread_rwlock_tryrdlock(pthread_rwlock_t* rwlock)
{
#if defined (ZCE_OS_WINDOWS)

    BOOL bret = ::TryAcquireSRWLockShared(&rwlock->rwlock_slim_);
    if (FALSE == bret)
    {
        errno = EBUSY;
        return -1;
    }
    ::TlsSetValue(rwlock->tls_rwlock_status_, (LPVOID)READ_LOCK);
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_tryrdlock(rwlock);
#endif
}

//读取锁的超时锁定，这个代码UNP V2并没有给出，
int zce::pthread_rwlock_timedrdlock(pthread_rwlock_t* rwlock,
                                    const ::timespec* abs_timeout_spec)
{
#if defined (ZCE_OS_WINDOWS)
    //如果有超时，计算相对超时时间
    timeval abs_time;
    if (abs_timeout_spec)
    {
        //得到相对时间，这个折腾，

        abs_time = zce::make_timeval(abs_timeout_spec);
    }

    while (::TryAcquireSRWLockShared(&rwlock->rwlock_slim_) != TRUE)
    {
        if (abs_timeout_spec)
        {
            timeval now_time = zce::gettimeofday();
            timeval timeout_time = zce::timeval_sub(abs_time, now_time, true);
            DWORD wait_msec = (DWORD)zce::total_milliseconds(timeout_time);
            if (wait_msec > 0)
            {
                ::SleepConditionVariableSRW(&rwlock->cv_, &rwlock->rwlock_slim_, wait_msec, 0);
            }
            else
            {
                errno = ETIMEDOUT;
                return -1;
            }
        }
    }
    ::TlsSetValue(rwlock->tls_rwlock_status_, (LPVOID)READ_LOCK);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_timedrdlock(rwlock, abs_timeout_spec);
#endif
}

//非标准，读取锁的超时锁定，时间参数调整成timeval，
int zce::pthread_rwlock_timedrdlock(pthread_rwlock_t* rwlock,
                                    const timeval* abs_timeout_val)
{
    //这个时间是绝对值时间，要调整为相对时间
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::pthread_rwlock_timedrdlock(rwlock, &abs_timeout_spec);
}

//获取写锁
int zce::pthread_rwlock_wrlock(pthread_rwlock_t* rwlock)
{
#if defined (ZCE_OS_WINDOWS)

    ::AcquireSRWLockExclusive(&(rwlock->rwlock_slim_));
    ::TlsSetValue(rwlock->tls_rwlock_status_, (LPVOID)WRITE_LOCK);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_wrlock(rwlock);
#endif
}

//尝试能否拥有写锁，非阻塞方式
int zce::pthread_rwlock_trywrlock(pthread_rwlock_t* rwlock)
{
#if defined (ZCE_OS_WINDOWS)

    //如果用WIN自带的读写锁
    BOOL bret = ::TryAcquireSRWLockExclusive(&(rwlock->rwlock_slim_));
    if (FALSE == bret)
    {
        errno = EBUSY;
        return -1;
    }
    ::TlsSetValue(rwlock->tls_rwlock_status_, (LPVOID)WRITE_LOCK);
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_trywrlock(rwlock);
#endif
}

//获取写锁，并且等待到超时为止，
int zce::pthread_rwlock_timedwrlock(pthread_rwlock_t* rwlock,
                                    const ::timespec* abs_timeout_spec)
{
#if defined (ZCE_OS_WINDOWS)
    timeval abs_time;
    if (abs_timeout_spec)
    {
        //得到相对时间，这个折腾，

        abs_time = zce::make_timeval(abs_timeout_spec);
    }

    while (::TryAcquireSRWLockExclusive(&rwlock->rwlock_slim_) != TRUE)
    {
        if (abs_timeout_spec)
        {
            timeval now_time = zce::gettimeofday();
            timeval timeout_time = zce::timeval_sub(abs_time, now_time, true);
            DWORD wait_msec = (DWORD)zce::total_milliseconds(timeout_time);
            if (wait_msec > 0)
            {
                ::SleepConditionVariableSRW(&rwlock->cv_, &rwlock->rwlock_slim_, wait_msec, 0);
            }
            else
            {
                errno = ETIMEDOUT;
                return -1;
            }
        }
    }
    ::TlsSetValue(rwlock->tls_rwlock_status_, (LPVOID)WRITE_LOCK);
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_timedwrlock(rwlock, abs_timeout_spec);
#endif
}

//非标准，读取锁的超时锁定，时间参数调整成timeval，
int zce::pthread_rwlock_timedwrlock(pthread_rwlock_t* rwlock,
                                    const timeval* abs_timeout_val)
{
    //这个时间是绝对值时间，要调整为相对时间
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::pthread_rwlock_timedwrlock(rwlock, &abs_timeout_spec);
}

//解除锁定，这个函数可以解除读取锁定和写入锁定，不需要特别指明
int zce::pthread_rwlock_unlock(pthread_rwlock_t* rwlock)
{
#if defined (ZCE_OS_WINDOWS)

    LPVOID rw_staus = ::TlsGetValue(rwlock->tls_rwlock_status_);
    if (rw_staus == (LPVOID)READ_LOCK)
    {
        ::ReleaseSRWLockShared(&rwlock->rwlock_slim_);
    }
    else if (rw_staus == (LPVOID)WRITE_LOCK)
    {
        ::ReleaseSRWLockExclusive(&rwlock->rwlock_slim_);
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
    ::TlsSetValue(rwlock->tls_rwlock_status_, (LPVOID)NO_LOCK);

    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_unlock(rwlock);
#endif
}

int zce::pthread_rwlock_wrunlock(pthread_rwlock_t* rwlock)
{
#if defined (ZCE_OS_WINDOWS)

    ::ReleaseSRWLockExclusive(&rwlock->rwlock_slim_);
    ::TlsSetValue(rwlock->tls_rwlock_status_, (LPVOID)NO_LOCK);

    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_unlock(rwlock);
#endif
}

int zce::pthread_rwlock_rdunlock(pthread_rwlock_t* rwlock)
{
#if defined (ZCE_OS_WINDOWS)
    ::ReleaseSRWLockShared(&rwlock->rwlock_slim_);
    ::TlsSetValue(rwlock->tls_rwlock_status_, (LPVOID)0);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_unlock(rwlock);
#endif
}
