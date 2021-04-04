#ifndef SOARING_LIB_NOTIFY_TRANSACTION_TASK_BASE_H_
#define SOARING_LIB_NOTIFY_TRANSACTION_TASK_BASE_H_

#include "soar_zerg_frame.h"
#include "soar_fsm_notify_trans_mgr.h"

class Zerg_App_Frame;
template <class _ZCE_SYNCH > class NotifyTrans_Manger;

class NotifyTrans_TaskBase : public ZCE_Thread_Task
{
    //��һ����Ե
    friend class NotifyTrans_Manger<ZCE_MT_SYNCH>;

public:
    NotifyTrans_TaskBase();
    virtual ~NotifyTrans_TaskBase();
public:

    //��ʼ��
    int initialize( NotifyTrans_Manger<ZCE_MT_SYNCH> *trans_notify_mgr,
                    size_t once_max_get_sendqueue = DEFAULT_ONCE_MAX_GET_SENDQUEUE,
                    SERVICES_ID mgr_svc_id =  SERVICES_ID(0, 0),
                    SERVICES_ID thread_svc_id = SERVICES_ID(0, 0)
                  );
    //ֹͣ�߳�����
    void stop_task_run();

public:

    //��¡�Լ������ڹ��������Լ�
    virtual NotifyTrans_TaskBase *task_clone() const = 0;

protected:

    //�����ʼ����ʱ��
    virtual int task_initialize();
    //�����ʼ����ʱ��
    virtual int task_finish();

    //task�Լ�����ĺ��������
    virtual int task_moonlighting (size_t &send_frame_num);

    //�������FRAME�Ĵ���
    virtual int taskprocess_appframe(const Zerg_App_Frame *app_frame) = 0;

protected:

    //�����ݷ����������
    template <class T>
    int pushbak_mgr_recvqueue(const Zerg_App_Frame *recv_frame,
                              unsigned int cmd,
                              const T &info,
                              unsigned int option
                             )
    {
        Zerg_App_Frame *rsp_msg = reinterpret_cast<Zerg_App_Frame *>(task_frame_buf_);
        rsp_msg->init_framehead(Zerg_App_Frame::MAX_LEN_OF_APPFRAME, option, cmd);

        rsp_msg->frame_uid_ = recv_frame->frame_uid_;

        rsp_msg->recv_service_ = recv_frame->send_service_;
        rsp_msg->proxy_service_ = recv_frame->proxy_service_;
        rsp_msg->send_service_ = recv_frame->recv_service_;

        //��д�Լ�transaction_id_,
        rsp_msg->transaction_id_ = 0;
        rsp_msg->backfill_trans_id_ = recv_frame->transaction_id_;
        rsp_msg->app_id_ = recv_frame->app_id_;

        //�������͵�MSG Block
        int ret = rsp_msg->appdata_encode(Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

        if (ret != 0 )
        {
            return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
        }

        //����Ĵ�����Ϊ�˱������������Ǵӵ����Ͻ���QUEUE������������������
        //������dequeue,enqueue����һ���̵ܶĹ��̣�ʹ������QUEUE���ͻ��ͷţ����Դ���NULL������Ҳ���ԡ�
        //���������ʱ�ȽϺã�1s��ʱ����ڷ��������ԣ���������
        //����ط�����try��ԭ����������Ҫ���д��������鷳��
        ZCE_Time_Value wait_sec(1, 0);
        ret = trans_notify_mgr_->enqueue_recvqueue(rsp_msg,
                                                   &wait_sec);

        //�������Ǽ������ֵ�����ۿ������޵ȴ�������ǰ��Ĵ�����������
        if ( ret != 0 )
        {
            ZCE_LOG(RS_ERROR, "[framework] Wait NULL seconds to enqueue_recvqueue but fail.Recv queue is full or transaction main task process too slow to process request.");
            return ret;
        }

        DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "TO RECV QUEUE FRAME", rsp_msg);
        return 0;
    }

    //�����ݷ����������
    template <class T>
    int pushbak_mgr_recvqueue(unsigned int cmd,
                              const T &info,
                              unsigned int backfill_trans_id,
                              unsigned int user_id = 0,
                              unsigned int option = 0
                             )
    {
        Zerg_App_Frame *rsp_msg = reinterpret_cast<Zerg_App_Frame *>(task_frame_buf_);
        rsp_msg->init_framehead(Zerg_App_Frame::MAX_LEN_OF_APPFRAME, option, cmd);

        rsp_msg->frame_uid_ = user_id;

        SERVICES_ID proxy_svcid(0, 0);
        rsp_msg->recv_service_ = mgr_svc_id_;
        rsp_msg->proxy_service_ = proxy_svcid;
        rsp_msg->send_service_ = thread_svc_id_;

        //��д�Լ�transaction_id_,
        rsp_msg->transaction_id_ = 0;
        rsp_msg->backfill_trans_id_ = backfill_trans_id;
        rsp_msg->app_id_ = 0;

        //�������͵�MSG Block
        int ret = rsp_msg->appdata_encode(Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

        if (ret != 0 )
        {
            return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
        }

        //����Ĵ�����Ϊ�˱������������Ǵӵ����Ͻ���QUEUE������������������
        //������dequeue,enqueue����һ���̵ܶĹ��̣�ʹ������QUEUE���ͻ��ͷţ����Դ���NULL������Ҳ���ԡ�
        //���������ʱ�ȽϺã�1s��ʱ����ڷ��������ԣ���������
        //����ط�����try��ԭ����������Ҫ���д��������鷳��
        ZCE_Time_Value wait_sec(1, 0);

        ret = trans_notify_mgr_->enqueue_recvqueue(rsp_msg,
                                                   &wait_sec);

        //�������Ǽ������ֵ�����ۿ������޵ȴ�������ǰ��Ĵ�����������
        if ( ret != 0 )
        {
            ZCE_LOG(RS_ERROR, "[framework] Wait NULL seconds to enqueue_recvqueue but fail.Recv queue is full or transaction main task process too slow to process request.");
            return ret;
        }

        DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "TO RECV QUEUE FRAME", rsp_msg );
        return 0;
    }

protected:
    //
    virtual int svc (void);

protected:

    //Ĭ��һ������MGR���Ͷ�����ȡ����frame�����������õĹ����ԭ�������Ƕ��߳�
    static const size_t DEFAULT_ONCE_MAX_GET_SENDQUEUE = 32;

    //������Ϣ״̬����ֵ
    static const size_t DEFAULT_IDLE_PROCESS_THRESHOLD = 32;

protected:

    //Trans ������
    NotifyTrans_Manger<ZCE_MT_SYNCH>      *trans_notify_mgr_;

    //һ������MGR���Ͷ�����ȡ����frame������
    size_t                                 once_max_get_sendqueue_;

    //���
    SERVICES_ID                            mgr_svc_id_;

    //
    SERVICES_ID                            thread_svc_id_;

    //TASK�Ƿ�������
    bool                                   task_run_;

    //QQPET APPFRAME
    Zerg_App_Frame                        *task_frame_buf_;

};

#endif //#ifndef SOARING_LIB_NOTIFY_TRANSACTION_TASK_BASE_H_

