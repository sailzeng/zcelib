#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/common.h"
#include "zce/os_adapt/mutex.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/semaphore.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/condi.h"

//用多个WINDOWS20008的条件变量对象模拟PTHREAD CV条件变量
#ifndef ZCE_IS_USE_WIN2008_SIMULATE_PCV
#define ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond) ( (ZCE_BIT_IS_SET((cond)->outer_lock_type_ , PTHREAD_MUTEX_RECURSIVE)) \
                                                && (ZCE_BIT_ISNOT_SET((cond)->outer_lock_type_ , PTHREAD_MUTEX_TIMEOUT)) )
#endif

//ZCE_SUPPORT_WINSVR2008 == 1里面的代码都自成一段，看代码的时候注意
//都是WIN SERVER 2008后，WINDOWS自己的条件变量的封装，

//
int zce::pthread_condattr_init(pthread_condattr_t* attr)
{
#if defined (ZCE_OS_WINDOWS)

    //WINDOWS下默认就是递归的，你要我搞个非递归的我还要折腾
    attr->outer_lock_type_ = PTHREAD_MUTEX_RECURSIVE;
    //初始化名称
    attr->cv_name_[PATH_MAX] = '\0';
    attr->cv_name_[0] = '\0';
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_condattr_init(attr);
#endif
}

//
int zce::pthread_condattr_destroy(pthread_condattr_t* attr)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(attr);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_condattr_destroy(attr);
#endif
}

//条件变量对象的初始化
int zce::pthread_cond_init(pthread_cond_t* cond,
                           const pthread_condattr_t* attr)
{
#if defined (ZCE_OS_WINDOWS)

    if (attr)
    {
        cond->outer_lock_type_ = attr->outer_lock_type_;
    }
    else
    {
        cond->outer_lock_type_ = PTHREAD_MUTEX_RECURSIVE;
    }

    //WIN SERVER 2008，VISTA 后支持条件变量

    //如果是线程内部的，而且是递归的，而且没有超时功能，可以用Windows的条件条件变量干活
    ZCE_ASSERT(ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond));
    ::InitializeConditionVariable(&(cond->cv_object_));
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_init(cond, attr);
#endif
}

//初始化条件变量对象，不同的平台给不同的默认定义
//非标准，但是建议你使用，简单多了,
//如果要多进程共享，麻烦你老给个名字，同时在LINUX平台下，你必须pthread_condattr_t放入共享内存
int zce::pthread_cond_initex(pthread_cond_t* cond,
                             bool win_mutex_or_sema)
{
    //前面有错误返回，
    int result = 0;

    pthread_condattr_t attr;
    result = zce::pthread_condattr_init(&attr);
    if (0 != result)
    {
        return result;
    }

    //这个是我在WINDOWS下用的，用于某些时候我可以用临界区，而不是更重的互斥量
#if defined (ZCE_OS_WINDOWS)
    //默认还是用递归的锁
    attr.outer_lock_type_ |= PTHREAD_MUTEX_RECURSIVE;
    if (win_mutex_or_sema)
    {
        attr.outer_lock_type_ |= PTHREAD_MUTEX_TIMEOUT;
    }

#elif defined (ZCE_OS_LINUX)
    ZCE_UNUSED_ARG(win_mutex_or_sema);

    result = ::pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
    if (0 != result)
    {
        return result;
    }
#endif

    result = zce::pthread_cond_init(cond, &attr);
    zce::pthread_condattr_destroy(&attr);

    if (0 != result)
    {
        return EINVAL;
    }

    return 0;
}

//条件变量对象的销毁
int zce::pthread_cond_destroy(pthread_cond_t* cond)
{
#if defined (ZCE_OS_WINDOWS)

    //WINDOWS的条件变量没有释放的需求
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_destroy(cond);
#endif
}

//条件变量等待一段时间，超时后继续
int zce::pthread_cond_timedwait(pthread_cond_t* cond,
                                pthread_mutex_t* external_mutex,
                                const ::timespec* abs_timespec_out)
{
#if defined (ZCE_OS_WINDOWS)

    //如果外部的MUTEX的类型和共享方式不是我们所预期的，滚蛋
    if (external_mutex->lock_type_ != cond->outer_lock_type_)
    {
        ZCE_ASSERT(false);
        return EINVAL;
    }
    DWORD wait_msec = INFINITE;

    //如果有超时，计算相对超时时间
    if (abs_timespec_out)
    {
        //得到相对时间，这个折腾，
        timeval now_time = zce::gettimeofday();
        timeval abs_time = zce::make_timeval(abs_timespec_out);

        timeval timeout_time = zce::timeval_sub(abs_time, now_time, true);
        wait_msec = static_cast<DWORD>(zce::total_milliseconds(timeout_time));
    }

    //WINDOWS的条件变量没有释放
    BOOL bret = ::SleepConditionVariableCS(
        &(cond->cv_object_),
        &(external_mutex->thr_nontimeout_mutex_),
        wait_msec);

    if (bret == FALSE)
    {
        //SleepConditionVariableCS 是在GetLastError看结果，
        //这些API的设计继续反映出WINDOWS的前后不一。
        if (::GetLastError() == WAIT_TIMEOUT)
        {
            return ETIMEDOUT;
        }
        else
        {
            return EINVAL;
        }
    }
    return 0;

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_cond_timedwait(cond,
                                    external_mutex,
                                    abs_timespec_out);
#endif
}

//条件变量等待一段时间，超时后继续,时间变量用我内部统一的timeval
int zce::pthread_cond_timedwait(pthread_cond_t* cond,
                                pthread_mutex_t* external_mutex,
                                const timeval* abs_timeout_val)
{
    assert(abs_timeout_val);
    //这个时间是绝对值时间，要调整为相对时间
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::pthread_cond_timedwait(cond,
                                       external_mutex,
                                       &abs_timeout_spec);
}

//条件变量等待
int zce::pthread_cond_wait(pthread_cond_t* cond,
                           pthread_mutex_t* external_mutex)
{
#if defined (ZCE_OS_WINDOWS)
    //这样写是为了避免函数冲突告警，
    const ::timespec* abs_timespec_out = NULL;
    return zce::pthread_cond_timedwait(cond,
                                       external_mutex,
                                       abs_timespec_out);
#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_cond_wait(cond,
                               external_mutex);
#endif
}

//
int zce::pthread_cond_broadcast(pthread_cond_t* cond)
{
#if defined (ZCE_OS_WINDOWS)

    //使用WINDOWS的条件变量
    ::WakeAllConditionVariable(&(cond->cv_object_));
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_broadcast(cond);
#endif
}

//发信号
int zce::pthread_cond_signal(pthread_cond_t* cond)
{
#if defined (ZCE_OS_WINDOWS)

    //在调用这个方式前，外部的锁必须是锁上的，
    //使用WINDOWS的条件变量
#if defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008

    ::WakeConditionVariable(&(cond->cv_object_));
    return 0;
#endif

#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_signal(cond);
#endif
}