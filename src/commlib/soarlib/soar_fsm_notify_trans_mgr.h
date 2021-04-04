/******************************************************************************************
Copyright           : 2000-2004, Fullsail Technology (Shenzhen) Company Limited.
FileName            : soar_fsm_notify_trans_mgr.h
Author              : Sail(ZENGXING)//Author name here
Version             :
Date Of Creation    : 2008��9��22��
Description         :

Others              :
Function List       :
1.  ......
Modification History:
1.Date  :
Author  :
Modification  :
******************************************************************************************/

#ifndef SOARING_LIB_NOTIFY_TRANSACTION_MANAGER_H_
#define SOARING_LIB_NOTIFY_TRANSACTION_MANAGER_H_

#include "soar_zerg_frame.h"
#include "soar_zerg_frame_malloc.h"
#include "soar_fsm_trans_mgr.h"
#include "soar_mmap_buspipe.h"

class NotifyTrans_TaskBase;

template <class _ZCE_SYNCH >
class NotifyTrans_Manger : public Transaction_Manager
{
protected:

    //ZCE_Message_Queue_Deque�ײ�ʵ���õ�Deque
    typedef ZCE_Message_Queue_Deque<_ZCE_SYNCH, Zerg_App_Frame *>  APPFRAME_MESSAGE_QUEUE;
    //APPFRAME�ķ�����
    typedef AppFrame_Mallocor_Mgr<typename _ZCE_SYNCH::MUTEX>     APPFRAME_MALLOCOR;

    //FRAME���е�ˮλ�꣬���ǵ�����MessageQueue�н���ŵ���ָ�룬������������Ѿ�������
    static const size_t FRAME_QUEUE_WATER_MARK = 102400;

public:
    //
    enum NOTIFYTRANS_MANAGER_QUEUE
    {
        MANAGER_QUEUE_SEND,
        MANAGER_QUEUE_RECV,
    };

    //�е�������ƶ��QUEUE���еģ�����ʵ�ڿ���������������Ժ�Ӱѣ�Ӧ�ú����״��죬
protected:
    //���͵�MSG QUEUE
    APPFRAME_MESSAGE_QUEUE          *send_msg_queue_;

    //���ܵ�MSG QUEUE
    APPFRAME_MESSAGE_QUEUE          *recv_msg_queue_;

    //APPFRAME���ڴ�������
    APPFRAME_MALLOCOR               *frame_mallocor_;

    //TASK������
    size_t                           task_number_;
    //���п�¡��TASK ԭ��
    NotifyTrans_TaskBase            *clone_task_;
    //
    NotifyTrans_TaskBase           **task_list_;

    // push���ݽ�����ʱ������������˵����ȴ�ʱ��
    ZCE_Time_Value enqueue_timeout_;

public:

    /******************************************************************************************
    Author          : Sail(ZENGXING)  Date Of Creation: 2008��9��22��
    Function        : NotifyTrans_Manger
    Return          : NULL
    Parameter List  :
    Param1: size_t szregtrans     ע��TRANS�ĸ���
    Param2: size_t sztransmap     �����TRANS�ĸ���
    Param3: SERVICES_ID selfsvr  �Լ��ķ�����ID
    Param4: ZCE_Timer_Queue* timer_queue ������
    Param5: Soar_MMAP_BusPipe* zerg_mmap_pipe �ܵ�����
    Param6: AppFrame_Mallocor_Mgr<typename _ZCE_SYNCH::MUTEX>* frame_mallocor FRAME������
    Description     : ���캯��
    Calls           :
    Called By       :
    Other           :
    Modify Record   :
    ******************************************************************************************/
    NotifyTrans_Manger(size_t szregtrans,
                       size_t sztransmap,
                       SERVICES_ID selfsvr,
                       ZCE_Timer_Queue_Base *timer_queue,
                       Soar_MMAP_BusPipe *zerg_mmap_pipe,
                       AppFrame_Mallocor_Mgr<typename _ZCE_SYNCH::MUTEX> *frame_mallocor
                      ):
        send_msg_queue_(NULL),
        recv_msg_queue_(NULL),
        frame_mallocor_(NULL),
        task_number_(0),
        clone_task_(NULL),
        task_list_(NULL)
    {
        initialize(szregtrans, sztransmap, selfsvr, timer_queue, zerg_mmap_pipe, frame_mallocor);
    }

    //
    NotifyTrans_Manger():
        send_msg_queue_(NULL),
        recv_msg_queue_(NULL),
        frame_mallocor_(NULL),
        task_number_(0),
        clone_task_(NULL),
        task_list_(NULL)
    {
    }

    //��������
    virtual ~NotifyTrans_Manger();

public:

    //��ʼ��
    void initialize(size_t  szregtrans,
                    size_t sztransmap,
                    const SERVICES_ID &selfsvr,
                    const ZCE_Time_Value &enqueue_timeout,
                    ZCE_Timer_Queue_Base *timer_queue,
                    Soar_MMAP_BusPipe *zerg_mmap_pipe,
                    APPFRAME_MALLOCOR *frame_mallocor)
    {
        //��������FRAME���ȵ���Manager�ڲ�������
        size_t max_frame_len = frame_mallocor->get_max_framelen();
        Transaction_Manager::initialize(timer_queue,
                                        szregtrans,
                                        sztransmap,
                                        selfsvr,

                                        zerg_mmap_pipe,
                                        static_cast<unsigned int>(max_frame_len));
        //
        frame_mallocor_ = frame_mallocor;

        enqueue_timeout_ = enqueue_timeout;

        send_msg_queue_ = new APPFRAME_MESSAGE_QUEUE(FRAME_QUEUE_WATER_MARK);

        recv_msg_queue_ = new APPFRAME_MESSAGE_QUEUE(FRAME_QUEUE_WATER_MARK);

    }

    //����ӽ��ն���ȡ����FRAME
    int process_recvqueue_frame(size_t &proc_frame, size_t &create_trans)
    {
        int ret = 0;
        create_trans = 0;

        //
        for (proc_frame = 0; proc_frame < MAX_ONCE_PROCESS_FRAME ;  ++proc_frame)
        {

            Zerg_App_Frame *tmp_frame = NULL;
            //
            ret = recv_msg_queue_->try_dequeue(tmp_frame);

            //�������ȡ������
            if (ret != 0 )
            {
                //������δ������ڵ��ԣ���ʱ����,ע��TRY�Ĵ��󷵻�EAGAIN����ʱ����ETIME
                //if ( zce::last_error() != EAGAIN )
                //{
                //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
                //  return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL;
                //}
                return 0;
            }

            DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "FROM RECV QUEUE FRAME:", tmp_frame);

            //�Ƿ񴴽�һ������
            bool bcrtcx = false;

            //����һ��
            tmp_frame->recv_service_ = self_svc_id_;

            //tmp_frame  ���ϻ���
            ret = process_appframe(tmp_frame, bcrtcx);
            //�ͷ��ڴ�
            frame_mallocor_->free_appframe(tmp_frame);

            //
            if (ret != 0)
            {
                continue;
            }

            //������һ������
            if (true == bcrtcx)
            {
                ++create_trans;
            }
        }

        //
        return 0;
    }

    //�����߳�
    int active_notify_task(NotifyTrans_TaskBase *clone_task, size_t task_num, size_t task_stack_size = 1024 * 1024);
    //ͣ��NOTIFY TASK
    int stop_notify_task();

    //��recv����Ϣ������ȥһ�����ݳ��������г�ʱ�ȴ�
    int dequeue_recvqueue(Zerg_App_Frame *&get_frame, ZCE_Time_Value *tv)
    {
        int ret = recv_msg_queue_->dequeue(get_frame, tv);

        //����ֵС��0��ʾʧ��
        if (ret < 0)
        {
            //������δ������ڵ��ԣ���ʱ����,ע��TRY�Ĵ��󷵻�EAGAIN����ʱ����ETIME
            //if ( zce::last_error() != ETIME )
            //{
            //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
            //}
            return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL;
        }

        return 0;
    }

    //��recv����Ϣ������ȥһ�����ݳ����������г�ʱ�ȴ�
    int trydequeue_recvqueue(Zerg_App_Frame *&get_frame)
    {
        int ret = recv_msg_queue_->try_dequeue(get_frame);

        //����ֵС��0��ʾʧ��
        if (ret < 0)
        {
            //������δ������ڵ��ԣ���ʱ����,ע��TRY�Ĵ��󷵻�EAGAIN����ʱ����ETIME
            //if ( zce::last_error() != EAGAIN )
            //{
            //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
            //}
            return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL;
        }

        return 0;
    }

    //��send����Ϣ������ȥһ�����ݳ��������г�ʱ�ȴ�
    int dequeue_sendqueue(Zerg_App_Frame *&get_frame, ZCE_Time_Value &tv)
    {
        int ret = 0;
        ret = send_msg_queue_->dequeue(get_frame, tv);

        if (ret < 0)
        {
            //������δ������ڵ��ԣ���ʱ����,ע��TRY�Ĵ��󷵻�EAGAIN����ʱ����ETIME
            //if ( zce::last_error() != ETIME )
            //{
            //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
            //}
            return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_DEQUEUE_FAIL;
        }

        return 0;
    }

    //��send����Ϣ������ȥһ�����ݳ����������г�ʱ�ȴ�
    int trydequeue_sendqueue(Zerg_App_Frame *&get_frame)
    {
        int ret = 0;
        ret = send_msg_queue_->try_dequeue(get_frame);

        if (ret < 0)
        {
            //������δ������ڵ��ԣ���ʱ����,ע��TRY�Ĵ��󷵻�EAGAIN����ʱ����ETIME
            //if ( zce::last_error() != EAGAIN )
            //{
            //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
            //}
            return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_DEQUEUE_FAIL;
        }

        return 0;
    }

    bool is_sendqueue_empty() const
    {
        return send_msg_queue_->empty();
    }

    //�ۺ�frame_mallocor_�Ĺ���
    //�ӳ��ӷ���һ��APPFRAME
    Zerg_App_Frame *alloc_appframe(size_t frame_len)
    {
        return frame_mallocor_->alloc_appframe(frame_len);
    }
    //�ͷ�һ��APPFRAME������
    void free_appframe(Zerg_App_Frame *proc_frame)
    {
        frame_mallocor_->free_appframe(proc_frame);
    }

public:

    //��SEND���з�������,��TASK����
    template< class T>
    int enqueue_sendqueue(
        unsigned int cmd,
        unsigned int qquin,
        unsigned int trans_id,
        unsigned int backfill_trans_id,
        const T &info,
        unsigned int app_id = 0,
        unsigned int option = 0)
    {
        SERVICES_ID proxysvc(0, 0);
        return enqueue_sendqueue(cmd,
                                 qquin,
                                 trans_id,
                                 backfill_trans_id,
                                 self_svc_id_,
                                 proxysvc,
                                 self_svc_id_,
                                 info,
                                 app_id,
                                 option);
    }

    //��SEND���з�������,��TASK����
    template< class T>
    int enqueue_sendqueue(
        unsigned int cmd,
        unsigned int qquin,
        unsigned int trans_id,
        unsigned int backfill_trans_id,
        const SERVICES_ID &rcvsvc,
        const SERVICES_ID &proxysvc,
        const SERVICES_ID &sndsvc,
        const T &info,
        unsigned int app_id,
        unsigned int option)
    {
        Zerg_App_Frame *rsp_msg = reinterpret_cast<Zerg_App_Frame *>(trans_send_buffer_);
        rsp_msg->init_framehead(Zerg_App_Frame::MAX_LEN_OF_APPFRAME, option, cmd);

        rsp_msg->frame_uid_ = qquin;
        rsp_msg->transaction_id_ = trans_id;
        rsp_msg->recv_service_ = rcvsvc;
        rsp_msg->proxy_service_ = proxysvc;
        rsp_msg->send_service_ = sndsvc;

        //��д�Լ�transaction_id_,��ʵ���Լ�������ID,������������ҵ��Լ�
        rsp_msg->backfill_trans_id_ = backfill_trans_id;
        rsp_msg->app_id_ = app_id;

        //�������͵�MSG Block
        int ret = rsp_msg->appdata_encode(Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

        if (ret != 0 )
        {
            return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
        }

        //�������������ռ����ѭ��
        ret = enqueue_sendqueue(rsp_msg, false);
        DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "TO SEND QUEUE FRAME", rsp_msg);

        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[framework] Wait %d seconds to enqueue_sendqueue but fail. \
                       Send queue is full or task process too slow to process request.",
                    enqueue_timeout_.sec());
            return ret;
        }

        return 0;
    }

    /******************************************************************************************
    Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008��3��17��
    Function        : enqueue_sendqueue
    Return          : int
    Parameter List  :
    Param: const Zerg_App_Frame* post_frame ���͵�FRAME
    Param: bool alloc_frame                 ��һ��������FRAME�Ƿ��Ǵ�POOL�м�ȡ����
    Description     :
    Calls           :
    Called By       :
    Other           :
    Modify Record   :
    ******************************************************************************************/
    int enqueue_sendqueue(Zerg_App_Frame *post_frame, bool alloc_frame)
    {
        int ret = 0;
        Zerg_App_Frame *tmp_frame = NULL;

        //����Ǵӳ����м�ȡ����FRAME����ʲô������
        if ( alloc_frame )
        {
            tmp_frame = post_frame;
        }
        //������ǣ��ʹӳ����м临��һ��FRAME
        else
        {
            frame_mallocor_->clone_appframe(post_frame, tmp_frame);
        }

        ZCE_Time_Value tv = enqueue_timeout_;
        ret = send_msg_queue_->enqueue(tmp_frame, tv);

        //����ֵС��0��ʾʧ��
        if (ret < 0)
        {
            ZCE_LOG(RS_ERROR, "[framework] Post message to send queue fail.ret =%d, uid=%u cmd=%u",
                    ret, tmp_frame->frame_uid_, tmp_frame->frame_command_);

            // �Ӹ����
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TASK_QUEUE_SEND_FAIL,
                                                         tmp_frame->app_id_,
                                                         0);
            return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_ENQUEUE_FAIL;
        }

        //����ʱ�򿪣�
        //ZCE_LOGMSG_DEBUG(RS_DEBUG,"[framework] Send queue message_count:%u message_bytes:%u. ",
        //    send_msg_queue_->size(),
        //    send_msg_queue_->size() * sizeof(Zerg_App_Frame *));
        return 0;
    }

    /******************************************************************************************
    Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008��3��17��
    Function        : enqueue_recvqueue
    Return          : int
    Parameter List  :
    Param1: const Zerg_App_Frame* post_frame POST���͵ĵ�FRAME����
    Param2: ZCE_Time_Value* tv               ���ʱ��
    Description     : ��������ݶ����м䣬����һ��APPFRAME
    Calls           :
    Called By       :
    Other           :
    Modify Record   :
    ******************************************************************************************/
    int enqueue_recvqueue(const Zerg_App_Frame *post_frame, const ZCE_Time_Value *tv)
    {
        int ret = 0;
        Zerg_App_Frame *tmp_frame = NULL;
        frame_mallocor_->clone_appframe(post_frame, tmp_frame);
        ret = recv_msg_queue_->enqueue(tmp_frame, *tv);

        //����ֵС��0��ʾʧ��
        if (ret < 0)
        {
            ZCE_LOG(RS_ERROR, "[framework] Post message to recv queue fail.ret =%d.", ret);
            return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_ENQUEUE_FAIL;
        }

        //����ʱ�򿪣�
        //ZCE_LOGMSG_DEBUG(RS_DEBUG,"[framework] Recv queue message_count:%u message_bytes:%u. ",
        //  recv_msg_queue_->size(),
        //  recv_msg_queue_->size() * sizeof(Zerg_App_Frame *));

        return 0;
    }
};

//
typedef NotifyTrans_Manger<ZCE_MT_SYNCH> MT_NOTIFY_TRANS_MANGER ;
//

#endif //_SOARING_LIB_NOTIFY_TRANSACTION_MANAGER_H_

