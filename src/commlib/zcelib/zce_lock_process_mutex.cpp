#include "zce_predefine.h"
#include "zce_os_adapt_mutex.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_rwlock.h"
#include "zce_os_adapt_semaphore.h"
#include "zce_os_adapt_error.h"
#include "zce_trace_debugging.h"
#include "zce_time_value.h"
#include "zce_lock_process_mutex.h"

//构造函数,
//mutex_name 互斥量的名称，必选参数，在WIN下是互斥量的名称，在LINUX，是共享内存的文件名称，
//（如果是WIN下的非递归锁，是个信号灯的名称）
//WINDOWS的核心对象的名称被用于标识一个核心对象（互斥量，信号灯），而LINUX下的pthread_xxx同步对象，如果对象在共享内存里面，
//那么就是进程间同步对象，当然还要注意属性PTHREAD_PROCESS_SHARED的设置
ZCE_Process_Mutex::ZCE_Process_Mutex (const char *mutex_name, bool recursive):
    lock_(NULL)
{

    ZCE_ASSERT( mutex_name);
    int ret = 0;

#if defined ZCE_OS_WINDOWS
    //第一次发现Windows 下居然还简单一点
    lock_ = new pthread_mutex_t();

#elif defined ZCE_OS_LINUX
    //
    ret = posix_sharemem_.open(mutex_name, sizeof(pthread_mutex_t), false);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_ShareMem_Posix::open", ret);
        return;
    }

    lock_ = reinterpret_cast<pthread_mutex_t *>( posix_sharemem_.addr());

#endif

    ret = zce::pthread_mutex_initex(lock_,
                                    true,
                                    recursive,
                                    true,
                                    mutex_name);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }

}

//析构函数，
ZCE_Process_Mutex::~ZCE_Process_Mutex (void)
{
    int ret = 0;
    ret = zce::pthread_mutex_destroy (lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_destroy", ret);
        return;
    }

#if defined ZCE_OS_WINDOWS

    if (lock_)
    {
        delete lock_;
        lock_ = NULL;
    }

#elif defined ZCE_OS_LINUX
    posix_sharemem_.close();
    posix_sharemem_.remove();
    lock_ = NULL;
#endif
}

//锁定
void ZCE_Process_Mutex::lock()
{
    int ret = 0;
    ret = zce::pthread_mutex_lock(lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_lock", ret);
        return;
    }
}

//尝试锁定
bool ZCE_Process_Mutex::try_lock()
{
    int ret = 0;
    ret = zce::pthread_mutex_trylock(lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,
void ZCE_Process_Mutex::unlock()
{
    int ret = 0;
    ret = zce::pthread_mutex_unlock(lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_unlock", ret);
        return;
    }
}

//绝对时间
bool ZCE_Process_Mutex::systime_lock(const ZCE_Time_Value &abs_time)
{
    int ret = 0;
    ret = zce::pthread_mutex_timedlock(lock_, abs_time);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_timedlock", ret);
        return false;
    }

    return true;
}

//相对时间
bool ZCE_Process_Mutex::duration_lock(const ZCE_Time_Value &relative_time)
{
    timeval abs_time = zce::gettimeofday();
    abs_time = zce::timeval_add(abs_time, relative_time);
    return systime_lock(abs_time);
}

//取出内部的锁的指针
pthread_mutex_t *ZCE_Process_Mutex::get_lock()
{
    return lock_;
}

