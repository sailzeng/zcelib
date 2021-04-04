#include "zce_predefine.h"
#include "zce_os_adapt_thread.h"
#include "zce_thread_task.h"
#include "zce_thread_wait_mgr.h"

//����ʵ��
ZCE_Thread_Wait_Manager *ZCE_Thread_Wait_Manager::instance_ = NULL;

//���캯����
ZCE_Thread_Wait_Manager::ZCE_Thread_Wait_Manager()
{
}

//
ZCE_Thread_Wait_Manager::~ZCE_Thread_Wait_Manager()
{
}

//�����Ҫ������Ҫ�Լ��Ǽǣ�
void ZCE_Thread_Wait_Manager::record_wait_thread(ZCE_THREAD_ID wait_thr_id, int wait_group_id )
{
    MANAGE_WAIT_INFO wait_thread(wait_thr_id, wait_group_id);

    wait_thread_list_.push_back(wait_thread);
}

//�Ǽ�һ��Ҫ���еȴ�����ȴ��߳�
void ZCE_Thread_Wait_Manager::record_wait_thread(const ZCE_Thread_Task *wait_thr_task)
{
    MANAGE_WAIT_INFO wait_thread(wait_thr_task->thread_id(), wait_thr_task->group_id());
    wait_thread_list_.push_back(wait_thread);
}

//�����е��߳��˳�
void ZCE_Thread_Wait_Manager::wait_all()
{
    //ע������ÿ�ζ��Ǹ�begin
    while (wait_thread_list_.size() > 0)
    {
        MANAGE_WAIT_INFO wait_thread = *wait_thread_list_.begin();
        //�ȴ�����߳��˳�
        zce::pthread_join(wait_thread.wait_thr_id_);
        //
        wait_thread_list_.pop_front();
    }
}

//�ȴ�һ��GROUP���߳��˳�
void ZCE_Thread_Wait_Manager::wait_group(int group_id)
{
    //ע������ÿ�ζ��Ǹ�begin
    MANAGE_WAIT_THREAD_LIST::iterator iter_temp = wait_thread_list_.begin();

    while (wait_thread_list_.end() != iter_temp)
    {
        if (iter_temp->wait_group_id_ == group_id)
        {
            MANAGE_WAIT_INFO wait_thread = *iter_temp;
            //�ȴ�����߳��˳�
            zce::pthread_join(wait_thread.wait_thr_id_);

            //�ȱ���ԭ���ĵ�������ΪҪɾ���ĵ�����
            iter_temp = wait_thread_list_.erase(iter_temp);
        }
        else
        {
            //Go
            ++iter_temp;
        }
    }
}

//�õ�Ψһ�ĵ���ʵ��
ZCE_Thread_Wait_Manager *ZCE_Thread_Wait_Manager::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new ZCE_Thread_Wait_Manager();
    }

    return instance_;
}

//�������ʵ��
void ZCE_Thread_Wait_Manager::clean_instance()
{
    if (instance_)
    {
        delete instance_;
    }

    instance_ = NULL;
    return;
}
