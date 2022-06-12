#include "zce/predefine.h"
#include "zce/os_adapt/common.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/mutex.h"
#include "zce/os_adapt/spin.h"

//SPIN 锁的初始化
int zce::pthread_spin_init(pthread_spinlock_t* lock,
                           int pshared) noexcept
{
#if defined (ZCE_OS_WINDOWS)

    int ret = 0;

    //进行初始化，多线程下用临界区模拟
    ret = zce::pthread_spin_initex(lock,
                                   (pshared == PTHREAD_PROCESS_SHARED) ? true : false,
                                   NULL);

    if (0 != ret)
    {
        return ret;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    return ::pthread_spin_init(lock, pshared);
#endif
}

//SPIN 锁的初始化扩展版本
int zce::pthread_spin_initex(pthread_spinlock_t* lock,
                             bool process_share,
                             const char* spin_name) noexcept
{
#if defined (ZCE_OS_WINDOWS)

    int ret = 0;

    //进行初始化，多线程下用临界区模拟
    ret = zce::pthread_mutex_initex(lock,
                                    process_share,
                                    true,
                                    false,
                                    spin_name);

    if (0 != ret)
    {
        return ret;
    }

    //4000是MSDN给出的参考数据。
    const DWORD WIN_CS_SPIN_DEFAULT = 4096;

    //没有进程间共享
    if (false == process_share)
    {
        ::SetCriticalSectionSpinCount(&(lock->thr_nontimeout_mutex_), WIN_CS_SPIN_DEFAULT);
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    ZCE_UNUSED_ARG(spin_name);
    return ::pthread_spin_init(lock,
                               process_share == true ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE);
#endif
}

//SPIN 锁的销毁
int zce::pthread_spin_destroy(pthread_spinlock_t* lock) noexcept
{
#if defined (ZCE_OS_WINDOWS)
    //Windows下用临界区或者互斥量模拟
    return zce::pthread_mutex_destroy(lock);
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_destroy(lock);
#endif
}

//SPIN 锁的加锁
int zce::pthread_spin_lock(pthread_spinlock_t* lock)noexcept
{
#if defined (ZCE_OS_WINDOWS)
    //Windows下用临界区或者互斥量模拟
    return zce::pthread_mutex_lock(lock);
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_lock(lock);
#endif
}

//SPIN 锁的尝试加锁
int zce::pthread_spin_trylock(pthread_spinlock_t* lock) noexcept
{
#if defined (ZCE_OS_WINDOWS)
    //Windows下用临界区或者互斥量模拟
    return zce::pthread_mutex_trylock(lock);
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_trylock(lock);
#endif
}

//SPIN 锁的解锁
int zce::pthread_spin_unlock(pthread_spinlock_t* lock) noexcept
{
#if defined (ZCE_OS_WINDOWS)
    //Windows下用临界区或者互斥量模拟
    return zce::pthread_mutex_unlock(lock);
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_unlock(lock);
#endif
}