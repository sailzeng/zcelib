#include "zce_predefine.h"
#include "zce_os_adapt_mutex.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_rwlock.h"
#include "zce_os_adapt_semaphore.h"
#include "zce_os_adapt_error.h"
#include "zce_log_logging.h"

#include "zce_lock_thread_mutex.h"

/************************************************************************************************************
Class           : ZCE_Thread_Light_Mutex �������Ļ����������ṩ��ʱ��
************************************************************************************************************/

//���캯��
ZCE_Thread_Light_Mutex::ZCE_Thread_Light_Mutex ()
{
    int ret = 0;

    ret = zce::pthread_mutex_initex(&lock_,
                                    false,
                                    true,
                                    false,
                                    NULL);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }

}

//���ٻ�����
ZCE_Thread_Light_Mutex::~ZCE_Thread_Light_Mutex (void)
{
    int ret = 0;
    ret = zce::pthread_mutex_destroy (&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_destroy", ret);
        return;
    }
}

//����
void ZCE_Thread_Light_Mutex::lock()
{
    int ret = 0;
    ret = zce::pthread_mutex_lock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_lock", ret);
        return;
    }
}

//��������
bool ZCE_Thread_Light_Mutex::try_lock()
{
    int ret = 0;
    ret = zce::pthread_mutex_trylock(&lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//����,
void ZCE_Thread_Light_Mutex::unlock()
{
    int ret = 0;
    ret = zce::pthread_mutex_unlock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_unlock", ret);
        return;
    }
}

//ȡ���ڲ�������ָ��
pthread_mutex_t *ZCE_Thread_Light_Mutex::get_lock()
{
    return &lock_;
}

/************************************************************************************************************
Class           : ZCE_Thread_Recursive_Mutex
************************************************************************************************************/
//���캯��
ZCE_Thread_Recursive_Mutex::ZCE_Thread_Recursive_Mutex ()
{
    int ret = 0;

    //����ط�Ψһ�����治ͬ�ľ���need_timeout������Ϊ��true
    ret = zce::pthread_mutex_initex(&lock_,
                                    false,
                                    true,
                                    true,
                                    NULL);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }
}

//�����������ͷ�MUTEX��Դ
ZCE_Thread_Recursive_Mutex::~ZCE_Thread_Recursive_Mutex (void)
{
    int ret = 0;
    ret = zce::pthread_mutex_destroy (&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_destroy", ret);
        return;
    }
}

//����
void ZCE_Thread_Recursive_Mutex::lock()
{
    int ret = 0;
    ret = zce::pthread_mutex_lock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_lock", ret);
        return;
    }
}

//��������
bool ZCE_Thread_Recursive_Mutex::try_lock()
{
    int ret = 0;
    ret = zce::pthread_mutex_trylock(&lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//����,
void ZCE_Thread_Recursive_Mutex::unlock()
{
    int ret = 0;
    ret = zce::pthread_mutex_unlock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_unlock", ret);
        return;
    }
}

//����ʱ��
bool ZCE_Thread_Recursive_Mutex::systime_lock(const ZCE_Time_Value &abs_time)
{
    int ret = 0;
    ret = zce::pthread_mutex_timedlock(&lock_, abs_time);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_timedlock", ret);
        return false;
    }

    return true;
}

//���ʱ��
bool ZCE_Thread_Recursive_Mutex::duration_lock(const ZCE_Time_Value &relative_time)
{
    timeval abs_time = zce::gettimeofday();
    abs_time = zce::timeval_add(abs_time, relative_time);
    return systime_lock(abs_time);
}

//ȡ���ڲ�������ָ��
pthread_mutex_t *ZCE_Thread_Recursive_Mutex::get_lock()
{
    return &lock_;
}

/************************************************************************************************************
Class           : ZCE_Thread_NONR_Mutex
************************************************************************************************************/
//���캯��,name���������Բ��������������ܽ��Ϳ���ֲ��
//���Խ���һ��Ϊʲô���Բ���name��������ΪWindows�����������źŵ�ģ�⣬��Windows���źŵ����̻߳����£�����Ҫһ��������
ZCE_Thread_NONR_Mutex::ZCE_Thread_NONR_Mutex ()
{
    //�߳���
    int ret = 0;

    //ע��recursive������Ϊ��false
    ret = zce::pthread_mutex_initex(&lock_,
                                    false,
                                    false,
                                    true,
                                    NULL);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }

}

//�����������ͷ�MUTEX��Դ
ZCE_Thread_NONR_Mutex::~ZCE_Thread_NONR_Mutex (void)
{

    int ret = 0;
    ret = zce::pthread_mutex_destroy (&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_destroy", ret);
        return;
    }

}

//����
void ZCE_Thread_NONR_Mutex::lock()
{

    int ret = 0;
    ret = zce::pthread_mutex_lock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_lock", ret);
        return;
    }
}

//��������
bool ZCE_Thread_NONR_Mutex::try_lock()
{
    int ret = 0;
    ret = zce::pthread_mutex_trylock(&lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//����,
void ZCE_Thread_NONR_Mutex::unlock()
{
    int ret = 0;
    ret = zce::pthread_mutex_unlock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_unlock", ret);
        return;
    }

}

//����ʱ��
bool ZCE_Thread_NONR_Mutex::systime_lock(const ZCE_Time_Value &abs_time)
{
    int ret = 0;
    ret = zce::pthread_mutex_timedlock(&lock_, abs_time);

    if (0 != ret)
    {
        //��ETIME==>ETIMEDOUT������û��ע���޸�������⣬derrickhu��æ������������⣬�ش��޸ġ�
        if (errno != ETIMEDOUT)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_timedlock", ret);
        }

        return false;
    }

    return true;

}

//���ʱ��
bool ZCE_Thread_NONR_Mutex::duration_lock(const ZCE_Time_Value &relative_time)
{
    ZCE_Time_Value abs_time(zce::gettimeofday());
    abs_time += relative_time;
    return systime_lock(abs_time);
}

