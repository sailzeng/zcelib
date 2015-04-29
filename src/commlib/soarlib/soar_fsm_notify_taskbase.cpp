#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_fsm_notify_taskbase.h"
#include "soar_fsm_notify_trans_mgr.h"
//
NotifyTrans_TaskBase::NotifyTrans_TaskBase():
    trans_notify_mgr_(NULL),
    once_max_get_sendqueue_(DEFAULT_ONCE_MAX_GET_SENDQUEUE),
    task_run_(false),
    task_frame_buf_(NULL)
{
    task_frame_buf_ = new(Zerg_App_Frame::MAX_LEN_OF_APPFRAME + 16) Zerg_App_Frame();
    task_frame_buf_->init_framehead(Zerg_App_Frame::MAX_LEN_OF_APPFRAME);
}

//
NotifyTrans_TaskBase::~NotifyTrans_TaskBase()
{
    if (task_frame_buf_ )
    {
        delete task_frame_buf_;
        task_frame_buf_ = NULL;
    }
}

//
int NotifyTrans_TaskBase::initialize(NotifyTrans_Manger<ZCE_MT_SYNCH> *trans_notify_mgr,
                                     size_t once_max_get_sendqueue,
                                     SERVICES_ID mgr_svc_id,
                                     SERVICES_ID thread_svc_id)
{
    int ret = 0;
    trans_notify_mgr_ = trans_notify_mgr;

    once_max_get_sendqueue_ = once_max_get_sendqueue;

    //记录两个svcid
    mgr_svc_id_ = mgr_svc_id;
    thread_svc_id_ = thread_svc_id;

    ret = task_initialize();

    if (ret != 0)
    {
        return ret;
    }

    return 0;
};

//
void NotifyTrans_TaskBase::stop_task_run()
{
    task_run_ = false;
}

int NotifyTrans_TaskBase::svc (void)
{
    ZCE_LOG(RS_INFO, "[framework] Task stop start run. thread id = %u", ZCE_LIB::pthread_self());

    int ret = 0;
    task_run_ = true;
    size_t idle = 0;

    //
    for (; task_run_;)
    {

        //如果已经取出成功
        size_t recv_frame_num = 0;

        for (; recv_frame_num <= once_max_get_sendqueue_; ++recv_frame_num)
        {
            Zerg_App_Frame *tmp_frame = NULL;

            //忙的时候只测试，不阻塞等待
            if (idle <= DEFAULT_IDLE_PROCESS_THRESHOLD)
            {
                ret = trans_notify_mgr_->trydequeue_sendqueue(tmp_frame);
            }
            //不忙的时候，可以让他等待在队列上
            else
            {
                ZCE_Time_Value tv(0, 1000000);
                ret = trans_notify_mgr_->dequeue_sendqueue(tmp_frame, tv);
            }

            if (ret != 0)
            {
                break;
            }

            // 一旦不忙时收到数据，idle状态改为忙
            idle = 0;
            DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "FROM SEND QUEUE FRAME:", tmp_frame);

            ret = taskprocess_appframe(tmp_frame);
            //回收FRAME
            trans_notify_mgr_->free_appframe(tmp_frame);

            if (ret != 0)
            {
                ZCE_LOG(RS_ERROR, "[framework] taskprocess_appframe ret =%u", ret);
            }
        }

        size_t send_frame_num = 0;
        //Task 干私活
        ret = task_moonlighting (send_frame_num);

        //控制程序的忙闲状态
        if (0 == send_frame_num  && 0 == recv_frame_num )
        {
            idle ++;
        }
        else
        {
            idle = 0;
        }
    }

    ZCE_LOG(RS_INFO, "[framework] Task stop run. thread id = %u", ZCE_LIB::pthread_self());

    return 0;
}

//
int NotifyTrans_TaskBase::task_initialize()
{
    return 0;
}

//
int NotifyTrans_TaskBase::task_finish()
{
    return 0;
}

int NotifyTrans_TaskBase::task_moonlighting (size_t &send_frame_num)
{
    send_frame_num = 0;
    return 0;
}

