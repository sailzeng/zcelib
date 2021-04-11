/******************************************************************************************
Copyright           : 2000-2004, Fullsail Technology (Shenzhen) Company Limited.
FileName            : soar_fsm_notify_trans_mgr.h
Author              : Sail(ZENGXING)//Author name here
Version             :
Date Of Creation    : 2008年9月22日
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

    //ZCE_Message_Queue_Deque底层实现用的Deque
    typedef ZCE_Message_Queue_Deque<_ZCE_SYNCH, Zerg_App_Frame *>  APPFRAME_MESSAGE_QUEUE;
    //APPFRAME的分配器
    typedef AppFrame_Mallocor_Mgr<typename _ZCE_SYNCH::MUTEX>     APPFRAME_MALLOCOR;

    //FRAME队列的水位标，考虑倒由于MessageQueue中奖存放的是指针，这个数量级别已经不少了
    static const size_t FRAME_QUEUE_WATER_MARK = 102400;

public:
    //
    enum NOTIFYTRANS_MANAGER_QUEUE
    {
        MANAGER_QUEUE_SEND,
        MANAGER_QUEUE_RECV,
    };

    //有点梦想设计多个QUEUE队列的，但是实在看不到需求，如果有以后加把，应该很容易打造，
protected:
    //发送的MSG QUEUE
    APPFRAME_MESSAGE_QUEUE          *send_msg_queue_;

    //接受的MSG QUEUE
    APPFRAME_MESSAGE_QUEUE          *recv_msg_queue_;

    //APPFRAME的内存分配池子
    APPFRAME_MALLOCOR               *frame_mallocor_;

    //TASK的数量
    size_t                           task_number_;
    //进行克隆的TASK 原件
    NotifyTrans_TaskBase            *clone_task_;
    //
    NotifyTrans_TaskBase           **task_list_;

    // push数据进队列时，如果队列满了的最多等待时间
    ZCE_Time_Value enqueue_timeout_;

public:

    /******************************************************************************************
    Author          : Sail(ZENGXING)  Date Of Creation: 2008年9月22日
    Function        : NotifyTrans_Manger
    Return          : NULL
    Parameter List  :
    Param1: size_t szregtrans     注册TRANS的个数
    Param2: size_t sztransmap     处理的TRANS的个数
    Param3: SERVICES_ID selfsvr  自己的服务器ID
    Param4: ZCE_Timer_Queue* timer_queue 翻译器
    Param5: Soar_MMAP_BusPipe* zerg_mmap_pipe 管道数据
    Param6: AppFrame_Mallocor_Mgr<typename _ZCE_SYNCH::MUTEX>* frame_mallocor FRAME分配器
    Description     : 构造函数
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

    //析构函数
    virtual ~NotifyTrans_Manger();

public:

    //初始化
    void initialize(size_t  szregtrans,
                    size_t sztransmap,
                    const SERVICES_ID &selfsvr,
                    const ZCE_Time_Value &enqueue_timeout,
                    ZCE_Timer_Queue_Base *timer_queue,
                    Soar_MMAP_BusPipe *zerg_mmap_pipe,
                    APPFRAME_MALLOCOR *frame_mallocor)
    {
        //根据最大的FRAME长度调整Manager内部的数据
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

    //处理从接收队列取出的FRAME
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

            //如果不能取出数据
            if (ret != 0 )
            {
                //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
                //if ( zce::last_error() != EAGAIN )
                //{
                //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
                //  return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL;
                //}
                return 0;
            }

            DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "FROM RECV QUEUE FRAME:", tmp_frame);

            //是否创建一个事务，
            bool bcrtcx = false;

            //增加一步
            tmp_frame->recv_service_ = self_svc_id_;

            //tmp_frame  马上回收
            ret = process_appframe(tmp_frame, bcrtcx);
            //释放内存
            frame_mallocor_->free_appframe(tmp_frame);

            //
            if (ret != 0)
            {
                continue;
            }

            //创建了一个事务
            if (true == bcrtcx)
            {
                ++create_trans;
            }
        }

        //
        return 0;
    }

    //激活线程
    int active_notify_task(NotifyTrans_TaskBase *clone_task, size_t task_num, size_t task_stack_size = 1024 * 1024);
    //停下NOTIFY TASK
    int stop_notify_task();

    //从recv的消息队列中去一个数据出来，进行超时等待
    int dequeue_recvqueue(Zerg_App_Frame *&get_frame, ZCE_Time_Value *tv)
    {
        int ret = recv_msg_queue_->dequeue(get_frame, tv);

        //返回值小于0表示失败
        if (ret < 0)
        {
            //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
            //if ( zce::last_error() != ETIME )
            //{
            //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
            //}
            return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL;
        }

        return 0;
    }

    //从recv的消息队列中去一个数据出来，不进行超时等待
    int trydequeue_recvqueue(Zerg_App_Frame *&get_frame)
    {
        int ret = recv_msg_queue_->try_dequeue(get_frame);

        //返回值小于0表示失败
        if (ret < 0)
        {
            //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
            //if ( zce::last_error() != EAGAIN )
            //{
            //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
            //}
            return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_DEQUEUE_FAIL;
        }

        return 0;
    }

    //从send的消息队列中去一个数据出来，进行超时等待
    int dequeue_sendqueue(Zerg_App_Frame *&get_frame, ZCE_Time_Value &tv)
    {
        int ret = 0;
        ret = send_msg_queue_->dequeue(get_frame, tv);

        if (ret < 0)
        {
            //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
            //if ( zce::last_error() != ETIME )
            //{
            //  ZCE_LOG(RS_DEBUG,"[framework] Recv queue dequeue fail ,ret=%u,errno =%u",ret,zce::last_error());
            //}
            return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_DEQUEUE_FAIL;
        }

        return 0;
    }

    //从send的消息队列中去一个数据出来，不进行超时等待
    int trydequeue_sendqueue(Zerg_App_Frame *&get_frame)
    {
        int ret = 0;
        ret = send_msg_queue_->try_dequeue(get_frame);

        if (ret < 0)
        {
            //下面这段代码用于调试，暂时不打开,注意TRY的错误返回EAGAIN，超时返回ETIME
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

    //聚合frame_mallocor_的功能
    //从池子分配一个APPFRAME
    Zerg_App_Frame *alloc_appframe(size_t frame_len)
    {
        return frame_mallocor_->alloc_appframe(frame_len);
    }
    //释放一个APPFRAME到池子
    void free_appframe(Zerg_App_Frame *proc_frame)
    {
        frame_mallocor_->free_appframe(proc_frame);
    }

public:

    //向SEND队列发送数据,让TASK接收
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

    //向SEND队列发送数据,让TASK接收
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

        rsp_msg->frame_userid_ = qquin;
        rsp_msg->transaction_id_ = trans_id;
        rsp_msg->recv_service_ = rcvsvc;
        rsp_msg->proxy_service_ = proxysvc;
        rsp_msg->send_service_ = sndsvc;

        //填写自己transaction_id_,其实是自己的事务ID,方便回来可以找到自己
        rsp_msg->backfill_trans_id_ = backfill_trans_id;
        rsp_msg->app_id_ = app_id;

        //拷贝发送的MSG Block
        int ret = rsp_msg->appdata_encode(Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

        if (ret != 0 )
        {
            return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
        }

        //相信这个锁不会占据主循环
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
    Author          : Sailzeng <sailzeng.cn@gmail.com>  Date Of Creation: 2008年3月17日
    Function        : enqueue_sendqueue
    Return          : int
    Parameter List  :
    Param: const Zerg_App_Frame* post_frame 发送的FRAME
    Param: bool alloc_frame                 上一个参数的FRAME是否是从POOL中间取出的
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

        //如果是从池子中间取出的FRAME，就什么都不做
        if ( alloc_frame )
        {
            tmp_frame = post_frame;
        }
        //如果不是，就从池子中间复制一个FRAME
        else
        {
            frame_mallocor_->clone_appframe(post_frame, tmp_frame);
        }

        ZCE_Time_Value tv = enqueue_timeout_;
        ret = send_msg_queue_->enqueue(tmp_frame, tv);

        //返回值小于0表示失败
        if (ret < 0)
        {
            ZCE_LOG(RS_ERROR, "[framework] Post message to send queue fail.ret =%d, uid=%u cmd=%u",
                    ret, tmp_frame->frame_userid_, tmp_frame->frame_command_);

            // 加个监控
            Soar_Stat_Monitor::instance()->increase_once(COMM_STAT_TASK_QUEUE_SEND_FAIL,
                                                         tmp_frame->app_id_,
                                                         0);
            return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_ENQUEUE_FAIL;
        }

        //测试时打开，
        //ZCE_LOGMSG_DEBUG(RS_DEBUG,"[framework] Send queue message_count:%u message_bytes:%u. ",
        //    send_msg_queue_->size(),
        //    send_msg_queue_->size() * sizeof(Zerg_App_Frame *));
        return 0;
    }

    /******************************************************************************************
    Author          : Sailzeng <sailzeng.cn@gmail.com>  Date Of Creation: 2008年3月17日
    Function        : enqueue_recvqueue
    Return          : int
    Parameter List  :
    Param1: const Zerg_App_Frame* post_frame POST发送的的FRAME数据
    Param2: ZCE_Time_Value* tv               相对时间
    Description     : 向接收数据队列中间，发送一个APPFRAME
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

        //返回值小于0表示失败
        if (ret < 0)
        {
            ZCE_LOG(RS_ERROR, "[framework] Post message to recv queue fail.ret =%d.", ret);
            return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_ENQUEUE_FAIL;
        }

        //测试时打开，
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

