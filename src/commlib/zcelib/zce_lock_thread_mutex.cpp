#include "zce_predefine.h"
#include "zce_os_adapt_mutex.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_rwlock.h"
#include "zce_os_adapt_semaphore.h"
#include "zce_os_adapt_error.h"
#include "zce_trace_debugging.h"

#include "zce_lock_thread_mutex.h"

/************************************************************************************************************
Class           : ZCE_Thread_Light_Mutex 轻量级的互斥锁，不提供超时。
************************************************************************************************************/

//构造函数
ZCE_Thread_Light_Mutex::ZCE_Thread_Light_Mutex ()
{
    int ret = 0;

    ret = ZCE_LIB::pthread_mutex_initex(&lock_,
                                        false,
                                        true,
                                        false,
                                        NULL);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_initex", ret);
        return;
    }

}

//销毁互斥量
ZCE_Thread_Light_Mutex::~ZCE_Thread_Light_Mutex (void)
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_destroy (&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_destroy", ret);
        return;
    }
}

//锁定
void ZCE_Thread_Light_Mutex::lock()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_lock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_lock", ret);
        return;
    }
}

//尝试锁定
bool ZCE_Thread_Light_Mutex::try_lock()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_trylock(&lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,
void ZCE_Thread_Light_Mutex::unlock()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_unlock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_unlock", ret);
        return;
    }
}

//取出内部的锁的指针
pthread_mutex_t *ZCE_Thread_Light_Mutex::get_lock()
{
    return &lock_;
}

/************************************************************************************************************
Class           : ZCE_Thread_Recursive_Mutex
************************************************************************************************************/
//构造函数
ZCE_Thread_Recursive_Mutex::ZCE_Thread_Recursive_Mutex ()
{
    int ret = 0;

    //这个地方唯一和上面不同的就是need_timeout被调整为了true
    ret = ZCE_LIB::pthread_mutex_initex(&lock_,
                                        false,
                                        true,
                                        true,
                                        NULL);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_initex", ret);
        return;
    }
}

//析构函数，释放MUTEX资源
ZCE_Thread_Recursive_Mutex::~ZCE_Thread_Recursive_Mutex (void)
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_destroy (&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_destroy", ret);
        return;
    }
}

//锁定
void ZCE_Thread_Recursive_Mutex::lock()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_lock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_lock", ret);
        return;
    }
}

//尝试锁定
bool ZCE_Thread_Recursive_Mutex::try_lock()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_trylock(&lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,
void ZCE_Thread_Recursive_Mutex::unlock()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_unlock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_unlock", ret);
        return;
    }
}

//绝对时间
bool ZCE_Thread_Recursive_Mutex::systime_lock(const ZCE_Time_Value &abs_time)
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_timedlock(&lock_, abs_time);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_timedlock", ret);
        return false;
    }

    return true;
}

//相对时间
bool ZCE_Thread_Recursive_Mutex::duration_lock(const ZCE_Time_Value &relative_time)
{
    timeval abs_time = ZCE_LIB::gettimeofday();
    abs_time = ZCE_LIB::timeval_add(abs_time, relative_time);
    return systime_lock(abs_time);
}

//取出内部的锁的指针
pthread_mutex_t *ZCE_Thread_Recursive_Mutex::get_lock()
{
    return &lock_;
}

/************************************************************************************************************
Class           : ZCE_Thread_NONR_Mutex
************************************************************************************************************/
//构造函数,name参数，可以不带，带反而可能降低可移植性
//稍稍解释一下为什么可以不带name参数，因为Windows下我们是用信号灯模拟，但Windows的信号灯在线程环境下，不需要一定有名字
ZCE_Thread_NONR_Mutex::ZCE_Thread_NONR_Mutex ()
{
    //线程锁
    int ret = 0;

    //注意recursive被调整为了false
    ret = ZCE_LIB::pthread_mutex_initex(&lock_,
                                        false,
                                        false,
                                        true,
                                        NULL);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_initex", ret);
        return;
    }

}

//析构函数，释放MUTEX资源
ZCE_Thread_NONR_Mutex::~ZCE_Thread_NONR_Mutex (void)
{

    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_destroy (&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_destroy", ret);
        return;
    }

}

//锁定
void ZCE_Thread_NONR_Mutex::lock()
{

    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_lock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_lock", ret);
        return;
    }
}

//尝试锁定
bool ZCE_Thread_NONR_Mutex::try_lock()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_trylock(&lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,
void ZCE_Thread_NONR_Mutex::unlock()
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_unlock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_unlock", ret);
        return;
    }

}

//绝对时间
bool ZCE_Thread_NONR_Mutex::systime_lock(const ZCE_Time_Value &abs_time)
{
    int ret = 0;
    ret = ZCE_LIB::pthread_mutex_timedlock(&lock_, abs_time);

    if (0 != ret)
    {
        //在ETIME==>ETIMEOUT调整后没有注意修改这个问题，derrickhu帮忙发现了这个问题，特此修改。
        if (errno != ETIMEDOUT)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_mutex_timedlock", ret);
        }

        return false;
    }

    return true;

}

//相对时间
bool ZCE_Thread_NONR_Mutex::duration_lock(const ZCE_Time_Value &relative_time)
{
    ZCE_Time_Value abs_time(ZCE_LIB::gettimeofday());
    abs_time += relative_time;
    return systime_lock(abs_time);
}

