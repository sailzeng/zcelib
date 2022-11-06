#include "zce/predefine.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/condi.h"
#include "zce/os_adapt/time.h"
#include "zce/logger/logging.h"
#include "zce/lock/thread_mutex.h"
#include "zce/lock/thread_condi.h"

namespace zce
{
//---------------------------------------------------------------------------------------
//为Thread_Light_Mutex做的特化
//
thread_condition::thread_condition()
{
    int ret = 0;
    ret = zce::pthread_cond_initex(&lock_, false);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_initex =%u", ret);
        return;
    }
}

//!析构函数
thread_condition::~thread_condition(void)
{
    //销毁条件变量
    const int ret = zce::pthread_cond_destroy(&lock_);
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "zce::pthread_cond_destroy ret =%u", ret);
        return;
    }
}

//等待
void thread_condition::wait(thread_light_mutex* external_mutex)noexcept
{
    int ret = zce::pthread_cond_wait(&lock_,
                                     external_mutex->get_lock());

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_init", ret);
        return;
    }
}

//我根据Thread_Light_Mutex，Thread_Recursive_Mutex给了特化实现

//绝对时间超时的的等待，超时后解锁
bool thread_condition::wait_until(thread_light_mutex* external_mutex,
                                  const zce::time_value& abs_time) noexcept
{
    int ret = zce::pthread_cond_timedwait(&lock_,
                                          external_mutex->get_lock(),
                                          abs_time);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_init", ret);
        return false;
    }

    return true;
}

//相对时间的超时锁定等待，超时后，解锁
bool thread_condition::wait_for(thread_light_mutex* external_mutex,
                                const zce::time_value& relative_time) noexcept
{
    zce::time_value abs_time(zce::gettimeofday());
    abs_time += relative_time;
    return wait_until(external_mutex, abs_time);
}

//!给一个等待线程发送信号 Signal one waiting thread.
void thread_condition::notify_one(void) noexcept
{
    //
    auto ret = zce::pthread_cond_signal(&lock_);
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "zce::pthread_cond_signal ret = %u", ret);
        return;
    }
}

//!给所有的等待线程广播信号 Signal *all* waiting threads.
void thread_condition::notify_all(void) noexcept
{
    //
    auto ret = zce::pthread_cond_broadcast(&lock_);
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "zce::pthread_cond_broadcast = %u", ret);
        return;
    }
}

//---------------------------------------------------------------------------------------
//为Thread_Recursive_Mutex做的处理
thread_recursive_condition::thread_recursive_condition()
{
    int ret = 0;
    ret = zce::pthread_cond_initex(&lock_, false);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_initex =%u", ret);
        return;
    }
}

//!析构函数
thread_recursive_condition::~thread_recursive_condition(void)
{
    //销毁条件变量
    const int ret = zce::pthread_cond_destroy(&lock_);
    if (0 != ret)
    {
        ZCE_LOG(RS_ERROR, "zce::pthread_cond_destroy ret =%u", ret);
        return;
    }
}

//等待
void thread_recursive_condition::wait(thread_recursive_mutex* external_mutex) noexcept
{
    int ret = zce::pthread_cond_wait(&lock_,
                                     external_mutex->get_lock());

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_init", ret);
        return;
    }
}

//绝对时间超时的的等待，超时后解锁
bool thread_recursive_condition::wait_until(thread_recursive_mutex* external_mutex,
                                            const zce::time_value& abs_time) noexcept
{
    int ret = zce::pthread_cond_timedwait(&lock_,
                                          external_mutex->get_lock(),
                                          abs_time);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_init", ret);
        return false;
    }
    return true;
}

//相对时间的超时锁定等待，超时后，解锁
bool thread_recursive_condition::wait_for(thread_recursive_mutex* external_mutex,
                                          const zce::time_value& relative_time) noexcept
{
    zce::time_value abs_time(zce::gettimeofday());
    abs_time += relative_time;
    return wait_until(external_mutex, abs_time);
}

//!给一个等待线程发送信号 Signal one waiting thread.
void thread_recursive_condition::notify_one(void) noexcept
{
    //
    auto ret = zce::pthread_cond_signal(&lock_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_signal ret ", ret);
        return;
    }
}

//!给所有的等待线程广播信号 Signal *all* waiting threads.
void thread_recursive_condition::notify_all(void) noexcept
{
    //
    auto ret = zce::pthread_cond_broadcast(&lock_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_cond_broadcast", ret);
        return;
    }
}
}