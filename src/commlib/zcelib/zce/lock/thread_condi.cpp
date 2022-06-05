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
//等待
template <>
void Thread_Condition<Thread_Light_Mutex>::wait(Thread_Light_Mutex* external_mutex)
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
template <>
bool Thread_Condition<Thread_Light_Mutex>::wait_until(Thread_Light_Mutex* external_mutex,
                                                      const zce::Time_Value& abs_time)
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
template <>
bool Thread_Condition<Thread_Light_Mutex>::wait_for(Thread_Light_Mutex* external_mutex,
                                                    const zce::Time_Value& relative_time)
{
    zce::Time_Value abs_time(zce::gettimeofday());
    abs_time += relative_time;
    return wait_until(external_mutex, abs_time);
}

//---------------------------------------------------------------------------------------
//为Thread_Light_Mutex做的特化
//等待
template <>
void Thread_Condition<Thread_Recursive_Mutex>::wait(Thread_Recursive_Mutex* external_mutex)
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
template <>
bool Thread_Condition<Thread_Recursive_Mutex>::wait_until(Thread_Recursive_Mutex* external_mutex,
                                                          const zce::Time_Value& abs_time)
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
template <>
bool Thread_Condition<Thread_Recursive_Mutex>::wait_for(Thread_Recursive_Mutex* external_mutex,
                                                        const zce::Time_Value& relative_time)
{
    zce::Time_Value abs_time(zce::gettimeofday());
    abs_time += relative_time;
    return wait_until(external_mutex, abs_time);
}
}