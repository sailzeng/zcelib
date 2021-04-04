
#include "zce_predefine.h"
#include "zce_os_adapt_semaphore.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_error.h"
#include "zce_log_logging.h"
#include "zce_lock_thread_semaphore.h"

//���캯��,Ĭ�ϴ��������źŵƣ��߳���һ���������źŵƾ��㹻��,
ZCE_Thread_Semaphore::ZCE_Thread_Semaphore(unsigned int init_value):
    lock_(NULL)
{
    int ret = 0;

    lock_ = new sem_t ();
    ret = zce::sem_init(lock_, 0, init_value);

    if (0 != ret )
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::sem_init fail.", ret);
    }
}

ZCE_Thread_Semaphore::~ZCE_Thread_Semaphore()
{
    //û�г�ʼ����
    if (!lock_)
    {
        return;
    }

    zce::sem_destroy(lock_);

    //sem_destroy�����ͷţ�
    delete lock_;
    lock_ = NULL;
}

//����
void ZCE_Thread_Semaphore::lock()
{
    //�źŵ�����
    int ret =  zce::sem_wait (lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::sem_wait", ret);
        return;
    }
}

//��������
bool ZCE_Thread_Semaphore::try_lock()
{
    //�źŵ�����
    int ret =  zce::sem_trywait (lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//����,
void ZCE_Thread_Semaphore::unlock()
{
    int ret = zce::sem_post (lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::sem_post", ret);
        return;
    }
}

//����ʱ�䳬ʱ�ĵ���������ʱ�����
bool ZCE_Thread_Semaphore::systime_lock(const ZCE_Time_Value &abs_time)
{
    int ret = 0;
    ret = zce::sem_timedwait(lock_, abs_time);

    if (0 != ret)
    {
        if (errno != ETIMEDOUT)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::sem_timedwait", ret);
        }

        return false;
    }

    return true;
}

//���ʱ��ĳ�ʱ��������ʱ�󣬽���
bool ZCE_Thread_Semaphore::duration_lock(const ZCE_Time_Value &relative_time)
{
    timeval abs_time = zce::gettimeofday();
    abs_time = zce::timeval_add(abs_time, relative_time);
    return systime_lock(abs_time);
}

