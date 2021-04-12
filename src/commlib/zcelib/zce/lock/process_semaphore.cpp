
#include "zce/predefine.h"
#include "zce/os_adapt/semaphore.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"
#include "zce/time/time_value.h"
#include "zce/lock/process_semaphore.h"

//构造函数,
ZCE_Process_Semaphore::ZCE_Process_Semaphore(unsigned int init_value,
                                             const char *sem_name):
    lock_(NULL)
{

    ZCE_ASSERT(sem_name);

    int ret = 0;

    sema_name_[0] = '\0';
    sema_name_[sizeof(sema_name_) - 1] = '\0';

    //玩有名的信号灯,名字可以考虑用unique_name函数获得

    strncpy(sema_name_, sem_name, PATH_MAX);

    lock_ = zce::sem_open(sem_name, O_CREAT, ZCE_DEFAULT_FILE_PERMS, init_value);

    if (!lock_)
    {
        ret = -1;
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::sem_open fail.", ret);
    }

}

ZCE_Process_Semaphore::~ZCE_Process_Semaphore()
{
    //没有初始化过
    if (!lock_)
    {
        return;
    }

    //如果名字长度不是0，表示是有名
    if ( '\0' != sema_name_[0] )
    {
        //释放，关闭信号灯对象，删除名字关联的文件
        zce::sem_close(lock_);
        zce::sem_unlink(sema_name_);
        lock_ = NULL;
    }
    else
    {
        zce::sem_destroy(lock_);

        //sem_destroy不会释放，
        delete lock_;
        lock_ = NULL;
    }

}

//锁定
void ZCE_Process_Semaphore::lock()
{
    //信号灯锁定
    int ret =  zce::sem_wait (lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::sem_wait", ret);
        return;
    }
}

//尝试锁定
bool ZCE_Process_Semaphore::try_lock()
{
    //信号灯锁定
    int ret =  zce::sem_trywait (lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,
void ZCE_Process_Semaphore::unlock()
{
    int ret = zce::sem_post (lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::sem_post", ret);
        return;
    }
}

//绝对时间超时的的锁定，超时后解锁
bool ZCE_Process_Semaphore::systime_lock(const ZCE_Time_Value &abs_time)
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

//相对时间的超时锁定，超时后，解锁
bool ZCE_Process_Semaphore::duration_lock(const ZCE_Time_Value &relative_time)
{
    timeval abs_time = zce::gettimeofday();
    abs_time = zce::timeval_add(abs_time, relative_time);
    return systime_lock(abs_time);
}

