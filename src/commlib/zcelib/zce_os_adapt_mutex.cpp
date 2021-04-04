#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_string.h"
#include "zce_os_adapt_semaphore.h"
#include "zce_os_adapt_mutex.h"

/*****************************************************************************/
// MUTEXES BEGIN
/*****************************************************************************/

//���ٽ���ģ���PTHREAD MUTEX��Ҫ������ڲ����̣߳����ݹ飬����Ҫ��ʱ
#ifndef ZCE_IS_CS_SIMULATE_PMUTEX
#define ZCE_IS_CS_SIMULATE_PMUTEX(mutex) ( (PTHREAD_PROCESS_PRIVATE == (mutex)->lock_shared_  ) \
                                           && (ZCE_BIT_IS_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_RECURSIVE))  \
                                           && (ZCE_BIT_ISNOT_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_TIMEOUT)) )
#endif

//��MUTEXģ��PTHREAD MUTEX������̣��ݹ飬���߽����ڲ����ݹ飬�����г�ʱ
#ifndef ZCE_IS_MUTEX_SIMULATE_PMUTEX
#define ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex) ( ( (PTHREAD_PROCESS_SHARED == (mutex)->lock_shared_ ) \
                                                && (ZCE_BIT_IS_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_RECURSIVE)) ) \
                                              || ( (PTHREAD_PROCESS_PRIVATE == (mutex)->lock_shared_) \
                                                   && (ZCE_BIT_IS_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_RECURSIVE))  \
                                                   && (ZCE_BIT_IS_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_TIMEOUT)) ) )
#endif

//���źŵ�ģ��PTHREAD MUTEX���ǵݹ�
#ifndef ZCE_IS_SEMA_SIMULATE_PMUTEX
#define ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex) (!ZCE_BIT_IS_SET((mutex)->lock_type_ ,PTHREAD_MUTEX_RECURSIVE))
#endif

//���������Գ�ʼ��
int zce::pthread_mutexattr_init (pthread_mutexattr_t *attr)
{
#if defined (ZCE_OS_WINDOWS)
    //�̶߳���
    attr->lock_shared_ = PTHREAD_PROCESS_PRIVATE;
    //WINDOWS��Ĭ�Ͼ��ǵݹ�ģ���Ҫ�Ҹ���ǵݹ���һ�Ҫ����
    attr->lock_type_ = PTHREAD_MUTEX_RECURSIVE;
    //��ʼ������
    attr->mutex_name_[PATH_MAX] = '\0';
    attr->mutex_name_[0] = '\0';
    return 0;

#elif defined (ZCE_OS_LINUX)

    return ::pthread_mutexattr_init (attr);
#endif
}

//��������������
int zce::pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(attr);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutexattr_init (attr);
#endif
}

//�������������ù�������PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE
int zce::pthread_mutexattr_setpshared (pthread_mutexattr_t *attr,
                                       int pshared)
{
#if defined (ZCE_OS_WINDOWS)

    attr->lock_shared_ = pshared;

    //���MUTEX�������ǿյ�,�����������ֵ��
    if ( '\0' == attr->mutex_name_[0] && PTHREAD_PROCESS_SHARED == pshared )
    {
        const char *MUTEX_PREFIX = "MUTEX";
        zce::prefix_unique_name(MUTEX_PREFIX, attr->mutex_name_, PATH_MAX);
    }

    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutexattr_setpshared (attr, pshared);
#endif
}

//ȡ���̵߳Ĺ�������
int zce::pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr,
                                      int *pshared)
{
#if defined (ZCE_OS_WINDOWS)
    *pshared = attr->lock_shared_;
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutexattr_getpshared (attr, pshared);
#endif
}

//�����̵߳����ԣ�PTHREAD_MUTEX_XXXX�ļ���ֵ�����Ի���|һ�����ö������
int zce::pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
#if defined (ZCE_OS_WINDOWS)
    attr->lock_type_ = type;
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutexattr_settype (attr, type);
#endif
}

//ȡ���̵߳�����
int zce::pthread_mutexattr_gettype(const pthread_mutexattr_t *attr,
                                   int *type)
{
#if defined (ZCE_OS_WINDOWS)
    *type = attr->lock_type_;
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutexattr_gettype (attr, type);
#endif
}

//�����̵߳����ԣ���ͬ��ƽ̨����ͬ��Ĭ�϶���
//�Ǳ�׼�����ǽ�����ʹ�ã��򵥶���,
//���Ҫ����̹����鷳���ϸ������֣�ͬʱ��LINUXƽ̨�£������pthread_mutex_t���빲���ڴ�
int zce::pthread_mutex_initex(pthread_mutex_t *mutex,
                              bool process_share,
                              bool recursive,
                              bool need_timeout,
                              const char *mutex_name)
{

    //ǰ���д��󷵻أ�
    int result = 0;
    pthread_mutexattr_t attr;
    result = zce::pthread_mutexattr_init (&attr);

    if (0 != result)
    {
        return result;
    }

    int lock_shared = 0;
    if (process_share)
    {
        lock_shared = PTHREAD_PROCESS_SHARED;

        //���̹��������и����֣�WIN��LINUX������һ����LINUX�������ִ��������ڴ�
        if (mutex_name)
        {
#if defined (ZCE_OS_WINDOWS)
            strncpy(attr.mutex_name_, mutex_name, PATH_MAX);
#endif
        }
    }
    else
    {
        lock_shared = PTHREAD_PROCESS_PRIVATE;
    }

    //���ù�������
    result = zce::pthread_mutexattr_setpshared(&attr, lock_shared);

    if (0 != result)
    {
        zce::pthread_mutexattr_destroy (&attr);
        return -1;
    }

    int lock_type = 0;

    if ( recursive )
    {
        lock_type |= PTHREAD_MUTEX_RECURSIVE;
    }

    //���������WINDOWS���õģ�����ĳЩʱ���ҿ������ٽ����������Ǹ��صĻ�����
#if defined (ZCE_OS_WINDOWS)

    if (need_timeout)
    {
        lock_type |= PTHREAD_MUTEX_TIMEOUT;
    }

#else
    ZCE_UNUSED_ARG(need_timeout);
#endif

    //��������
    result = zce::pthread_mutexattr_settype(&attr, lock_type);

    if (0 != result)
    {
        zce::pthread_mutexattr_destroy (&attr);
        return result;
    }

    result = zce::pthread_mutex_init(mutex, &attr);
    zce::pthread_mutexattr_destroy (&attr);

    if (0 != result)
    {
        return result;
    }

    return 0;
}

//pthread mutex ��ʼ��,
//��Windows�¸��ݲ�ͬ����Ҫ���ֱ��ʹ�ò�ͬ��ͬ������ģ��Mutex��
//�����Ҫ�ǵݹ�������ô���źŵ�
//����ǵݹ�ģ��߳��ڲ��ݹ�ģ����Ҳ���Ҫ��ʱ�ȴ��������������ٽ���ģ��
//�����Ҫ�ݹ�ģ��߳��ڲ���Ҫ��ʱ�ģ����̼�ģ���ôѡ��MUTEX��
int zce::pthread_mutex_init (pthread_mutex_t *mutex,
                             const pthread_mutexattr_t *attr)
{
    //
#if defined (ZCE_OS_WINDOWS)

    //����attr�Ƿ�ΪNULL�����NULL����ʼ��ΪĬ�ϲ���
    const char *mutex_name = NULL;
    if (attr)
    {
        mutex->lock_shared_ = attr->lock_shared_;
        mutex->lock_type_ = attr->lock_type_;
        mutex_name = (( '\0' == attr->mutex_name_[0]  ) ? NULL : attr->mutex_name_);
    }
    else
    {
        mutex->lock_shared_ = PTHREAD_PROCESS_PRIVATE;
        mutex->lock_type_ = PTHREAD_MUTEX_NORMAL;
    }

    //��ʵ�ڲ���Ϊ�ڴ�й©��̫�����飬������γ�ʼ�����һ���û��̫�õķ��ӣ����ȽϿ��еķ�����pthread_mutex_t�ù��캯��.?��

    //����������źŵ�ģ��PTHREAD MUTEX ���ǵݹ飩
    if (ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex))
    {
        mutex->non_recursive_mutex_ = NULL;

        //ע���źŵƺ�������return -1��ʾʧ�ܣ����󱣴���errno������ط�Ҫת��

        //�������Ҫ���֣����������źŵ�
        if ( NULL == mutex_name )
        {
            mutex->non_recursive_mutex_ = new sem_t();
            int ret = zce::sem_init(mutex->non_recursive_mutex_,
                                    mutex->lock_shared_,
                                    1,
                                    1);

            if (ret != 0)
            {
                delete mutex->non_recursive_mutex_;
                mutex->non_recursive_mutex_ = NULL;

                //�õ����Ĵ������û�о���EINVAL
                return last_error_with_default(EINVAL);
            }
        }
        else
        {
            //,��ǰֵ�����ֵ��������1
            mutex->non_recursive_mutex_ = zce::sem_open(mutex_name,
                                                        O_CREAT,
                                                        ZCE_DEFAULT_FILE_PERMS,
                                                        1,
                                                        1);
        }

        if (!mutex->non_recursive_mutex_)
        {
            return last_error_with_default(EINVAL);
        }

        return 0;
    }
    //���������CSģ��PTHREAD MUTEX�������ڲ����ݹ飬����Ҫ��ʱ
    else if (ZCE_IS_CS_SIMULATE_PMUTEX(mutex))
    {
        //����쳣��VISTA����̭��
        __try
        {
            ::InitializeCriticalSection (&mutex->thr_nontimeout_mutex_);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return ENOMEM;
        }

        return 0;
    }
    //�������MUTEXģ��PTHREAD MUTEX�������ڲ����ݹ������Ҫ��ʱ ���߽����ⲿ���ݹ飬
    else if (ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex))
    {
        mutex->recursive_mutex_ = ::CreateMutexA (NULL,
                                                  FALSE,
                                                  mutex_name);

        if (mutex->recursive_mutex_ == 0)
        {
            return EINVAL;
        }
        else
        {
            //��������Ѿ��ɹ��Ĵ���������Ϊ������Ѿ����ڣ���ʱ�᷵�ش��� ERROR_ALREADY_EXISTS
            errno = ::GetLastError ();
            return 0;
        }
    }
    else
    {
        return EINVAL;
    }

    //��Զ�������
    //return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutex_init (mutex, attr);
#endif
}

//����MUTEX����
int zce::pthread_mutex_destroy (pthread_mutex_t *mutex)
{
#if defined (ZCE_OS_WINDOWS)

    //����������źŵ�ģ��PTHREAD MUTEX ���ǵݹ飩
    if (ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex))
    {
        int ret = 0;

        //�ر��źŵ�,�����û�г�ʼ���͹رգ�������ģ������ĺ������Ĳ��ò�ͬ�ķ�ʽ
        if (  true == mutex->non_recursive_mutex_->sem_unnamed_ )
        {
            ret = zce::sem_destroy(mutex->non_recursive_mutex_);

            if (0 != ret)
            {
                return last_error_with_default(EINVAL);
            }
        }
        else
        {
            ret = zce::sem_close(mutex->non_recursive_mutex_);

            if (0 != ret)
            {
                return last_error_with_default(EINVAL);
            }

            //WINDOWS��ûӴ��Ҫ����sem_unlink��͵���������͵����Ҫ��sem_closeǰҪ�õ��������
            //zce::sem_unlink(sema_name);
        }

        return 0;
    }
    //���������CSģ��PTHREAD MUTEX�������ڲ����ݹ飬����Ҫ��ʱ
    else if (ZCE_IS_CS_SIMULATE_PMUTEX(mutex))
    {
        ::DeleteCriticalSection (&(mutex->thr_nontimeout_mutex_));
        return 0;
    }
    //�������MUTEXģ��PTHREAD MUTEX�������ڲ����ݹ������Ҫ��ʱ ���߽����ⲿ���ݹ飬
    else if (ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex))
    {
        ::CloseHandle (mutex->recursive_mutex_);
        return 0;
    }
    else
    {
        return EINVAL;
    }

#elif defined (ZCE_OS_LINUX)

    return ::pthread_mutex_destroy (mutex);
#endif
}

int zce::pthread_mutex_lock (pthread_mutex_t *mutex)
{

#if defined (ZCE_OS_WINDOWS)

    //����������źŵ�ģ��PTHREAD MUTEX ���ǵݹ飩
    if (ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex))
    {
        //�����źŵ�
        int ret = zce::sem_wait(mutex->non_recursive_mutex_);

        if (0 != ret)
        {
            return last_error_with_default(EINVAL);
        }

        return 0;
    }
    //���������CSģ��PTHREAD MUTEX�������ڲ����ݹ飬����Ҫ��ʱ
    else if (ZCE_IS_CS_SIMULATE_PMUTEX(mutex))
    {
        ::EnterCriticalSection (&(mutex->thr_nontimeout_mutex_));
        return 0;
    }
    //�������MUTEXģ��PTHREAD MUTEX�������ڲ����ݹ������Ҫ��ʱ ���߽����ⲿ���ݹ飬
    else if (ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex))
    {
        DWORD retsult = ::WaitForSingleObject (mutex->recursive_mutex_, INFINITE);

        if (WAIT_OBJECT_0 == retsult || WAIT_ABANDONED == retsult)
        {
            return 0;
        }
        else
        {
            return EINVAL;
        }
    }
    else
    {
        return EINVAL;
    }

#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutex_lock(mutex);
# endif

}

//pthread mutex ��ʱ������
int zce::pthread_mutex_timedlock(pthread_mutex_t *mutex,
                                 const ::timespec *abs_timeout_spec)
{
#if defined (ZCE_OS_WINDOWS)
    //
    assert(abs_timeout_spec);

    //�õ����ʱ�䣬������ڣ�
    timeval now_time = zce::gettimeofday();
    timeval abs_time = zce::make_timeval(abs_timeout_spec);

    timeval timeout_time = zce::timeval_sub(abs_time, now_time, true);

    //����ǿ��Եݹ��
    if (ZCE_BIT_IS_SET(mutex->lock_type_, PTHREAD_MUTEX_RECURSIVE))
    {
        if  ( (PTHREAD_PROCESS_SHARED == mutex->lock_shared_ )
              || (PTHREAD_PROCESS_PRIVATE == mutex->lock_shared_
                  && ZCE_BIT_IS_SET(mutex->lock_type_, PTHREAD_MUTEX_TIMEOUT))
            )
        {
            //�ȴ�ʱ�䴥��
            DWORD retsult = ::WaitForSingleObject (mutex->recursive_mutex_,
                                                   static_cast<DWORD>( zce::total_milliseconds(timeout_time)));

            if (WAIT_OBJECT_0 == retsult || WAIT_ABANDONED == retsult)
            {
                return 0;
            }
            else if (WAIT_TIMEOUT == retsult)
            {
                return ETIMEDOUT;
            }
            else
            {
                return EINVAL;
            }
        }
        //��ȷ�����ַ�ʽ�ǲ�֧�ֳ�ʱ�ģ�����������ʶ����ô��˳�ʼ������
        else if ( PTHREAD_PROCESS_PRIVATE == mutex->lock_shared_
                  && !ZCE_BIT_IS_SET(mutex->lock_type_, PTHREAD_MUTEX_TIMEOUT) )
        {
            return ENOTSUP;
        }
        else
        {
            return EINVAL;
        }
    }
    //�ǵݹ����
    else
    {
        //�����źŵƣ���ʱ�˳�
        int ret = zce::sem_timedwait(mutex->non_recursive_mutex_, abs_timeout_spec);

        if (0 != ret )
        {
            return last_error_with_default(EINVAL);
        }

        return 0;
    }

#elif defined (ZCE_OS_LINUX)
    int result = 0;
    result = ::pthread_mutex_timedlock (mutex, abs_timeout_spec);

    //ACE�Ĵ���������һ��ת������ID�ģ���ETIMEDOUTת��ΪETIME����ETIME��ETIMEDOUTֵһ�㲻һ����
    //���ҿ���LINUX���ֲ᷵�ص���ETIMEDOUT��ACE�Ĵ�������SUN���̺߳�������ģ�

    if (result != 0 && result == ETIME)
    {
        errno = ETIMEDOUT;
    }

    return result;

#endif
}

//pthread mutex ��ʱ�������Ǳ�׼ʵ��,�������ڲ���ʱ�����timeval
int zce::pthread_mutex_timedlock (pthread_mutex_t *mutex,
                                  const timeval *abs_timeout_val)
{
    assert(abs_timeout_val);
    //���ʱ���Ǿ���ֵʱ�䣬Ҫ����Ϊ���ʱ��
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::pthread_mutex_timedlock(mutex, &abs_timeout_spec);
}

//pthread mutex ���Լ���
int zce::pthread_mutex_trylock (pthread_mutex_t *mutex)
{

#if defined (ZCE_OS_WINDOWS)

    //����������źŵ�ģ��PTHREAD MUTEX ���ǵݹ飩
    if (ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex))
    {
        //�����źŵ�
        int ret = zce::sem_trywait(mutex->non_recursive_mutex_);

        if (0 != ret)
        {
            return last_error_with_default(EINVAL);
        }

        return 0;
    }
    //���������CSģ��PTHREAD MUTEX�������ڲ����ݹ飬����Ҫ��ʱ
    else if (ZCE_IS_CS_SIMULATE_PMUTEX(mutex))
    {
        //������Խ����ٽ���������ֵӦ����TRUE
        BOOL bool_ret = ::TryEnterCriticalSection(&(mutex->thr_nontimeout_mutex_));

        if ( FALSE == bool_ret)
        {
            return EBUSY;
        }

        return 0;
    }
    //�������MUTEXģ��PTHREAD MUTEX�������ڲ����ݹ������Ҫ��ʱ ���߽����ⲿ���ݹ飬
    else if (ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex))
    {
        //�ȴ�0����ʱ�䴥������������������try
        DWORD retsult = ::WaitForSingleObject (mutex->recursive_mutex_, 0);

        if (WAIT_OBJECT_0 == retsult || WAIT_ABANDONED == retsult)
        {
            return 0;
        }
        else if (WAIT_TIMEOUT == retsult)
        {
            return EBUSY;
        }
        else
        {
            return EINVAL;
        }
    }
    else
    {
        return EINVAL;
    }

#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutex_trylock (mutex);
#endif

}

//pthread mutex ����
int zce::pthread_mutex_unlock (pthread_mutex_t *mutex)
{

#if defined (ZCE_OS_WINDOWS)

    int ret = 0;

    //����������źŵ�ģ��PTHREAD MUTEX ���ǵݹ飩
    if (ZCE_IS_SEMA_SIMULATE_PMUTEX(mutex))
    {
        //�����źŵ�
        ret = zce::sem_post(mutex->non_recursive_mutex_);

        if (0 != ret)
        {
            return last_error_with_default(EINVAL);
        }

        return 0;
    }
    //���������CSģ��PTHREAD MUTEX�������ڲ����ݹ飬����Ҫ��ʱ
    else if (ZCE_IS_CS_SIMULATE_PMUTEX(mutex))
    {
        ::LeaveCriticalSection (&(mutex->thr_nontimeout_mutex_));
        return 0;
    }
    //�������MUTEXģ��PTHREAD MUTEX�������ڲ����ݹ������Ҫ��ʱ ���߽����ⲿ���ݹ飬
    else if (ZCE_IS_MUTEX_SIMULATE_PMUTEX(mutex))
    {
        ::ReleaseMutex (mutex->recursive_mutex_);
        return 0;
    }
    else
    {
        return EINVAL;
    }

#elif defined (ZCE_OS_LINUX)
    return ::pthread_mutex_unlock(mutex);
#endif
}

