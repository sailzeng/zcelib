
#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_mutex.h"
#include "zce_os_adapt_condi.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_rwlock.h"
#include "zce_log_logging.h"

//��д���Ķ���ĳ�ʼ��
int zce::pthread_rwlock_init(pthread_rwlock_t *rwlock,
                             const pthread_rwlockattr_t *attr)
{
#if defined (ZCE_OS_WINDOWS)

    //�����������Ұ��ظ���ʼ�����Ƿ��ʼ���ĸ����ж�ɾ���ˣ��о�����������û��Ҫ

    //������ù��ֻ��ģ��
    int result = 0;

    if (attr)
    {
        rwlock->priority_to_write_ = attr->priority_to_write_;
    }
    else
    {
        rwlock->priority_to_write_ = false;
    }
    pthread_mutexattr_t mutex_attr;
    zce::pthread_mutexattr_init(&mutex_attr);
    mutex_attr.lock_shared_ = PTHREAD_PROCESS_PRIVATE;
    mutex_attr.lock_type_ = PTHREAD_MUTEX_RECURSIVE;

    //��ʼ������ͬ������

    //һЩ��������д�ı���
    if ( (result = zce::pthread_mutex_init(&rwlock->rw_mutex_, &mutex_attr)) != 0)
    {
        return EINVAL;
    }

    //�ȴ���ȡ������������ʼ��
    if ( (result = zce::pthread_cond_initex(&rwlock->rw_condreaders_,
                                            false )) != 0)
    {
        zce::pthread_mutex_destroy(&rwlock->rw_mutex_);
        return EINVAL;
    }

    //�ȴ�д�������������ʼ��
    if ( (result = zce::pthread_cond_initex(&rwlock->rw_condwriters_,
                                            false)) != 0)
    {
        zce::pthread_cond_destroy(&rwlock->rw_condreaders_);
        zce::pthread_mutex_destroy(&rwlock->rw_mutex_);
        return EINVAL;
    }

    rwlock->rw_nwaitreaders_ = 0;
    rwlock->rw_nwaitwriters_ = 0;
    rwlock->rw_refcount_ = 0;

    return 0;

#elif defined (ZCE_OS_LINUX)

    return ::pthread_rwlock_init(rwlock, attr);
#endif

}

//��ʼ����д������
int zce::pthread_rwlock_initex(pthread_rwlock_t *rwlock,
                               bool  priority_to_write)
{
    int result = 0;
    pthread_rwlockattr_t attr;

#if defined ZCE_OS_WINDOWS
    attr.priority_to_write_ = priority_to_write;

#elif defined ZCE_OS_LINUX

    ZCE_UNUSED_ARG(priority_to_write);

    //��ʵ�����룬������NULL����ʵ��һ��
    result = ::pthread_rwlockattr_init(&attr);

    if (result != 0)
    {
        return result;
    }

    result = ::pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
    if (result != 0)
    {
        return result;
    }

#endif

    result = zce::pthread_rwlock_init(rwlock, &attr);

    if (result != 0)
    {
        return result;
    }

    return 0;
}

//��д���Ķ��������
int zce::pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
#if defined (ZCE_OS_WINDOWS)

    //WIN SVR 2008�Ժ��������֧��
#if defined (ZCE_USE_WIN_SLIM)

    //::ReleaseSRWLockShared(&(rwlock->rwlock_slim_));
    return 0;

#else

    //���еȴ��ģ���������
    if (rwlock->rw_refcount_ != 0
        || rwlock->rw_nwaitreaders_ != 0
        || rwlock->rw_nwaitwriters_ != 0)
    {
        return EBUSY;
    }

    zce::pthread_mutex_destroy(&rwlock->rw_mutex_);
    zce::pthread_cond_destroy(&rwlock->rw_condreaders_);
    zce::pthread_cond_destroy(&rwlock->rw_condwriters_);

    return 0;
#endif

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_destroy(rwlock);
#endif
}

//��ö�ȡ����
int zce::pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{

#if defined (ZCE_OS_WINDOWS)

    //���ϱ�����
    int  result = zce::pthread_mutex_lock(&rwlock->rw_mutex_);

    if (0 != result)
    {
        return (result);
    }

    //PTHREAD WIN32��ʵ��������ط�û�������������ȴ���ԭ��λ�ã�������Ϊ������覴õģ�
    //���ܻᵼ�½����CPUѭ��

    //�ȴ���ö�д�������������д��������Ҫд������ڵȴ���ƫ��д����
    while ((rwlock->rw_refcount_ < 0)
           || (true == rwlock->priority_to_write_ && rwlock->rw_nwaitwriters_ > 0))
    {
        rwlock->rw_nwaitreaders_++;
        //����wait������rw_mutex_�ᱻ�򿪣��������˻��������ʱ�����
        result = zce::pthread_cond_wait(&rwlock->rw_condreaders_,
                                        &(rwlock->rw_mutex_));
        rwlock->rw_nwaitreaders_--;

        if (result != 0)
        {
            break;
        }
    }

    //��õ��˶���������
    if (result == 0)
    {
        rwlock->rw_refcount_++;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);

#elif defined (ZCE_OS_LINUX)

    return ::pthread_rwlock_rdlock(rwlock);
#endif

}

//���Ի�ȡ��ȡ��
int zce::pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{

#if defined (ZCE_OS_WINDOWS)

    int result = zce::pthread_mutex_lock(&rwlock->rw_mutex_);

    if (result != 0)
    {
        return (result);
    }

    //���������д��������Ҫд������ڵȴ�,��ô�Ͳ���get��ȡ��
    if ((rwlock->rw_refcount_ < 0)
        || (true == rwlock->priority_to_write_ && rwlock->rw_nwaitwriters_ > 0))
    {
        result = EBUSY;
    }
    else
    {
        rwlock->rw_refcount_++;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_tryrdlock(rwlock);
#endif
}

//��ȡ���ĳ�ʱ�������������UNP V2��û�и�����
int zce::pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock,
                                    const ::timespec *abs_timeout_spec)
{
#if defined (ZCE_OS_WINDOWS)

    int result = zce::pthread_mutex_timedlock(&rwlock->rw_mutex_,
                                              abs_timeout_spec);

    //���е����Ϊɶphtread�ĺܶຯ���þ���ʱ���ˣ�abs_timeout_spec
    if (result != 0)
    {
        return (result);
    }

    //�ȴ���ö�д�������������д��������Ҫд������ڵȴ���ƫ��д����
    while ((rwlock->rw_refcount_ < 0)
           || (true == rwlock->priority_to_write_ && rwlock->rw_nwaitwriters_ > 0))
    {
        rwlock->rw_nwaitreaders_++;
        //����wait������rw_mutex_�ᱻ�򿪣��������˻��������ʱ�����
        result = zce::pthread_cond_timedwait(&rwlock->rw_condreaders_,
                                             &(rwlock->rw_mutex_),
                                             abs_timeout_spec);
        rwlock->rw_nwaitreaders_--;

        if (result != 0)
        {
            break;
        }
    }

    //��õ��˶���������
    if (result == 0)
    {
        rwlock->rw_refcount_++;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_timedrdlock(rwlock, abs_timeout_spec);
#endif
}

//�Ǳ�׼����ȡ���ĳ�ʱ������ʱ�����������timeval��
int zce::pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock,
                                    const timeval *abs_timeout_val)
{
    //���ʱ���Ǿ���ֵʱ�䣬Ҫ����Ϊ���ʱ��
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::pthread_rwlock_timedrdlock(rwlock, &abs_timeout_spec);
}

//��ȡд��
int zce::pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
#if defined (ZCE_OS_WINDOWS)

    int result = pthread_mutex_lock(&rwlock->rw_mutex_);
    if (result != 0)
    {
        return (result);
    }

    //���������ʹ���������۶�д����Ҫ�ȴ��������ȡ���ȣ�������˻��ڵȴ�����Ҳ�ȴ�
    while ((rwlock->rw_refcount_ != 0)
           || (false == rwlock->priority_to_write_ && rwlock->rw_nwaitreaders_ > 0))
    {
        rwlock->rw_nwaitwriters_++;
        result = zce::pthread_cond_wait(&rwlock->rw_condwriters_,
                                        &(rwlock->rw_mutex_));
        rwlock->rw_nwaitwriters_--;

        if (result != 0)
        {
            break;
        }
    }

    //��ʶ����д�߻��
    if (result == 0)
    {
        rwlock->rw_refcount_ = -1;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_wrlock(rwlock);
#endif
}

//�����ܷ�ӵ��д������������ʽ
int zce::pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{

#if defined (ZCE_OS_WINDOWS)

    int result = zce::pthread_mutex_lock(&rwlock->rw_mutex_);

    if ( result != 0)
    {
        return (result);
    }

    //����ж���д�ߴ��ڣ��Ͳ���ӵ��д��
    if ( (rwlock->rw_refcount_ != 0)
         || (false == rwlock->priority_to_write_ && rwlock->rw_nwaitreaders_ > 0) )
    {
        result = EBUSY;
    }
    //�����ӵ��д��
    else
    {
        rwlock->rw_refcount_ = -1;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_trywrlock(rwlock);
#endif
}

//��ȡд�������ҵȴ�����ʱΪֹ��
int zce::pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock,
                                    const ::timespec *abs_timeout_spec)
{
#if defined (ZCE_OS_WINDOWS)

    int result = zce::pthread_mutex_timedlock(&rwlock->rw_mutex_, abs_timeout_spec);
    if ( result != 0)
    {
        return (result);
    }

    //���������ʹ���������۶�д����Ҫ�ȴ��������ȡ���ȣ�������˻��ڵȴ�����Ҳ�ȴ�
    while ((rwlock->rw_refcount_ != 0)
           || (false == rwlock->priority_to_write_ && rwlock->rw_nwaitreaders_ > 0) )
    {
        rwlock->rw_nwaitwriters_++;
        result = zce::pthread_cond_timedwait(&rwlock->rw_condwriters_,
                                             &(rwlock->rw_mutex_),
                                             abs_timeout_spec);
        rwlock->rw_nwaitwriters_--;

        if (result != 0)
        {
            break;
        }
    }

    if (result == 0)
    {
        rwlock->rw_refcount_ = -1;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_timedwrlock(rwlock, abs_timeout_spec);
#endif
}

//�Ǳ�׼����ȡ���ĳ�ʱ������ʱ�����������timeval��
int zce::pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock,
                                    const timeval *abs_timeout_val)
{
    //���ʱ���Ǿ���ֵʱ�䣬Ҫ����Ϊ���ʱ��
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::pthread_rwlock_timedwrlock(rwlock, &abs_timeout_spec);
}

//�������������������Խ����ȡ������д������������Ҫ�ر�ָ��
int zce::pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
#if defined (ZCE_OS_WINDOWS)

    //������
    int result = zce::pthread_mutex_lock(&rwlock->rw_mutex_);

    if ( result != 0)
    {
        return (result);
    }

    //����Ƕ���ռ��������
    if (rwlock->rw_refcount_ > 0)
    {
        rwlock->rw_refcount_--;
    }
    //��������ң�д�ߣ�ռ��������
    else if (rwlock->rw_refcount_ == -1)
    {
        rwlock->rw_refcount_ = 0;
    }
    //�����ϲ��ᵽ���
    else
    {
        //�������Ӧ���������д���ˣ�û�м���������������˽�������
    }

    //���ݶ����Ȼ���д�����ȣ����д���
    //���д����
    if (rwlock->priority_to_write_)
    {
        //�����ʱ����д����˵ȴ������ȸ����ҷ����ź�
        if (rwlock->rw_nwaitwriters_ > 0)
        {
            if (rwlock->rw_refcount_ == 0)
            {
                result = zce::pthread_cond_signal(&rwlock->rw_condwriters_);
            }
        }
        //�����ʱ���ж��ߵ��ڵȴ��������������㲥
        else if (rwlock->rw_nwaitreaders_ > 0)
        {
            result = zce::pthread_cond_broadcast(&rwlock->rw_condreaders_);
        }
    }
    //����Ƕ�ȡ����
    else
    {
        if (rwlock->rw_nwaitreaders_ > 0)
        {
            result = zce::pthread_cond_broadcast(&rwlock->rw_condreaders_);
        }
        //�����ʱ����д����˵ȴ������ȸ����ҷ����ź�
        else if (rwlock->rw_nwaitwriters_ > 0)
        {
            if (rwlock->rw_refcount_ == 0)
            {
                result = zce::pthread_cond_signal(&rwlock->rw_condwriters_);
            }
        }

        //�����ʱ���ж��ߵ��ڵȴ��������������㲥
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_rwlock_unlock(rwlock);
#endif

}

