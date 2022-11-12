#include "zce/predefine.h"
#include "zce/os_adapt/mutex.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/rwlock.h"
#include "zce/os_adapt/semaphore.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"
#include "zce/lock/thread_mutex.h"

/************************************************************************************************************
Class           : thread_mutex 轻量级的互斥锁，不提供超时。
************************************************************************************************************/

namespace zce
{
//构造函数
thread_mutex::thread_mutex()
{
    int ret = 0;

    ret = zce::pthread_mutex_initex(&lock_,
                                    false,
                                    true,
                                    false,
                                    nullptr);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }
}

//销毁互斥量
thread_mutex::~thread_mutex(void)
{
    int ret = 0;
    ret = zce::pthread_mutex_destroy(&lock_);
    if (0 != ret)
    {
        //ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_destroy", ret);
        return;
    }
}

//锁定
void thread_mutex::lock() noexcept
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
bool thread_mutex::try_lock() noexcept
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
void thread_mutex::unlock() noexcept
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
pthread_mutex_t* thread_mutex::get_lock()
{
    return &lock_;
}

/************************************************************************************************************
Class           : thread_recursive_mutex
************************************************************************************************************/
//构造函数
thread_recursive_mutex::thread_recursive_mutex()
{
    int ret = 0;

    //这个地方唯一和上面不同的就是need_timeout被调整为了true
    ret = zce::pthread_mutex_initex(&lock_,
                                    false,
                                    true,
                                    true,
                                    nullptr);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }
}

//析构函数，释放MUTEX资源
thread_recursive_mutex::~thread_recursive_mutex(void)
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
void thread_recursive_mutex::lock() noexcept
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
bool thread_recursive_mutex::try_lock() noexcept
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
void thread_recursive_mutex::unlock() noexcept
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
bool thread_recursive_mutex::try_lock_until(const zce::time_value& abs_time) noexcept
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
bool thread_recursive_mutex::try_lock_for(const zce::time_value& relative_time) noexcept
{
    timeval abs_time = zce::gettimeofday();
    abs_time = zce::timeval_add(abs_time, relative_time);
    return try_lock_until(abs_time);
}

//取出内部的锁的指针
pthread_mutex_t* thread_recursive_mutex::get_lock()
{
    return &lock_;
}

/************************************************************************************************************
Class           : thread_nonr_mutex
************************************************************************************************************/
//构造函数,name参数，可以不带，带反而可能降低可移植性
//稍稍解释一下为什么可以不带name参数，因为Windows下我们是用信号灯模拟，但Windows的信号灯在线程环境下，不需要一定有名字
thread_nonr_mutex::thread_nonr_mutex()
{
    //线程锁
    int ret = 0;

    //注意recursive被调整为了false
    ret = zce::pthread_mutex_initex(&lock_,
                                    false,
                                    false,
                                    true,
                                    nullptr);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }
}

//析构函数，释放MUTEX资源
thread_nonr_mutex::~thread_nonr_mutex(void)
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
void thread_nonr_mutex::lock() noexcept
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
bool thread_nonr_mutex::try_lock() noexcept
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
void thread_nonr_mutex::unlock() noexcept
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
bool thread_nonr_mutex::wait_until(const zce::time_value& abs_time) noexcept
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
bool thread_nonr_mutex::wait_for(const zce::time_value& relative_time) noexcept
{
    zce::time_value abs_time(zce::gettimeofday());
    abs_time += relative_time;
    return wait_until(abs_time);
}
}