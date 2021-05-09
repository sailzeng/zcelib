#include "zce/predefine.h"
#include "zce/os_adapt/thread.h"
#include "zce/thread/thread_task.h"
#include "zce/thread/thread_wait_mgr.h"

//单子实例
ZCE_Thread_Wait_Manager* ZCE_Thread_Wait_Manager::instance_ = NULL;

//构造函数等
ZCE_Thread_Wait_Manager::ZCE_Thread_Wait_Manager()
{
}

//
ZCE_Thread_Wait_Manager::~ZCE_Thread_Wait_Manager()
{
}

//如果需要管理处理，要自己登记，
void ZCE_Thread_Wait_Manager::record_wait_thread(ZCE_THREAD_ID wait_thr_id, int wait_group_id)
{
    MANAGE_WAIT_INFO wait_thread(wait_thr_id, wait_group_id);

    wait_thread_list_.push_back(wait_thread);
}

//登记一个要进行等待处理等待线程
void ZCE_Thread_Wait_Manager::record_wait_thread(const ZCE_Thread_Task* wait_thr_task)
{
    MANAGE_WAIT_INFO wait_thread(wait_thr_task->thread_id(), wait_thr_task->group_id());
    wait_thread_list_.push_back(wait_thread);
}

//等所有的线程退出
void ZCE_Thread_Wait_Manager::wait_all()
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
void ZCE_Thread_Wait_Manager::wait_group(int group_id)
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
ZCE_Thread_Wait_Manager* ZCE_Thread_Wait_Manager::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new ZCE_Thread_Wait_Manager();
    }

    return instance_;
}

//清除单子实例
void ZCE_Thread_Wait_Manager::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}