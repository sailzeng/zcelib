#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_fsm_notify_taskbase.h"
#include "soar_fsm_notify_trans_mgr.h"

//��������
template <>
NotifyTrans_Manger<ZCE_NULL_SYNCH>::~NotifyTrans_Manger()
{
    //ɾ��MSG QUEUE
    if (send_msg_queue_)
    {
        delete send_msg_queue_;
        send_msg_queue_ = NULL;
    }

    //
    if (recv_msg_queue_)
    {
        delete recv_msg_queue_;
        recv_msg_queue_ = NULL;
    }
}

//��������
template <>
NotifyTrans_Manger<ZCE_MT_SYNCH>::~NotifyTrans_Manger()
{
    //ɾ��MSG QUEUE
    if (send_msg_queue_)
    {
        delete send_msg_queue_;
        send_msg_queue_ = NULL;
    }

    //
    if (recv_msg_queue_)
    {
        delete recv_msg_queue_;
        recv_msg_queue_ = NULL;
    }

    if (frame_mallocor_)
    {
        delete frame_mallocor_;
        frame_mallocor_ = NULL;
    }

    //ɾ��TASK����
    if (task_list_)
    {
        for (size_t i = 0; i < task_number_; ++i)
        {
            delete task_list_[i];
            task_list_[i] = NULL;
        }

        delete []task_list_;
        task_list_ = NULL;
    }
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008��3��5��
Function        : NotifyTrans_Manger<ACE_NULL_SYNCH>::active_notify_task
Return          : int
Parameter List  :
  Param1: NotifyTrans_TaskBase*
  Param2: size_t
Description     : ���̰߳汾����֧�����������ֱ�Ӷ��������군���군
Calls           :
Called By       :
Other           : ��(��ָ�������)����ɱ�ġ�����HW�������ֵܣ�����ũ������ѽ
Modify Record   :
******************************************************************************************/
template <>
int NotifyTrans_Manger<ZCE_NULL_SYNCH>::active_notify_task(NotifyTrans_TaskBase * /*clone_task*/,
                                                           size_t /*task_num*/,
                                                           size_t /*task_stack_size*/ )
{
    ZCE_ASSERT(false);
    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008��3��5��
Function        : NotifyTrans_Manger<ZCE_MT_SYNCH>::active_notify_task
Return          : int
Parameter List  :
  Param1: NotifyTrans_TaskBase* clone_task CLONE���̵߳�ʵ���������new��ȥ��
  Param2: size_t task_num                 �̵߳�����
Description     : ����N���̣߳�
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <>
int NotifyTrans_Manger<ZCE_MT_SYNCH>::active_notify_task(NotifyTrans_TaskBase *clone_task,
                                                         size_t task_num,
                                                         size_t task_stack_size)
{
    int ret = 0;
    //�������ֻ�ý���һ��
    ZCE_ASSERT(task_list_ == NULL);

    task_number_ = task_num;
    clone_task_ = clone_task;

    task_list_ = new NotifyTrans_TaskBase*[task_number_];

    //��ʼ��
    for (size_t i = 0; i < task_number_; ++i)
    {
        task_list_[i] = clone_task_->task_clone();
        ret = task_list_[i]->initialize(this);

        //�κ�һ�����ɹ��������ش���
        if (ret != 0)
        {
            return ret;
        }
    }

    //ÿ����������һ���߳�ʵ����������͵�����ǣ����Ժ�����ʵ���̳߳���
    for (size_t i = 0; i < task_number_; ++i)
    {
        //ʹ��һ���������������GROUP ID
        const size_t ACTIVATE_TASK_GROUP = 2011105;

        //ע������Ĳ���1active
        ZCE_THREAD_ID threadid;
        ret = task_list_[i]->activate(ACTIVATE_TASK_GROUP,
                                      &threadid,
                                      PTHREAD_CREATE_JOINABLE,
                                      task_stack_size);

        if (ret != 0)
        {
            ZCE_LOG(RS_ALERT, "[framework] Activate Thread fail.Please check system config.id [%u] Stack size [%u].",
                    static_cast<unsigned int>(i),
                    static_cast<unsigned int>(task_stack_size));
            return -1;
        }
    }

    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008��3��17��
Function        : NotifyTrans_Manger<ACE_NULL_SYNCH>::stop_notify_task
Return          : int
Parameter List  : NULL
Description     : ֪ͨTASKͣ����
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <>
int NotifyTrans_Manger<ZCE_NULL_SYNCH>::stop_notify_task()
{
    ZCE_ASSERT(false);
    //
    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008��3��17��
Function        : NotifyTrans_Manger<ZCE_MT_SYNCH>::stop_notify_task
Return          : int
Parameter List  : NULL
Description     : ֪ͨ���е��߳�ֹͣ����
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <>
int NotifyTrans_Manger<ZCE_MT_SYNCH>::stop_notify_task()
{

    //֪ͨ���е��߳�ֹͣ����
    for (size_t i = 0; i < task_number_; ++i)
    {
        task_list_[i]->stop_task_run();
    }

    //�ȴ����е��߳��˳�
    for (size_t i = 0; i < task_number_; ++i)
    {
        task_list_[i]->wait_join();
    }

    //֪ͨ�˳��̵߳�TASK�����н�������
    for (size_t i = 0; i < task_number_; ++i)
    {
        task_list_[i]->task_finish();
    }

    //
    return 0;
}

