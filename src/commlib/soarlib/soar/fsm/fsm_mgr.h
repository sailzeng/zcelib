#ifndef SOARING_LIB_TRANSACTION_MANAGER_H_
#define SOARING_LIB_TRANSACTION_MANAGER_H_

#include "soar/zerg/frame_malloc.h"
#include "soar/svrd/mmap_buspipe.h"

//是否按照zengyu所说的将所有的IO接口继承处理???
//我心中充满了无数的问号
//如果就是这个咚咚最后是一个框架，我几乎认为认为这个事务模型不用修改.但是.....

//简单，但是不要太简单.....
//感觉我的想法是比较简单的,表面看我没有事件队列,其实我用了Reactor进行管理,其实内部实际是由队列的.



/************************************************************************************
2008年12月26日,圣诞的后面一天，这一周简直是浑天黑地,不过还好是周末了
身为一道彩虹，雨过就要闪亮整片天空，让我深爱的你感到光荣
从新审视这段代码，Susantian建议加入一个消息队列,她的要求很合理，有一个队列就
提供一种在内部异步调用事务的方法,
原来是有一个队列的，但是，但是，我还是要说说历史，这段代码的光辉岁月
最早我们的代码是用ACE的Notify机制的队列的，
组最早的事务处理模型
RECV PIPE===============>ACE Notify 的队列==================>
                                                             事务管理器处理
SEND PIPE<===================================================

但是在发现ACE Notify机制有一定问题后我们改为了不要队列。具体见《ACE陷阱》
取消ACE_NOtify机制后的模型,
RECV PIPE==========================================>
                                                    事务管理器处理
SEND PIPE<==========================================

而后我们为了TASK处理，我们改了一个新的模式
对于多TASK的事务处理模型:
RECV PIPE =====================>          ==============> Send队列(对事务是Send)
                                 事务处理                                  多TASK处理
SEND PIPE <=====================          <============= Recv队列(对事务是Recv)

为什么还是希望在PIPE的处理机制之外，再加入一条新的队列处理消息吗?
因为如果所有的处理都是同步调用(函数立即返回的方式),就少了很多变化，而且一些触发机制不能使用.
所以我希望实现的新的模型是
RECV PIPE==================================>
                                           事务管理器处理  <==============> 消息队列
SEND PIPE<=================================

************************************************************************************/
class Soar_MMAP_BusPipe;
class FSM_Base;
class soar::Zerg_Frame;



/******************************************************************************************
class Transaction_Manager
******************************************************************************************/
class  FSM_Manager: public ZCE_Async_FSMMgr
{
    //声明友元
    friend class FSM_Base;

protected:


    ///内部的APPFRAME的消息队列，
    typedef ZCE_Message_Queue_Deque<ZCE_NULL_SYNCH,soar::Zerg_Frame *> Inner_Frame_Queue;
    ///内部的APPFRAME的分配器，只在Mgr内部使用，单线程，用于给内部提供一些异步化的处理
    typedef ZergFrame_Mallocor<ZCE_Null_Mutex> Inner_Frame_Mallocor;

    

public:

    //管理器的构造函数
    FSM_Manager();
    virtual ~FSM_Manager();

protected:

    /*!
    * @brief      处理一个收到的命令，
    * @return     int
    * @param      zerg_frame 处理的事务的帧数据，zerg_frame帧的生命周期由process_appframe函数管理
    * @param      crttx 是否创建事务
    * @note
    */
    int process_appframe(soar::Zerg_Frame *zerg_frame,bool &crttx);

public:


    //处理管道的数据
    int process_pipe_frame(size_t &proc_frame,size_t &create_trans);
    //处理消息队列的数据
    int process_queue_frame(size_t &proc_frame,size_t &create_trans);


    /*!
    * @brief      对某一个用户的一个命令的事务进行加锁
    * @return     int
    * @param      user_id 用户ID
    * @param      trnas_lock_id 加锁的ID,可以和命令字相同，或者不同
    * @param      trans_cmd     事务的命令，仅仅用于日志输出
    * @note       事务锁的意思是保证一个时刻，只能一个这样的事务,事务锁不阻塞
    */
    int lock_userid_fsm_cmd(uint32_t user_id,
                            uint32_t trnas_lock_id,
                            uint32_t trans_cmd);
    //对某一个用户的一个命令的事务进行解锁
    void unlock_userid_fsm_cmd(uint32_t user_id,
                               uint32_t trnas_lock_id);


    //初始化,住一个几个默认参数
    int initialize(ZCE_Timer_Queue_Base *timer_queue,
                   size_t szregtrans,
                   size_t sztransmap,
                   const soar::SERVICES_INFO &selfsvr,
                   Soar_MMAP_BusPipe *zerg_mmap_pipe,
                   size_t max_frame_len = soar::Zerg_Frame::MAX_LEN_OF_APPFRAME,
                   bool init_inner_queue = false,
                   bool init_lock_pool = false);

    //
    void finish();

    //得到一个SvrInfo
    const soar::SERVICES_INFO *self_svc_info();
    //得到一个SvrID
    const soar::SERVICES_ID *self_svc_id();

    //取得管理器的负载因子
    void get_manager_load_foctor(uint32_t &load_max,
                                 uint32_t &load_cur);

    void get_manager_load_foctor2(uint32_t &load_max,
                                  uint32_t &load_cur);


    //打开Trans统计信息，得到一个当前时钟
    void enable_trans_statistics(const ZCE_Time_Value *stat_clock);

    //DUMP所有的统计信息
    void dump_statistics_info() const;
    //DUMP所有的事务信息
    void dump_all_trans_info() const;
    //DUMP所有的的Tans POOL 信息
    void dump_trans_pool_info() const;

    //Dump 所有DEBUG信息
    void dump_all_debug_info() const;

    //----------------------------------------------------------------------------------------------------------

    //假装收到一个消息，进行处理,参数有点多，建议你使用的时候再进行一次封装
    //用于单元测试等
    template< class T> 
    int fake_receive_frame(uint32_t cmd,
                           uint32_t user_id,
                           uint32_t fsm_id,
                           uint32_t backfill_fsm_id,
                           const soar::SERVICES_ID& proxy_svc,
                           const soar::SERVICES_ID& snd_svc,
                           const T& info,
                           uint32_t option);

    //假装收到一个消息(buffer)
    int fake_receive_frame(const soar::Zerg_Frame* fake_recv);

    //----------------------------------------------------------------------------------------------------------
    //Post一个FRAME数据到消息队列,简单版本，没有特殊要求，你可以用这个
    template< class T>
    int post_msg_to_queue(uint32_t cmd,
                         uint32_t user_id,
                         const T& msg,
                         uint32_t option = 0);

    //Post一个FRAME数据到消息队列，可以伪造一些消息，但是我不知道提供出来是否是好事,
    template< class T>
    int post_msg_to_queue(uint32_t cmd,
                         uint32_t user_id,
                         uint32_t fsm_id,
                         uint32_t backfill_fsm_id,
                         const soar::SERVICES_ID& rcvsvc,
                         const soar::SERVICES_ID& proxysvc,
                         const soar::SERVICES_ID& sndsvc,
                         const T& msg,
                         uint32_t option);

    //----------------------------------------------------------------------------------------------------------
    //管理器发送一个命令给一个服务器
    template< class T> 
    int sendmsg_to_service(uint32_t cmd,
                           uint32_t user_id,
                           uint32_t fsm_id,
                           uint32_t backfill_fsm_id,
                           const soar::SERVICES_ID& rcvsvc,
                           const soar::SERVICES_ID& proxysvc,
                           const soar::SERVICES_ID& sndsvc,
                           const T& msg,
                           uint32_t option);

    //----------------------------------------------------------------------------------------------------------
    //发送一个数据到PIPE
    int sendfame_to_pipe(const soar::Zerg_Frame *proc_frame);

    //
    int sendbuf_to_pipe(const soar::Zerg_Head& zerg_head,
                        const char* buf,
                        size_t buf_len);

protected:

    //发送一消息头给一个服务器,内部函数
    int mgr_sendmsghead_to_service(uint32_t cmd,
                                   uint32_t user_id,
                                   const soar::SERVICES_ID &rcvsvc,
                                   const soar::SERVICES_ID &proxysvc,
                                   uint32_t backfill_fsm_id = 0,
                                   uint32_t option = 0);

    //----------------------------------------------------------------------------------------------------------
protected:

    //发送一个数据到QUEUE
    int postmsg_to_queue(soar::Zerg_Frame *post_frame);


private:

public:
    //为了SingleTon类准备
    //实例赋值
    static void instance(FSM_Manager *);
    //获得实例
    static FSM_Manager *instance();
    //清除实例
    static void clean_instance();

protected:

    //一次最大处理的FRAME个数
    static const size_t MAX_ONCE_PROCESS_FRAME = 1024;

    //池子每次扩展的事务个数
    static const size_t POOL_EXTEND_TRANSACTION_NUM = 1024;

    //回收的阈值
    static const size_t RECYCLE_POOL_THRESHOLD_VALUE = 2048;
    //
    static const size_t INIT_FRAME_MALLOC_NUMBER = 2048;
    //
    static const size_t MAX_QUEUE_NODE_NUMBER = 10 * 1024 * 1024;


protected:

    //最大的事件个数
    size_t max_trans_;

    //自己的Services Info
    soar::SERVICES_INFO self_svc_info_;

    //共享内存的管道
    Soar_MMAP_BusPipe *zerg_mmap_pipe_;

    //统计时钟
    const ZCE_Time_Value *statistics_clock_;

    //发送的缓冲区
    soar::Zerg_Frame *trans_send_buffer_;
    //接受数据缓冲区
    soar::Zerg_Frame *trans_recv_buffer_;

    // fake数据缓冲区
    soar::Zerg_Frame *fake_recv_buffer_;

    //内部FRAME分配器
    Inner_Frame_Mallocor *inner_frame_mallocor_;
    //内部FRAME的队列
    Inner_Frame_Queue *message_queue_;

    //统计分析的一些变量
    //产生事务的总量记录
    uint64_t           gen_trans_counter_;
    //一个周期内产生的事务总数
    uint32_t           cycle_gentrans_counter_;

protected:
    //SingleTon的指针
    static FSM_Manager *instance_;

};


//
inline const soar::SERVICES_INFO *FSM_Manager::self_svc_info()
{
    return &self_svc_info_;
}

inline const soar::SERVICES_ID *FSM_Manager::self_svc_id()
{
    return &(self_svc_info_.svc_id_);
}


//假装收到一个消息，进行处理,参数有点多，建议你使用的时候再进行一次封装
template< class T>
int FSM_Manager::fake_receive_frame(uint32_t cmd,
                                    uint32_t user_id,
                                    uint32_t fsm_id,
                                    uint32_t backfill_fsm_id,
                                    const soar::SERVICES_ID& proxy_svc,
                                    const soar::SERVICES_ID& snd_svc,
                                    const T& info,
                                    uint32_t option)
{
    int ret = 0;

    soar::Zerg_Frame *tmp_frame = reinterpret_cast<soar::Zerg_Frame *>(fake_recv_buffer_);
    tmp_frame->init_head(soar::Zerg_Frame::MAX_LEN_OF_APPFRAME,option,cmd);

    tmp_frame->user_id_ = user_id;
    tmp_frame->send_service_ = snd_svc;
    tmp_frame->recv_service_ = self_svc_info_;
    tmp_frame->proxy_service_ = proxy_svc;

    tmp_frame->fsm_id_ = trans_id;
    tmp_frame->backfill_fsm_id_ = backfill_trans_id;

    ret = tmp_frame->appdata_encode(soar::Zerg_Frame::MAX_LEN_OF_APPFRAME_DATA,info);

    if (ret != 0)
    {
        return ret;
    }

    //处理一个收到的命令，
    bool crttx;
    ret = process_appframe(tmp_frame,crttx);

    if (ret != 0 && ret != SOAR_RET::ERROR_TRANS_HAS_FINISHED)
    {
        return ret;
    }

    return 0;
}




//管理器发送一个命令给一个服务器,内部函数
template< class T>
int FSM_Manager::sendmsg_to_service(uint32_t cmd,
                                    uint32_t user_id,
                                    uint32_t fsm_id,
                                    uint32_t backfill_fsm_id,
                                    const soar::SERVICES_ID& rcvsvc,
                                    const soar::SERVICES_ID& proxysvc,
                                    const soar::SERVICES_ID& sndsvc,
                                    const T& msg,
                                    uint32_t option)
{
    //[注意]一下这个地方，recv和send参数两边的顺序是反的
    return zerg_mmap_pipe_->pipe_sendmsg_to_service(cmd,
                                                    user_id,
                                                    trans_id,
                                                    backfill_trans_id,
                                                    rcvsvc,
                                                    proxysvc,
                                                    sndsvc,
                                                    msg,
                                                    option);
}

//Post一个FRAME数据到消息队列，可以伪造一些消息，但是我不知道提供出来是否是好事
template< class T>
int FSM_Manager::post_msg_to_queue(uint32_t cmd,
                                   uint32_t user_id,
                                   uint32_t fsm_id,
                                   uint32_t backfill_fsm_id,
                                   const soar::SERVICES_ID& rcvsvc,
                                   const soar::SERVICES_ID& proxysvc,
                                   const soar::SERVICES_ID& sndsvc,
                                   const T& msg,
                                   uint32_t option)
{
    soar::Zerg_Frame *rsp_msg = reinterpret_cast<soar::Zerg_Frame *>(trans_send_buffer_);
    rsp_msg->init_head(soar::Zerg_Frame::MAX_LEN_OF_APPFRAME,option,cmd);

    rsp_msg->user_id_ = user_id;
    rsp_msg->fsm_id_ = fsm_id;
    rsp_msg->recv_service_ = rcvsvc;
    rsp_msg->proxy_service_ = proxysvc;
    rsp_msg->send_service_ = sndsvc;

    //填写自己transaction_id_,其实是自己的事务ID,方便回来可以找到自己
    rsp_msg->backfill_fsm_id_ = backfill_fsm_id;

    //拷贝发送的MSG Block
    int ret = rsp_msg->appdata_encode(soar::Zerg_Frame::MAX_LEN_OF_APPFRAME_DATA,info);
    if (ret != 0)
    {
        return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
    }

    //相信这个锁不会占据主循环
    ret = postmsg_to_queue(rsp_msg);
    DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG,"TO MESSAGE QUEUE FRAME",rsp_msg);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR,"[framework] mgr_postframe_to_msgqueue but fail.Send queue is full or task process too slow to process request.");
        return ret;
    }
    return 0;
}

//Post一个FRAME数据到消息队列
template< class T>
int FSM_Manager::post_msg_to_queue(uint32_t cmd,
                                   uint32_t user_id,
                                   const T& msg,
                                   uint32_t option)
{
    soar::SERVICES_ID rcvsvc = self_svc_info_;
    soar::SERVICES_ID proxysvc(0,0);
    soar::SERVICES_ID sndsvc = self_svc_info_;
    return mgr_postframe_to_msgqueue(
        cmd,
        user_id,
        0,
        0,
        rcvsvc,
        proxysvc,
        sndsvc,
        info,
        0,
        option);
}

#endif //SOARING_LIB_TRANSACTION_MANAGER_H_

