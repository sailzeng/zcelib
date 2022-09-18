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
    ZCE_UNUSED_ARG(lock);
    if (pshared & PTHREAD_PROCESS_SHARED)
    {
        assert(false);
    }
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_init(lock, pshared);
#endif
}

//SPIN 锁的销毁
int zce::pthread_spin_destroy(pthread_spinlock_t* lock) noexcept
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(lock);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_destroy(lock);
#endif
}

//SPIN 锁的加锁
int zce::pthread_spin_lock(pthread_spinlock_t* lock)noexcept
{
#if defined (ZCE_OS_WINDOWS)
    while (lock->flag_.test_and_set())
    {
    }
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_lock(lock);
#endif
}

//SPIN 锁的尝试加锁
int zce::pthread_spin_trylock(pthread_spinlock_t* lock) noexcept
{
#if defined (ZCE_OS_WINDOWS)
    if (lock->flag_.test_and_set())
    {
        return 0;
    }
    return EBUSY;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_trylock(lock);
#endif
    }

//SPIN 锁的解锁
int zce::pthread_spin_unlock(pthread_spinlock_t* lock) noexcept
{
#if defined (ZCE_OS_WINDOWS)
    lock->flag_.clear();
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_unlock(lock);
#endif
}