
//线程的互斥量，有轻量的锁，递归锁，非递归锁，读写锁

#include "zce_predefine.h"

#include "zce_os_adapt_mutex.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_rwlock.h"
#include "zce_os_adapt_semaphore.h"

#include "zce_trace_debugging.h"
#include "zce_lock_thread_rw_mutex.h"

/************************************************************************************************************
Class           : ZCE_Thread_RW_Mutex
************************************************************************************************************/
//构造函数
ZCE_Thread_RW_Mutex::ZCE_Thread_RW_Mutex()
{
    //pthread_rwlockattr_t属性的初始化
    int ret = 0;

    ret = ZCE_LIB::pthread_rwlock_initex(&rw_lock_, false);

    if (0 != ret )
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_init", ret);
        return;
    }

}

ZCE_Thread_RW_Mutex::~ZCE_Thread_RW_Mutex()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_destroy (&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_rwlock_destroy", ret);
        return;
    }
}

//读取锁
void ZCE_Thread_RW_Mutex::lock_read()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_rdlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_rwlock_rdlock", ret);
        return;
    }
}

//尝试读取锁
bool ZCE_Thread_RW_Mutex::try_lock_read()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_trywrlock(&rw_lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//绝对时间
bool ZCE_Thread_RW_Mutex::systime_lock_read(const ZCE_Time_Value &abs_time)
{
    int ret = 0;

    ret = ZCE_LIB::pthread_rwlock_timedrdlock(&rw_lock_,
                                              abs_time);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_rwlock_timedrdlock", ret);
        return false;
    }

    return true;
}
//相对时间
bool ZCE_Thread_RW_Mutex::duration_lock_read(const ZCE_Time_Value &relative_time)
{
    ZCE_Time_Value abs_time(ZCE_LIB::gettimeofday());
    abs_time += relative_time;
    return systime_lock_read(abs_time);
}

//写锁定
void ZCE_Thread_RW_Mutex::lock_write()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_wrlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_rwlock_wrlock", ret);
        return;
    }
}

//尝试读取锁
bool ZCE_Thread_RW_Mutex::try_lock_write()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_trywrlock(&rw_lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//写锁定超时，绝对时间
bool ZCE_Thread_RW_Mutex::systime_lock_write(const ZCE_Time_Value &abs_time)
{
    int ret = 0;

    ret = ZCE_LIB::pthread_rwlock_timedwrlock(&rw_lock_,
                                              abs_time);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_timedlock", ret);
        return false;
    }

    return true;
}

//写锁定超时，相对时间
bool ZCE_Thread_RW_Mutex::duration_lock_write(const ZCE_Time_Value &relative_time)
{
    ZCE_Time_Value abs_time(ZCE_LIB::gettimeofday());
    abs_time += relative_time;
    return systime_lock_write(abs_time);
}

//解锁,如果是读写锁也只需要这一个函数
void ZCE_Thread_RW_Mutex::unlock()
{
    //解锁
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_unlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_rwlock_unlock", ret);
        return;
    }
}

//取出内部的锁的指针
pthread_rwlock_t *ZCE_Thread_RW_Mutex::get_lock()
{
    return &rw_lock_;
}

/************************************************************************************************************
Class           : ZCE_Thread_Light_RW_Mutex 轻量级的读写锁，不提供超时等函数
************************************************************************************************************/
//构造函数
ZCE_Thread_Light_RW_Mutex::ZCE_Thread_Light_RW_Mutex()
{
    //pthread_rwlockattr_t属性的初始化
    int ret = 0;

    bool use_win2008_rwlock = false;

#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1
    use_win2008_rwlock = true;
#endif

    ret = ZCE_LIB::pthread_rwlock_initex(&rw_lock_, use_win2008_rwlock);

    if (0 != ret )
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_init", ret);
        return;
    }

}

ZCE_Thread_Light_RW_Mutex::~ZCE_Thread_Light_RW_Mutex()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_destroy (&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_rwlock_destroy", ret);
        return;
    }
}

//读取锁
void ZCE_Thread_Light_RW_Mutex::lock_read()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_rdlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_rwlock_rdlock", ret);
        return;
    }
}

//尝试读取锁
bool ZCE_Thread_Light_RW_Mutex::try_lock_read()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_trywrlock(&rw_lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//写锁定
void ZCE_Thread_Light_RW_Mutex::lock_write()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_wrlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_rwlock_wrlock", ret);
        return;
    }
}

//尝试读取锁
bool ZCE_Thread_Light_RW_Mutex::try_lock_write()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_trywrlock(&rw_lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,如果是读写锁也只需要这一个函数
void ZCE_Thread_Light_RW_Mutex::unlock()
{
    //解锁
    int ret = 0;
    ret = ZCE_LIB::pthread_rwlock_unlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_rwlock_unlock", ret);
        return;
    }
}

//取出内部的锁的指针
pthread_rwlock_t *ZCE_Thread_Light_RW_Mutex::get_lock()
{
    return &rw_lock_;
}

