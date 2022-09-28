#include "soar/predefine.h"
#include "soar/enum/error_code.h"
#include "soar/zerg/frame_zerg.h"
#include "soar/zerg/sndrcv_base.h"

/******************************************************************************************
class sendrecv_msg_base
******************************************************************************************/
sendrecv_msg_base::sendrecv_msg_base() :
    test_frame_len_(0),
    trans_id_builder_(0),
    msg_send_frame_(NULL),
    msg_recv_frame_(NULL),
    recv_trans_id_(0)
{
}

sendrecv_msg_base::~sendrecv_msg_base()
{
    //清理已经分配的缓冲区
    if (msg_send_frame_)
    {
        soar::zerg_frame::delete_frame(msg_send_frame_);
        msg_send_frame_ = NULL;
    }

    if (msg_recv_frame_)
    {
        soar::zerg_frame::delete_frame(msg_recv_frame_);
        msg_recv_frame_ = NULL;
    }
}

void sendrecv_msg_base::set_services_id(const soar::SERVICES_ID& recv_service,
                                        const soar::SERVICES_ID& send_service,
                                        const soar::SERVICES_ID& proxy_service,
                                        size_t frame_len)

{
    msg_recv_service_ = recv_service;
    msg_send_service_ = send_service;
    msg_proxy_service_ = proxy_service;
    test_frame_len_ = frame_len;

    //new一个APPFRAME,
    msg_send_frame_ = soar::zerg_frame::new_frame(test_frame_len_);
    msg_send_frame_->init_head(static_cast<unsigned int>(test_frame_len_));

    msg_recv_frame_ = soar::zerg_frame::new_frame(test_frame_len_);
    msg_recv_frame_->init_head(static_cast<unsigned int>(test_frame_len_));
}

//取得收到的事务ID
void sendrecv_msg_base::get_recv_transid(unsigned int& trans_id)
{
    trans_id = recv_trans_id_;
}

unsigned int sendrecv_msg_base::get_send_transid()
{
    return trans_id_builder_;
}

unsigned int sendrecv_msg_base::get_backfill_transid()
{
    return backfill_trans_id_;
}

//取得测试的APPFRAME
soar::zerg_frame* sendrecv_msg_base::get_send_appframe()
{
    return msg_send_frame_;
}

//取得接收的APPFRAME
soar::zerg_frame* sendrecv_msg_base::get_recv_appframe()
{
    return msg_recv_frame_;
}