#include "zce/predefine.h"
#include "zce/os_adapt/mutex.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/rwlock.h"
#include "zce/os_adapt/semaphore.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"
#include "zce/lock/thread_mutex.h"

/************************************************************************************************************
Class           : Thread_Light_Mutex 轻量级的互斥锁，不提供超时。
************************************************************************************************************/

namespace zce
{
//构造函数
Thread_Light_Mutex::Thread_Light_Mutex()
{
    int ret = 0;

    ret = zce::pthread_mutex_initex(&lock_,
                                    false,
                                    true,
                                    false,
                                    NULL);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }
}

//销毁互斥量
Thread_Light_Mutex::~Thread_Light_Mutex(void)
{
    int ret = 0;
    ret = zce::pthread_mutex_destroy(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_destroy", ret);
        return;
    }
}

//锁定
void Thread_Light_Mutex::lock() noexcept
{
    int ret = 0;
    ret = zce::pthread_mutex_lock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_lock", ret);
        return;
    }
}

//尝试锁定
bool Thread_Light_Mutex::try_lock() noexcept
{
    int ret = 0;
    ret = zce::pthread_mutex_trylock(&lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,
void Thread_Light_Mutex::unlock() noexcept
{
    int ret = 0;
    ret = zce::pthread_mutex_unlock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_unlock", ret);
        return;
    }
}

//取出内部的锁的指针
pthread_mutex_t* Thread_Light_Mutex::get_lock()
{
    return &lock_;
}

/************************************************************************************************************
Class           : Thread_Recursive_Mutex
************************************************************************************************************/
//构造函数
Thread_Recursive_Mutex::Thread_Recursive_Mutex()
{
    int ret = 0;

    //这个地方唯一和上面不同的就是need_timeout被调整为了true
    ret = zce::pthread_mutex_initex(&lock_,
                                    false,
                                    true,
                                    true,
                                    NULL);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }
}

//析构函数，释放MUTEX资源
Thread_Recursive_Mutex::~Thread_Recursive_Mutex(void)
{
    int ret = 0;
    ret = zce::pthread_mutex_destroy(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_destroy", ret);
        return;
    }
}

//锁定
void Thread_Recursive_Mutex::lock() noexcept
{
    int ret = 0;
    ret = zce::pthread_mutex_lock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_lock", ret);
        return;
    }
}

//尝试锁定
bool Thread_Recursive_Mutex::try_lock() noexcept
{
    int ret = 0;
    ret = zce::pthread_mutex_trylock(&lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,
void Thread_Recursive_Mutex::unlock() noexcept
{
    int ret = 0;
    ret = zce::pthread_mutex_unlock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_unlock", ret);
        return;
    }
}

//绝对时间
bool Thread_Recursive_Mutex::try_lock_until(const zce::time_value& abs_time) noexcept
{
    int ret = 0;
    ret = zce::pthread_mutex_timedlock(&lock_, abs_time);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_timedlock", ret);
        return false;
    }

    return true;
}

//相对时间
bool Thread_Recursive_Mutex::try_lock_for(const zce::time_value& relative_time) noexcept
{
    timeval abs_time = zce::gettimeofday();
    abs_time = zce::timeval_add(abs_time, relative_time);
    return try_lock_until(abs_time);
}

//取出内部的锁的指针
pthread_mutex_t* Thread_Recursive_Mutex::get_lock()
{
    return &lock_;
}

/************************************************************************************************************
Class           : Thread_NONR_Mutex
************************************************************************************************************/
//构造函数,name参数，可以不带，带反而可能降低可移植性
//稍稍解释一下为什么可以不带name参数，因为Windows下我们是用信号灯模拟，但Windows的信号灯在线程环境下，不需要一定有名字
Thread_NONR_Mutex::Thread_NONR_Mutex()
{
    //线程锁
    int ret = 0;

    //注意recursive被调整为了false
    ret = zce::pthread_mutex_initex(&lock_,
                                    false,
                                    false,
                                    true,
                                    NULL);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }
}

//析构函数，释放MUTEX资源
Thread_NONR_Mutex::~Thread_NONR_Mutex(void)
{
    int ret = 0;
    ret = zce::pthread_mutex_destroy(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_destroy", ret);
        return;
    }
}

//锁定
void Thread_NONR_Mutex::lock() noexcept
{
    int ret = 0;
    ret = zce::pthread_mutex_lock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_lock", ret);
        return;
    }
}

//尝试锁定
bool Thread_NONR_Mutex::try_lock() noexcept
{
    int ret = 0;
    ret = zce::pthread_mutex_trylock(&lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,
void Thread_NONR_Mutex::unlock() noexcept
{
    int ret = 0;
    ret = zce::pthread_mutex_unlock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_unlock", ret);
        return;
    }
}

//绝对时间
bool Thread_NONR_Mutex::wait_until(const zce::time_value& abs_time) noexcept
{
    int ret = 0;
    ret = zce::pthread_mutex_timedlock(&lock_, abs_time);

    if (0 != ret)
    {
        //在ETIME==>ETIMEDOUT调整后没有注意修改这个问题，derrickhu帮忙发现了这个问题，特此修改。
        if (errno != ETIMEDOUT)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_timedlock", ret);
        }

        return false;
    }

    return true;
}

//相对时间
bool Thread_NONR_Mutex::wait_for(const zce::time_value& relative_time) noexcept
{
    zce::time_value abs_time(zce::gettimeofday());
    abs_time += relative_time;
    return wait_until(abs_time);
}
}