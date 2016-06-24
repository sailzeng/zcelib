/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_thread_condi.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年2月1日
* @brief
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_LOCK_THREAD_CONDI_H_
#define ZCE_LIB_LOCK_THREAD_CONDI_H_

#include "zce_trace_debugging.h"
#include "zce_lock_base.h"
#include "zce_os_adapt_condi.h"

//线程的条件变量类,为了方便用了模版类，但请你直接用两个typedef
template <class MUTEX>
class ZCE_Thread_Condition  : public ZCE_Condition_Base
{
public:

    //构造函数
    ZCE_Thread_Condition ()
    {
        int ret = 0;

        ret = ZCE_LIB::pthread_cond_initex(&lock_, false);

        if (0 != ret)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_cond_initex", ret);
            return;
        }

    }

    //析构函数
    virtual ~ZCE_Thread_Condition (void)
    {
        //销毁条件变量
        int ret =  ZCE_LIB::pthread_cond_destroy(&lock_);

        if (0 != ret)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_cond_init", ret);
            return;
        }
    }

    //我根据ZCE_Thread_Light_Mutex，ZCE_Thread_Recursive_Mutex给了特化实现

    //等待
    virtual void wait (MUTEX *external_mutex);

    //绝对时间超时的的等待，超时后解锁
    virtual bool systime_wait(MUTEX *external_mutex, const ZCE_Time_Value &abs_time);

    //相对时间的超时锁定等待，超时后，解锁
    virtual bool duration_wait(MUTEX *external_mutex, const ZCE_Time_Value &relative_time);

    // 给一个等待线程发送信号 Signal one waiting thread.
    virtual void signal (void)
    {
        //
        int ret = ZCE_LIB::pthread_cond_signal(&lock_);

        if (0 != ret)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_cond_signal", ret);
            return ;
        }
    }

    ///给所有的等待线程广播信号 Signal *all* waiting threads.
    virtual void broadcast (void)
    {
        //
        int ret = ZCE_LIB::pthread_cond_broadcast(&lock_);

        if (0 != ret)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::pthread_cond_broadcast", ret);
            return;
        }
    }

protected:

    ///条件变量对象
    pthread_cond_t    lock_;

};

//你可以直接用这两个特化的类

///使用线程MUTEX
typedef ZCE_Thread_Condition<ZCE_Thread_Light_Mutex>        ZCE_Thread_Condition_Mutex;
///使用可递归的MUTEX的类
typedef ZCE_Thread_Condition<ZCE_Thread_Recursive_Mutex>    ZCE_Thread_Condition_Recursive_Mutex;

#endif //ZCE_LIB_LOCK_THREAD_CONDI_H_

