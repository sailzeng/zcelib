/************************************************************************************************************
Copyright           : 2004-2011  Fullsail Technology (Shenzhen) Company Limited.
FileName            : zce_lock_thread_mutex.cpp
Author              : Sailzeng ZENGXING
Version             :
Date Of Creation    : 2011年9月13日
Description         : 线程的互斥量，有轻量的锁，递归锁，非递归锁，读写锁

Others              :
Function List       :
    1.  ......
Modification History:
    1.Date  :
      Author  :
      Modification  :
************************************************************************************************************/

#include "zce_predefine.h"
#include "zce_os_adapt_spin.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_rwlock.h"
#include "zce_os_adapt_semaphore.h"
#include "zce_os_adapt_error.h"
#include "zce_log_logging.h"

#include "zce_lock_thread_spin.h"

/************************************************************************************************************
Class           : ZCE_Thread_Spin_Mutex
************************************************************************************************************/

//构造函数
ZCE_Thread_Spin_Mutex::ZCE_Thread_Spin_Mutex ()
{
    int ret = 0;

    ret = zce::pthread_spin_initex(&lock_,
                                   false,
                                   NULL);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_initex", ret);
        return;
    }

}

//销毁互斥量
ZCE_Thread_Spin_Mutex::~ZCE_Thread_Spin_Mutex (void)
{
    int ret = 0;
    ret = zce::pthread_spin_destroy (&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_destroy", ret);
        return;
    }
}

//锁定
void ZCE_Thread_Spin_Mutex::lock()
{
    int ret = 0;
    ret = zce::pthread_spin_lock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_lock", ret);
        return;
    }
}

//尝试锁定
bool ZCE_Thread_Spin_Mutex::try_lock()
{
    int ret = 0;
    ret = zce::pthread_spin_trylock(&lock_);

    if (0 != ret)
    {
        return false;
    }

    return true;
}

//解锁,
void ZCE_Thread_Spin_Mutex::unlock()
{
    int ret = 0;
    ret = zce::pthread_spin_unlock(&lock_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::pthread_mutex_unlock", ret);
        return;
    }
}

//取出内部的锁的指针
pthread_spinlock_t *ZCE_Thread_Spin_Mutex::get_lock()
{
    return &lock_;
}

