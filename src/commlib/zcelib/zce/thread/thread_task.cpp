#include "zce/predefine.h"
#include "zce/os_adapt/thread.h"
#include "zce/thread/thread_task.h"

namespace zce
{
/************************************************************************************************************
Class           : Thread_Task
************************************************************************************************************/

//
void thread_task::svc_run(void* args)
{
    thread_task* t = (thread_task*)args;

    // Call the Task's svc() hook method.
    t->svc_fuc();
    //保存返回的结果
    //t->thread_return_ = svc_status;

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

//让出CPU时间
int thread_task::yield()
{
    return zce::pthread_yield();
}

//========================================================================================

//单子实例
thread_task_wait* thread_task_wait::instance_ = nullptr;

//构造函数等
thread_task_wait::thread_task_wait()
{
}

//
thread_task_wait::~thread_task_wait()
{
}

//如果需要管理处理，要自己登记，
void thread_task_wait::record_wait_thread(ZCE_THREAD_ID wait_thr_id,
                                          int wait_group_id)
{
    MANAGE_WAIT_INFO wait_thread(wait_thr_id, wait_group_id);

    wait_thread_list_.push_back(wait_thread);
}

//登记一个要进行等待处理等待线程
void thread_task_wait::record_wait_thread(const zce::thread_task* wait_thr_task)
{
    MANAGE_WAIT_INFO wait_thread(wait_thr_task->thread_id(),
                                 wait_thr_task->group_id());
    wait_thread_list_.push_back(wait_thread);
}

//等所有的线程退出
void thread_task_wait::wait_all()
{
    //注意下面每次都是干begin
    while (wait_thread_list_.size() > 0)
    {
        MANAGE_WAIT_INFO wait_thread = *wait_thread_list_.begin();
        //等待这个线程退出
        zce::pthread_join(wait_thread.wait_thr_id_);
        //
        wait_thread_list_.pop_front();
    }
}

//等待一个GROUP的线程退出
void thread_task_wait::wait_group(int group_id)
{
    //注意下面每次都是干begin
    MANAGE_WAIT_THREAD_LIST::iterator iter_temp = wait_thread_list_.begin();

    while (wait_thread_list_.end() != iter_temp)
    {
        if (iter_temp->wait_group_id_ == group_id)
        {
            MANAGE_WAIT_INFO wait_thread = *iter_temp;
            //等待这个线程退出
            zce::pthread_join(wait_thread.wait_thr_id_);

            //先保存原来的迭代器作为要删除的迭代器
            iter_temp = wait_thread_list_.erase(iter_temp);
        }
        else
        {
            //Go
            ++iter_temp;
        }
    }
}

//得到唯一的单子实例
thread_task_wait* thread_task_wait::instance()
{
    if (instance_ == nullptr)
    {
        instance_ = new thread_task_wait();
    }

    return instance_;
}

//清除单子实例
void thread_task_wait::clear_inst()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = nullptr;
    return;
}
}