#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/fsm/fsmtask_taskbase.h"
#include "soar/fsm/fsmtask_mgr.h"

namespace soar
{
// 构造函数
fsmtask_manger::fsmtask_manger()
{
}

//析构函数
fsmtask_manger::~fsmtask_manger()
{
    //删除MSG QUEUE
    if (send_msg_queue_)
    {
        delete send_msg_queue_;
        send_msg_queue_ = nullptr;
    }

    //
    if (recv_msg_queue_)
    {
        delete recv_msg_queue_;
        recv_msg_queue_ = nullptr;
    }

    if (frame_mallocor_)
    {
        delete frame_mallocor_;
        frame_mallocor_ = nullptr;
    }

    //删除TASK对象
    if (task_list_)
    {
        for (size_t i = 0; i < task_number_; ++i)
        {
            delete task_list_[i];
            task_list_[i] = nullptr;
            delete thread_list_[i];
            thread_list_[i] = nullptr;
        }

        delete[]task_list_;
        task_list_ = nullptr;
        delete[]thread_list_;
        thread_list_ = nullptr;
    }
}

//初始化
void fsmtask_manger::initialize(size_t  szregtrans,
                                size_t sztransmap,
                                const soar::SERVICES_INFO& selfsvr,
                                const zce::time_value& enqueue_timeout,
                                zce::timer_queue* timer_queue,
                                soar::svrd_buspipe* zerg_mmap_pipe,
                                APPFRAME_MALLOCOR* frame_mallocor)
{
    //根据最大的FRAME长度调整Manager内部的数据
    size_t max_frame_len = frame_mallocor->get_max_framelen();
    fsm_manager::initialize(timer_queue,
                            szregtrans,
                            sztransmap,
                            selfsvr,
                            zerg_mmap_pipe,
                            static_cast<unsigned int>(max_frame_len));
    //
    frame_mallocor_ = frame_mallocor;

    enqueue_timeout_ = enqueue_timeout;
    send_msg_queue_ = new APPFRAME_MESSAGE_QUEUE(FRAME_QUEUE_WATER_MARK);
    recv_msg_queue_ = new APPFRAME_MESSAGE_QUEUE(FRAME_QUEUE_WATER_MARK);
}

//激活N个线程，
int fsmtask_manger::active_notify_task(fsmtask_taskbase* clone_task,
                                       size_t task_num,
                                       size_t task_stack_size)
{
    int ret = 0;
    //这个函数只用进来一次
    ZCE_ASSERT(task_list_ == nullptr);

    task_number_ = task_num;
    clone_task_ = clone_task;

    task_list_ = new fsmtask_taskbase * [task_number_];
    thread_list_ = new zce::thread_task * [task_number_];
    //初始化
    for (size_t i = 0; i < task_number_; ++i)
    {
        //使用一个特殊的日期做完GROUP ID
        const size_t ACTIVATE_TASK_GROUP = 2011105;

        task_list_[i] = clone_task_->task_clone();
        ret = task_list_[i]->initialize(this);
        //任何一个不成功，都返回错误
        if (ret != 0)
        {
            return ret;
        }
        thread_list_[i] = new zce::thread_task();
        thread_list_[i]->attr_init(PTHREAD_CREATE_JOINABLE,
                                   task_stack_size,
                                   ACTIVATE_TASK_GROUP);
    }

    //每个对象启动一个线程实例，这样是偷懒但是，可以很容易实现线程池子
    for (size_t i = 0; i < task_number_; ++i)
    {
        //注意下面的参数1active

        ret = thread_list_[i]->activate(&fsmtask_taskbase::task_run,
                                        task_list_[i]);
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
int fsmtask_manger::stop_notify_task()
{
    //通知所有的线程停止运行
    for (size_t i = 0; i < task_number_; ++i)
    {
        task_list_[i]->stop_task_run();
    }

    //等待所有的线程退出
    for (size_t i = 0; i < task_number_; ++i)
    {
        thread_list_[i]->wait_join();
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
int fsmtask_manger::process_recvqueue_frame(size_t& proc_frame, size_t& create_trans)
{
    int ret = 0;
    create_trans = 0;

    //
    for (proc_frame = 0; proc_frame < MAX_ONCE_PROCESS_FRAME; ++proc_frame)
    {
        soar::zerg_frame* tmp_frame = nullptr;
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

        DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG, "FROM RECV QUEUE FRAME:", tmp_frame);

        //是否创建一个事务，
        bool create_fsm = false;

        //增加一步
        tmp_frame->recv_service_ = self_svc_info_.svc_id_;

        //tmp_frame  马上回收
        ret = process_frame(tmp_frame, create_fsm);
        //释放内存
        frame_mallocor_->free_appframe(tmp_frame);

        //
        if (ret != 0)
        {
            continue;
        }

        //创建了一个事务
        if (true == create_fsm)
        {
            ++create_trans;
        }
    }
    //
    return 0;
}

//向发送队列放入frame
int fsmtask_manger::enqueue_sendqueue(soar::zerg_frame* post_frame, bool alloc_frame)
{
    int ret = 0;
    soar::zerg_frame* tmp_frame = nullptr;

    //如果是从池子中间取出的FRAME，就什么都不做
    if (alloc_frame)
    {
        tmp_frame = post_frame;
    }
    //如果不是，就从池子中间复制一个FRAME
    else
    {
        frame_mallocor_->clone_appframe(post_frame, tmp_frame);
    }

    //不能直接放入enqueue_timeout_，这个值会改变
    zce::time_value tv = enqueue_timeout_;
    ret = send_msg_queue_->enqueue_wait(tmp_frame, tv);
    auto monitor = soar::stat_monitor::instance();
    //返回值小于0表示失败
    if (ret < 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] Post message to send queue fail.ret =%d, uid=%u cmd=%u",
                ret, tmp_frame->user_id_, tmp_frame->command_);

        // 加个监控
        monitor->add_one(COMM_STAT_TASK_QUEUE_SEND_FAIL,
                         self_svc_info_.business_id_,
                         post_frame->command_);
        return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_ENQUEUE_FAIL;
    }

    //测试时打开，
    //ZCE_LOG_DEBUG(RS_DEBUG,"[framework] Send queue message_count:%u message_bytes:%u. ",
    //    send_msg_queue_->size(),
    //    send_msg_queue_->size() * sizeof(soar::zerg_frame *));
    monitor->add_one(COMM_STAT_TASK_QUEUE_SEND_SUCC,
                     self_svc_info_.business_id_,
                     post_frame->command_);
    return 0;
}

////从recv的消息队列中去一个数据出来，进行超时等待
//int fsmtask_manger::dequeue_recvqueue(soar::zerg_frame *&get_frame,zce::time_value &tv)
//{
//    int ret = recv_msg_queue_->dequeue(get_frame,tv);
//    auto monitor = soar::stat_monitor::instance();
//    //返回值小于0表示失败
//    if (ret < 0)
//    {
//        //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
//        //if ( zce::last_error() != ETIME )
//        //{
//        //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
//        //}
//        return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL;
//    }
//    soar::stat_monitor::instance()->
//        add_one(COMM_STAT_TASK_QUEUE_RECV_COUNT,
//                      self_svc_info_.business_id_,
//                      get_frame->command_);
//    return 0;
//}

////从recv的消息队列中去一个数据出来，不进行超时等待
//int fsmtask_manger::trydequeue_recvqueue(soar::zerg_frame *&get_frame)
//{
//    int ret = recv_msg_queue_->try_dequeue(get_frame);
//    //返回值小于0表示失败
//    if (ret < 0)
//    {
//        //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
//        //if ( zce::last_error() != EAGAIN )
//        //{
//        //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
//        //}
//        return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL;
//    }
//    soar::stat_monitor::instance()->
//        add_one(COMM_STAT_TASK_QUEUE_RECV_COUNT,
//                      self_svc_info_.business_id_,
//                      get_frame->command_);
//    return 0;
//}
//
////从send的消息队列中去一个数据出来，进行超时等待
//int fsmtask_manger::dequeue_sendqueue(soar::zerg_frame *&get_frame,zce::time_value &tv)
//{
//    int ret = 0;
//    ret = send_msg_queue_->dequeue(get_frame,tv);
//    if (ret < 0)
//    {
//        //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
//        //if ( zce::last_error() != ETIME )
//        //{
//        //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
//        //}
//        return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_DEQUEUE_FAIL;
//    }
//
//    return 0;
//}
//
////从send的消息队列中去一个数据出来，不进行超时等待
//int fsmtask_manger::trydequeue_sendqueue(soar::zerg_frame *&get_frame)
//{
//    int ret = 0;
//    ret = send_msg_queue_->try_dequeue(get_frame);
//    if (ret < 0)
//    {
//        //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
//        //if ( zce::last_error() != EAGAIN )
//        //{
//        //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
//        //}
//        return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_DEQUEUE_FAIL;
//    }
//    return 0;
//}
}