#include "zce/predefine.h"
#include "zce/os_adapt/thread.h"
#include "zce/thread/thread_task.h"

namespace zce
{
/************************************************************************************************************
Class           : Thread_Task
************************************************************************************************************/
thread_task::thread_task() :
    group_id_(INVALID_GROUP_ID),
    thread_id_(0),
    thread_return_(0)
{
}

thread_task::~thread_task()
{
}

//
void thread_task::svc_run(void* args)
{
    thread_task* t = (thread_task*)args;

    // Call the Task's svc() hook method.
    int const svc_status = t->svc();
    //保存返回的结果
    t->thread_return_ = svc_status;

    zce::pthread_exit();

    return;
}

//创建一个线程
int thread_task::activate(int group_id,
                          ZCE_THREAD_ID* threadid,
                          int detachstate,
                          size_t stacksize,
                          int threadpriority)
{
    int ret = 0;
    //创建线程
    ret = zce::pthread_createex(thread_task::svc_run,
                                static_cast<void*> (this),
                                threadid,
                                detachstate,
                                stacksize,
                                threadpriority
    );

    if (0 != ret)
    {
        return ret;
    }

    group_id_ = group_id;
    thread_id_ = *threadid;
    return 0;
}

//线程结束后的返回值int 类型
int thread_task::thread_return()
{
    return thread_return_;
}

//得到group id
int thread_task::group_id() const
{
    return group_id_;
}

ZCE_THREAD_ID thread_task::thread_id() const
{
    return thread_id_;
}

//脱离绑定关系
int thread_task::detach()
{
    return zce::pthread_detach(thread_id_);
}

//
int thread_task::wait_join()
{
    return zce::pthread_join(thread_id_);
}

//需要继承的处理的函数,理论上重载这一个函数就OK
int thread_task::svc(void)
{
    return 0;
}

//让出CPU时间
int thread_task::yield()
{
    return zce::pthread_yield();
}
}