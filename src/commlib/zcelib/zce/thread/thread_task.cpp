#include "zce/predefine.h"
#include "zce/os_adapt/thread.h"
#include "zce/thread/thread_task.h"

/************************************************************************************************************
Class           : ZCE_Thread_Task
************************************************************************************************************/
ZCE_Thread_Task::ZCE_Thread_Task():
    group_id_(INVALID_GROUP_ID),
    thread_id_(0),
    thread_return_(0)
{
}

ZCE_Thread_Task::~ZCE_Thread_Task()
{
}

//
void ZCE_Thread_Task::svc_run (void *args)
{

    ZCE_Thread_Task *t = (ZCE_Thread_Task *) args;

    // Call the Task's svc() hook method.
    int const svc_status = t->svc ();
    //保存返回的结果
    t->thread_return_ = svc_status;

    zce::pthread_exit();

    return;
}

//创建一个线程
int ZCE_Thread_Task::activate(int group_id,
                              ZCE_THREAD_ID *threadid,
                              int detachstate,
                              size_t stacksize,
                              int threadpriority)
{
    int ret = 0;
    //创建线程
    ret = zce::pthread_createex(ZCE_Thread_Task::svc_run,
                                static_cast<void *> (this),
                                threadid,
                                detachstate,
                                stacksize,
                                threadpriority
                               );

    if ( 0 != ret)
    {
        return ret;
    }

    group_id_ = group_id;
    thread_id_  = *threadid;
    return 0;
}

//线程结束后的返回值int 类型
int ZCE_Thread_Task::thread_return()
{
    return thread_return_;
}

//得到group id
int ZCE_Thread_Task::group_id() const
{
    return group_id_;
}

ZCE_THREAD_ID ZCE_Thread_Task::thread_id() const
{
    return thread_id_;
}

//脱离绑定关系
int ZCE_Thread_Task::detach()
{
    return zce::pthread_detach(thread_id_);
}

//
int ZCE_Thread_Task::wait_join()
{
    return zce::pthread_join(thread_id_);
}

//需要继承的处理的函数,理论上重载这一个函数就OK
int ZCE_Thread_Task::svc (void)
{
    return 0;
}

//让出CPU时间
int ZCE_Thread_Task::yield()
{
    return zce::pthread_yield();
}

