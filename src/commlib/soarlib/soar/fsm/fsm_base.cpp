#include "soar/predefine.h"

#include "soar/zerg/frame_zerg.h"
#include "soar/zerg/services_info.h"
#include "soar/stat/monitor.h"
#include "soar/stat/define.h"
#include "soar/fsm/fsm_base.h"
#include "soar/fsm/fsm_mgr.h"

namespace soar
{
//构造函数
FSM_Base::FSM_Base(FSM_Manager* pmngr,
                   uint32_t create_cmd,
                   bool trans_locker) :
    zce::Async_FSM(pmngr, create_cmd),
    trans_manager_(pmngr),
    trans_locker_(trans_locker)
{
}

//析构函数
FSM_Base::~FSM_Base()
{
}

//事务内存重置
void FSM_Base::on_init()
{
    trans_create_ = true;
    req_zerg_head_.clear();
    trans_timeout_id_ = -1;
    active_auto_stop_ = true;
    trans_touchtimer_id_ = -1;
    trans_create_time_ = 0;
    trace_log_pri_ = RS_DEBUG;
    running_errno_ = 0;

    trans_init();
}

//根据Frame初始化得到对方发送的信息
void FSM_Base::create_init(const soar::Zerg_Frame* proc_frame)
{
    proc_frame->get_head(req_zerg_head_);

    //如果有监控选项，提高日志级别，保证部分日志得到输出
    if (proc_frame->frame_option_.option_ & soar::Zerg_Frame::DESC_TRACK_MONITOR)
    {
        trace_log_pri_ = RS_INFO;
    }
    soar::Stat_Monitor::instance()->add_one(COMM_STAT_TRANS_START_SUCC,
                                            req_zerg_head_.command_,
                                            0);
    return;
}

void FSM_Base::on_run(bool& continued)
{
    const soar::Zerg_Frame* recv_frame = nullptr;
    trans_manager_->get_process_frame(recv_frame);
    //如果是第一次创建事物的时候，
    if (trans_create_)
    {
        create_init(recv_frame);
        trans_create_ = false;

        ZCE_LOG(trace_log_pri_,
                "%s::trans_run create,transaction id:[%u],trans stage:[%u],"
                "request cmd [%u] trans id:[%u].",
                typeid(*this).name(),
                asyncobj_id_,
                fsm_stage_,
                req_zerg_head_.command_,
                req_zerg_head_.fsm_id_);
    }

    ZCE_LOG(trace_log_pri_,
            "%s::trans_run start,transaction id:[%u],trans stage:[%u],"
            "recv frame cmd [%u] trans id:[%u].",
            typeid(*this).name(),
            asyncobj_id_,
            fsm_stage_,
            recv_frame->command_,
            recv_frame->fsm_id_);

    trans_run(recv_frame, continued);

    ZCE_LOG(trace_log_pri_,
            "%s::trans_run end,transaction id:[%u],trans stage:[%u],"
            "recv frame cmd [%u] trans id:[%u],continue[%s] ,error [%d].",
            typeid(*this).name(),
            asyncobj_id_,
            fsm_stage_,
            recv_frame->command_,
            recv_frame->fsm_id_,
            continued ? "TRUE" : "FALSE",
            running_errno_
    );

    if (!continued)
    {
        if (running_errno_ == 0)
        {
            // 成功退出，修改监控数据
            soar::Stat_Monitor::instance()->add_one(COMM_STAT_TRANS_END_SUCC,
                                                    req_zerg_head_.command_,
                                                    0);
        }
        else
        {
            // 失败退出，修改监控数据
            soar::Stat_Monitor::instance()->add_one(COMM_STAT_TRANS_END_FAIL,
                                                    req_zerg_head_.command_,
                                                    running_errno_);
            soar::Stat_Monitor::instance()->add_one(COMM_STAT_TRANS_END_FAIL_BY_SVCTYPE,
                                                    req_zerg_head_.send_service_.services_type_,
                                                    running_errno_);
        }
    }
}

void FSM_Base::on_timeout(const zce::Time_Value& now_time,
                          bool& continue_run)
{
    ZCE_LOG(trace_log_pri_, "%s::trans_timeout start,transaction id:[%u],trans stage:[%u],",
            typeid(*this).name(),
            asyncobj_id_,
            fsm_stage_
    );

    trans_timeout(now_time, continue_run);

    ZCE_LOG(trace_log_pri_, "%s::trans_timeout end,transaction id:[%u],trans stage:[%u],"
            "continue[%s] ,error [%d].",
            typeid(*this).name(),
            asyncobj_id_,
            fsm_stage_,
            continue_run ? "TRUE" : "FALSE",
            running_errno_);

    soar::Stat_Monitor::instance()->add_one(COMM_STAT_TRANS_PROC_TIMEOUT,
                                            req_zerg_head_.command_,
                                            0);

    if (!continue_run)
    {
        if (running_errno_ == 0)
        {
            // 成功退出，修改监控数据
            soar::Stat_Monitor::instance()->add_one(COMM_STAT_TRANS_END_SUCC,
                                                    req_zerg_head_.command_,
                                                    0);
        }
        else
        {
            // 失败退出，修改监控数据
            soar::Stat_Monitor::instance()->add_one(COMM_STAT_TRANS_END_FAIL,
                                                    req_zerg_head_.command_,
                                                    running_errno_);
            soar::Stat_Monitor::instance()->add_one(COMM_STAT_TRANS_END_FAIL_BY_SVCTYPE,
                                                    req_zerg_head_.send_service_.services_type_,
                                                    running_errno_);
        }
    }
}

//检查接受到的FRAME的数据和命令
int FSM_Base::check_receive_frame(const soar::Zerg_Frame* recv_frame,
                                  uint32_t wait_cmd)
{
    //
    if (wait_cmd != CMD_INVALID_CMD && recv_frame->command_ != wait_cmd)
    {
        ZCE_LOG(RS_ERROR, "[framework] %s:check_receive_frame error,FSM id [%u] Wait command[%u],"
                "Recieve command[%u] transaction ID:[%u].",
                typeid(*this).name(),
                asyncobj_id_,
                wait_cmd,
                recv_frame->command_,
                recv_frame->fsm_id_
        );
        return SOAR_RET::ERROR_TRANSACTION_NEED_CMD_ERROR;
    }

    //
    return 0;
}

//检测包头和包体的user_id以及发送的service_id是否一致,
int FSM_Base::check_req_userid(uint32_t user_id) const
{
    if (user_id != req_zerg_head_.user_id_)
    {
        ZCE_LOG(RS_ERROR,
                "[framework] QQuin in header , body and snd_service is diffrent.body: %u|"
                " request appframe header: %u.",
                user_id,
                req_zerg_head_.user_id_);
        return SOAR_RET::ERROR_USERID_INCERTITUDE;
    }

    return 0;
}

//关闭请求的的Service的连接
int FSM_Base::close_request_service() const
{
    ZCE_LOG(RS_INFO,
            "[framework] close_request_service() at req_command_=%u,fsm_stage_=%d,req_zerg_head_.user_id_=%u.",
            req_zerg_head_.command_,
            get_stage(),
            req_zerg_head_.user_id_);
    soar::Zerg_Head cmd_head;
    cmd_head.command_ = INNER_RSP_CLOSE_SOCKET;
    cmd_head.recv_service_ = req_zerg_head_.send_service_;
    cmd_head.proxy_service_ = req_zerg_head_.proxy_service_;
    cmd_head.send_service_ = req_zerg_head_.recv_service_;
    return trans_manager_->sendbuf_to_pipe(cmd_head, NULL, 0);
}

//对当前用户的，当前事务命令字进行加锁
int FSM_Base::lock_cmd_userid()
{
    return trans_manager_->lock_only_one(req_zerg_head_.command_,
                                         req_zerg_head_.user_id_);
}
//对当前用户的，当前事务命令字进行解锁
void FSM_Base::unlock_cmd_userid()
{
    return trans_manager_->unlock_only_one(req_zerg_head_.command_,
                                           req_zerg_head_.user_id_);
}

//!DUMP输出事务的所有信息
void FSM_Base::dump(zce::LOG_PRIORITY log_priority,
                    const char* outstr) const
{
    ZCE_LOG(log_priority, "%s.FSM ID =%u user id =%u cmd =%u stage=%u timeid=%u"
            "Request send [%u|%u] proxy [%u|%u] recv[%u|%u] request fmsid[%u]. ",
            outstr,
            asyncobj_id_,
            req_zerg_head_.user_id_,
            req_zerg_head_.command_,
            fsm_stage_,
            trans_timeout_id_,
            req_zerg_head_.send_service_.services_type_,
            req_zerg_head_.send_service_.services_id_,
            req_zerg_head_.proxy_service_.services_type_,
            req_zerg_head_.proxy_service_.services_id_,
            req_zerg_head_.recv_service_.services_type_,
            req_zerg_head_.recv_service_.services_id_,
            req_zerg_head_.fsm_id_,
            trans_touchtimer_id_);
    return;
}


}