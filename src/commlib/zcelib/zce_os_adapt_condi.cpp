
#include "zce_predefine.h"
#include "zce_log_logging.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_mutex.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_semaphore.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_condi.h"

//�ö��WINDOWS20008��������������ģ��PTHREAD CV��������
#ifndef ZCE_IS_USE_WIN2008_SIMULATE_PCV
#define ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond) ( (ZCE_BIT_IS_SET((cond)->outer_lock_type_ , PTHREAD_MUTEX_RECURSIVE)) \
                                                && (ZCE_BIT_ISNOT_SET((cond)->outer_lock_type_ , PTHREAD_MUTEX_TIMEOUT)) )
#endif

//ZCE_SUPPORT_WINSVR2008 == 1����Ĵ��붼�Գ�һ�Σ��������ʱ��ע��
//����WIN SERVER 2008��WINDOWS�Լ������������ķ�װ��

//
int zce::pthread_condattr_init(pthread_condattr_t *attr)
{
#if defined (ZCE_OS_WINDOWS)

    //WINDOWS��Ĭ�Ͼ��ǵݹ�ģ���Ҫ�Ҹ���ǵݹ���һ�Ҫ����
    attr->outer_lock_type_ = PTHREAD_MUTEX_RECURSIVE;
    //��ʼ������
    attr->cv_name_[PATH_MAX] = '\0';
    attr->cv_name_[0] = '\0';
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_condattr_init (attr);
#endif
}

//
int zce::pthread_condattr_destroy(pthread_condattr_t *attr)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(attr);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_condattr_destroy (attr);
#endif
}

//������������ĳ�ʼ��
int zce::pthread_cond_init(pthread_cond_t *cond,
                           const pthread_condattr_t *attr)
{
#if defined (ZCE_OS_WINDOWS)

    if (attr)
    {
        cond->outer_lock_type_ = attr->outer_lock_type_;
    }
    else
    {
        cond->outer_lock_type_ = PTHREAD_MUTEX_RECURSIVE;
    }

    //WIN SERVER 2008��VISTA ��֧����������
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    //������߳��ڲ��ģ������ǵݹ�ģ�����û�г�ʱ���ܣ�������Windows���������������ɻ�
    if (ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond))
    {
        ::InitializeConditionVariable(&(cond->cv_object_));
        return 0;
    }

#endif

    cond->simulate_cv_.block_sema_ = NULL;
    cond->simulate_cv_.finish_broadcast_ = NULL;
    cond->simulate_cv_.waiters_ = 0;
    cond->simulate_cv_.was_broadcast_ = false;

    char *sem_block_ptr = NULL, *sem_finish_ptr = NULL;
    char sem_block_name[PATH_MAX + 1], sem_finish_name[PATH_MAX + 1];
    sem_block_name[PATH_MAX] = '\0';
    sem_finish_name[PATH_MAX] = '\0';

    //��δ���ֻ��WIN32���ã��Ҽ���
    pthread_mutexattr_t waiters_lock_attr;
    zce::pthread_mutexattr_init(&waiters_lock_attr);
    waiters_lock_attr.lock_shared_ = PTHREAD_PROCESS_PRIVATE;
    waiters_lock_attr.lock_type_ = PTHREAD_MUTEX_RECURSIVE;

    //��ʼ���̵߳Ļ�����
    int result = 0;
    result = zce::pthread_mutex_init(&cond->simulate_cv_.waiters_lock_,
                                     &waiters_lock_attr);

    if (result != 0)
    {
        return result;
    }

    cond->simulate_cv_.block_sema_ = zce::sem_open(sem_block_ptr,
                                                   O_CREAT,
                                                   ZCE_DEFAULT_FILE_PERMS,
                                                   0);

    //���ʧ���ˣ�Ҫ����ǰ���õ���Դ
    if (!cond->simulate_cv_.block_sema_)
    {
        zce::pthread_mutex_destroy(&cond->simulate_cv_.waiters_lock_);
        return -1;
    }

    cond->simulate_cv_.finish_broadcast_ = zce::sem_open(sem_finish_ptr,
                                                         O_CREAT,
                                                         ZCE_DEFAULT_FILE_PERMS,
                                                         0);

    //���ʧ���ˣ�Ҫ����ǰ���õ���Դ,���ֶַ�������Դ���鷳
    if (!cond->simulate_cv_.finish_broadcast_)
    {
        zce::pthread_mutex_destroy(&cond->simulate_cv_.waiters_lock_);
        zce::sem_close(cond->simulate_cv_.block_sema_);
        //��ʵû��
        //zce::sem_unlink(sem_block_name);
        return EINVAL;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_init (cond, attr);
#endif
}

//��ʼ�������������󣬲�ͬ��ƽ̨����ͬ��Ĭ�϶���
//�Ǳ�׼�����ǽ�����ʹ�ã��򵥶���,
//���Ҫ����̹����鷳���ϸ������֣�ͬʱ��LINUXƽ̨�£������pthread_condattr_t���빲���ڴ�
int zce::pthread_cond_initex(pthread_cond_t *cond,
                             bool win_mutex_or_sema)
{

    //ǰ���д��󷵻أ�
    int result = 0;

    pthread_condattr_t attr;
    result = zce::pthread_condattr_init (&attr);
    if (0 != result)
    {
        return result;
    }

    //���������WINDOWS���õģ�����ĳЩʱ���ҿ������ٽ����������Ǹ��صĻ�����
#if defined (ZCE_OS_WINDOWS)
    //Ĭ�ϻ����õݹ����
    attr.outer_lock_type_ |= PTHREAD_MUTEX_RECURSIVE;

    if (win_mutex_or_sema)
    {
        attr.outer_lock_type_ |= PTHREAD_MUTEX_TIMEOUT;
    }

#elif defined (ZCE_OS_LINUX)
    ZCE_UNUSED_ARG(win_mutex_or_sema);

    result = ::pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
    if (0 != result)
    {
        return result;
    }
#endif

    result = zce::pthread_cond_init(cond, &attr);
    zce::pthread_condattr_destroy (&attr);

    if (0 != result)
    {
        return EINVAL;
    }

    return 0;
}

//�����������������
int zce::pthread_cond_destroy(pthread_cond_t *cond)
{
#if defined (ZCE_OS_WINDOWS)

    //ʹ��WINDOWS����������
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    if (ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond))
    {
        //WINDOWS����������û���ͷ�
        return 0;
    }

#endif

    zce::pthread_mutex_destroy(&cond->simulate_cv_.waiters_lock_);

    zce::sem_close(cond->simulate_cv_.block_sema_);
    zce::sem_close(cond->simulate_cv_.finish_broadcast_);

    //WINƽ̨�£�����������������͵��
    //zce::sem_unlink(sem_name);

    cond->simulate_cv_.block_sema_ = NULL;
    cond->simulate_cv_.finish_broadcast_ = NULL;

    cond->simulate_cv_.waiters_ = 0;
    cond->simulate_cv_.was_broadcast_ = false;

    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_destroy (cond);
#endif
}

//���������ȴ�һ��ʱ�䣬��ʱ�����
int zce::pthread_cond_timedwait(pthread_cond_t *cond,
                                pthread_mutex_t *external_mutex,
                                const ::timespec *abs_timespec_out)
{

#if defined (ZCE_OS_WINDOWS)

    //����ⲿ��MUTEX�����ͺ͹���ʽ����������Ԥ�ڵģ�����
    if ( external_mutex->lock_type_ != cond->outer_lock_type_ )
    {
        ZCE_ASSERT(false);
        return EINVAL;
    }

    //ʹ��WINDOWS2008����������
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    if (ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond))
    {
        DWORD wait_msec = INFINITE;

        //����г�ʱ��������Գ�ʱʱ��
        if (abs_timespec_out)
        {
            //�õ����ʱ�䣬������ڣ�
            timeval now_time = zce::gettimeofday();
            timeval abs_time = zce::make_timeval(abs_timespec_out);

            timeval timeout_time = zce::timeval_sub(abs_time, now_time, true);
            wait_msec = static_cast<DWORD>( zce::total_milliseconds(timeout_time));
        }

        //WINDOWS����������û���ͷ�
        BOOL bret = ::SleepConditionVariableCS(
                        &(cond->cv_object_),
                        &(external_mutex->thr_nontimeout_mutex_),
                        wait_msec);

        if (bret == FALSE)
        {
            //SleepConditionVariableCS ����GetLastError�������
            //��ЩAPI����Ƽ�����ӳ��WINDOWS��ǰ��һ��
            if (::GetLastError() == WAIT_TIMEOUT)
            {
                return ETIMEDOUT;
            }
            else
            {
                return EINVAL;
            }
        }

        return 0;
    }

#endif  //ʹ��WINDOWS2008����������

    // Prevent race conditions on the <waiters_> count.
    zce::pthread_mutex_lock (&(cond->simulate_cv_.waiters_lock_));
    ++(cond->simulate_cv_.waiters_);
    zce::pthread_mutex_unlock (&(cond->simulate_cv_.waiters_lock_));

    int result = 0;

    //���ⲿ�������½�����
    //�����ͷ���Դ���д���������ͷ�ʧ�ܣ����������
    zce::pthread_mutex_unlock (external_mutex);

    ///@note����ط�����ĳ�����飬Ҳ���������ⲽ�������ⲽ�Ƿ�Ҫ
    ///�γ�ԭ�Ӳ��������������Douglas C. Schmidt and Irfan Pyarali���������й�������
    ///������Ϊ�����õ����źŵƣ����������ʹ���˲�ӵ�����ط��õ�external_mutex��������
    ///signal���߹㲥��Ҳ�������������������������Ϊ���Ǻǡ�
    ///��Ȼ����������⣬�ͻ���SignalObjectAndWait��

    //����ǳ�ʱ�ȴ����ͽ��еȴ�
    if (abs_timespec_out)
    {
        result = zce::sem_timedwait(cond->simulate_cv_.block_sema_,
                                    abs_timespec_out);

    }
    else
    {
        result = zce::sem_wait (cond->simulate_cv_.block_sema_);
    }

    //��¼����
    if (result != 0)
    {
        result = zce::last_error_with_default(EINVAL);
    }

    //ͬ�������⾺��
    zce::pthread_mutex_lock (&cond->simulate_cv_.waiters_lock_);
    //�źŵ��Ѿ��˳������ٵȴ�������
    --(cond->simulate_cv_.waiters_);
    bool const last_waiter = (cond->simulate_cv_.was_broadcast_
                              && cond->simulate_cv_.waiters_ == 0);
    zce::pthread_mutex_unlock (&cond->simulate_cv_.waiters_lock_);

    if (result == 0)
    {
        //��������ر𿴲����ĵط���������ͼӦ�������һ������֪�㲥�ߣ�
        //��������һ���ˣ�֪ͨbroadcaster�������������һ�����ˣ�������˳��ˣ����õ��ˡ�
        //����ط����źŵ���ʵ��һЩ���⣬��Ϊ�����ڹ�ƽ�ԣ����������ģ��Ҫ��㲥��ʱ���ⲿ��
        //���ּ��ϣ��������ⲻ��
        if (last_waiter)
        {
            // Release the signaler/broadcaster if we're the last waiter.
            zce::sem_post (cond->simulate_cv_.finish_broadcast_);
        }
    }

    //���ⲿ�������¼���
    zce::pthread_mutex_lock (external_mutex);

    return result;

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_cond_timedwait(cond,
                                    external_mutex,
                                    abs_timespec_out);

#endif
}

//���������ȴ�һ��ʱ�䣬��ʱ�����,ʱ����������ڲ�ͳһ��timeval
int zce::pthread_cond_timedwait(pthread_cond_t *cond,
                                pthread_mutex_t *external_mutex,
                                const timeval *abs_timeout_val)
{
    assert(abs_timeout_val);
    //���ʱ���Ǿ���ֵʱ�䣬Ҫ����Ϊ���ʱ��
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::pthread_cond_timedwait(cond,
                                       external_mutex,
                                       &abs_timeout_spec);
}

//���������ȴ�
int zce::pthread_cond_wait(pthread_cond_t *cond,
                           pthread_mutex_t *external_mutex)
{
#if defined (ZCE_OS_WINDOWS)
    //����д��Ϊ�˱��⺯����ͻ�澯��
    const ::timespec *abs_timespec_out = NULL;
    return zce::pthread_cond_timedwait(cond,
                                       external_mutex,
                                       abs_timespec_out);
#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_cond_wait(cond,
                               external_mutex);
#endif
}

//
int zce::pthread_cond_broadcast(pthread_cond_t *cond)
{
#if defined (ZCE_OS_WINDOWS)

    //�ڵ��������ʽǰ���ⲿ�������������ϵģ�������ط��������ʣ���ʵPOSIX��û���ر���ȷ˵�������⣩
    // The <external_mutex> must be locked before this call is made.

    //ʹ��WINDOWS����������
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    if (ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond))
    {
        ::WakeAllConditionVariable(&(cond->cv_object_));
        return 0;
    }

#endif

    // This is needed to ensure that <waiters_> and <was_broadcast_> are
    // consistent relative to each other.
    zce::pthread_mutex_lock (&cond->simulate_cv_.waiters_lock_);
    bool have_waiters = false;

    if (cond->simulate_cv_.waiters_ > 0)
    {
        // We are broadcasting, even if there is just one waiter...
        // Record the fact that we are broadcasting.  This helps the
        // cond_wait() method know how to optimize itself.  Be sure to
        // set this with the <waiters_lock_> held.
        cond->simulate_cv_.was_broadcast_ = true;
        have_waiters = true;
    }

    zce::pthread_mutex_unlock (&cond->simulate_cv_.waiters_lock_);
    int result = 0;

    if (have_waiters)
    {
        //ACE�Ƚ�ϲ������if�ķ�ʽ���Ҳ����ر�ϰ�ߣ����ڶ�㴦��Ĺ������������Ҳ���պ�
        //�������еĵȴ���,
        if ( zce::sem_post (cond->simulate_cv_.block_sema_, cond->simulate_cv_.waiters_) != 0)
        {
            result = EINVAL;
        }

        //ע������������ʵ���ǲ������ģ���Ϊ��ʵ�������Ͻ���������仰��������仰Ҳ������ԭ�ӵģ�
        //����Ҳ��post block_sema_ ��wait���߳�ȡ��ִ��Ȩ����finish_broadcast_�Ѿ�post�ˣ�
        //��ô�����û���κ������ˣ�ACE��Դ�����������õ�SignalObjectAndWait
        //��ACE��ʵ��ҲҪ���ҵ���broadcast�ǣ��ⲿ���Ǽ��ϵģ����԰�
        // Wait for all the awakened threads to acquire their part of
        // the counting semaphore.
        else if (zce::sem_wait (cond->simulate_cv_.finish_broadcast_) != 0 )
        {
            result = EINVAL;
        }

        //�����������Ҫ���ⲿ�������ϵģ�����was_broadcast_�ĵ���ҲOK
        // This is okay, even without the <waiters_lock_> held because
        // no other waiter threads can wake up to access it.
        cond->simulate_cv_.was_broadcast_ = false;
    }

    return result;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_broadcast(cond);
#endif
}

//���ź�
int zce::pthread_cond_signal(pthread_cond_t *cond)
{
#if defined (ZCE_OS_WINDOWS)

    //�ڵ��������ʽǰ���ⲿ�������������ϵģ�
    //ʹ��WINDOWS����������
#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

    if (ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond))
    {
        ::WakeConditionVariable(&(cond->cv_object_));
        return 0;
    }

#endif

    int result = 0;
    //�Ƿ������ڵȴ�
    zce::pthread_mutex_lock (&cond->simulate_cv_.waiters_lock_);
    bool const have_waiters = cond->simulate_cv_.waiters_ > 0;
    zce::pthread_mutex_unlock (&cond->simulate_cv_.waiters_lock_);

    if (have_waiters)
    {
        result = zce::sem_post (cond->simulate_cv_.block_sema_);

        if (0 != result )
        {
            return EINVAL;
        }
    }

    // No-op
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_signal(cond);
#endif
}

