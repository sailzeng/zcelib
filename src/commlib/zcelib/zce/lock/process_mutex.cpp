#include "zce/predefine.h"
#include "zce/os_adapt/mutex.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/rwlock.h"
#include "zce/os_adapt/semaphore.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"
#include "zce/time/time_value.h"
#include "zce/lock/process_mutex.h"

namespace zce
{
//构造函数,
//mutex_name 互斥量的名称，必选参数，在WIN下是互斥量的名称，在LINUX，是共享内存的文件名称，
//（如果是WIN下的非递归锁，是个信号灯的名称）
//WINDOWS的核心对象的名称被用于标识一个核心对象（互斥量，信号灯），而LINUX下的pthread_xxx同步对象，如果对象在共享内存里面，
//那么就是进程间同步对象，当然还要注意属性PTHREAD_PROCESS_SHARED的设置
process_mutex::process_mutex(const char* mutex_name, bool recursive) :
    lock_(NULL)
{
    ZCE_ASSERT(mutex_name);
    int ret = 0;

#if defined ZCE_OS_WINDOWS
    //第一次发现Windows 下居然还简单一点
    lock_ = new pthread_mutex_t();

#elif defined ZCE_OS_LINUX
    //
    ret = posix_sharemem_.open(mutex_name, sizeof(pthread_mutex_t), false);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::SHM_Posix::open", ret);
        return;
    }

    lock_ = reinterpret_cast<pthread_mutex_t*>(posix_sharemem_.addr());

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
process_mutex::~process_mutex(void)
{
    int ret = 0;
    ret = zce::pthread_mutex_destroy(lock_);

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
void process_mutex::lock() noexcept
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
bool process_mutex::try_lock() noexcept
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
void process_mutex::unlock() noexcept
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
bool process_mutex::try_lock_until(const zce::time_value& abs_time) noexcept
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
bool process_mutex::try_lock_for(const zce::time_value& relative_time) noexcept
{
    timeval abs_time = zce::gettimeofday();
    abs_time = zce::timeval_add(abs_time, relative_time);
    return try_lock_until(abs_time);
}

//取出内部的锁的指针
pthread_mutex_t* process_mutex::get_lock()
{
    return lock_;
}
}