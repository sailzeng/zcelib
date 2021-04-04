#include "soar_predefine.h"

#include "soar_fsm_notify_transbase.h"

/******************************************************************************************
class Notify_Trans_Base
******************************************************************************************/

/******************************************************************************************
class Notify_Trans_Abnormal_Base ��ĳЩ����ʱ��(�����ο���),ʱʹ�ã����Ƕ�������ǿ�ҵİ��ã�����Ҫ��
******************************************************************************************/
Notify_Trans_Abnormal_Base::Notify_Trans_Abnormal_Base(Transaction_Manager *trans_notify_mgr,
                                                       unsigned int create_cmd)
    : Notify_Trans_Base<ZCE_MT_SYNCH>(trans_notify_mgr, create_cmd)
    , abnormal_frame_(NULL)
{
    //����Ҫ��trans_notify_mgr������trans_notify_mgr�����࣬
}

Notify_Trans_Abnormal_Base::~Notify_Trans_Abnormal_Base()
{
};

/******************************************************************************************
Author          : Sail(ZENGXING)  Date Of Creation: 2008��9��22��
Function        : MallocSendQueueFrame ����ʹ���������֮ǰ��ȷ��Ҫ��ʲô��
Return          : void
Parameter List  :
Param1: size_t frame_len ��Ҫ�����FRAME�ĳ���
Description     :  ����һ���뷢�Ͷ��з���FRAME
Calls           :
Called By       :
Other           : ������������ṩ��ϣ���������FRAME�����ĵط�,��Ҫ�ı�FRAME��ͷ������,
Modify Record   : ��GetSendQueueFrame,pushbak_mgr_sendqueueһ��ʹ��
******************************************************************************************/
void Notify_Trans_Abnormal_Base::malloc_abnormalframe(size_t frame_len)
{
    //�����ظ������������
    ZCE_ASSERT (abnormal_frame_ == NULL);
    abnormal_frame_ = trans_notify_mgr_->alloc_appframe(frame_len);
    return ;
}

//ȡ��FRAME���в�������Ҫ�ı䳤�ȵ�FRAME HEAD�����ݣ���ʵ�Ƽ�ʹ������ĺ���
Zerg_App_Frame *Notify_Trans_Abnormal_Base::get_abnormal_frame()
{
    ZCE_ASSERT (abnormal_frame_ != NULL);
    return abnormal_frame_;
}

//ȡ��FRAME�����������ڲ���
char *Notify_Trans_Abnormal_Base::get_abnormal_framedata()
{
    ZCE_ASSERT (abnormal_frame_ != NULL);
    return abnormal_frame_->frame_appdata_;
}

/******************************************************************************************
Author          : Sail(ZENGXING)  Date Of Creation: 2008��12��16��
Function        : Notify_Trans_Abnormal_Base::pushbak_mgr_sendqueue
Return          : int
Parameter List  :
  Param1: unsigned int cmd     ������
  Param2: unsigned int option  ѡ��
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Notify_Trans_Abnormal_Base::pushbak_mgr_sendqueue(unsigned int cmd,
                                                      unsigned int option)
{
    int ret = 0;
    //
    unsigned int frame_len = abnormal_frame_->frame_length_;
    abnormal_frame_->init_framehead(frame_len, option, cmd);

    abnormal_frame_->frame_uid_ = req_user_id_;
    //��д�Լ�transaction_id_,��ʵ���Լ�������ID,������������ҵ��Լ�
    abnormal_frame_->transaction_id_ = asyncobj_id_;
    abnormal_frame_->backfill_trans_id_ = req_session_id_;

    SERVICES_ID proxysvc(0, 0);
    SERVICES_ID selfsvc = *(trans_notify_mgr_->self_svc_info());
    abnormal_frame_->recv_service_ = selfsvc;
    abnormal_frame_->proxy_service_ = proxysvc;
    abnormal_frame_->send_service_ = selfsvc;

    abnormal_frame_->app_id_ = req_game_app_id_;


    ret = trans_notify_mgr_->enqueue_sendqueue(abnormal_frame_, true);

    if (ret != 0)
    {
        return ret;
    }

    //���ͳɹ�����������Ѿ�����Send Queue��
    abnormal_frame_ = NULL;

    return 0;
};

//���պ�Ĵ���������Դ���ͷţ��ȵȣ�������֤�����������������ã���������������
void Notify_Trans_Abnormal_Base::finish()
{
    //��ʾsndqueue_frame_û�б�����ʹ��,����
    if (abnormal_frame_)
    {
        trans_notify_mgr_->free_appframe(abnormal_frame_);
        abnormal_frame_ = NULL;
    }

    Transaction_Base::finish();
    return;
}

