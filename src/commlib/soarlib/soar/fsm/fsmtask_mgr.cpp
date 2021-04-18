#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/fsm/fsmtask_taskbase.h"
#include "soar/fsm/fsmtask_mgr.h"


// 构造函数
FSMTask_Manger::FSMTask_Manger()
{
}

//析构函数
FSMTask_Manger::~FSMTask_Manger()
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




//激活N个线程，
int FSMTask_Manger::active_notify_task(FSMTask_TaskBase *clone_task,
                                                         size_t task_num,
                                                         size_t task_stack_size)
{
    int ret = 0;
    //这个函数只用进来一次
    ZCE_ASSERT(task_list_ == NULL);

    task_number_ = task_num;
    clone_task_ = clone_task;

    task_list_ = new FSMTask_TaskBase*[task_number_];

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
            ZCE_LOG(RS_ALERT, "[framework] Activate Thread fail.Please check system config.id [%u] Stack size [%u].",
                    static_cast<unsigned int>(i),
                    static_cast<unsigned int>(task_stack_size));
            return -1;
        }
    }

    return 0;
}



//通知所有的线程停止运行
int FSMTask_Manger::stop_notify_task()
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


//处理从接收队列取出的FRAME
int FSMTask_Manger::process_recvqueue_frame(size_t &proc_frame,size_t &create_trans)
{
    int ret = 0;
    create_trans = 0;

    //
    for (proc_frame = 0; proc_frame < MAX_ONCE_PROCESS_FRAME; ++proc_frame)
    {

        soar::Zerg_Frame *tmp_frame = NULL;
        //
        ret = recv_msg_queue_->try_dequeue(tmp_frame);

        //如果不能取出数据
        if (ret != 0)
        {
            //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
            //if ( zce::last_error() != EAGAIN )
            //{
            //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
            //  return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL;
            //}
            return 0;
        }

        DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG,"FROM RECV QUEUE FRAME:",tmp_frame);

        //是否创建一个事务，
        bool bcrtcx = false;

        //增加一步
        tmp_frame->recv_service_ = self_svc_info_.svc_id_;

        //tmp_frame  马上回收
        ret = process_appframe(tmp_frame,bcrtcx);
        //释放内存
        frame_mallocor_->free_appframe(tmp_frame);

        //
        if (ret != 0)
        {
            continue;
        }

        //创建了一个事务
        if (true == bcrtcx)
        {
            ++create_trans;
        }
    }
    //
    return 0;
}


//向发送队列放入frame
int FSMTask_Manger::enqueue_sendqueue(soar::Zerg_Frame *post_frame,bool alloc_frame)
{
    int ret = 0;
    soar::Zerg_Frame *tmp_frame = NULL;

    //如果是从池子中间取出的FRAME，就什么都不做
    if (alloc_frame)
    {
        tmp_frame = post_frame;
    }
    //如果不是，就从池子中间复制一个FRAME
    else
    {
        frame_mallocor_->clone_appframe(post_frame,tmp_frame);
    }

    //不能直接放入enqueue_timeout_，这个值会改变
    ZCE_Time_Value tv = enqueue_timeout_;
    ret = send_msg_queue_->enqueue(tmp_frame,tv);

    //返回值小于0表示失败
    if (ret < 0)
    {
        ZCE_LOG(RS_ERROR,"[framework] Post message to send queue fail.ret =%d, uid=%u cmd=%u",
                ret,tmp_frame->user_id_,tmp_frame->command_);

        // 加个监控
        Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TASK_QUEUE_SEND_FAIL,
                                                     self_svc_info_.business_id_,
                                                     0);
        return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_ENQUEUE_FAIL;
    }

    //测试时打开，
    //ZCE_LOGMSG_DEBUG(RS_DEBUG,"[framework] Send queue message_count:%u message_bytes:%u. ",
    //    send_msg_queue_->size(),
    //    send_msg_queue_->size() * sizeof(soar::Zerg_Frame *));
    return 0;
}

//从recv的消息队列中去一个数据出来，进行超时等待
int FSMTask_Manger::dequeue_recvqueue(soar::Zerg_Frame *&get_frame,ZCE_Time_Value &tv)
{
    int ret = recv_msg_queue_->dequeue(get_frame,tv);
    //返回值小于0表示失败
    if (ret < 0)
    {
        //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
        //if ( zce::last_error() != ETIME )
        //{
        //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
        //}
        return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL;
    }

    return 0;
}

//从recv的消息队列中去一个数据出来，不进行超时等待
int FSMTask_Manger::trydequeue_recvqueue(soar::Zerg_Frame *&get_frame)
{
    int ret = recv_msg_queue_->try_dequeue(get_frame);
    //返回值小于0表示失败
    if (ret < 0)
    {
        //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
        //if ( zce::last_error() != EAGAIN )
        //{
        //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
        //}
        return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL;
    }

    return 0;
}

//从send的消息队列中去一个数据出来，进行超时等待
int FSMTask_Manger::dequeue_sendqueue(soar::Zerg_Frame *&get_frame,ZCE_Time_Value &tv)
{
    int ret = 0;
    ret = send_msg_queue_->dequeue(get_frame,tv);
    if (ret < 0)
    {
        //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
        //if ( zce::last_error() != ETIME )
        //{
        //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
        //}
        return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_DEQUEUE_FAIL;
    }

    return 0;
}

//从send的消息队列中去一个数据出来，不进行超时等待
int FSMTask_Manger::trydequeue_sendqueue(soar::Zerg_Frame *&get_frame)
{
    int ret = 0;
    ret = send_msg_queue_->try_dequeue(get_frame);

    if (ret < 0)
    {
        //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
        //if ( zce::last_error() != EAGAIN )
        //{
        //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
        //}
        return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_DEQUEUE_FAIL;
    }
    return 0;
}


