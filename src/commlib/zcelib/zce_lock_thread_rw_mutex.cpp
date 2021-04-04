
//�̵߳Ļ��������������������ݹ������ǵݹ�������д��

#include "zce_predefine.h"

#include "zce_os_adapt_mutex.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_rwlock.h"
#include "zce_os_adapt_semaphore.h"

#include "zce_log_logging.h"
#include "zce_lock_thread_rw_mutex.h"

/************************************************************************************************************
Class           : ZCE_Thread_RW_Mutex
************************************************************************************************************/
//���캯��
ZCE_Thread_RW_Mutex::ZCE_Thread_RW_Mutex()
{
    //pthread_rwlockattr_t���Եĳ�ʼ��
    int ret = 0;

    ret = zce::pthread_rwlock_initex(&rw_lock_, false);

    if (0 != ret )
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_init", ret);
        return;
    }

}

ZCE_Thread_RW_Mutex::~ZCE_Thread_RW_Mutex()
{
    int ret = 0;
    ret = zce::pthread_rwlock_destroy (&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_destroy", ret);
        return;
    }
}

//��ȡ��
void ZCE_Thread_RW_Mutex::lock_read()
{
    int ret = 0;
    ret = zce::pthread_rwlock_rdlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_rdlock", ret);
        return;
    }
}

//���Զ�ȡ��
bool ZCE_Thread_RW_Mutex::try_lock_read()
{
    int ret = 0;
    ret = zce::pthread_rwlock_trywrlock(&rw_lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//����ʱ��
bool ZCE_Thread_RW_Mutex::systime_lock_read(const ZCE_Time_Value &abs_time)
{
    int ret = 0;

    ret = zce::pthread_rwlock_timedrdlock(&rw_lock_,
                                          abs_time);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_timedrdlock", ret);
        return false;
    }

    return true;
}
//���ʱ��
bool ZCE_Thread_RW_Mutex::duration_lock_read(const ZCE_Time_Value &relative_time)
{
    ZCE_Time_Value abs_time(zce::gettimeofday());
    abs_time += relative_time;
    return systime_lock_read(abs_time);
}

//д����
void ZCE_Thread_RW_Mutex::lock_write()
{
    int ret = 0;
    ret = zce::pthread_rwlock_wrlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_wrlock", ret);
        return;
    }
}

//���Զ�ȡ��
bool ZCE_Thread_RW_Mutex::try_lock_write()
{
    int ret = 0;
    ret = zce::pthread_rwlock_trywrlock(&rw_lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//д������ʱ������ʱ��
bool ZCE_Thread_RW_Mutex::systime_lock_write(const ZCE_Time_Value &abs_time)
{
    int ret = 0;

    ret = zce::pthread_rwlock_timedwrlock(&rw_lock_,
                                          abs_time);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_timedlock", ret);
        return false;
    }

    return true;
}

//д������ʱ�����ʱ��
bool ZCE_Thread_RW_Mutex::duration_lock_write(const ZCE_Time_Value &relative_time)
{
    ZCE_Time_Value abs_time(zce::gettimeofday());
    abs_time += relative_time;
    return systime_lock_write(abs_time);
}



//��д��
void ZCE_Thread_RW_Mutex::unlock_write()
{
    //����
    int ret = 0;
    ret = zce::pthread_rwlock_unlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_unlock", ret);
        return;
    }
}

//�����
void ZCE_Thread_RW_Mutex::unlock_read()
{
    //����
    int ret = 0;
    ret = zce::pthread_rwlock_unlock(&rw_lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_rwlock_unlock", ret);
        return;
    }
}

//ȡ���ڲ�������ָ��
pthread_rwlock_t *ZCE_Thread_RW_Mutex::get_lock()
{
    return &rw_lock_;
}


/************************************************************************************************************
Class           : ZCE_Thread_Win_RW_Mutex �������Ķ�д�������ṩ��ʱ�Ⱥ���
************************************************************************************************************/


#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

//�����WIN�Դ��Ķ�д��

//���캯��
ZCE_Thread_Win_RW_Mutex::ZCE_Thread_Win_RW_Mutex()
{
    ::InitializeSRWLock(&(this->rwlock_slim_));
}

ZCE_Thread_Win_RW_Mutex::~ZCE_Thread_Win_RW_Mutex()
{
}

//��ȡ��
void ZCE_Thread_Win_RW_Mutex::lock_read()
{
    ::AcquireSRWLockShared(&(this->rwlock_slim_));
    return;
}

//���Զ�ȡ��
bool ZCE_Thread_Win_RW_Mutex::try_lock_read()
{
    //�����WIN�Դ��Ķ�д��
    BOOL bret = ::TryAcquireSRWLockShared(&(this->rwlock_slim_));
    if (FALSE == bret)
    {
        errno = EBUSY;
        return false;
    }
    return true;
}

//д����
void ZCE_Thread_Win_RW_Mutex::lock_write()
{
    ::AcquireSRWLockExclusive(&(this->rwlock_slim_));
    return;
}

//���Զ�ȡ��
bool ZCE_Thread_Win_RW_Mutex::try_lock_write()
{
    //�����WIN�Դ��Ķ�д��
    BOOL bret = ::TryAcquireSRWLockExclusive(&(this->rwlock_slim_));
    if (FALSE == bret)
    {
        errno = EBUSY;
        return false;
    }
    return true;
}

//����,����Ƕ�д��Ҳֻ��Ҫ��һ������
void ZCE_Thread_Win_RW_Mutex::unlock_read()
{
    ::ReleaseSRWLockShared(&(this->rwlock_slim_));
}

//��϶���VS2019��һ��BUG�������ϲ�Ӧ��������澯
#pragma warning (disable:26110)

void ZCE_Thread_Win_RW_Mutex::unlock_write()
{
    ::ReleaseSRWLockExclusive(&(this->rwlock_slim_));
}

#pragma warning (default:26110)

//ȡ���ڲ�������ָ��
SRWLOCK *ZCE_Thread_Win_RW_Mutex::get_lock()
{
    return &rwlock_slim_;
}

#endif