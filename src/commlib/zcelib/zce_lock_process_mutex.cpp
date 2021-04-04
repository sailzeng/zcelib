#include "zce_predefine.h"
#include "zce_os_adapt_mutex.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_rwlock.h"
#include "zce_os_adapt_semaphore.h"
#include "zce_os_adapt_error.h"
#include "zce_log_logging.h"
#include "zce_time_value.h"
#include "zce_lock_process_mutex.h"

//���캯��,
//mutex_name �����������ƣ���ѡ��������WIN���ǻ����������ƣ���LINUX���ǹ����ڴ���ļ����ƣ�
//�������WIN�µķǵݹ������Ǹ��źŵƵ����ƣ�
//WINDOWS�ĺ��Ķ�������Ʊ����ڱ�ʶһ�����Ķ��󣨻��������źŵƣ�����LINUX�µ�pthread_xxxͬ��������������ڹ����ڴ����棬
//��ô���ǽ��̼�ͬ�����󣬵�Ȼ��Ҫע������PTHREAD_PROCESS_SHARED������
ZCE_Process_Mutex::ZCE_Process_Mutex (const char *mutex_name, bool recursive):
    lock_(NULL)
{

    ZCE_ASSERT( mutex_name);
    int ret = 0;

#if defined ZCE_OS_WINDOWS
    //��һ�η���Windows �¾�Ȼ����һ��
    lock_ = new pthread_mutex_t();

#elif defined ZCE_OS_LINUX
    //
    ret = posix_sharemem_.open(mutex_name, sizeof(pthread_mutex_t), false);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_ShareMem_Posix::open", ret);
        return;
    }

    lock_ = reinterpret_cast<pthread_mutex_t *>( posix_sharemem_.addr());

#endif

    ret = zce::pthread_mutex_initex(lock_,
                                    true,
                                    recursive,
                                    true,
                                    mutex_name);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }

}

//����������
ZCE_Process_Mutex::~ZCE_Process_Mutex (void)
{
    int ret = 0;
    ret = zce::pthread_mutex_destroy (lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_destroy", ret);
        return;
    }

#if defined ZCE_OS_WINDOWS

    if (lock_)
    {
        delete lock_;
        lock_ = NULL;
    }

#elif defined ZCE_OS_LINUX
    posix_sharemem_.close();
    posix_sharemem_.remove();
    lock_ = NULL;
#endif
}

//����
void ZCE_Process_Mutex::lock()
{
    int ret = 0;
    ret = zce::pthread_mutex_lock(lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_lock", ret);
        return;
    }
}

//��������
bool ZCE_Process_Mutex::try_lock()
{
    int ret = 0;
    ret = zce::pthread_mutex_trylock(lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//����,
void ZCE_Process_Mutex::unlock()
{
    int ret = 0;
    ret = zce::pthread_mutex_unlock(lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_unlock", ret);
        return;
    }
}

//����ʱ��
bool ZCE_Process_Mutex::systime_lock(const ZCE_Time_Value &abs_time)
{
    int ret = 0;
    ret = zce::pthread_mutex_timedlock(lock_, abs_time);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_timedlock", ret);
        return false;
    }

    return true;
}

//���ʱ��
bool ZCE_Process_Mutex::duration_lock(const ZCE_Time_Value &relative_time)
{
    timeval abs_time = zce::gettimeofday();
    abs_time = zce::timeval_add(abs_time, relative_time);
    return systime_lock(abs_time);
}

//ȡ���ڲ�������ָ��
pthread_mutex_t *ZCE_Process_Mutex::get_lock()
{
    return lock_;
}

