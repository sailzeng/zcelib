
#include "zce_predefine.h"
#include "zce_os_adapt_semaphore.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_error.h"
#include "zce_trace_log_debug.h"
#include "zce_lock_thread_semaphore.h"

//构造函数,默认创建匿名信号灯，线程下一般用匿名信号灯就足够了,
ZCE_Thread_Semaphore::ZCE_Thread_Semaphore(unsigned int init_value):
    lock_(NULL)
{
    int ret = 0;

    lock_ = new sem_t ();
    ret = ZCE_LIB::sem_init(lock_, 0, init_value);

    if (0 != ret )
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::sem_init fail.", ret);
    }
}

ZCE_Thread_Semaphore::~ZCE_Thread_Semaphore()
{
    //没有初始化过
    if (!lock_)
    {
        return;
    }

    ZCE_LIB::sem_destroy(lock_);

    //sem_destroy不会释放，
    delete lock_;
    lock_ = NULL;
}

//锁定
void ZCE_Thread_Semaphore::lock()
{
    //信号灯锁定
    int ret =  ZCE_LIB::sem_wait (lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::sem_wait", ret);
        return;
    }
}

//尝试锁定
bool ZCE_Thread_Semaphore::try_lock()
{
    //信号灯锁定
    int ret =  ZCE_LIB::sem_trywait (lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,
void ZCE_Thread_Semaphore::unlock()
{
    int ret = ZCE_LIB::sem_post (lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::sem_post", ret);
        return;
    }
}

//绝对时间超时的的锁定，超时后解锁
bool ZCE_Thread_Semaphore::systime_lock(const ZCE_Time_Value &abs_time)
{
    int ret = 0;
    ret = ZCE_LIB::sem_timedwait(lock_, abs_time);

    if (0 != ret)
    {
        if (errno != ETIMEDOUT)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::sem_timedwait", ret);
        }

        return false;
    }

    return true;
}

//相对时间的超时锁定，超时后，解锁
bool ZCE_Thread_Semaphore::duration_lock(const ZCE_Time_Value &relative_time)
{
    timeval abs_time = ZCE_LIB::gettimeofday();
    abs_time = ZCE_LIB::timeval_add(abs_time, relative_time);
    return systime_lock(abs_time);
}

