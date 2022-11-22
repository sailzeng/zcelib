/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   notify_trans_mgr.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2008年9月22日
* @brief
*
*
* @details
*
*
*
* @note       重新整理代码时，我废弃了很多一些模版变态的地方，明确是多线程的场景，不要瞎搞
*
*/

#pragma once

#include "soar/zerg/frame_zerg.h"
#include "soar/zerg/frame_malloc.h"
#include "soar/fsm/fsm_mgr.h"
#include "soar/svrd/svrd_buspipe.h"

namespace soar
{
class fsmtask_taskbase;
class fsmtask_manger : public fsm_manager
{
protected:

    //zce::msgrings_sema底层实现用的Deque
    typedef zce::msgrings_sema<soar::zerg_frame*>  APPFRAME_MESSAGE_QUEUE;
    //APPFRAME的分配器
    typedef zergframe_mallocor<typename zce::MT_SYNCH::MUTEX>     APPFRAME_MALLOCOR;

    //FRAME队列的水位标，考虑倒由于MessageQueue中奖存放的是指针，这个数量级别已经不少了
    static const size_t FRAME_QUEUE_WATER_MARK = 102400;

public:

    // 构造函数
    fsmtask_manger();
    //析构函数
    virtual ~fsmtask_manger();

public:

    /*!
    * @brief      初始化
    * @param      szregtrans      注册TRANS的个数
    * @param      sztransmap      处理的TRANS的个数
    * @param      selfsvr         自己的服务器ID
    * @param      enqueue_timeout 超时时间
    * @param      timer_queue     定时器处理器
    * @param      zerg_mmap_pipe  管道数据
    * @param      frame_mallocor
    * @note
    */
    void initialize(size_t  szregtrans,
                    size_t sztransmap,
                    const soar::SERVICES_INFO& selfsvr,
                    const zce::time_value& enqueue_timeout,
                    zce::timer_queue* timer_queue,
                    soar::svrd_buspipe* zerg_mmap_pipe,
                    APPFRAME_MALLOCOR* frame_mallocor);

    //处理从接收队列取出的FRAME
    int process_recvqueue_frame(size_t& proc_frame, size_t& create_trans);

    /*!
    * @brief      激活线程
    * @return     int
    * @param      clone_task     CLONE的线程的实例，你最好new进去，
    * @param      task_num       线程的数量
    * @param      task_stack_size 线程堆栈的大小
    * @note
    */
    int active_notify_task(fsmtask_taskbase* clone_task,
                           size_t task_num,
                           size_t task_stack_size = 1024 * 1024);
    //停下NOTIFY TASK
    int stop_notify_task();

    //从recv的消息队列中去一个数据出来，进行超时等待
    int dequeue_recvqueue(soar::zerg_frame*& get_frame,
                          zce::time_value& tv);

    //从recv的消息队列中去一个数据出来，不进行超时等待
    int trydequeue_recvqueue(soar::zerg_frame*& get_frame);

    //从send的消息队列中去一个数据出来，进行超时等待
    int dequeue_sendqueue(soar::zerg_frame*& get_frame,
                          zce::time_value& tv);

    //从send的消息队列中去一个数据出来，不进行超时等待
    int trydequeue_sendqueue(soar::zerg_frame*& get_frame);

    bool is_sendqueue_empty() const
    {
        return send_msg_queue_->empty();
    }

    //聚合frame_mallocor_的功能
    //从池子分配一个APPFRAME
    soar::zerg_frame* alloc_appframe(size_t frame_len)
    {
        return frame_mallocor_->alloc_appframe(frame_len);
    }
    //释放一个APPFRAME到池子
    void free_appframe(soar::zerg_frame* proc_frame)
    {
        frame_mallocor_->free_appframe(proc_frame);
    }

public:

    //向SEND队列发送数据,让TASK接收
    template< class T>
    int enqueue_sendqueue(
        uint32_t cmd,
        uint32_t user_id,
        uint32_t fsm_id,
        uint32_t backfill_fsm_id,
        const T& msg,
        uint32_t option = 0)
    {
        soar::SERVICES_ID proxysvc(0, 0);
        return enqueue_sendqueue(cmd,
                                 user_id,
                                 fsm_id,
                                 backfill_fsm_id,
                                 self_svc_info_,
                                 proxysvc,
                                 self_svc_info_,
                                 msg,
                                 option);
    }

    //向SEND队列发送数据,让TASK接收
    template< class T>
    int enqueue_sendqueue(uint32_t cmd,
                          uint32_t user_id,
                          uint32_t fsm_id,
                          uint32_t backfill_fsm_id,
                          const soar::SERVICES_ID& rcvsvc,
                          const soar::SERVICES_ID& proxysvc,
                          const soar::SERVICES_ID& sndsvc,
                          const T& msg,
                          uint32_t option)
    {
        soar::zerg_frame* rsp_msg = reinterpret_cast<soar::zerg_frame*>(trans_send_buffer_);
        rsp_msg->init_head(soar::zerg_frame::MAX_LEN_OF_FRAME, option, cmd);

        rsp_msg->user_id_ = user_id;
        rsp_msg->fsm_id_ = fsm_id;
        rsp_msg->recv_service_ = rcvsvc;
        rsp_msg->proxy_service_ = proxysvc;
        rsp_msg->send_service_ = sndsvc;

        //填写自己transaction_id_,其实是自己的事务ID,方便回来可以找到自己
        rsp_msg->backfill_fsm_id_ = backfill_fsm_id;

        //拷贝发送的MSG Block
        int ret = rsp_msg->appdata_encode(soar::zerg_frame::MAX_LEN_OF_DATA, msg);

        if (ret != 0)
        {
            return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
        }

        //相信这个锁不会占据主循环
        ret = enqueue_sendqueue(rsp_msg, false);
        DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG, "TO SEND QUEUE FRAME", rsp_msg);

        if (ret != 0)
        {
            ZCE_LOG(RS_ERROR, "[framework] Wait %d seconds to enqueue_sendqueue but fail. \
                       Send queue is full or task process too slow to process request.",
                    enqueue_timeout_.sec());
            return ret;
        }

        return 0;
    }

    /*!
    * @brief
    * @return     int
    * @param      post_frame   发送的FRAME
    * @param      alloc_frame  上一个参数的FRAME是否是从POOL中间取出的
    * @note
    */
    int enqueue_sendqueue(soar::zerg_frame* post_frame,
                          bool alloc_frame);

    /*!
    * @brief      向接收数据队列中间，发送一个APPFRAME
    * @return     int
    * @param      post_frame POST发送的的FRAME数据
    * @param      tv  相对时间
    * @note
    */
    int enqueue_recvqueue(const soar::zerg_frame* post_frame,
                          const zce::time_value* tv)
    {
        int ret = 0;
        soar::zerg_frame* tmp_frame = nullptr;
        frame_mallocor_->clone_appframe(post_frame, tmp_frame);
        ret = recv_msg_queue_->enqueue_wait(tmp_frame, *tv);

        //返回值小于0表示失败
        if (ret < 0)
        {
            ZCE_LOG(RS_ERROR, "[framework] Post message to recv queue fail.ret =%d.", ret);
            return SOAR_RET::ERROR_NOTIFY_RECV_QUEUE_ENQUEUE_FAIL;
        }

        //测试时打开，
        //ZCE_LOG_DEBUG(RS_DEBUG,"[framework] Recv queue message_count:%u message_bytes:%u. ",
        //  recv_msg_queue_->size(),
        //  recv_msg_queue_->size() * sizeof(soar::zerg_frame *));

        return 0;
    }

    //有点梦想设计多个QUEUE队列的，但是实在看不到需求，如果有以后加把，应该很容易打造，
protected:

    //发送的MSG QUEUE
    APPFRAME_MESSAGE_QUEUE* send_msg_queue_ = nullptr;

    //接受的MSG QUEUE
    APPFRAME_MESSAGE_QUEUE* recv_msg_queue_ = nullptr;

    //APPFRAME的内存分配池子
    APPFRAME_MALLOCOR* frame_mallocor_ = nullptr;

    //TASK的数量
    size_t  task_number_ = 0;
    //进行克隆的TASK 原件
    fsmtask_taskbase* clone_task_ = nullptr;
    //
    fsmtask_taskbase** task_list_ = nullptr;
    //thread_task 线程对象，可以比较容易的切成
    zce::thread_task** thread_list_ = nullptr;

    // push数据进队列时，如果队列满了的最多等待时间
    zce::time_value     enqueue_timeout_;
};
}
