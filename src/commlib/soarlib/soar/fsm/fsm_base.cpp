#include "soar/predefine.h"

#include "soar/zerg/frame_zerg.h"
#include "soar/zerg/services_info.h"
#include "soar/stat/monitor.h"
#include "soar/stat/define.h"
#include "soar/fsm/fsm_base.h"
#include "soar/fsm/fsm_mgr.h"


//构造函数
FSM_Base::FSM_Base(FSM_Manager *mgr,
                   unsigned int create_cmd,
                   bool trans_locker) :
    ZCE_Async_FSM(mgr, create_cmd),
    trans_manager_(mgr),
    trans_locker_(trans_locker),
    trans_create_(true)
{
}

//析构函数
FSM_Base::~FSM_Base()
{
}

//事务内存重置
void FSM_Base::on_start()
{
    trans_create_ = true;
    req_zerg_head_.clear();
    trans_timeout_id_ = -1;
    active_auto_stop_ = true;
    trans_touchtimer_id_ = -1;
    trans_create_time_ = 0;
    trace_log_pri_ = RS_DEBUG;
    running_errno_ = 0;
}




//根据Frame初始化得到对方发送的信息
void FSM_Base::create_init(soar::Zerg_Frame *proc_frame)
{
    proc_frame->get_head(req_zerg_head_);
    
    //如果有监控选项，提高日志级别，保证部分日志得到输出
    if (proc_frame->frame_option_.option_ & soar::Zerg_Frame::DESC_MONITOR_TRACK)
    {
        trace_log_pri_ = RS_INFO;
    }
    return;
}

void FSM_Base::on_run(void *outer_data, bool &continue_run)
{
    soar::Zerg_Frame *recv_frame = (soar::Zerg_Frame *)outer_data;

    //如果是第一次创建事物的时候，
    if (trans_create_)
    {
        create_init(recv_frame);
        trans_create_ = false;

        ZCE_LOG(trace_log_pri_, "%s::trans_run create,transaction id:[%u],trans stage:[%u],"
                "request cmd [%u] trans id:[%u].",
                typeid(*this).name(),
                asyncobj_id_,
                fsm_stage_,
                req_zerg_head_.command_,
                req_zerg_head_.fsm_id_
               );
    }


    ZCE_LOG(trace_log_pri_, "%s::trans_run start,transaction id:[%u],trans stage:[%u],"
            "recv frame cmd [%u] trans id:[%u].",
            typeid(*this).name(),
            asyncobj_id_,
            fsm_stage_,
            recv_frame->command_,
            recv_frame->fsm_id_
           );

    trans_run(recv_frame, continue_run);

    ZCE_LOG(trace_log_pri_, "%s::trans_run end,transaction id:[%u],trans stage:[%u],"
            "recv frame cmd [%u] trans id:[%u],continue[%s] ,error [%d].",
            typeid(*this).name(),
            asyncobj_id_,
            fsm_stage_,
            recv_frame->command_,
            recv_frame->fsm_id_,
            continue_run ? "TRUE" : "FALSE",
            running_errno_
           );

    if (!continue_run)
    {
        if (running_errno_ == 0)
        {
            // 成功退出，修改监控数据
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_END_SUCC,
                                                         trans_manager_->self_svc_info_.business_id_,
                                                         req_zerg_head_.command_);
        }
        else
        {
            // 失败退出，修改监控数据
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_END_FAIL,
                                                         trans_manager_->self_svc_info_.business_id_,
                                                         req_zerg_head_.command_);
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_PROC_ERRNO,
                                                         trans_manager_->self_svc_info_.business_id_,
                                                         running_errno_);
        }
    }
}


void FSM_Base::on_timeout(const ZCE_Time_Value &now_time,
                                  bool &continue_run)
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
            running_errno_
           );

    if (!continue_run)
    {
        if (running_errno_ == 0)
        {
            // 成功退出，修改监控数据
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_END_SUCC,
                                                         trans_manager_->self_svc_info_.business_id_,
                                                         req_zerg_head_.command_);
        }
        else
        {
            // 失败退出，修改监控数据
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_END_FAIL,
                                                         trans_manager_->self_svc_info_.business_id_,
                                                         req_zerg_head_.command_);
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_PROC_ERRNO,
                                                         trans_manager_->self_svc_info_.business_id_,
                                                         running_errno_);
        }
    }
}


//检查接受到的FRAME的数据和命令
int FSM_Base::check_receive_frame(const soar::Zerg_Frame *recv_frame,
                                          unsigned int wait_cmd)
{
    //
    if (wait_cmd != CMD_INVALID_CMD && recv_frame->command_ != wait_cmd)
    {
        ZCE_LOG(RS_ERROR, "[framework] %s:check_receive_frame error,Transaction id [%u] Wait command[%u],"
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


//用于检查请求的IP地址是否是内部IP地址,是返回0，不是返回非0
int FSM_Base::check_request_internal() const
{
    return (zce::is_internal(req_ip_address_)) ? 0 : -1;
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
        return SOAR_RET::ERROR_QQUIN_INCERTITUDE;
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

    return trans_manager_->mgr_sendmsghead_to_service(INNER_RSP_CLOSE_SOCKET,
                                                      req_zerg_head_.user_id_,
                                                      req_zerg_head_.recv_service_,
                                                      req_zerg_head_.proxy_service_);
}


//对当前用户的一个锁ID进行加锁
int FSM_Base::lock_userid_key(unsigned int one_key)
{
    return trans_manager_->lock_userid_fsm_cmd(req_zerg_head_.user_id_, 
                                                one_key,
                                                req_zerg_head_.command_);
}
//对当前用户的一个锁ID进行解锁
void FSM_Base::unlock_userid_key(unsigned int one_key)
{
    return trans_manager_->unlock_userid_fsm_cmd(req_zerg_head_.user_id_,
                                                  one_key);
}
//对当前用户的，当前事务命令字进行加锁
int FSM_Base::lock_userid_cmd()
{
    return trans_manager_->lock_userid_fsm_cmd(req_zerg_head_.user_id_,
                                                req_zerg_head_.command_,
                                                req_zerg_head_.command_);
}
//对当前用户的，当前事务命令字进行解锁
void FSM_Base::unlock_userid_cmd()
{
    return trans_manager_->unlock_userid_fsm_cmd(req_zerg_head_.user_id_, 
                                                 req_zerg_head_.command_);
}

//DUMP所有的事物的信息
void FSM_Base::dump_transa_info(std::ostringstream &strstream) const
{
    strstream << "ID:" << asyncobj_id_ << " uid:" << req_zerg_head_.user_id_ << " Cmd:" << req_zerg_head_.command_ << " Stage:" << std::dec << fsm_stage_ << " ";
    strstream << "ReqSndSvr:" << req_zerg_head_.send_service_.services_type_ << "|" << req_zerg_head_.send_service_.services_id_ \
              << " ReqRcvSvr:" << req_zerg_head_.recv_service_.services_type_ << "|" << req_zerg_head_.recv_service_.services_id_ \
              << " Reqproxy:" << req_zerg_head_.proxy_service_.services_type_  << "|" << req_zerg_head_.proxy_service_.services_id_ << " ";
    strstream << "ReqtransID:" << req_zerg_head_.fsm_id_ << " TimeoutID:" << trans_timeout_id_ << " TouchID:" << trans_touchtimer_id_ << " ";
    return;
}

//DUMP输出事务的所有信息
void FSM_Base::output_trans_info(const char *outstr ) const
{
    std::ostringstream strstream;
    dump_transa_info(strstream);
    ZCE_LOG(RS_DEBUG, "[framework] %s:%s", outstr, strstream.str().c_str());
}

int FSM_Base::request_peer_buf(uint32_t cmd,
                               uint32_t user_id,
                               const soar::SERVICES_ID &rcv_svc,
                               const unsigned char *buf,
                               size_t buf_len,
                               uint32_t option)
{
    soar::SERVICES_ID proxy_svc(0, 0);
    return sendbuf_to_service(cmd,
                              user_id,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_id()),
                              buf,
                              buf_len,
                              option);
}

int FSM_Base::request_proxy_buf(uint32_t cmd,
                                uint32_t user_id,
                                const soar::SERVICES_ID &proxy_svc,
                                const soar::SERVICES_ID &recv_svc,
                                const unsigned char *buf,
                                size_t buf_len,
                                uint32_t option)
{

    return sendbuf_to_service(cmd,
                              user_id,
                              this->asyncobj_id_,
                              0,
                              recv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_id()),
                              buf,
                              buf_len,
                              option);
}

int FSM_Base::sendbuf_to_service(uint32_t cmd,
                                 uint32_t user_id,
                                 uint32_t fsm_id,
                                 uint32_t backfill_fsm_id,
                                 const soar::SERVICES_ID &rcv_svc,
                                 const soar::SERVICES_ID &proxy_svc,
                                 const soar::SERVICES_ID &snd_svc,
                                 const unsigned char *buf,
                                 size_t buf_len,
                                 uint32_t option)
{
    // 如果请求的命令要求要监控，后面的处理进行监控
    if (req_frame_option_ & soar::Zerg_Frame::DESC_MONITOR_TRACK)
    {
        option |= soar::Zerg_Frame::DESC_MONITOR_TRACK;
    }

    //条用管理器的发送函数
    return trans_manager_->mgr_sendbuf_to_service(cmd,
                                                  user_id,
                                                  fsm_id,
                                                  backfill_fsm_id,
                                                  rcv_svc,
                                                  proxy_svc,
                                                  snd_svc,
                                                  buf,
                                                  buf_len,
                                                  option);
}

int FSM_Base::response_buf_sendback(uint32_t cmd,
                                    uint32_t user_id,
                                    const unsigned char *buf,
                                    unsigned int buf_len,
                                    uint32_t option)
{
    //合入版本和协议信息
    option = req_zerg_head_.u32_option_ & 0xFF000000;
    //
    return sendbuf_to_service(cmd,
                              user_id,
                              this->asyncobj_id_,
                              this->req_zerg_head_.fsm_id_,
                              this->req_zerg_head_.send_service_,
                              this->req_zerg_head_.proxy_service_,
                              this->req_zerg_head_.recv_service_,
                              buf,
                              buf_len,
                              option);
}

