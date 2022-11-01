#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/fsm/fsmtask_taskbase.h"
#include "soar/fsm/fsmtask_mgr.h"

namespace soar
{
//
fsmtask_taskbase::fsmtask_taskbase() :
    fsmtask_mgr_(nullptr),
    once_max_get_sendqueue_(DEFAULT_ONCE_MAX_GET_SENDQUEUE),
    task_run_(false),
    task_frame_buf_(nullptr)
{
    task_frame_buf_ = soar::zerg_frame::new_frame(soar::zerg_frame::MAX_LEN_OF_FRAME + 16);
    task_frame_buf_->init_head(soar::zerg_frame::MAX_LEN_OF_FRAME);
}

//
fsmtask_taskbase::~fsmtask_taskbase()
{
    if (task_frame_buf_)
    {
        soar::zerg_frame::delete_frame(task_frame_buf_);
        task_frame_buf_ = nullptr;
    }
}

//
int fsmtask_taskbase::initialize(fsmtask_manger* trans_notify_mgr,
                                 size_t once_max_get_sendqueue,
                                 soar::SERVICES_ID mgr_svc_id,
                                 soar::SERVICES_ID thread_svc_id)
{
    int ret = 0;
    fsmtask_mgr_ = trans_notify_mgr;

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
void fsmtask_taskbase::stop_task_run()
{
    task_run_ = false;
}

int fsmtask_taskbase::svc(void)
{
    ZCE_LOG(RS_INFO, "[framework] Task stop start run. thread id = %u", zce::pthread_self());

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
            soar::zerg_frame* tmp_frame = nullptr;
            //忙的时候只测试，不阻塞等待
            if (idle <= DEFAULT_IDLE_PROCESS_THRESHOLD)
            {
                ret = fsmtask_mgr_->trydequeue_sendqueue(tmp_frame);
            }
            //不忙的时候，可以让他等待在队列上
            else
            {
                zce::time_value tv(0, 1000000);
                ret = fsmtask_mgr_->dequeue_sendqueue(tmp_frame, tv);
            }

            if (ret != 0)
            {
                break;
            }

            // 一旦不忙时收到数据，idle状态改为忙
            idle = 0;
            DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG, "FROM SEND QUEUE FRAME:", tmp_frame);

            ret = taskprocess_appframe(tmp_frame);
            //回收FRAME
            fsmtask_mgr_->free_appframe(tmp_frame);

            if (ret != 0)
            {
                ZCE_LOG(RS_ERROR, "[framework] taskprocess_appframe ret =%u", ret);
            }
        }

        size_t send_frame_num = 0;
        //Task 干私活
        ret = task_moonlighting(send_frame_num);

        //控制程序的忙闲状态
        if (0 == send_frame_num && 0 == recv_frame_num)
        {
            idle++;
        }
        else
        {
            idle = 0;
        }
    }

    ZCE_LOG(RS_INFO, "[framework] Task stop run. thread id = %u", zce::pthread_self());

    return 0;
}

//
int fsmtask_taskbase::task_initialize()
{
    return 0;
}

//
int fsmtask_taskbase::task_finish()
{
    return 0;
}

int fsmtask_taskbase::task_moonlighting(size_t& send_frame_num)
{
    send_frame_num = 0;
    return 0;
}
}