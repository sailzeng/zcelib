
#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_mutex.h"
#include "zce_os_adapt_condi.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_rwlock.h"
#include "zce_trace_debugging.h"

//读写锁的对象的初始化
int ZCE_LIB::pthread_rwlock_init(pthread_rwlock_t *rwlock,
                                 const pthread_rwlockattr_t *attr)
{
#if defined (ZCE_OS_WINDOWS)

    //考虑再三，我把重复初始化，是否初始化的各种判定删除了，感觉…………，没必要

    //WIN SVR 2008以后，用特殊的支持
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    if (rwlock->use_win_slim_)
    {
        ::InitializeSRWLock(&(rwlock->rwlock_slim_));
        return 0;
    }

#endif
    //其他倒霉蛋只能模拟

    int result = 0;

    if (attr)
    {
        rwlock->simulate_rw_.priority_to_write_ = attr->priority_to_write_;
    }
    else
    {
        rwlock->simulate_rw_.priority_to_write_ = false;
    }
    pthread_mutexattr_t mutex_attr;
    ZCE_LIB::pthread_mutexattr_init(&mutex_attr);
    mutex_attr.lock_shared_ = PTHREAD_PROCESS_PRIVATE;
    mutex_attr.lock_type_ = PTHREAD_MUTEX_RECURSIVE;

    //初始化几个同步对象

    //一些数据区改写的保护
    if ( (result = ZCE_LIB::pthread_mutex_init(&rwlock->simulate_rw_.rw_mutex_, &mutex_attr)) != 0)
    {
        return EINVAL;
    }

    //等待读取的条件变量初始化
    if ( (result = ZCE_LIB::pthread_cond_initex(&rwlock->simulate_rw_.rw_condreaders_,
                                                false )) != 0)
    {
        ZCE_LIB::pthread_mutex_destroy(&rwlock->simulate_rw_.rw_mutex_);
        return EINVAL;
    }

    //等待写入的条件变量初始化
    if ( (result = ZCE_LIB::pthread_cond_initex(&rwlock->simulate_rw_.rw_condwriters_,
                                                false)) != 0)
    {
        ZCE_LIB::pthread_cond_destroy(&rwlock->simulate_rw_.rw_condreaders_);
        ZCE_LIB::pthread_mutex_destroy(&rwlock->simulate_rw_.rw_mutex_);
        return EINVAL;
    }

    rwlock->simulate_rw_.rw_nwaitreaders_ = 0;
    rwlock->simulate_rw_.rw_nwaitwriters_ = 0;
    rwlock->simulate_rw_.rw_refcount_ = 0;

    return 0;

#elif defined (ZCE_OS_LINUX)

    return ::pthread_rwlock_init(rwlock, attr);
#endif

}

//初始化读写锁对象
int ZCE_LIB::pthread_rwlock_initex(pthread_rwlock_t *rwlock,
                                   bool  use_win_slim,
                                   bool  priority_to_write)
{
    int result = 0;
    pthread_rwlockattr_t attr;

#if defined ZCE_OS_WINDOWS
    attr.priority_to_write_ = priority_to_write;
    //如果支持2008才能设置这个变量
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1
    rwlock->use_win_slim_ = use_win_slim;
#else
    rwlock->use_win_slim_ = false;
    ZCE_UNUSED_ARG(use_win_slim);
#endif

#elif defined ZCE_OS_LINUX

    ZCE_UNUSED_ARG(use_win_slim);
    ZCE_UNUSED_ARG(priority_to_write);

    //其实我在想，不如搞个NULL，其实都一样
    result = ::pthread_rwlockattr_init(&attr);

    if (result != 0)
    {
        return result;
    }

    result = ::pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
    if (result != 0)
    {
        return result;
    }

#endif

    result = ZCE_LIB::pthread_rwlock_init(rwlock, &attr);

    if (result != 0)
    {
        return result;
    }

    return 0;
}

//读写锁的对象的销毁
int ZCE_LIB::pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
#if defined (ZCE_OS_WINDOWS)

    //WIN SVR 2008以后，用特殊的支持
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    if (rwlock->use_win_slim_)
    {
        //::ReleaseSRWLockShared(&(rwlock->rwlock_slim_));
        return 0;
    }

#endif

    //还有等待的，不能销毁
    if (rwlock->simulate_rw_.rw_refcount_ != 0
        || rwlock->simulate_rw_.rw_nwaitreaders_ != 0
        || rwlock->simulate_rw_.rw_nwaitwriters_ != 0)
    {
        return EBUSY;
    }

    ZCE_LIB::pthread_mutex_destroy(&rwlock->simulate_rw_.rw_mutex_);
    ZCE_LIB::pthread_cond_destroy(&rwlock->simulate_rw_.rw_condreaders_);
    ZCE_LIB::pthread_cond_destroy(&rwlock->simulate_rw_.rw_condwriters_);

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_destroy(rwlock);
#endif
}

//获得读取的锁
int ZCE_LIB::pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{

#if defined (ZCE_OS_WINDOWS)

#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    //如果用WIN自带的读写锁
    if (rwlock->use_win_slim_)
    {
        ::AcquireSRWLockShared(&(rwlock->rwlock_slim_));
		rwlock->slim_mode_ = ZCE_SLIM_USE_SHARED_LOCK;
        return 0;
    }

#endif  //WIN SERVER 2008

    //加上保护锁
    int  result = ZCE_LIB::pthread_mutex_lock(&rwlock->simulate_rw_.rw_mutex_);

    if ( 0 != result  )
    {
        return(result);
    }

    //PTHREAD WIN32的实现在这个地方没有用条件变量等待，原因位置，个人认为这是有瑕疵的，
    //可能会导致进入高CPU循环

    //等待获得读写锁，如果有人在写，或者有要写入的人在等待，偏向写优先
    while (  (rwlock->simulate_rw_.rw_refcount_ < 0)
             || ( true == rwlock->simulate_rw_.priority_to_write_ && rwlock->simulate_rw_.rw_nwaitwriters_ > 0 ) )
    {
        rwlock->simulate_rw_.rw_nwaitreaders_++;
        //进入wait函数，simulate_rw_.rw_mutex_会被打开，让其他人活动，出来的时候会获得
        result = ZCE_LIB::pthread_cond_wait(&rwlock->simulate_rw_.rw_condreaders_,
                                            &(rwlock->simulate_rw_.rw_mutex_));
        rwlock->simulate_rw_.rw_nwaitreaders_--;

        if (result != 0)
        {
            break;
        }
    }

    //获得的了读的锁控制
    if (result == 0)
    {
        rwlock->simulate_rw_.rw_refcount_++;
    }

    ZCE_LIB::pthread_mutex_unlock(&rwlock->simulate_rw_.rw_mutex_);
    return (result);

#elif defined (ZCE_OS_LINUX)

    return ::pthread_rwlock_rdlock(rwlock);
#endif

}

//尝试获取读取锁
int ZCE_LIB::pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{

#if defined (ZCE_OS_WINDOWS)

#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    //如果用WIN自带的读写锁
    if (rwlock->use_win_slim_)
    {
        BOOL bret = ::TryAcquireSRWLockShared(&(rwlock->rwlock_slim_));
        if (FALSE == bret )
        {
            return EBUSY;
        }
		rwlock->slim_mode_ = ZCE_SLIM_USE_SHARED_LOCK;
        return 0;
    }

#endif //WIN SERVER 2008

    int result = ZCE_LIB::pthread_mutex_lock(&rwlock->simulate_rw_.rw_mutex_);

    if ( result != 0)
    {
        return(result);
    }

    //如果有人在写，或者有要写入的人在等待,那么就不能get读取锁
    if ((rwlock->simulate_rw_.rw_refcount_ < 0)
        || ( true == rwlock->simulate_rw_.priority_to_write_ && rwlock->simulate_rw_.rw_nwaitwriters_ > 0 ))
    {
        result = EBUSY;
    }
    else
    {
        rwlock->simulate_rw_.rw_refcount_++;
    }

    ZCE_LIB::pthread_mutex_unlock(&rwlock->simulate_rw_.rw_mutex_);
    return(result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_tryrdlock(rwlock);
#endif
}

//读取锁的超时锁定，这个代码UNP V2并没有给出，
int ZCE_LIB::pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock,
                                        const ::timespec *abs_timeout_spec)
{
#if defined (ZCE_OS_WINDOWS)

#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    //如果用WIN自带的读写锁
    if (rwlock->use_win_slim_)
    {
        return EINVAL;
    }

#endif

    int result = ZCE_LIB::pthread_mutex_timedlock(&rwlock->simulate_rw_.rw_mutex_,
                                                  abs_timeout_spec);

    //我有点理解为啥phtread的很多函数用绝对时间了，abs_timeout_spec
    if ( result != 0)
    {
        return(result);
    }

    //等待获得读写锁，如果有人在写，或者有要写入的人在等待，偏向写优先
    while ( (rwlock->simulate_rw_.rw_refcount_ < 0)
            || (true == rwlock->simulate_rw_.priority_to_write_ && rwlock->simulate_rw_.rw_nwaitwriters_ > 0) )
    {
        rwlock->simulate_rw_.rw_nwaitreaders_++;
        //进入wait函数，simulate_rw_.rw_mutex_会被打开，让其他人活动，出来的时候会获得
        result = ZCE_LIB::pthread_cond_timedwait(&rwlock->simulate_rw_.rw_condreaders_,
                                                 &(rwlock->simulate_rw_.rw_mutex_),
                                                 abs_timeout_spec);
        rwlock->simulate_rw_.rw_nwaitreaders_--;

        if (result != 0)
        {
            break;
        }
    }

    //获得的了读的锁控制
    if (result == 0)
    {
        rwlock->simulate_rw_.rw_refcount_++;
    }

    ZCE_LIB::pthread_mutex_unlock(&rwlock->simulate_rw_.rw_mutex_);
    return(result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_timedrdlock(rwlock, abs_timeout_spec);
#endif
}

//非标准，读取锁的超时锁定，时间参数调整成timeval，
int ZCE_LIB::pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock,
                                        const timeval *abs_timeout_val)
{
    //这个时间是绝对值时间，要调整为相对时间
    ::timespec abs_timeout_spec = ZCE_LIB::make_timespec(abs_timeout_val);
    return ZCE_LIB::pthread_rwlock_timedrdlock(rwlock, &abs_timeout_spec);
}

//获取写锁
int ZCE_LIB::pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
#if defined (ZCE_OS_WINDOWS)

#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    //如果用WIN自带的读写锁
    if (rwlock->use_win_slim_)
    {
        ::AcquireSRWLockExclusive(&(rwlock->rwlock_slim_));
		rwlock->slim_mode_ = ZCE_SLIM_USE_EXCLUSIVE_LOCK;
        return 0;
    }

#endif

    int result = pthread_mutex_lock(&rwlock->simulate_rw_.rw_mutex_);
    if ( result != 0)
    {
        return(result);
    }

    //如果有人在使用锁，无论读写，就要等待，如果读取优先，如果有人还在等待读，也等待
    while ( (rwlock->simulate_rw_.rw_refcount_ != 0)
            || (false == rwlock->simulate_rw_.priority_to_write_ && rwlock->simulate_rw_.rw_nwaitreaders_ > 0) )
    {
        rwlock->simulate_rw_.rw_nwaitwriters_++;
        result = ZCE_LIB::pthread_cond_wait(&rwlock->simulate_rw_.rw_condwriters_,
                                            &(rwlock->simulate_rw_.rw_mutex_));
        rwlock->simulate_rw_.rw_nwaitwriters_--;

        if (result != 0)
        {
            break;
        }
    }

    //标识锁被写者获得
    if (result == 0)
    {
        rwlock->simulate_rw_.rw_refcount_ = -1;
    }

    ZCE_LIB::pthread_mutex_unlock(&rwlock->simulate_rw_.rw_mutex_);
    return(result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_wrlock(rwlock);
#endif
}

//尝试能否拥有写锁，非阻塞方式
int ZCE_LIB::pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{

#if defined (ZCE_OS_WINDOWS)

#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    //如果用WIN自带的读写锁
    if (rwlock->use_win_slim_)
    {
        BOOL bret = ::TryAcquireSRWLockExclusive(&(rwlock->rwlock_slim_));
        if (FALSE == bret )
        {
            return EBUSY;
        }
		rwlock->slim_mode_ = ZCE_SLIM_USE_EXCLUSIVE_LOCK;
        return 0;
    }

#endif //WIN SERVER 2008

    int result = ZCE_LIB::pthread_mutex_lock(&rwlock->simulate_rw_.rw_mutex_);

    if ( result != 0)
    {
        return(result);
    }

    //如果有读，写者存在，就不能拥有写锁
    if ( (rwlock->simulate_rw_.rw_refcount_ != 0)
         || (false == rwlock->simulate_rw_.priority_to_write_ && rwlock->simulate_rw_.rw_nwaitreaders_ > 0) )
    {
        result = EBUSY;
    }
    //否则就拥有写锁
    else
    {
        rwlock->simulate_rw_.rw_refcount_ = -1;
    }

    ZCE_LIB::pthread_mutex_unlock(&rwlock->simulate_rw_.rw_mutex_);
    return(result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_trywrlock(rwlock);
#endif
}

//获取写锁，并且等待到超时为止，
int ZCE_LIB::pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock,
                                        const ::timespec *abs_timeout_spec)
{
#if defined (ZCE_OS_WINDOWS)

#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    //如果用WIN自带的读写锁
    if (rwlock->use_win_slim_)
    {
        return EINVAL;
    }

#endif

    int result = ZCE_LIB::pthread_mutex_timedlock(&rwlock->simulate_rw_.rw_mutex_, abs_timeout_spec);

    if ( result != 0)
    {
        return(result);
    }

    //如果有人在使用锁，无论读写，就要等待，如果读取优先，如果有人还在等待读，也等待
    while ((rwlock->simulate_rw_.rw_refcount_ != 0)
           || (false == rwlock->simulate_rw_.priority_to_write_ && rwlock->simulate_rw_.rw_nwaitreaders_ > 0) )
    {
        rwlock->simulate_rw_.rw_nwaitwriters_++;
        result = ZCE_LIB::pthread_cond_timedwait(&rwlock->simulate_rw_.rw_condwriters_,
                                                 &(rwlock->simulate_rw_.rw_mutex_),
                                                 abs_timeout_spec);
        rwlock->simulate_rw_.rw_nwaitwriters_--;

        if (result != 0)
        {
            break;
        }
    }

    if (result == 0)
    {
        rwlock->simulate_rw_.rw_refcount_ = -1;
    }

    ZCE_LIB::pthread_mutex_unlock(&rwlock->simulate_rw_.rw_mutex_);
    return(result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_timedwrlock(rwlock, abs_timeout_spec);
#endif
}

//非标准，读取锁的超时锁定，时间参数调整成timeval，
int ZCE_LIB::pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock,
                                        const timeval *abs_timeout_val)
{
    //这个时间是绝对值时间，要调整为相对时间
    ::timespec abs_timeout_spec = ZCE_LIB::make_timespec(abs_timeout_val);
    return ZCE_LIB::pthread_rwlock_timedwrlock(rwlock, &abs_timeout_spec);
}

//解除锁定，这个函数可以解除读取锁定和写入锁定，不需要特别指明
int ZCE_LIB::pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
#if defined (ZCE_OS_WINDOWS)

#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    //如果用WIN自带的读写锁
    if (rwlock->use_win_slim_)
    {
        if (ZCE_SLIM_USE_SHARED_LOCK == rwlock->slim_mode_)
        {
            ::ReleaseSRWLockShared(&(rwlock->rwlock_slim_));
        }
        else
        {
            ::ReleaseSRWLockExclusive(&(rwlock->rwlock_slim_));
        }
        return 0;
    }

#endif

    //上锁，
    int result = ZCE_LIB::pthread_mutex_lock(&rwlock->simulate_rw_.rw_mutex_);

    if ( result != 0)
    {
        return(result);
    }

    //如果是读者占用了锁，
    if (rwlock->simulate_rw_.rw_refcount_ > 0)
    {
        rwlock->simulate_rw_.rw_refcount_--;
    }
    //如果是作家（写者）占用了锁，
    else if (rwlock->simulate_rw_.rw_refcount_ == -1)
    {
        rwlock->simulate_rw_.rw_refcount_ = 0;
    }
    //理论上不会到这儿
    else
    {
        //到这儿，应该是你代码写错了，没有加锁，但是你调用了解锁函数
    }

    //根据读优先还是写入优先，进行处理
    //如果写优先
    if (rwlock->simulate_rw_.priority_to_write_)
    {
        //如果这时候，有写入的人等待，优先给作家发个信号
        if (rwlock->simulate_rw_.rw_nwaitwriters_ > 0)
        {
            if (rwlock->simulate_rw_.rw_refcount_ == 0)
            {
                result = ZCE_LIB::pthread_cond_signal(&rwlock->simulate_rw_.rw_condwriters_);
            }
        }
        //如果这时候，有读者的在等待，给读者做个广播
        else if (rwlock->simulate_rw_.rw_nwaitreaders_ > 0)
        {
            result = ZCE_LIB::pthread_cond_broadcast(&rwlock->simulate_rw_.rw_condreaders_);
        }
    }
    //如果是读取优先
    else
    {
        if (rwlock->simulate_rw_.rw_nwaitreaders_ > 0)
        {
            result = ZCE_LIB::pthread_cond_broadcast(&rwlock->simulate_rw_.rw_condreaders_);
        }
        //如果这时候，有写入的人等待，优先给作家发个信号
        else if (rwlock->simulate_rw_.rw_nwaitwriters_ > 0)
        {
            if (rwlock->simulate_rw_.rw_refcount_ == 0)
            {
                result = ZCE_LIB::pthread_cond_signal(&rwlock->simulate_rw_.rw_condwriters_);
            }
        }

        //如果这时候，有读者的在等待，给读者做个广播
    }

    ZCE_LIB::pthread_mutex_unlock(&rwlock->simulate_rw_.rw_mutex_);
    return(result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_unlock(rwlock);
#endif

}

