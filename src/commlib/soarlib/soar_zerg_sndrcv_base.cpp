#include "soar_predefine.h"
#include "soar_error_code.h"
#include "soar_services_info.h"
#include "soar_zerg_frame.h"
#include "soar_zerg_sndrcv_base.h"

/******************************************************************************************
class Tibetan_SendRecv_Package
******************************************************************************************/
SendRecv_Package_Base::SendRecv_Package_Base():
    tibetan_recv_service_(0, 0),
    tibetan_send_service_(0, 0),
    tibetan_proxy_service_(0, 0),
    test_frame_len_(0),
    trans_id_builder_(0),
    tibetan_send_appframe_(NULL),
    tibetan_recv_appframe_(NULL),
    recv_trans_id_(0)
{
}

SendRecv_Package_Base::~SendRecv_Package_Base()
{
    //�����Ѿ�����Ļ�����
    if (tibetan_send_appframe_)
    {
        Zerg_App_Frame::delete_frame(tibetan_send_appframe_);
        tibetan_send_appframe_ = NULL;
    }

    if (tibetan_recv_appframe_)
    {
        Zerg_App_Frame::delete_frame(tibetan_recv_appframe_);
        tibetan_recv_appframe_ = NULL;
    }
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008��4��25��
Function        : Tibetan_SendRecv_Package::set_services_id
Return          : void
Parameter List  :
  Param1: const SERVICES_ID& recv_service  ���յķ�����ID
  Param2: const SERVICES_ID& send_service  ���͵ķ�����ID
  Param3: const SERVICES_ID& proxy_service PROXY�ķ�����ID
  Param4: size_t frame_len ׼����FRAME����
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
void SendRecv_Package_Base::set_services_id(const SERVICES_ID &recv_service,
                                            const SERVICES_ID &send_service,
                                            const SERVICES_ID &proxy_service,
                                            size_t frame_len)

{
    tibetan_recv_service_  = recv_service;
    tibetan_send_service_ = send_service;
    tibetan_proxy_service_ = proxy_service;
    test_frame_len_ = frame_len;

    //newһ��APPFRAME,
    tibetan_send_appframe_ = Zerg_App_Frame::new_frame(test_frame_len_);
    tibetan_send_appframe_->init_framehead(static_cast<unsigned int>(test_frame_len_));

    tibetan_recv_appframe_ = Zerg_App_Frame::new_frame(test_frame_len_);
    tibetan_recv_appframe_->init_framehead(static_cast<unsigned int>(test_frame_len_));

}

//ȡ���յ�������ID
void SendRecv_Package_Base::get_recv_transid(unsigned int &trans_id)
{
    trans_id = recv_trans_id_;
}

unsigned int SendRecv_Package_Base::get_send_transid()
{
    return trans_id_builder_;

}

unsigned int SendRecv_Package_Base::get_backfill_transid()
{
    return backfill_trans_id_;
}

//ȡ�ò��Ե�APPFRAME
Zerg_App_Frame *SendRecv_Package_Base::get_send_appframe()
{
    return tibetan_send_appframe_;
}

//ȡ�ý��յ�APPFRAME
Zerg_App_Frame *SendRecv_Package_Base::get_recv_appframe()
{
    return tibetan_recv_appframe_;
}
