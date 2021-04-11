#ifndef SOARING_LIB_TRANSACTION_MANAGER_H_
#define SOARING_LIB_TRANSACTION_MANAGER_H_

#include "soar_zerg_frame_malloc.h"
#include "soar_mmap_buspipe.h"

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
class Transaction_Base;
class Zerg_App_Frame;

/******************************************************************************************
struct TRANS_LOCK_RECORD 加锁的记录单元
******************************************************************************************/
struct SOARING_EXPORT TRANS_LOCK_RECORD
{
public:
    //要加锁的USER ID,
    unsigned int     lock_user_id_;
    //事务的加锁ID，如果就是一个命令对应一个锁，建议直接使用命令字
    //如果是多个命令对一个东东加锁，建议占位一个命令，然后对那个命令加锁，
    unsigned int     lock_trans_cmd_;

public:
    TRANS_LOCK_RECORD(unsigned int lock_qq_uin, unsigned int lock_trans_command);
    TRANS_LOCK_RECORD();
    ~TRANS_LOCK_RECORD();
};

//得到KEY的HASH函数
struct SOARING_EXPORT HASH_OF_TRANS_LOCK
{
public:
    size_t operator()(const TRANS_LOCK_RECORD &lock_rec) const
    {
        return (size_t(lock_rec.lock_user_id_) + lock_rec.lock_trans_cmd_);
    }
};

//判断相等的函数
class SOARING_EXPORT EQUAL_OF_TRANS_LOCK
{
public:
    bool operator()(const TRANS_LOCK_RECORD &right, const TRANS_LOCK_RECORD &left) const
    {
        return (right.lock_user_id_ == left.lock_user_id_ && right.lock_trans_cmd_ == left.lock_trans_cmd_);
    }

};

/******************************************************************************************
class Transaction_Manager
******************************************************************************************/
class SOARING_EXPORT Transaction_Manager : public ZCE_Async_FSMMgr
{
    //声明友元
    friend class Transaction_Base;

protected:


    ///
    typedef ZCE_Message_Queue_Deque<ZCE_NULL_SYNCH, Zerg_App_Frame *> INNER_FRAME_MESSAGE_QUEUE;
    ///APPFRAME的分配器
    typedef AppFrame_Mallocor_Mgr<ZCE_Null_Mutex>                     INNER_APPFRAME_MALLOCOR;

    //内部的锁的数量
    typedef unordered_set<TRANS_LOCK_RECORD, HASH_OF_TRANS_LOCK, EQUAL_OF_TRANS_LOCK>  INNER_TRANS_LOCK_POOL;

public:

    //管理器的构造函数
    Transaction_Manager();
    virtual ~Transaction_Manager();

protected:


    //处理一个收到的命令，
    int process_appframe( Zerg_App_Frame *ppetappframe, bool &crttx );

public:


    //处理管道的数据
    int process_pipe_frame(size_t &proc_frame, size_t &create_trans);
    //处理消息队列的数据
    int process_queue_frame(size_t &proc_frame, size_t &create_trans);

    //对某一个用户的一个命令的事务进行加锁
    int lock_qquin_trnas_cmd(unsigned int user_id,
                             unsigned int trnas_lock_id,
                             unsigned int trans_cmd);
    //对某一个用户的一个命令的事务进行加锁
    void unlock_qquin_trans_cmd(unsigned int user_id,
                                unsigned int trnas_lock_id);


    /*!
    * @brief      注册命令以及对应的事务处理的类
    * @return     int
    * @param      cmd 注册的命令字
    * @param      ptxbase 命令对应的处理Handler，最后会删除
    * @param      if_lock_trans 这个事务是否加锁，事务锁的意思是保证一个时刻，只能一个这样的事务,事务锁不阻塞
    * @param      lock_trans_cmd 加锁的ID,可以是命令ID,也可以多个命令共用一个个ID,
    * @note       事务锁的意思是保证一个时刻，只能一个这样的事务,事务锁不阻塞
    *             这个地方违背了谁申请，谁删除的原则，不好，但是……
    */
    int register_trans_cmd(unsigned int cmd,
                           Transaction_Base *ptxbase,
                           bool if_lock_trans = false,
                           unsigned int lock_trans_cmd = 0);

    //通过事务ID得到相应的事务指针
    int get_handler_by_transid(unsigned int transid, unsigned int trans_cmd, Transaction_Base *&ptxbase);

    //创建一个事务，比如自己要发送某个数据给其他服务器开始的事务.
    int create_self(Transaction_Base *ptxbase);

    //初始化,住一个几个默认参数
    int initialize(ZCE_Timer_Queue_Base *timer_queue,
                   size_t szregtrans,
                   size_t sztransmap,
                   const SERVICES_ID &selfsvr,
                   Soar_MMAP_BusPipe *zerg_mmap_pipe,
                   unsigned int max_frame_len = Zerg_App_Frame::MAX_LEN_OF_APPFRAME,
                   bool init_inner_queue = false,
                   bool init_lock_pool = false);

    //
    void finish();

    //得到一个SvrInfo
    inline const SERVICES_ID *self_svc_info();

    //取得管理器的负载因子
    void get_manager_load_foctor(unsigned int &load_max,
                                 unsigned int &load_cur);

    void get_manager_load_foctor2(unsigned int &load_max,
                                  unsigned int &load_cur);

    //注销TransID.
    int unregiester_trans_id(unsigned int transid,
                             unsigned int trans_cmd,
                             int run_state,
                             time_t trans_start);

    //打开Trans统计信息，得到一个当前时钟
    void enable_trans_statistics (const ZCE_Time_Value *stat_clock);

    //DUMP所有的统计信息
    void dump_statistics_info() const;
    //DUMP所有的事务信息
    void dump_all_trans_info() const ;
    //DUMP所有的的Tans POOL 信息
    void dump_trans_pool_info() const;

    //Dump 所有DEBUG信息
    void dump_all_debug_info() const;

    //----------------------------------------------------------------------------------------------------------

    //假装收到一个消息，进行处理,参数少的简化版本
    template< class T> int fake_receive_appframe(unsigned int cmd,
                                                 unsigned int qquin,
                                                 const SERVICES_ID &snd_svc,
                                                 const T &info,
                                                 unsigned int option = 0);

    //假装收到一个消息，进行处理,参数有点多，建议你使用的时候再进行一次封装
    template< class T> int fake_receive_appframe(unsigned int cmd,
                                                 unsigned int qquin,
                                                 unsigned int trans_id,
                                                 unsigned int backfill_trans_id,
                                                 const SERVICES_ID &proxy_svc,
                                                 const SERVICES_ID &snd_svc,
                                                 const T &info,
                                                 unsigned int app_id,
                                                 unsigned int option );

    //假装收到一个消息(buffer)
    inline int fake_receive_appframe_buffer(unsigned int cmd,
                                            unsigned int qquin,
                                            unsigned int trans_id,
                                            unsigned int backfill_trans_id,
                                            const SERVICES_ID &proxy_svc,
                                            const SERVICES_ID &snd_svc,
                                            const char *send_buff,
                                            unsigned int buff_size,
                                            unsigned int app_id,
                                            unsigned int option);

    //----------------------------------------------------------------------------------------------------------
    //Post一个FRAME数据到消息队列,简单版本，没有特殊要求，你可以用这个
    template< class T> int mgr_postframe_to_msgqueue(unsigned int cmd,
                                                     unsigned int qquin,
                                                     const T &info,
                                                     unsigned int option = 0);

    //Post一个FRAME数据到消息队列，可以伪造一些消息，但是我不知道提供出来是否是好事,
    template< class T> int mgr_postframe_to_msgqueue(unsigned int cmd,
                                                     unsigned int qquin,
                                                     unsigned int trans_id,
                                                     unsigned int backfill_trans_id,
                                                     const SERVICES_ID &rcvsvc,
                                                     const SERVICES_ID &proxysvc,
                                                     const SERVICES_ID &sndsvc,
                                                     const T &info,
                                                     unsigned int app_id,
                                                     unsigned int option);

    //----------------------------------------------------------------------------------------------------------
    //管理器发送一个命令给一个服务器,内部函数
    template< class T> int mgr_sendmsg_to_service(unsigned int cmd,
                                                  unsigned int qquin,
                                                  unsigned int trans_id,
                                                  unsigned int backfill_trans_id,
                                                  const SERVICES_ID &rcvsvc,
                                                  const SERVICES_ID &proxysvc,
                                                  const SERVICES_ID &sndsvc,
                                                  const T &info,
                                                  unsigned int app_id,
                                                  unsigned int option);

    // 发送buf到某个service, buf是打好的包，满足某些需要转发buf的需求
    int mgr_sendbuf_to_service(unsigned int cmd,
                               unsigned int qquin,
                               unsigned int trans_id,
                               unsigned int backfill_trans_id,
                               const SERVICES_ID &rcvsvc,
                               const SERVICES_ID &proxysvc,
                               const SERVICES_ID &sndsvc,
                               const unsigned char *buf,
                               size_t buf_len,
                               unsigned int app_id,
                               unsigned int option);

    //发送一个数据到PIPE
    int push_back_sendpipe(Zerg_App_Frame *proc_frame);

protected:
    //发送一消息头给一个服务器,内部函数
    int mgr_sendmsghead_to_service(unsigned int cmd,
                                   unsigned int qquin,
                                   const SERVICES_ID &rcvsvc,
                                   const SERVICES_ID &proxysvc,
                                   unsigned int backfill_trans_id = 0,
                                   unsigned int app_id = 0,
                                   unsigned int option = 0);

    //----------------------------------------------------------------------------------------------------------
protected:

    //发送一个数据到QUEUE
    int mgr_postframe_to_msgqueue(Zerg_App_Frame *post_frame);

    //注册TransID.
    int regiester_trans_id(unsigned int transid, unsigned int trans_cmd, Transaction_Base *ptxbase);

private:

public:
    //为了SingleTon类准备
    //实例赋值
    static void instance(Transaction_Manager *);
    //获得实例
    static Transaction_Manager *instance();
    //清除实例
    static void clean_instance();

protected:

    //一次最大处理的FRAME个数
    static const size_t MAX_ONCE_PROCESS_FRAME = 1024;

    //池子每次扩展的事务个数
    static const size_t POOL_EXTEND_TRANSACTION_NUM = 1024;

    //回收的阈值
    static const size_t RECYCLE_POOL_THRESHOLD_VALUE = 2048;

    static const size_t INIT_FRAME_MALLOC_NUMBER = 2048;

    //QUEUE FRAME队列的水位标，考虑倒由于MessageQueue中奖存放的是指针，
    //但是长度应该应该是按照APPFRAME的帧头计算的。这个数量级别的长度已经不小了
    static const size_t INNER_QUEUE_WATER_MARK = Zerg_App_Frame::LEN_OF_APPFRAME_HEAD * 102400;

protected:

    //锁的池子
    INNER_TRANS_LOCK_POOL       trans_lock_pool_;

    //最大的事件个数
    size_t                      max_trans_;


    //自己的Services Info
    SERVICES_ID                 self_svc_id_;

    //共享内存的管道
    Soar_MMAP_BusPipe          *zerg_mmap_pipe_;

    //统计时钟
    const ZCE_Time_Value       *statistics_clock_;

    //发送的缓冲区
    Zerg_App_Frame             *trans_send_buffer_;
    //接受数据缓冲区
    Zerg_App_Frame             *trans_recv_buffer_;

    // fake数据缓冲区
    Zerg_App_Frame             *fake_recv_buffer_;

    //内部FRAME分配器
    INNER_APPFRAME_MALLOCOR    *inner_frame_malloc_;
    //内部FRAME的队列
    INNER_FRAME_MESSAGE_QUEUE  *inner_message_queue_;

    //统计分析的一些变量
    //产生事务的总量记录
    uint64_t                    gen_trans_counter_;
    //一个周期内产生的事务总数
    unsigned int                cycle_gentrans_counter_;

protected:
    //SingleTon的指针
    static Transaction_Manager *instance_;

};

//假装收到一个消息，进行处理,参数少的简化版本
template< class T>
int Transaction_Manager::fake_receive_appframe(unsigned int cmd,
                                               unsigned int qquin,
                                               const SERVICES_ID &snd_svc,
                                               const T &info,
                                               unsigned int option)
{

    return fake_receive_appframe(cmd,
                                 qquin,
                                 0,
                                 0,
                                 SERVICES_ID(0, 0),
                                 snd_svc,
                                 info,
                                 0,
                                 option);
}

//假装收到一个消息，进行处理,参数有点多，建议你使用的时候再进行一次封装
template< class T>
int Transaction_Manager::fake_receive_appframe(unsigned int cmd,
                                               unsigned int qquin,
                                               unsigned int trans_id,
                                               unsigned int backfill_trans_id,
                                               const SERVICES_ID &proxy_svc,
                                               const SERVICES_ID &snd_svc,
                                               const T &info,
                                               unsigned int app_id,
                                               unsigned int option )
{
    int ret = 0;

    Zerg_App_Frame *tmp_frame = reinterpret_cast<Zerg_App_Frame *>(fake_recv_buffer_);
    tmp_frame->init_framehead(Zerg_App_Frame::MAX_LEN_OF_APPFRAME, option, cmd);

    tmp_frame->frame_userid_ = qquin;
    tmp_frame->send_service_ = snd_svc;
    tmp_frame->recv_service_ = self_svc_id_;
    tmp_frame->proxy_service_ = proxy_svc;

    tmp_frame->transaction_id_ = trans_id;
    tmp_frame->backfill_trans_id_ = backfill_trans_id;
    tmp_frame->app_id_ = app_id;

    ret = tmp_frame->appdata_encode(Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA, info);

    if (ret != 0)
    {
        return ret;
    }

    //处理一个收到的命令，
    bool crttx;
    ret = process_appframe(tmp_frame, crttx);

    if (ret != 0 && ret != SOAR_RET::ERROR_TRANS_HAS_FINISHED)
    {
        return ret;
    }

    return 0;
}

// recv_svr填的是自己，就假装收到一个包，如其名fake
int Transaction_Manager::fake_receive_appframe_buffer(unsigned int cmd,
                                                      unsigned int qquin,
                                                      unsigned int trans_id,
                                                      unsigned int backfill_trans_id,
                                                      const SERVICES_ID &proxy_svc,
                                                      const SERVICES_ID &snd_svc,
                                                      const char *recv_buffer,
                                                      unsigned int buff_size,
                                                      unsigned int app_id,
                                                      unsigned int option)
{
    int ret = 0;

    Zerg_App_Frame *tmp_frame = reinterpret_cast<Zerg_App_Frame *>(fake_recv_buffer_);
    tmp_frame->init_framehead(Zerg_App_Frame::MAX_LEN_OF_APPFRAME, option, cmd);

    tmp_frame->frame_userid_ = qquin;
    tmp_frame->send_service_ = snd_svc;
    tmp_frame->recv_service_ = self_svc_id_;
    tmp_frame->proxy_service_ = proxy_svc;

    tmp_frame->transaction_id_ = trans_id;
    tmp_frame->backfill_trans_id_ = backfill_trans_id;
    tmp_frame->app_id_ = app_id;

    tmp_frame->frame_length_ = (unsigned int)(buff_size + Zerg_App_Frame::LEN_OF_APPFRAME_HEAD);
    if (tmp_frame->frame_length_ > Zerg_App_Frame::MAX_LEN_OF_APPFRAME_DATA)
    {
        return SOAR_RET::ERROR_FRAME_DATA_IS_ERROR;
    }
    memcpy(tmp_frame->frame_appdata_, recv_buffer, buff_size);

    bool crttx = false;
    ret = process_appframe(tmp_frame, crttx);

    if (ret != 0 && ret != SOAR_RET::ERROR_TRANS_HAS_FINISHED)
    {
        return ret;
    }

    return 0;
}

//
inline const SERVICES_ID *Transaction_Manager::self_svc_info()
{
    return &self_svc_id_;
}

//管理器发送一个命令给一个服务器,内部函数
template< class T>
int Transaction_Manager::mgr_sendmsg_to_service(unsigned int cmd,
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
    //[注意]一下这个地方，recv和send参数两边的顺序是反的
    return zerg_mmap_pipe_->pipe_sendmsg_to_service(cmd,
                                                    qquin,
                                                    trans_id,
                                                    backfill_trans_id,
                                                    rcvsvc,
                                                    proxysvc,
                                                    sndsvc,
                                                    info,
                                                    app_id,
                                                    option);
}

//Post一个FRAME数据到消息队列，可以伪造一些消息，但是我不知道提供出来是否是好事
template< class T>
int Transaction_Manager::mgr_postframe_to_msgqueue(
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
    ret = mgr_postframe_to_msgqueue(rsp_msg);
    DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "TO MESSAGE QUEUE FRAME", rsp_msg);

    if (ret != 0)
    {
        ZCE_LOG(RS_ERROR, "[framework] mgr_postframe_to_msgqueue but fail.Send queue is full or task process too slow to process request.");
        return ret;
    }

    return 0;
}

//Post一个FRAME数据到消息队列
template< class T>
int Transaction_Manager::mgr_postframe_to_msgqueue(
    unsigned int cmd,
    unsigned int qquin,
    const T &info,
    unsigned int option)
{
    SERVICES_ID rcvsvc = self_svc_id_;
    SERVICES_ID proxysvc(0, 0);
    SERVICES_ID sndsvc = self_svc_id_;
    return mgr_postframe_to_msgqueue(
               cmd,
               qquin,
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

