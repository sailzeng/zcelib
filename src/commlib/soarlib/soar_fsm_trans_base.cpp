#include "soar_predefine.h"

#include "soar_zerg_frame.h"
#include "soar_services_info.h"
#include "soar_stat_monitor.h"
#include "soar_stat_define.h"
#include "soar_fsm_trans_base.h"
#include "soar_fsm_trans_mgr.h"


//���캯��
Transaction_Base::Transaction_Base(Transaction_Manager *pmngr,
                                   unsigned int create_cmd,
                                   bool trans_locker) :
    ZCE_Async_FSM(pmngr, create_cmd),
    trans_manager_(pmngr),
    trans_locker_(trans_locker),
    trans_create_(true),
    req_user_id_(0),
    req_command_(0),
    req_snd_service_(SERVICES_ID::INVALID_SERVICES_TYPE, SERVICES_ID::INVALID_SERVICES_ID),
    req_rcv_service_(SERVICES_ID::INVALID_SERVICES_TYPE, SERVICES_ID::INVALID_SERVICES_ID),
    req_proxy_service_(SERVICES_ID::INVALID_SERVICES_TYPE, SERVICES_ID::INVALID_SERVICES_ID),
    req_trans_id_(0),
    req_session_id_(0),
    req_game_app_id_(0),
    req_ip_address_(0),
    req_frame_option_(0),
    trans_timeout_id_(-1),
    active_auto_stop_(true),
    trans_touchtimer_id_(-1),
    trans_create_time_(0),
    trace_log_pri_(RS_DEBUG)
{
}

//��������
Transaction_Base::~Transaction_Base()
{
}

//�����ڴ�����
void Transaction_Base::on_start()
{
    trans_create_ = true;

    req_user_id_ = 0;
    req_command_ = 0;
    req_snd_service_.set_svcid(SERVICES_ID::INVALID_SERVICES_TYPE, SERVICES_ID::INVALID_SERVICES_ID);
    req_rcv_service_.set_svcid(SERVICES_ID::INVALID_SERVICES_TYPE, SERVICES_ID::INVALID_SERVICES_ID);
    req_proxy_service_.set_svcid(SERVICES_ID::INVALID_SERVICES_TYPE, SERVICES_ID::INVALID_SERVICES_ID);
    req_trans_id_ = 0;
    req_session_id_ = 0;
    req_game_app_id_ = 0;
    req_ip_address_ = 0;
    req_frame_option_ = 0;
    trans_timeout_id_ = -1;
    active_auto_stop_ = true;
    trans_touchtimer_id_ = -1;
    trans_create_time_ = 0;
    trace_log_pri_ = RS_DEBUG;
    running_errno_ = 0;
}




//����Frame��ʼ���õ��Է����͵���Ϣ
void Transaction_Base::create_init(Zerg_App_Frame *proc_frame)
{

    req_command_ = proc_frame->frame_command_;

    //�õ����ͷ���������Ϣ,��δ���ֻ�������ʱ������.
    req_snd_service_ = proc_frame->send_service_;
    req_rcv_service_ = proc_frame->recv_service_;
    req_proxy_service_ = proc_frame->proxy_service_;

    req_trans_id_ = proc_frame->transaction_id_;
    req_session_id_ = proc_frame->backfill_trans_id_;
    req_game_app_id_ =  proc_frame->app_id_;

    req_user_id_ = proc_frame->frame_uid_;

    req_ip_address_ = proc_frame->send_ip_address_;
    req_frame_option_ = proc_frame->frame_option_;

    //����м��ѡ������־���𣬱�֤������־�õ����
    if (proc_frame->frame_option_ & Zerg_App_Frame::DESC_MONITOR_TRACK)
    {
        trace_log_pri_ = RS_INFO;
    }

    return;
}

void Transaction_Base::on_run(void *outer_data, bool &continue_run)
{
    Zerg_App_Frame *recv_frame = (Zerg_App_Frame *)outer_data;

    //����ǵ�һ�δ��������ʱ��
    if (trans_create_)
    {
        create_init(recv_frame);
        trans_create_ = false;

        ZCE_LOG(trace_log_pri_, "%s::trans_run create,transaction id:[%u],trans stage:[%u],"
                "request cmd [%u] trans id:[%u].",
                typeid(*this).name(),
                asyncobj_id_,
                fsm_stage_,
                req_command_,
                req_trans_id_
               );
    }


    ZCE_LOG(trace_log_pri_, "%s::trans_run start,transaction id:[%u],trans stage:[%u],"
            "recv frame cmd [%u] trans id:[%u].",
            typeid(*this).name(),
            asyncobj_id_,
            fsm_stage_,
            recv_frame->frame_command_,
            recv_frame->transaction_id_
           );

    trans_run(recv_frame, continue_run);

    ZCE_LOG(trace_log_pri_, "%s::trans_run end,transaction id:[%u],trans stage:[%u],"
            "recv frame cmd [%u] trans id:[%u],continue[%s] ,error [%d].",
            typeid(*this).name(),
            asyncobj_id_,
            fsm_stage_,
            recv_frame->frame_command_,
            recv_frame->transaction_id_,
            continue_run ? "TRUE" : "FALSE",
            running_errno_
           );

    if (!continue_run)
    {
        if (running_errno_ == 0)
        {
            // �ɹ��˳����޸ļ������
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_END_SUCC,
                                                         req_game_app_id_,
                                                         req_command_);
        }
        else
        {
            // ʧ���˳����޸ļ������
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_END_FAIL,
                                                         req_game_app_id_,
                                                         req_command_);
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_PROC_ERRNO,
                                                         req_game_app_id_,
                                                         running_errno_);
        }
    }
}


void Transaction_Base::on_timeout(const ZCE_Time_Value &now_time,
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
            // �ɹ��˳����޸ļ������
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_END_SUCC,
                                                         req_game_app_id_,
                                                         req_command_);
        }
        else
        {
            // ʧ���˳����޸ļ������
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_END_FAIL,
                                                         req_game_app_id_,
                                                         req_command_);
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TRANS_PROC_ERRNO,
                                                         req_game_app_id_,
                                                         running_errno_);
        }
    }
}


//�����ܵ���FRAME�����ݺ�����
int Transaction_Base::check_receive_frame(const Zerg_App_Frame *recv_frame,
                                          unsigned int wait_cmd)
{
    //
    if (wait_cmd != CMD_INVALID_CMD && recv_frame->frame_command_ != wait_cmd)
    {
        ZCE_LOG(RS_ERROR, "[framework] %s:check_receive_frame error,Transaction id [%u] Wait command[%u],"
                "Recieve command[%u] transaction ID:[%u].",
                typeid(*this).name(),
                asyncobj_id_,
                wait_cmd,
                recv_frame->frame_command_,
                recv_frame->transaction_id_
               );
        return SOAR_RET::ERROR_TRANSACTION_NEED_CMD_ERROR;
    }

    //
    return 0;
}


//���ڼ�������IP��ַ�Ƿ����ڲ�IP��ַ,�Ƿ���0�����Ƿ��ط�0
int Transaction_Base::check_request_internal() const
{
    return (zce::is_internal(req_ip_address_)) ? 0 : -1;
}

//����ͷ�Ͱ����user_id�Լ����͵�service_id�Ƿ�һ��,
int Transaction_Base::check_req_userid(unsigned int user_id) const
{
    if (user_id != req_user_id_)
    {
        ZCE_LOG(RS_ERROR, "[framework] QQuin in header , body and snd_service is diffrent.body: %u| request appframe header: %u.",
                user_id, req_user_id_);
        return SOAR_RET::ERROR_QQUIN_INCERTITUDE;
    }

    return 0;
}

//�ر�����ĵ�Service������
int Transaction_Base::close_request_service() const
{

    ZCE_LOG(RS_INFO, "[framework] close_request_service() at req_command_=%u,fsm_stage_=%d,req_user_id_=%u.",
            req_command_,
            get_stage(),
            req_user_id_);

    return trans_manager_->mgr_sendmsghead_to_service(INNER_RSP_CLOSE_SOCKET,
                                                      req_user_id_,
                                                      req_rcv_service_,
                                                      req_proxy_service_);
}


//�Ե�ǰ�û���һ����ID���м���
int Transaction_Base::lock_qquin_key(unsigned int one_key)
{
    return trans_manager_->lock_qquin_trnas_cmd(req_user_id_, one_key, req_command_);
}
//�Ե�ǰ�û���һ����ID���н���
void Transaction_Base::unlock_qquin_key(unsigned int one_key)
{
    return trans_manager_->unlock_qquin_trans_cmd(req_user_id_, one_key);
}
//�Ե�ǰ�û��ģ���ǰ���������ֽ��м���
int Transaction_Base::lock_qquin_cmd()
{
    return trans_manager_->lock_qquin_trnas_cmd(req_user_id_, req_command_, req_command_);
}
//�Ե�ǰ�û��ģ���ǰ���������ֽ��н���
void Transaction_Base::unlock_qquin_cmd()
{
    return trans_manager_->unlock_qquin_trans_cmd(req_user_id_, req_command_);
}

//DUMP���е��������Ϣ
void Transaction_Base::dump_transa_info(std::ostringstream &strstream) const
{
    strstream << "ID:" << asyncobj_id_ << " uid:" << req_user_id_ << " Cmd:" << req_command_ << " Stage:" << std::dec << fsm_stage_ << " ";
    strstream << "ReqSndSvr:" << req_snd_service_.services_type_ << "|" << req_snd_service_.services_id_ \
              << " ReqRcvSvr:" << req_rcv_service_.services_type_ << "|" << req_rcv_service_.services_id_ \
              << " Reqproxy:" << req_proxy_service_.services_type_  << "|" << req_proxy_service_.services_id_ << " ";
    strstream << "ReqtransID:" << req_trans_id_ << " TimeoutID:" << trans_timeout_id_ << " TouchID:" << trans_touchtimer_id_ << " ";
    return;
}

//DUMP��������������Ϣ
void Transaction_Base::output_trans_info(const char *outstr ) const
{
    std::ostringstream strstream;
    dump_transa_info(strstream);
    ZCE_LOG(RS_DEBUG, "[framework] %s:%s", outstr, strstream.str().c_str());
}

int Transaction_Base::request_send_buf_to_peer(unsigned int cmd,
                                               unsigned int uid,
                                               const SERVICES_ID &rcv_svc,
                                               const unsigned char *buf,
                                               unsigned int buf_len,
                                               unsigned int app_id,
                                               unsigned int option)
{
    SERVICES_ID proxy_svc(0, 0);
    return sendbuf_to_service(cmd,
                              uid,
                              this->asyncobj_id_,
                              0,
                              rcv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              buf,
                              buf_len,
                              app_id,
                              option);
}

int Transaction_Base::request_send_buf_to_proxy(unsigned int cmd,
                                                unsigned int uid,
                                                const SERVICES_ID &proxy_svc,
                                                const SERVICES_ID &recv_svc,
                                                const unsigned char *buf,
                                                unsigned int buf_len,
                                                unsigned int app_id,
                                                unsigned int option)
{

    return sendbuf_to_service(cmd,
                              uid,
                              this->asyncobj_id_,
                              0,
                              recv_svc,
                              proxy_svc,
                              *(trans_manager_->self_svc_info()),
                              buf,
                              buf_len,
                              app_id,
                              option);
}

int Transaction_Base::sendbuf_to_service(unsigned int cmd,
                                         unsigned int uid,
                                         unsigned int transaction_id,
                                         unsigned int backfill_trans_id,
                                         const SERVICES_ID &rcv_svc,
                                         const SERVICES_ID &proxy_svc,
                                         const SERVICES_ID &snd_svc,
                                         const unsigned char *buf,
                                         size_t buf_len,
                                         unsigned int app_id,
                                         unsigned int option )
{
    // ������������Ҫ��Ҫ��أ�����Ĵ�����м��
    if (req_frame_option_ & Zerg_App_Frame::DESC_MONITOR_TRACK)
    {
        option |= Zerg_App_Frame::DESC_MONITOR_TRACK;
    }

    //���ù������ķ��ͺ���
    return trans_manager_->mgr_sendbuf_to_service(cmd,
                                                  uid,
                                                  transaction_id,
                                                  backfill_trans_id,
                                                  rcv_svc,
                                                  proxy_svc,
                                                  snd_svc,
                                                  buf,
                                                  buf_len,
                                                  app_id,
                                                  option);
}

int Transaction_Base::response_buf_sendback(unsigned int cmd,
                                            unsigned int uid,
                                            const unsigned char *buf,
                                            unsigned int buf_len,
                                            unsigned int option )
{
    // �ذ�����Ҫ�Ӽ�ر�ǰɣ�

    //����UDP���صĴ��벿��
    if (req_frame_option_ & Zerg_App_Frame::DESC_UDP_FRAME)
    {
        option |= Zerg_App_Frame::DESC_UDP_FRAME;
    }

    //
    return sendbuf_to_service(cmd,
                              uid,
                              this->asyncobj_id_,
                              this->req_trans_id_,
                              this->req_snd_service_,
                              this->req_proxy_service_,
                              this->req_rcv_service_,
                              buf,
                              buf_len,
                              this->req_game_app_id_,
                              option);
}

