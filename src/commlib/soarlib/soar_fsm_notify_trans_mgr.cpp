#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_fsm_notify_taskbase.h"
#include "soar_fsm_notify_trans_mgr.h"

//析构函数
template <>
NotifyTrans_Manger<ZCE_NULL_SYNCH>::~NotifyTrans_Manger()
{
    //删除MSG QUEUE
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

//析构函数
template <>
NotifyTrans_Manger<ZCE_MT_SYNCH>::~NotifyTrans_Manger()
{
    //删除MSG QUEUE
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

    //删除TASK对象
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
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月5日
Function        : NotifyTrans_Manger<ACE_NULL_SYNCH>::active_notify_task
Return          : int
Parameter List  :
  Param1: NotifyTrans_TaskBase*
  Param2: size_t
Description     : 单线程版本，不支持这个函数，直接断言让你完蛋，完蛋
Calls           :
Called By       :
Other           : 我(是指这个函数)会自杀的。悼念HW的两个兄弟，当码农不容易呀
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
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月5日
Function        : NotifyTrans_Manger<ZCE_MT_SYNCH>::active_notify_task
Return          : int
Parameter List  :
  Param1: NotifyTrans_TaskBase* clone_task CLONE的线程的实例，你最好new进去，
  Param2: size_t task_num                 线程的数量
Description     : 激活N个线程，
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
    //这个函数只用进来一次
    ZCE_ASSERT(task_list_ == NULL);

    task_number_ = task_num;
    clone_task_ = clone_task;

    task_list_ = new NotifyTrans_TaskBase*[task_number_];

    //初始化
    for (size_t i = 0; i < task_number_; ++i)
    {
        task_list_[i] = clone_task_->task_clone();
        ret = task_list_[i]->initialize(this);

        //任何一个不成功，都返回错误
        if (ret != 0)
        {
            return ret;
        }
    }

    //每个对象启动一个线程实例，这样是偷懒但是，可以很容易实现线程池子
    for (size_t i = 0; i < task_number_; ++i)
    {
        //使用一个特殊的日期做完GROUP ID
        const size_t ACTIVATE_TASK_GROUP = 2011105;

        //注意下面的参数1active
        ZCE_THREAD_ID threadid;
        ret = task_list_[i]->activate(ACTIVATE_TASK_GROUP,
                                      &threadid,
                                      PTHREAD_CREATE_JOINABLE,
                                      task_stack_size);

        if (ret != 0)
        {
            ZLOG_ALERT("[framework] Activate Thread fail.Please check system config.id [%u] Stack size [%u].",
                       static_cast<unsigned int>(i),
                       static_cast<unsigned int>(task_stack_size));
            return -1;
        }
    }

    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月17日
Function        : NotifyTrans_Manger<ACE_NULL_SYNCH>::stop_notify_task
Return          : int
Parameter List  : NULL
Description     : 通知TASK停下来
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
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年3月17日
Function        : NotifyTrans_Manger<ZCE_MT_SYNCH>::stop_notify_task
Return          : int
Parameter List  : NULL
Description     : 通知所有的线程停止运行
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
template <>
int NotifyTrans_Manger<ZCE_MT_SYNCH>::stop_notify_task()
{

    //通知所有的线程停止运行
    for (size_t i = 0; i < task_number_; ++i)
    {
        task_list_[i]->stop_task_run();
    }

    //等待所有的线程退出
    for (size_t i = 0; i < task_number_; ++i)
    {
        task_list_[i]->wait_join();
    }

    //通知退出线程的TASK，进行结束处理
    for (size_t i = 0; i < task_number_; ++i)
    {
        task_list_[i]->task_finish();
    }

    //
    return 0;
}

