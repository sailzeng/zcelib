
#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_semaphore.h"

//Ϊʲô��������ACE�������ޣ��������ޣ��������������������������ޣ���������������

//��ʼ��������һ���������������źŵ�,��Ӧ�����ٺ���sem_destroy
int zce::sem_init(sem_t *sem,
                  int pshared,
                  unsigned int init_value,
                  unsigned int max_val)
{

#if defined (ZCE_OS_WINDOWS)

    //�ַ�������Ϊ0��ʾ�������źŵ�
    sem->sem_unnamed_ = true;

    ZCE_UNUSED_ARG (pshared);

    HANDLE sem_handle = ::CreateSemaphoreA(NULL,
                                           init_value,
                                           max_val,
                                           NULL);

    if (sem_handle == 0)
    {
        return -1;
    }
    else
    {
        // Make sure to set errno to ERROR_ALREADY_EXISTS if necessary.
        errno = ::GetLastError ();
        sem->sem_hanlde_ = sem_handle;
        return 0;
    }

#elif defined (ZCE_OS_LINUX)
    //
    //�Ǳ�׼����
    ZCE_UNUSED_ARG(max_val);
    return ::sem_init (sem,
                       pshared,
                       init_value);
#endif
}

//��������(����)�źŵ�
//sem_destroy�����ͷ�sem����Ϊ�����������ڴ�
int zce::sem_destroy(sem_t *sem)
{
#if defined (ZCE_OS_WINDOWS)

    ::CloseHandle(sem->sem_hanlde_);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::sem_destroy(sem);
#endif
}

//��(����)�źŵ�,���ֵ���Ǳ�׼������������Ĭ��ֻ�����ˣ������Ҫ���ڴ��������źŵ�,
//�򿪺�Ҫʹ��sem_close��sem_unlink
sem_t *zce::sem_open(const char *name,
                     int oflag,
                     mode_t mode,
                     unsigned int init_value,
                     unsigned int max_val)
{
#if defined (ZCE_OS_WINDOWS)

    ZCE_UNUSED_ARG (oflag);
    ZCE_UNUSED_ARG (mode);

    //Ϊ�˱������������new����
    sem_t *ret_sem = new sem_t ();
    ret_sem->sem_unnamed_ = false;

    HANDLE sem_handle = ::CreateSemaphoreA(NULL,
                                           init_value,
                                           max_val,
                                           name);

    if (sem_handle == 0)
    {
        delete ret_sem;
        ret_sem = NULL;
        return SEM_FAILED;
    }
    else
    {
        // Make sure to set errno to ERROR_ALREADY_EXISTS if necessary.
        errno = ::GetLastError ();

        ret_sem->sem_hanlde_ = sem_handle;
        return ret_sem;
    }

#elif defined (ZCE_OS_LINUX)
    //�Ǳ�׼����
    ZCE_UNUSED_ARG(max_val);

    return ::sem_open (name,
                       oflag,
                       mode,
                       init_value);
#endif
}

//�ر��źŵ�
int zce::sem_close(sem_t *sem)
{
#if defined (ZCE_OS_WINDOWS)
    ::CloseHandle(sem->sem_hanlde_);
    delete sem;
    sem = NULL;
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::sem_close(sem);
#endif
}

//ɾ���źŵ�
int zce::sem_unlink(const char *name)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG (name);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::sem_unlink(name);
#endif
}

int zce::sem_post (sem_t *sem)
{

#if defined (ZCE_OS_WINDOWS)

    const LONG ONCE_POST_NUMBER = 1;
    BOOL  ret_bool = ::ReleaseSemaphore (sem->sem_hanlde_,
                                         ONCE_POST_NUMBER,
                                         NULL);

    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::sem_post(sem);
#endif

}

int zce::sem_post (sem_t *sem, u_int release_count)
{
#if defined (ZCE_OS_WINDOWS)

    BOOL  ret_bool = ::ReleaseSemaphore (sem->sem_hanlde_,
                                         release_count,
                                         NULL);

    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    //
    for (size_t i = 0; i < release_count; ++i)
    {
        if (::sem_post(sem) == -1)
        {
            return -1;
        }
    }

    return 0;
#endif
}

int zce::sem_trywait (sem_t *sem)
{

#if defined (ZCE_OS_WINDOWS)

    //�ȴ�0s���൱����������
    DWORD result = ::WaitForSingleObject (sem->sem_hanlde_, 0);

    if (result == WAIT_OBJECT_0)
    {
        return 0;
    }
    else
    {
        if (result == WAIT_TIMEOUT)
        {
            //Posix�Ĵ���Ӧ�������ó�EAGAIN��,�����Mutex�ķ��ػ���һ����Mutex������EBUSY
            errno = EAGAIN;
        }
        else
        {
            errno = GetLastError();
        }

        // This is a hack, we need to find an appropriate mapping...
        return -1;
    }

#elif defined (ZCE_OS_LINUX)
    // POSIX semaphores set errno to EAGAIN if trywait fails
    return ::sem_trywait (sem);
#endif

}

//�źŵ�����
int zce::sem_wait (sem_t *sem)
{
#if defined (ZCE_OS_WINDOWS)

    //INFINITE��ʶһ�µȴ�
    DWORD result = ::WaitForSingleObject (sem->sem_hanlde_, INFINITE);

    if (result == WAIT_OBJECT_0)
    {
        return 0;
    }
    else
    {
        errno = GetLastError();
        return -1;
    }

#elif defined (ZCE_OS_LINUX)
    // POSIX semaphores set errno to EAGAIN if trywait fails
    return ::sem_wait (sem);
#endif
}

//�źŵƳ�ʱ����
int zce::sem_timedwait(sem_t *sem, const ::timespec *abs_timeout_spec)
{

#if defined (ZCE_OS_WINDOWS)

    assert(abs_timeout_spec);

    //�õ����ʱ�䣬������ڣ�
    timeval now_time = zce::gettimeofday();
    timeval abs_time = zce::make_timeval(abs_timeout_spec);

    timeval timeout_time = zce::timeval_sub(abs_time, now_time, true);

    //�ȴ�ʱ�䴥��
    DWORD retsult = ::WaitForSingleObject (sem->sem_hanlde_,
                                           static_cast<DWORD>( zce::total_milliseconds(timeout_time)));

    if (WAIT_OBJECT_0 == retsult || WAIT_ABANDONED == retsult)
    {
        return 0;
    }
    else if (WAIT_TIMEOUT == retsult)
    {
        errno = ETIMEDOUT;
        return -1;
    }
    else
    {
        // This is a hack, we need to find an appropriate mapping...
        errno = ::GetLastError ();
        return -1;
    }

    /* NOTREACHED */

#elif defined (ZCE_OS_LINUX)

    int ret = ::sem_timedwait (sem, abs_timeout_spec);

    //һ���ϵͳETIME �� ETIMEDOUT �Ĵ���ֵ����̫һ������LINUX�ֲᣬ���ص�Ӧ����ETIMEDOUT
    if (ret == -1 && errno == ETIME)
    {
        errno = ETIMEDOUT;
    }

    return ret;
#endif
}

//�źŵƳ�ʱ����,�Ǳ�׼ʵ��,ʹ��timeval�ṹ��
int zce::sem_timedwait(sem_t *sem, const timeval *abs_timeout_val)
{
    assert(abs_timeout_val);
    //���ʱ���Ǿ���ֵʱ�䣬Ҫ����Ϊ���ʱ��
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::sem_timedwait(sem, &abs_timeout_spec);
}

//���ص�ǰ�źŵƵĵ�ǰֵ, ���ź���WINDOWS�²�֧�֣�ReleaseSemaphore�����ƹ��ܣ�����lReleaseCount��������Ϊ0
//�����ReleaseSemaphore��WaitForSingleObjectƴ��һ����ô���ܸ�������⣬
//΢���APIʵ�ֵ����á�Visual studioҲ��ŵ����������ԣ�������2010�汾�ǲ�Ҫ��ָ���ˡ�
int zce::sem_getvalue(sem_t *sem, int *sval)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(sem);
    ZCE_UNUSED_ARG(sval);
    errno = EINVAL;
    return -1;
#elif defined (ZCE_OS_LINUX)
    return ::sem_getvalue(sem, sval);
#endif
}

