#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_mutex.h"
#include "zce_os_adapt_spin.h"

//SPIN ���ĳ�ʼ��
int zce::pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
#if defined (ZCE_OS_WINDOWS)

    int ret = 0;

    //���г�ʼ�������߳������ٽ���ģ��
    ret = zce::pthread_spin_initex(lock,
                                   (pshared == PTHREAD_PROCESS_SHARED) ? true : false,
                                   NULL);

    if (0 != ret)
    {
        return ret;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    return ::pthread_spin_init (lock, pshared);
#endif
}

//SPIN ���ĳ�ʼ����չ�汾
int zce::pthread_spin_initex(pthread_spinlock_t *lock,
                             bool process_share,
                             const char *spin_name)
{

#if defined (ZCE_OS_WINDOWS)

    int ret = 0;

    //���г�ʼ�������߳������ٽ���ģ��
    ret = zce::pthread_mutex_initex(lock,
                                    process_share,
                                    true,
                                    false,
                                    spin_name);

    if (0 != ret)
    {
        return ret;
    }

    //4000��MSDN�����Ĳο����ݡ�
    const DWORD WIN_CS_SPIN_DEFAULT = 4096;

    //û�н��̼乲��
    if (false == process_share )
    {
        ::SetCriticalSectionSpinCount(&(lock->thr_nontimeout_mutex_), WIN_CS_SPIN_DEFAULT);
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    ZCE_UNUSED_ARG(spin_name);
    return ::pthread_spin_init (lock,
                                process_share == true ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE);
#endif
}

//SPIN ��������
int zce::pthread_spin_destroy(pthread_spinlock_t *lock)
{
#if defined (ZCE_OS_WINDOWS)
    //Windows�����ٽ������߻�����ģ��
    return zce::pthread_mutex_destroy(lock);
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_destroy (lock);
#endif
}

//SPIN ���ļ���
int zce::pthread_spin_lock(pthread_spinlock_t *lock)
{
#if defined (ZCE_OS_WINDOWS)
    //Windows�����ٽ������߻�����ģ��
    return zce::pthread_mutex_lock(lock);
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_lock (lock);
#endif
}

//SPIN ���ĳ��Լ���
int zce::pthread_spin_trylock(pthread_spinlock_t *lock)
{
#if defined (ZCE_OS_WINDOWS)
    //Windows�����ٽ������߻�����ģ��
    return zce::pthread_mutex_trylock(lock);
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_trylock (lock);
#endif

}

//SPIN ���Ľ���
int zce::pthread_spin_unlock(pthread_spinlock_t *lock)
{
#if defined (ZCE_OS_WINDOWS)
    //Windows�����ٽ������߻�����ģ��
    return zce::pthread_mutex_unlock(lock);
#elif defined (ZCE_OS_LINUX)
    return ::pthread_spin_unlock (lock);
#endif
}

