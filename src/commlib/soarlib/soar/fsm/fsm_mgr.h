#pragma once

#include "soar/zerg/frame_malloc.h"
#include "soar/svrd/svrd_buspipe.h"

//是否按照zengyu所说的将所有的IO接口继承处理???
//我心中充满了无数的问号
//如果就是这个咚咚最后是一个框架，我几乎认为认为这个事务模型不用修改.但是.....

//简单，但是不要太简单.....
//感觉我的想法是比较简单的,表面看我没有事件队列,其实我用了ZCE_Reactor进行管理,其实内部实际是由队列的.

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

namespace soar
{
class svrd_buspipe;
class fsm_base;
class zerg_frame;

/******************************************************************************************
class Transaction_Manager
******************************************************************************************/
class  FSM_Manager : public zce::Async_FSMMgr
{
    //声明友元
    friend class fsm_base;

protected:
    struct  ONLYONE_LOCK
    {
        //事务的加锁ID，如果就是一个命令对应一个锁，建议直接使用命令字
        //如果是多个命令对一个东东加锁，建议占位一个命令，然后对那个命令加锁，
        uint32_t     lock_trans_cmd_;
        //要加锁的USER ID,
        uint32_t     lock_user_id_;
    };

    //得到KEY的HASH函数
    struct HASH_OF_LOCK
    {
    public:
        size_t operator()(const ONLYONE_LOCK& lock_rec) const
        {
            return (size_t(lock_rec.lock_user_id_) + lock_rec.lock_trans_cmd_);
        }
    };

    //判断相等的函数
    struct EQUAL_OF_LOCK
    {
        bool operator()(const ONLYONE_LOCK& right,
                        const ONLYONE_LOCK& left) const
        {
            return (right.lock_user_id_ == left.lock_user_id_ && right.lock_trans_cmd_ == left.lock_trans_cmd_);
        }
    };

    ///内部的APPFRAME的消息队列，
    typedef zce::MsgRings_Sema<soar::zerg_frame*> Inner_Frame_Queue;
    ///内部的APPFRAME的分配器，只在Mgr内部使用，单线程，用于给内部提供一些异步化的处理
    typedef ZergFrame_Mallocor<zce::Null_Lock> Inner_Frame_Mallocor;
    //内部的锁的数量
    typedef std::unordered_set<ONLYONE_LOCK, HASH_OF_LOCK, EQUAL_OF_LOCK>  ONLY_ONE_LOCK_POOL;

public:

    //管理器的构造函数
    FSM_Manager();
    virtual ~FSM_Manager();

    //----------------------------------------------------------------------------------------------------------

public:

    /*!
    * @brief      初始化
    * @return     int
    * @param      timer_queue
    * @param      reg_fsm_num
    * @param      running_fsm_num
    * @param      selfsvr
    * @param      zerg_mmap_pipe
    * @param      max_frame_len
    * @param      init_inner_queue
    * @param      init_lock_pool
    * @note
    */
    int initialize(zce::timer_queue* timer_queue,
                   size_t  reg_fsm_num,
                   size_t running_fsm_num,
                   const soar::SERVICES_INFO& selfsvr,
                   soar::svrd_buspipe* zerg_mmap_pipe,
                   size_t max_frame_len = soar::zerg_frame::MAX_LEN_OF_FRAME,
                   bool init_inner_queue = false,
                   bool init_lock_pool = false);

    //销毁
    void terminate();

    //处理管道的数据
    int process_pipe_frame(size_t& proc_frame,
                           size_t& create_trans);
    //处理消息队列的数据
    int process_queue_frame(size_t& proc_frame,
                            size_t& create_trans);

    /*!
    * @brief
    * @return     int
    * @param      create_cmd 注册的命令
    * @param      fsm_base   FSM的指针，new产生
    * @param      usr_only_one 是否是一个用户一次处理一个命令
    * @note
    */
    int register_fsmobj(uint32_t create_cmd,
                        fsm_base* fsm_base,
                        bool usr_only_one);

    /*!
    * @brief      对某个命令的某些ID（一般是用户）的的事务进行加锁，保证一次只能有一个
    * @return     int 等于0表示成功
    * @param      cmd 命令字
    * @param      lock_id 一般是用户ID
    */
    int lock_only_one(uint32_t cmd,
                      uint32_t lock_id);

    //对某一个用户的一个命令的事务进行
    void unlock_only_one(uint32_t cmd,
                         uint32_t lock_id);

    //这个命令是否是锁定的命令
    bool is_onlyone_cmd(uint32_t cmd);

    //得到一个SvrInfo
    const soar::SERVICES_INFO* self_svc_info();
    //得到一个SvrID
    const soar::SERVICES_ID* self_svc_id();

    //打开Trans统计信息，得到一个当前时钟
    void enable_trans_statistics(const zce::time_value* stat_clock);

    //!得到frame信息
    int get_process_frame(const soar::zerg_frame*& zerg_frame);
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

    //!假装收到一个消息(buffer)
    int fake_receive_frame(const soar::zerg_frame* fake_recv);

    //----------------------------------------------------------------------------------------------------------

    //!Post一个FRAME数据到消息队列，可以伪造一些消息，但是我不知道提供出来是否是好事,
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

    //!发送一个数据到QUEUE
    int postmsg_to_queue(soar::zerg_frame* post_frame);

    //----------------------------------------------------------------------------------------------------------
    //!管理器发送一个命令给一个服务器
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
    //!发送一个数据到PIPE
    int sendfame_to_pipe(const soar::zerg_frame* proc_frame);

    //!
    int sendbuf_to_pipe(const soar::zerg_head& zerg_head,
                        const char* buf,
                        size_t buf_len);

protected:

    /*!
    * @brief      处理一个收到的命令，
    * @return     int
    * @param      zerg_frame 处理的事务的帧数据，zerg_frame帧的生命周期由process_appframe函数管理
    * @param      crttx 是否创建事务
    */
    int process_frame(soar::zerg_frame* zerg_frame,
                      bool& create_fsm);
public:
    //为了SingleTon类准备
    //实例赋值
    static void instance(FSM_Manager*);
    //获得实例
    static FSM_Manager* instance();
    //清除实例
    static void clear_inst();

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

    //自己的Services Info
    soar::SERVICES_INFO self_svc_info_;

    //共享内存的管道
    soar::svrd_buspipe* zerg_mmap_pipe_ = nullptr;

    //统计时钟
    const zce::time_value* statistics_clock_ = nullptr;

    //发送的缓冲区
    soar::zerg_frame* trans_send_buffer_ = nullptr;
    //接受数据缓冲区
    soar::zerg_frame* trans_recv_buffer_ = nullptr;

    //! fake数据缓冲区
    soar::zerg_frame* fake_recv_buffer_ = nullptr;
    //!
    soar::zerg_frame* process_frame_ = nullptr;

    //内部FRAME分配器
    Inner_Frame_Mallocor* inner_frame_mallocor_ = nullptr;
    //内部FRAME的队列
    Inner_Frame_Queue* message_queue_ = nullptr;

    //!ONLY ONE锁的池子
    ONLY_ONE_LOCK_POOL* only_one_lock_pool_ = nullptr;

    //!如果一个类型的状态机，一个用户对于只能创建一个，记录（命令字）到这个set
    std::unordered_set<uint32_t> onlyone_fms_cmd_set_;

    //!统计分析的一些变量
    //!产生事务的总量记录
    uint64_t gen_ksm_counter_ = 0;

protected:
    //!SingleTon的指针
    static FSM_Manager* instance_;
};

//
inline const soar::SERVICES_INFO* FSM_Manager::self_svc_info()
{
    return &self_svc_info_;
}

inline const soar::SERVICES_ID* FSM_Manager::self_svc_id()
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

    soar::zerg_frame* tmp_frame = reinterpret_cast<soar::zerg_frame*>(fake_recv_buffer_);
    tmp_frame->init_head(soar::zerg_frame::MAX_LEN_OF_APPFRAME, option, cmd);

    tmp_frame->user_id_ = user_id;
    tmp_frame->send_service_ = snd_svc;
    tmp_frame->recv_service_ = self_svc_info_;
    tmp_frame->proxy_service_ = proxy_svc;

    tmp_frame->fsm_id_ = fsm_id;
    tmp_frame->backfill_fsm_id_ = backfill_fsm_id;

    ret = tmp_frame->appdata_encode(soar::zerg_frame::MAX_LEN_OF_DATA, info);

    if (ret != 0)
    {
        return ret;
    }

    //处理一个收到的命令，
    bool create_fsm = false;
    ret = process_frame(tmp_frame, create_fsm);
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
                                    const T& info,
                                    uint32_t option)
{
    soar::zerg_frame* rsp_msg = reinterpret_cast<soar::zerg_frame*>(trans_send_buffer_);
    rsp_msg->init_head(soar::zerg_frame::MAX_LEN_OF_APPFRAME, option, cmd);
    rsp_msg->user_id_ = user_id;
    rsp_msg->fsm_id_ = fsm_id;
    rsp_msg->recv_service_ = rcvsvc;
    rsp_msg->proxy_service_ = proxysvc;
    rsp_msg->send_service_ = sndsvc;
    rsp_msg->u32_option_ = option;
    //填写自己transaction_id_,其实是自己的事务ID,方便回来可以找到自己
    rsp_msg->backfill_fsm_id_ = backfill_fsm_id;

    //拷贝发送的MSG Block
    int ret = rsp_msg->appdata_encode(soar::zerg_frame::MAX_LEN_OF_DATA,
                                      info);
    if (ret != 0)
    {
        return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
    }
    return zerg_mmap_pipe_->push_back_sendbus(rsp_msg);
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
                                   const T& info,
                                   uint32_t option)
{
    soar::zerg_frame* rsp_msg = reinterpret_cast<soar::zerg_frame*>(trans_send_buffer_);
    rsp_msg->init_head(soar::zerg_frame::MAX_LEN_OF_APPFRAME, option, cmd);
    rsp_msg->user_id_ = user_id;
    rsp_msg->fsm_id_ = fsm_id;
    rsp_msg->recv_service_ = rcvsvc;
    rsp_msg->proxy_service_ = proxysvc;
    rsp_msg->send_service_ = sndsvc;
    rsp_msg->u32_option_ = option;
    //填写自己transaction_id_,其实是自己的事务ID,方便回来可以找到自己
    rsp_msg->backfill_fsm_id_ = backfill_fsm_id;

    //拷贝发送的MSG Block
    int ret = rsp_msg->appdata_encode(soar::zerg_frame::MAX_LEN_OF_DATA,
                                      info);
    if (ret != 0)
    {
        return SOAR_RET::ERROR_APPFRAME_BUFFER_SHORT;
    }

    //相信这个锁不会占据主循环
    ret = postmsg_to_queue(rsp_msg);
    DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG, "TO MESSAGE QUEUE FRAME", rsp_msg);
    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] postmsg_to_queue but fail.Send queue is full "
                "or task process too slow to process request.");
        return ret;
    }
    return 0;
}
}
