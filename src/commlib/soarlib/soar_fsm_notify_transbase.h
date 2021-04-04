
#ifndef SOARING_LIB_NOTIFY_TRANSACTION_TRANSACTION_BASE_
#define SOARING_LIB_NOTIFY_TRANSACTION_TRANSACTION_BASE_

#include "soar_fsm_trans_base.h"
#include "soar_fsm_notify_trans_mgr.h"

//�е㻳�����ǲ���ģ���̬�ˡ�TMD��
/******************************************************************************************
class Notify_Trans_Base
******************************************************************************************/
template <class _ZCE_SYNCH >
class Notify_Trans_Base : public Transaction_Base
{
protected:

    //������
    NotifyTrans_Manger<_ZCE_SYNCH>    *trans_notify_mgr_;

public:

    //���Ҫ�󴫵ݵ�Transaction_Manager *������NotifyTrans_Manger�ģ�
    //������ˡ�
    //�����һ�����͵�������ѣ����ʹ������˷��DOWNCAST�������ǳ�ª�ġ�
    //�����Ҿ����ǲ��ֹ��ܻ�����NotifyTrans_Manger�ȽϺá���virual����Ҳ����̫��,���������ܣ�������ģ�庯�����ҡ�������
    Notify_Trans_Base(Transaction_Manager *trans_notify_mgr, unsigned int create_cmd)
        : Transaction_Base(trans_notify_mgr, create_cmd)
        , trans_notify_mgr_(NULL)
    {
        //����Ҫ��trans_notify_mgr������trans_notify_mgr�����࣬
        trans_notify_mgr_ = dynamic_cast<NotifyTrans_Manger<_ZCE_SYNCH>* > (trans_notify_mgr);
        ZCE_ASSERT(trans_notify_mgr_);
    }

protected:
    //
    virtual ~Notify_Trans_Base()
    {
    };

protected:

    //
    template <class T>
    int pushbak_mgr_sendqueue(unsigned int cmd,
                              const T &info,
                              unsigned int option = 0)
    {
        SERVICES_ID proxy_svc(0, 0);
        return trans_notify_mgr_->enqueue_sendqueue(cmd,
                                                    this->req_user_id_,
                                                    this->transaction_id_,
                                                    0,
                                                    info,
                                                    this->req_game_app_id_,
                                                    option);
    };

};

//����typedef

//���߳�������Notify Trans ��
typedef Notify_Trans_Base<ZCE_MT_SYNCH> MT_NOTIFY_TRANS_BASE ;
//��������Notify Trans ��
typedef Notify_Trans_Base<ZCE_NULL_SYNCH> NULL_NOTIFY_TRANS_BASE ;

/******************************************************************************************
class Notify_Trans_Abnormal_Base ��ĳЩ����ʱ��(�����ο���),ʱʹ�ã����Ƕ�������ǿ�ҵİ��ã�����Ҫ��
��ʹ�õ�ǰ���ǣ�
1.����ǿ�ҵ����ܰ����ߣ�ϣ������һ�ο���
2.��֪����Ҫʹ�õ�Frame�ĳ���,�����Լ����й������
3.��֪���Լ�����ʲô������Ը��е����

ʹ��Notify_Trans_Abnormal_Base�Ĺ��̱�����
malloc_abnormalframe
ʹ��get_abnormal_frame��
******************************************************************************************/
class Notify_Trans_Abnormal_Base : public Notify_Trans_Base<ZCE_MT_SYNCH>
{

    //���ó�private����Ŀ�ĵģ����ǲ������á�
private:
    //��ĳЩ����ʱ��(�����ο���)��Ҫ�Լ�������SEND QUEUE���͵�FRAME
    Zerg_App_Frame                        *abnormal_frame_;

public:
    //���캯��
    Notify_Trans_Abnormal_Base(Transaction_Manager *trans_notify_mgr, unsigned int create_cmd);
protected:
    //
    virtual ~Notify_Trans_Abnormal_Base();
public:

    //����õ�һ��ĳ�����ȵ�APP Frame
    void malloc_abnormalframe(size_t frame_len);

    //ȡ��FRAME���в�������Ҫ�ı䳤�ȵ�FRAME HEAD�����ݣ���
    Zerg_App_Frame *get_abnormal_frame();
    //ȡ��FRAME�����������ڲ���
    char *get_abnormal_framedata();

    //Ͷ��
    int pushbak_mgr_sendqueue(unsigned int cmd,
                              unsigned int option);

    //�����˻���sndqueue_frame_�Ĺ���
    virtual void finish();
};

#endif //#ifndef SOARING_LIB_NOTIFY_TRANSACTION_TRANSACTION_BASE_

