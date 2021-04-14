#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/common.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/string.h"
#include "zce/os_adapt/semaphore.h"
#include "zce/os_adapt/mutex.h"

/*****************************************************************************/
// MUTEXES BEGIN
/*****************************************************************************/

//用临界区模拟的PTHREAD MUTEX，要求进程内部（线程），递归，不需要超时
#ifndef ZCE_IS_CS_SIMULATE_PMUTEX
#define ZCE_IS_CS_SIMULATE_PMUTEX(mutex) ( (PTHREAD_PROCESS_PRIVATE == (mutex)->lock_shared_  ) \
                                           && (ZCE_BIT_IS_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_RECURSIVE))  \
                                           && (ZCE_BIT_ISNOT_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_TIMEOUT)) )
#endif

//用MUTEX模拟PTHREAD MUTEX，多进程，递归，或者进程内部，递归，而且有超时
#ifndef ZCE_IS_MUTEX_SIMULATE_PMUTEX
#define ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex) ( ( (PTHREAD_PROCESS_SHARED == (mutex)->lock_shared_ ) \
                                                && (ZCE_BIT_IS_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_RECURSIVE)) ) \
                                              || ( (PTHREAD_PROCESS_PRIVATE == (mutex)->lock_shared_) \
                                                   && (ZCE_BIT_IS_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_RECURSIVE))  \
                                                   && (ZCE_BIT_IS_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_TIMEOUT)) ) )
#endif

//用信号灯模拟PTHREAD MUTEX，非递归
#ifndef ZCE_IS_SEMA_SIMULATE_PMUTEX
#define ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex) (!ZCE_BIT_IS_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_RECURSIVE))
#endif

//互斥量属性初始化
int zce::pthread_mutexattr_init (pthread_mutexattr_t *attr)
{
#if defined (ZCE_OS_WINDOWS)
    //线程独有
    attr->lock_shared_ = PTHREAD_PROCESS_PRIVATE;
    //WINDOWS下默认就是递归的，你要我搞个非递归的我还要折腾
    attr->lock_type_ = PTHREAD_MUTEX_RECURSIVE;
    //初始化名称
    attr->mutex_name_[PATH_MAX] = '\0';
    attr->mutex_name_[0] = '\0';
    return 0;

#elif defined (ZCE_OS_LINUX)

    return ::pthread_mutexattr_init (attr);
#endif
}

//互斥量属性销毁
int zce::pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(attr);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutexattr_init (attr);
#endif
}

//互斥量属性设置共享属性PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
int zce::pthread_mutexattr_setpshared (pthread_mutexattr_t *attr,
                                       int pshared)
{
#if defined (ZCE_OS_WINDOWS)

    attr->lock_shared_ = pshared;

    //如果MUTEX的名字是空的,必须给他赋个值，
    if ( '\0' == attr->mutex_name_[0] && PTHREAD_PROCESS_SHARED == pshared )
    {
        const char *MUTEX_PREFIX = "MUTEX";
        zce::prefix_unique_name(MUTEX_PREFIX, attr->mutex_name_, PATH_MAX);
    }

    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutexattr_setpshared (attr, pshared);
#endif
}

//取得线程的共享属性
int zce::pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr,
                                      int *pshared)
{
#if defined (ZCE_OS_WINDOWS)
    *pshared = attr->lock_shared_;
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutexattr_getpshared (attr, pshared);
#endif
}

//设置线程的属性，PTHREAD_MUTEX_XXXX的几个值，可以或者|一次设置多个属性
int zce::pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
#if defined (ZCE_OS_WINDOWS)
    attr->lock_type_ = type;
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutexattr_settype (attr, type);
#endif
}

//取得线程的属性
int zce::pthread_mutexattr_gettype(const pthread_mutexattr_t *attr,
                                   int *type)
{
#if defined (ZCE_OS_WINDOWS)
    *type = attr->lock_type_;
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutexattr_gettype (attr, type);
#endif
}

//设置线程的属性，不同的平台给不同的默认定义
//非标准，但是建议你使用，简单多了,
//如果要多进程共享，麻烦你老给个名字，同时在LINUX平台下，你必须pthread_mutex_t放入共享内存
int zce::pthread_mutex_initex(pthread_mutex_t *mutex,
                              bool process_share,
                              bool recursive,
                              bool need_timeout,
                              const char *mutex_name)
{

    //前面有错误返回，
    int result = 0;
    pthread_mutexattr_t attr;
    result = zce::pthread_mutexattr_init (&attr);

    if (0 != result)
    {
        return result;
    }

    int lock_shared = 0;
    if (process_share)
    {
        lock_shared = PTHREAD_PROCESS_SHARED;

        //进程共享，必须有个名字，WIN和LINUX都必须一样，LINUX下用名字创建共享内存
        if (mutex_name)
        {
#if defined (ZCE_OS_WINDOWS)
            strncpy(attr.mutex_name_, mutex_name, PATH_MAX);
#endif
        }
    }
    else
    {
        lock_shared = PTHREAD_PROCESS_PRIVATE;
    }

    //设置共享属性
    result = zce::pthread_mutexattr_setpshared(&attr, lock_shared);

    if (0 != result)
    {
        zce::pthread_mutexattr_destroy (&attr);
        return -1;
    }

    int lock_type = 0;

    if ( recursive )
    {
        lock_type |= PTHREAD_MUTEX_RECURSIVE;
    }

    //这个是我在WINDOWS下用的，用于某些时候我可以用临界区，而不是更重的互斥量
#if defined (ZCE_OS_WINDOWS)

    if (need_timeout)
    {
        lock_type |= PTHREAD_MUTEX_TIMEOUT;
    }

#else
    ZCE_UNUSED_ARG(need_timeout);
#endif

    //设置属性
    result = zce::pthread_mutexattr_settype(&attr, lock_type);

    if (0 != result)
    {
        zce::pthread_mutexattr_destroy (&attr);
        return result;
    }

    result = zce::pthread_mutex_init(mutex, &attr);
    zce::pthread_mutexattr_destroy (&attr);

    if (0 != result)
    {
        return result;
    }

    return 0;
}

//pthread mutex 初始化,
//在Windows下根据不同的需要，分别会使用不同的同步对象模拟Mutex，
//如果需要非递归锁，那么用信号灯
//如果是递归的，线程内部递归的，而且不需要超时等待，用轻量级的临界区模拟
//如果需要递归的，线程内部需要超时的，进程间的，那么选择MUTEX，
int zce::pthread_mutex_init (pthread_mutex_t *mutex,
                             const pthread_mutexattr_t *attr)
{
    //
#if defined (ZCE_OS_WINDOWS)

    //根据attr是否为NULL，如果NULL，初始化为默认参数
    const char *mutex_name = NULL;
    if (attr)
    {
        mutex->lock_shared_ = attr->lock_shared_;
        mutex->lock_type_ = attr->lock_type_;
        mutex_name = (( '\0' == attr->mutex_name_[0]  ) ? NULL : attr->mutex_name_);
    }
    else
    {
        mutex->lock_shared_ = PTHREAD_PROCESS_PRIVATE;
        mutex->lock_type_ = PTHREAD_MUTEX_NORMAL;
    }

    //我实在不好为内存泄漏做太多事情，如果你多次初始化，我还真没有太好的法子，（比较可行的法子是pthread_mutex_t用构造函数.?）

    //如果合适用信号灯模拟PTHREAD MUTEX （非递归）
    if (ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex))
    {
        mutex->non_recursive_mutex_ = NULL;

        //注意信号灯函数都是return -1表示失败，错误保存在errno，这个地方要转换

        //如果不需要名字，创建匿名信号灯
        if ( NULL == mutex_name )
        {
            mutex->non_recursive_mutex_ = new sem_t();
            int ret = zce::sem_init(mutex->non_recursive_mutex_,
                                    mutex->lock_shared_,
                                    1,
                                    1);

            if (ret != 0)
            {
                delete mutex->non_recursive_mutex_;
                mutex->non_recursive_mutex_ = NULL;

                //得到最后的错误，如果没有就用EINVAL
                return last_error_with_default(EINVAL);
            }
        }
        else
        {
            //,当前值和最大值都调整成1
            mutex->non_recursive_mutex_ = zce::sem_open(mutex_name,
                                                        O_CREAT,
                                                        ZCE_DEFAULT_FILE_PERMS,
                                                        1,
                                                        1);
        }

        if (!mutex->non_recursive_mutex_)
        {
            return last_error_with_default(EINVAL);
        }

        return 0;
    }
    //如果合适用CS模拟PTHREAD MUTEX，进程内部，递归，不需要超时
    else if (ZCE_IS_CS_SIMULATE_PMUTEX(mutex))
    {
        //这个异常在VISTA后被淘汰了
        __try
        {
            ::InitializeCriticalSection (&mutex->thr_nontimeout_mutex_);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return ENOMEM;
        }

        return 0;
    }
    //如果合适MUTEX模拟PTHREAD MUTEX，进程内部，递归而且需要超时 或者进程外部，递归，
    else if (ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex))
    {
        mutex->recursive_mutex_ = ::CreateMutexA (NULL,
                                                  FALSE,
                                                  mutex_name);

        if (mutex->recursive_mutex_ == 0)
        {
            return EINVAL;
        }
        else
        {
            //到这儿是已经成功的创建，但因为其可能已经存在，此时会返回错误 ERROR_ALREADY_EXISTS
            errno = ::GetLastError ();
            return 0;
        }
    }
    else
    {
        return EINVAL;
    }

    //永远不到这儿
    //return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutex_init (mutex, attr);
#endif
}

//销毁MUTEX对象
int zce::pthread_mutex_destroy (pthread_mutex_t *mutex)
{
#if defined (ZCE_OS_WINDOWS)

    //如果合适用信号灯模拟PTHREAD MUTEX （非递归）
    if (ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex))
    {
        int ret = 0;

        //关闭信号灯,如果你没有初始化就关闭，会崩溃的，有名的和无名的采用不同的方式
        if (  true == mutex->non_recursive_mutex_->sem_unnamed_ )
        {
            ret = zce::sem_destroy(mutex->non_recursive_mutex_);

            if (0 != ret)
            {
                return last_error_with_default(EINVAL);
            }
        }
        else
        {
            ret = zce::sem_close(mutex->non_recursive_mutex_);

            if (0 != ret)
            {
                return last_error_with_default(EINVAL);
            }

            //WINDOWS下没哟必要调用sem_unlink，偷懒，如果不偷懒，要在sem_close前要得到这个名字
            //zce::sem_unlink(sema_name);
        }

        return 0;
    }
    //如果合适用CS模拟PTHREAD MUTEX，进程内部，递归，不需要超时
    else if (ZCE_IS_CS_SIMULATE_PMUTEX(mutex))
    {
        ::DeleteCriticalSection (&(mutex->thr_nontimeout_mutex_));
        return 0;
    }
    //如果合适MUTEX模拟PTHREAD MUTEX，进程内部，递归而且需要超时 或者进程外部，递归，
    else if (ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex))
    {
        ::CloseHandle (mutex->recursive_mutex_);
        return 0;
    }
    else
    {
        return EINVAL;
    }

#elif defined (ZCE_OS_LINUX)

    return ::pthread_mutex_destroy (mutex);
#endif
}

int zce::pthread_mutex_lock (pthread_mutex_t *mutex)
{

#if defined (ZCE_OS_WINDOWS)

    //如果合适用信号灯模拟PTHREAD MUTEX （非递归）
    if (ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex))
    {
        //锁定信号灯
        int ret = zce::sem_wait(mutex->non_recursive_mutex_);

        if (0 != ret)
        {
            return last_error_with_default(EINVAL);
        }

        return 0;
    }
    //如果合适用CS模拟PTHREAD MUTEX，进程内部，递归，不需要超时
    else if (ZCE_IS_CS_SIMULATE_PMUTEX(mutex))
    {
        ::EnterCriticalSection (&(mutex->thr_nontimeout_mutex_));
        return 0;
    }
    //如果合适MUTEX模拟PTHREAD MUTEX，进程内部，递归而且需要超时 或者进程外部，递归，
    else if (ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex))
    {
        DWORD retsult = ::WaitForSingleObject (mutex->recursive_mutex_, INFINITE);

        if (WAIT_OBJECT_0 == retsult || WAIT_ABANDONED == retsult)
        {
            return 0;
        }
        else
        {
            return EINVAL;
        }
    }
    else
    {
        return EINVAL;
    }

#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutex_lock(mutex);
# endif

}

//pthread mutex 超时锁定，
int zce::pthread_mutex_timedlock(pthread_mutex_t *mutex,
                                 const ::timespec *abs_timeout_spec)
{
#if defined (ZCE_OS_WINDOWS)
    //
    assert(abs_timeout_spec);

    //得到相对时间，这个折腾，
    timeval now_time = zce::gettimeofday();
    timeval abs_time = zce::make_timeval(abs_timeout_spec);

    timeval timeout_time = zce::timeval_sub(abs_time, now_time, true);

    //如果是可以递归的
    if (ZCE_BIT_IS_SET(mutex->lock_type_, PTHREAD_MUTEX_RECURSIVE))
    {
        if  ( (PTHREAD_PROCESS_SHARED == mutex->lock_shared_ )
              || (PTHREAD_PROCESS_PRIVATE == mutex->lock_shared_
                  && ZCE_BIT_IS_SET(mutex->lock_type_, PTHREAD_MUTEX_TIMEOUT))
            )
        {
            //等待时间触发
            DWORD retsult = ::WaitForSingleObject (mutex->recursive_mutex_,
                                                   static_cast<DWORD>( zce::total_milliseconds(timeout_time)));

            if (WAIT_OBJECT_0 == retsult || WAIT_ABANDONED == retsult)
            {
                return 0;
            }
            else if (WAIT_TIMEOUT == retsult)
            {
                return ETIMEDOUT;
            }
            else
            {
                return EINVAL;
            }
        }
        //明确了这种方式是不支持超时的，如果到这儿标识你调用错了初始化函数
        else if ( PTHREAD_PROCESS_PRIVATE == mutex->lock_shared_
                  && !ZCE_BIT_IS_SET(mutex->lock_type_, PTHREAD_MUTEX_TIMEOUT) )
        {
            return ENOTSUP;
        }
        else
        {
            return EINVAL;
        }
    }
    //非递归的锁
    else
    {
        //锁定信号灯，超时退出
        int ret = zce::sem_timedwait(mutex->non_recursive_mutex_, abs_timeout_spec);

        if (0 != ret )
        {
            return last_error_with_default(EINVAL);
        }

        return 0;
    }

#elif defined (ZCE_OS_LINUX)
    int result = 0;
    result = ::pthread_mutex_timedlock (mutex, abs_timeout_spec);

    //ACE的代码里面有一段转换错误ID的，将ETIMEDOUT转换为ETIME，（ETIME和ETIMEDOUT值一般不一样）
    //但我看过LINUX的手册返回的是ETIMEDOUT，ACE的代码是向SUN的线程函数靠齐的，

    if (result != 0 && result == ETIME)
    {
        errno = ETIMEDOUT;
    }

    return result;

#endif
}

//pthread mutex 超时锁定，非标准实现,是用我内部的时间变量timeval
int zce::pthread_mutex_timedlock (pthread_mutex_t *mutex,
                                  const timeval *abs_timeout_val)
{
    assert(abs_timeout_val);
    //这个时间是绝对值时间，要调整为相对时间
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::pthread_mutex_timedlock(mutex, &abs_timeout_spec);
}

//pthread mutex 尝试加锁
int zce::pthread_mutex_trylock (pthread_mutex_t *mutex)
{

#if defined (ZCE_OS_WINDOWS)

    //如果合适用信号灯模拟PTHREAD MUTEX （非递归）
    if (ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex))
    {
        //测试信号灯
        int ret = zce::sem_trywait(mutex->non_recursive_mutex_);

        if (0 != ret)
        {
            return last_error_with_default(EINVAL);
        }

        return 0;
    }
    //如果合适用CS模拟PTHREAD MUTEX，进程内部，递归，不需要超时
    else if (ZCE_IS_CS_SIMULATE_PMUTEX(mutex))
    {
        //如果可以进入临界区，返回值应该是TRUE
        BOOL bool_ret = ::TryEnterCriticalSection(&(mutex->thr_nontimeout_mutex_));

        if ( FALSE == bool_ret)
        {
            return EBUSY;
        }

        return 0;
    }
    //如果合适MUTEX模拟PTHREAD MUTEX，进程内部，递归而且需要超时 或者进程外部，递归，
    else if (ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex))
    {
        //等待0长度时间触发，不会阻塞，就是try
        DWORD retsult = ::WaitForSingleObject (mutex->recursive_mutex_, 0);

        if (WAIT_OBJECT_0 == retsult || WAIT_ABANDONED == retsult)
        {
            return 0;
        }
        else if (WAIT_TIMEOUT == retsult)
        {
            return EBUSY;
        }
        else
        {
            return EINVAL;
        }
    }
    else
    {
        return EINVAL;
    }

#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutex_trylock (mutex);
#endif

}

//pthread mutex 解锁
int zce::pthread_mutex_unlock (pthread_mutex_t *mutex)
{

#if defined (ZCE_OS_WINDOWS)

    int ret = 0;

    //如果合适用信号灯模拟PTHREAD MUTEX （非递归）
    if (ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex))
    {
        //解锁信号灯
        ret = zce::sem_post(mutex->non_recursive_mutex_);

        if (0 != ret)
        {
            return last_error_with_default(EINVAL);
        }

        return 0;
    }
    //如果合适用CS模拟PTHREAD MUTEX，进程内部，递归，不需要超时
    else if (ZCE_IS_CS_SIMULATE_PMUTEX(mutex))
    {
        ::LeaveCriticalSection (&(mutex->thr_nontimeout_mutex_));
        return 0;
    }
    //如果合适MUTEX模拟PTHREAD MUTEX，进程内部，递归而且需要超时 或者进程外部，递归，
    else if (ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex))
    {
        ::ReleaseMutex (mutex->recursive_mutex_);
        return 0;
    }
    else
    {
        return EINVAL;
    }

#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutex_unlock(mutex);
#endif
}

