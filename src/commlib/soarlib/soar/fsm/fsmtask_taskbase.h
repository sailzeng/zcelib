#pragma once

#include "soar/zerg/frame_zerg.h"
#include "soar/fsm/fsmtask_mgr.h"

class soar::Zerg_Frame;

namespace soar
{
class FSMTask_TaskBase: public ZCE_Thread_Task
{
    //给一个友缘
    friend class FSMTask_Manger;

public:
    FSMTask_TaskBase();
    virtual ~FSMTask_TaskBase();
public:

    //初始化
    int initialize(FSMTask_Manger* trans_notify_mgr,
                   size_t once_max_get_sendqueue = DEFAULT_ONCE_MAX_GET_SENDQUEUE,
                   soar::SERVICES_ID mgr_svc_id = soar::SERVICES_ID(0, 0),
                   soar::SERVICES_ID thread_svc_id = soar::SERVICES_ID(0, 0)
    );
    //停止线程运行
    void stop_task_run();

public:

    //克隆自己，用于工厂创建自己
    virtual FSMTask_TaskBase* task_clone() const = 0;

protected:

    //任务初始化的时候
    virtual int task_initialize();
    //任务初始化的时候
    virtual int task_finish();

    //task自己处理的函数，如果
    virtual int task_moonlighting(size_t& send_frame_num);

    //任务根据FRAME的处理
    virtual int taskprocess_appframe(const soar::Zerg_Frame* app_frame) = 0;

protected:

    //将数据放入管理器，
    template <class T>
    int pushbak_mgr_recvqueue(const soar::Zerg_Frame* recv_frame,
                              uint32_t cmd,
                              const T& msg,
                              uint32_t option
    )
    {
        soar::Zerg_Frame* rsp_msg = reinterpret_cast<soar::Zerg_Frame*>(task_frame_buf_);
        rsp_msg->init_head(soar::Zerg_Frame::MAX_LEN_OF_APPFRAME, option, cmd);

        rsp_msg->user_id_ = recv_frame->user_id_;

        rsp_msg->recv_service_ = recv_frame->send_service_;
        rsp_msg->proxy_service_ = recv_frame->proxy_service_;
        rsp_msg->send_service_ = recv_frame->recv_service_;

        //填写自己transaction_id_,
        rsp_msg->fsm_id_ = 0;
        rsp_msg->backfill_fsm_id_ = recv_frame->fsm_id_;

        //拷贝发送的MSG Block
        int ret = rsp_msg->appdata_encode(soar::Zerg_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

        if (ret != 0)
        {
            return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
        }

        //下面的代码是为了避免死锁，但是从道理上讲，QUEUE不存在死锁的条件，
        //所有人dequeue,enqueue都是一个很短的过程，使用完了QUEUE锁就会释放，所以传入NULL理论上也可以。
        //还是设个超时比较好，1s的时间对于服务器而言，很漫长了
        //这个地方不用try的原因是如果这儿要进行错误处理，很麻烦，
        zce::Time_Value wait_sec(1, 0);
        ret = trans_notify_mgr_->enqueue_recvqueue(rsp_msg,
                                                   &wait_sec);

        //按照我们计算的数值，理论可以无限等待，除非前面的处理能力很弱
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[framework] Wait NULL seconds to enqueue_recvqueue but fail.Recv queue is full or transaction main task process too slow to process request.");
            return ret;
        }

        DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG, "TO RECV QUEUE FRAME", rsp_msg);
        return 0;
    }

    //将数据放入管理器，
    template <class T>
    int pushbak_mgr_recvqueue(uint32_t cmd,
                              const T& msg,
                              uint32_t backfill_fsm_id,
                              uint32_t user_id = 0,
                              uint32_t option = 0
    )
    {
        soar::Zerg_Frame* rsp_msg = reinterpret_cast<soar::Zerg_Frame*>(task_frame_buf_);
        rsp_msg->init_head(soar::Zerg_Frame::MAX_LEN_OF_APPFRAME, option, cmd);

        rsp_msg->user_id_ = user_id;

        soar::SERVICES_ID proxy_svcid(0, 0);
        rsp_msg->recv_service_ = mgr_svc_id_;
        rsp_msg->proxy_service_ = proxy_svcid;
        rsp_msg->send_service_ = thread_svc_id_;

        //填写自己transaction_id_,
        rsp_msg->fsm_id_ = 0;
        rsp_msg->backfill_fsm_id_ = backfill_trans_id;
        rsp_msg->app_id_ = 0;

        //拷贝发送的MSG Block
        int ret = rsp_msg->appdata_encode(soar::Zerg_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

        if (ret != 0)
        {
            return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
        }

        //下面的代码是为了避免死锁，但是从道理上讲，QUEUE不存在死锁的条件，
        //所有人dequeue,enqueue都是一个很短的过程，使用完了QUEUE锁就会释放，所以传入NULL理论上也可以。
        //还是设个超时比较好，1s的时间对于服务器而言，很漫长了
        //这个地方不用try的原因是如果这儿要进行错误处理，很麻烦，
        zce::Time_Value wait_sec(1, 0);

        ret = trans_notify_mgr_->enqueue_recvqueue(rsp_msg,
                                                   &wait_sec);

        //按照我们计算的数值，理论可以无限等待，除非前面的处理能力很弱
        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[framework] Wait NULL seconds to enqueue_recvqueue but fail.Recv queue is full "
                    "or transaction main task process too slow to process request.");
            return ret;
        }

        DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG, "TO RECV QUEUE FRAME", rsp_msg);
        return 0;
    }

protected:
    //
    virtual int svc(void);

protected:

    //默认一次最大从MGR发送队列中取出的frame数量，不设置的过大的原因是这是多线程
    static const size_t DEFAULT_ONCE_MAX_GET_SENDQUEUE = 32;

    //进入休息状态的阈值
    static const size_t DEFAULT_IDLE_PROCESS_THRESHOLD = 32;

protected:

    //Trans 管理器
    FSMTask_Manger* trans_notify_mgr_;

    //一次最大从MGR发送队列中取出的frame数量，
    size_t              once_max_get_sendqueue_;

    //这个
    SERVICES_ID         mgr_svc_id_;

    //
    SERVICES_ID         thread_svc_id_;

    //TASK是否在运行
    bool                task_run_;

    //QQPET APPFRAME
    soar::Zerg_Frame* task_frame_buf_;
};
}
