#include "zce/predefine.h"
#include "zce/os_adapt/semaphore.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"
#include "zce/lock/thread_semaphore.h"

namespace zce
{
//构造函数,默认创建匿名信号灯，线程下一般用匿名信号灯就足够了,
thread_semaphore::thread_semaphore(unsigned int init_value) :
    lock_(NULL)
{
    int ret = 0;

    lock_ = new sem_t();
    ret = zce::sem_init(lock_, 0, init_value);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::sem_init fail.", ret);
    }
}

thread_semaphore::~thread_semaphore()
{
    //没有初始化过
    if (!lock_)
    {
        return;
    }
    zce::sem_destroy(lock_);
    //sem_destroy不会释放，
    delete lock_;
    lock_ = NULL;
}

//锁定
void thread_semaphore::acquire() noexcept
{
    //信号灯锁定
    int ret = zce::sem_wait(lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::sem_wait", ret);
        return;
    }
}

//尝试锁定
bool thread_semaphore::try_acquire() noexcept
{
    //信号灯锁定
    int ret = zce::sem_trywait(lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,
void thread_semaphore::release() noexcept
{
    int ret = zce::sem_post(lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::sem_post", ret);
        return;
    }
}

//绝对时间超时的的锁定，超时后解锁
bool thread_semaphore::try_acquire_until(const zce::time_value& abs_time) noexcept
{
    int ret = 0;
    ret = zce::sem_timedwait(lock_, abs_time);

    if (0 != ret)
    {
        if (errno != ETIMEDOUT)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::sem_timedwait", ret);
        }

        return false;
    }

    return true;
}

//相对时间的超时锁定，超时后，解锁
bool thread_semaphore::try_acquire_for(const zce::time_value& relative_time) noexcept
{
    timeval abs_time = zce::gettimeofday();
    abs_time = zce::timeval_add(abs_time, relative_time);
    return try_acquire_until(abs_time);
}
}