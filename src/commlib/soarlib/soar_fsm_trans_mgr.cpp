#include "soar_predefine.h"
#include "soar_zerg_frame.h"
#include "soar_fsm_trans_base.h"
#include "soar_zerg_svc_info.h"
#include "soar_zerg_frame_malloc.h"
#include "soar_fsm_trans_mgr.h"

/******************************************************************************************
struct CREATE_TRANS_RECORD 事务的统计信息
******************************************************************************************/
CREATE_TRANS_RECORD::CREATE_TRANS_RECORD(unsigned int trans_cmd ):
    trans_command_(trans_cmd),
    if_auto_trans_lock_(false),
    trans_lock_cmd_(0),
    create_trans_num_(0),
    destroy_right_num_(0),
    destroy_timeout_num_(0),
    destroy_exception_num_(0),
    trans_consume_time_(0)
{
}
CREATE_TRANS_RECORD::~CREATE_TRANS_RECORD()
{
};

/******************************************************************************************
struct TRANS_LOCK_RECORD 加锁的记录单元
******************************************************************************************/
TRANS_LOCK_RECORD::TRANS_LOCK_RECORD(unsigned int lock_qq_uin, unsigned int lock_trans_cmd):
    lock_qq_uin_(lock_qq_uin),
    lock_trans_cmd_(lock_trans_cmd)
{
}

TRANS_LOCK_RECORD::TRANS_LOCK_RECORD():
    lock_qq_uin_(0),
    lock_trans_cmd_(0)
{
}

TRANS_LOCK_RECORD::~TRANS_LOCK_RECORD()
{
}

/******************************************************************************************
class Transaction_Manager
******************************************************************************************/
Transaction_Manager *Transaction_Manager::instance_ = NULL;

Transaction_Manager::Transaction_Manager()
    : max_trans_(0)
    , trans_id_builder_(0)
    , self_svc_id_(0, 0)
    , timer_queue_(NULL)
    , zerg_mmap_pipe_(NULL)
    , statistics_clock_(NULL)
    , trans_send_buffer_ (NULL)
    , trans_recv_buffer_(NULL)
    , fake_recv_buffer_(NULL)
    , inner_frame_malloc_(NULL)
    , inner_message_queue_(NULL)
    , gen_trans_counter_(0)
    , cycle_gentrans_counter_(0)
{
}

//事务管理器的析构函数
Transaction_Manager::~Transaction_Manager()
{
    ZLOG_INFO("[framework] Transaction_Manager::~Transaction_Manager start.");

    //
    size_t trans_size  = transc_map_.size();
    ZLOG_INFO("[framework] close all transaction, transc_map_ size =%u", trans_size);

    //TMD在删除迭代器的这个阴沟里面翻了多少次船了，BS你的记忆力。
    for (size_t i = 0; i < trans_size; ++i)
    {
        //每次都从最开始的删除，第一个，你是被诅咒的。
        HASHMAP_OF_TRANSACTION::iterator mapiter = transc_map_.begin();
        Transaction_Base *pbase = mapiter->second;

        pbase->output_trans_info("[Application exit]:");
        //由于handle_close相当于将自己从transc_map_删除。所以每次都干掉第一个,
        pbase->handle_close();
    }

    //将内存池子里面的数据全部清理掉。好高兴，因为我释放了内存，从Inmoreliu那儿得到了一顿饭。
    HASHMAP_OF_POLLREGTRANS::iterator pooliter = regtrans_pool_map_.begin();
    HASHMAP_OF_POLLREGTRANS::iterator poolenditer = regtrans_pool_map_.end();

    for (; pooliter != poolenditer; ++pooliter)
    {
        unsigned int regframe_cmd = pooliter->first;
        POOL_OF_REGISTERTRANS &pool_reg_trans = (pooliter->second).crttrs_cmd_pool_;
        //记录信息数据
        ZLOG_INFO("[framework] Register command:%u size of pool:%u capacity of pool:%u.",
                  regframe_cmd,
                  pool_reg_trans.size(),
                  pool_reg_trans.capacity()
                 );

        //出现了问题，
        if (pool_reg_trans.size() != pool_reg_trans.capacity())
        {
            ZLOG_ERROR("[framework] Plase notice!! size[%u] != capacity[%u] may be exist memory leak.",
                       pool_reg_trans.size(),
                       pool_reg_trans.capacity());
        }

        //是否池子
        size_t pool_reg_trans_len = pool_reg_trans.size();

        for (size_t i = 0; i < pool_reg_trans_len; ++i)
        {
            Transaction_Base *ptsbase = NULL;
            pool_reg_trans.pop_front(ptsbase);

            delete ptsbase;
            ptsbase = NULL;
        }
    }

    //销毁内存分配器
    if (inner_frame_malloc_)
    {
        delete inner_frame_malloc_ ;
        inner_frame_malloc_ = NULL;
    }

    //销毁消息队列
    if (inner_message_queue_)
    {
        delete inner_message_queue_;
        inner_message_queue_ = NULL;
    }
}

void Transaction_Manager::initialize(size_t  szregtrans,
                                     size_t sztransmap,
                                     const SERVICES_ID &selfsvr,
                                     ZCE_Timer_Queue *timer_queue,
                                     Zerg_MMAP_BusPipe *zerg_mmap_pipe,
                                     unsigned int max_frame_len,
                                     bool init_inner_queue,
                                     bool init_lock_pool)
{

    ZCE_ASSERT(timer_queue != NULL);
    ZCE_ASSERT(zerg_mmap_pipe != NULL);

    regtrans_pool_map_.rehash(szregtrans + 128);
    transc_map_.rehash(sztransmap + 1024);
    max_trans_ = sztransmap;
    self_svc_id_ = selfsvr;
    timer_queue_ = timer_queue;
    zerg_mmap_pipe_ = zerg_mmap_pipe;

    trans_send_buffer_ = new (max_frame_len + 32) Zerg_App_Frame(CMD_INVALID_CMD, max_frame_len);
    trans_recv_buffer_ = new (max_frame_len + 32) Zerg_App_Frame(CMD_INVALID_CMD, max_frame_len);
    fake_recv_buffer_ = new (max_frame_len + 32) Zerg_App_Frame(CMD_INVALID_CMD, max_frame_len);

    //如果明确要求初始化内部的QUEUE,
    if (init_inner_queue)
    {
        inner_frame_malloc_ = new INNER_APPFRAME_MALLOCOR();
        inner_frame_malloc_->initialize(INIT_FRAME_MALLOC_NUMBER);

        inner_message_queue_ = new INNER_FRAME_MESSAGE_QUEUE(INNER_QUEUE_WATER_MARK);
        //inner_message_queue_->open(,INNER_QUEUE_WATER_MARK);
    }

    //初始化池子
    if (init_lock_pool)
    {
        //按照事务尺寸的一半初始化锁的数量
        trans_lock_pool_.rehash(sztransmap / 2);
    }
}


//注册命令以及对应的事务处理的类
int Transaction_Manager::register_trans_cmd(unsigned int cmd,
                                            Transaction_Base *ptxbase,
                                            bool if_auto_lock_trans,
                                            unsigned int lock_trans_cmd)
{
    //检查是否有重复的注册事务命令
#if defined DEBUG || defined _DEBUG
    HASHMAP_OF_POLLREGTRANS::iterator mapiter = regtrans_pool_map_.find(cmd);

    if (mapiter != regtrans_pool_map_.end())
    {
        ZLOG_ERROR("[framework] Find Repeat Command ID:%u From MAP.", cmd);
        delete ptxbase;
        return SOAR_RET::ERROR_FIND_REPEAT_CMD_ID;
    }

#endif

    CREATE_TRANS_RECORD ctr_trans_rec;
    ctr_trans_rec.trans_command_ = cmd;
    ctr_trans_rec.if_auto_trans_lock_ = if_auto_lock_trans;
    ctr_trans_rec.trans_lock_cmd_ = lock_trans_cmd;
    regtrans_pool_map_[cmd] = ctr_trans_rec;

    //初始化
    regtrans_pool_map_[cmd].crttrs_cmd_pool_.initialize(POOL_EXTEND_TRANSACTION_NUM);

    for (size_t i = 0; i < POOL_EXTEND_TRANSACTION_NUM; ++i)
    {
        Transaction_Base *tmp_txbase = ptxbase->create_self(timer_queue_, this);
        regtrans_pool_map_[cmd].crttrs_cmd_pool_.push_back(tmp_txbase);
    }

    //这个地方违背了谁申请，谁删除的原则，不好，但是
    delete ptxbase;

    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2006年4月3日
Function        : Transaction_Manager::regiester_trans_id
Return          : int ==0 表示成功,否则失败,可能是由于有重复的事务ID
Parameter List  :
  Param1: unsigned int transid    事务ID
  Param2: Transaction_Base* ptxbase 事务对应处理的Handler
Description     : 注册一个事务ID,以及事务ID对应处理的Handler
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Transaction_Manager::regiester_trans_id(unsigned int transid,
                                            unsigned int trans_cmd,
                                            Transaction_Base *ptxbase)
{
    //检查是否有重复的事务ID,由于transid是累加的，所以基本不会出现下段
#if defined DEBUG || defined _DEBUG
    HASHMAP_OF_TRANSACTION::iterator mapiter = transc_map_.find(transid);

    if (mapiter != transc_map_.end())
    {
        ZLOG_ERROR("[framework] Find Repeat Transaction ID:%u From MAP.", transid);
        return SOAR_RET::ERROR_FIND_REPEAT_TRANSACTION_ID;
    }

#endif

    //
    if (statistics_clock_)
    {
        ptxbase->trans_create_time_ = statistics_clock_->sec();
        ++(regtrans_pool_map_[trans_cmd].create_trans_num_);
    }

    transc_map_[transid] = ptxbase;

    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2006年4月3日
Function        : Transaction_Manager::unregiester_trans_id
Return          : int 表示成功,否则失败,可能是由于有重复的事务ID
Parameter List  :
  Param1: unsigned int transid 事务ID
Description     : 注销TransID.
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Transaction_Manager::unregiester_trans_id(unsigned int transid,
                                              unsigned int trans_cmd,
                                              int run_state,
                                              time_t trans_start)
{
    //检查是否有能找到对应的事件ID

    HASHMAP_OF_TRANSACTION::iterator mapiter = transc_map_.find(transid);

    if (mapiter == transc_map_.end())
    {
        ZLOG_ERROR("[framework] unregiester_trans_id,Can't Find Transaction ID:%u From MAP.", transid);
        return SOAR_RET::ERROR_CANNOT_FIND_TRANSACTION_ID;
    }

    //如果要进行统计
    if (statistics_clock_)
    {
        switch (run_state)
        {
            case Transaction_Base::INIT_RUN_STATE:
            case Transaction_Base::RIGHT_RUN_STATE:
                ++(regtrans_pool_map_[trans_cmd].destroy_right_num_);
                break;

            case Transaction_Base::TIMEOUT_RUN_STATE:
                ++(regtrans_pool_map_[trans_cmd].destroy_timeout_num_) ;
                break;

            case Transaction_Base::EXCEPTION_RUN_STATE:
            default:
                ++(regtrans_pool_map_[trans_cmd].destroy_exception_num_);
                break;
        }

        //统计耗费的时间
        (regtrans_pool_map_[trans_cmd].trans_consume_time_) += (statistics_clock_->sec() - trans_start);
    }

    //在erase之前保存指针
    Transaction_Base *rt_tsbase = mapiter->second;

    transc_map_.erase(transid);

    //返回池子
    return_clone_to_pool(trans_cmd, rt_tsbase);

    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年1月9日
Function        : Transaction_Manager::process_pipe_frame
Return          : int
Parameter List  :
  Param1: size_t& proc_frame
  Param2: size_t& create_trans
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Transaction_Manager::process_pipe_frame(size_t &proc_frame, size_t &create_trans)
{
    int ret = 0;
    create_trans = 0;

    Zerg_App_Frame *tmp_frame = reinterpret_cast<Zerg_App_Frame *>(trans_recv_buffer_);

    for (proc_frame = 0; zerg_mmap_pipe_->is_empty_bus(Zerg_MMAP_BusPipe::RECV_PIPE_ID) == false && proc_frame < MAX_ONCE_PROCESS_FRAME ;  ++proc_frame)
    {
        //
        ret = zerg_mmap_pipe_->pop_front_recvpipe(tmp_frame);

        if (ret !=  0)
        {
            return 0;
        }

        DEBUGDUMP_FRAME_HEAD(tmp_frame, "FROM RECV PIPE FRAME", RS_DEBUG);

        //是否创建一个事务，
        bool bcrtcx = false;
        //tmp_frame不用回收
        ret = process_appframe(tmp_frame, bcrtcx);

        //
        if (ret !=  0)
        {
            continue;
        }

        //创建了一个事务
        if ( true == bcrtcx )
        {
            ++create_trans;
        }
    }

    //
    return 0;
}

//将数据放入发送管道
int Transaction_Manager::push_back_sendpipe(Zerg_App_Frame *proc_frame)
{
    //Zerg_MMAP_BusPipe必须先初始化....
    return zerg_mmap_pipe_->push_back_sendpipe(proc_frame);
}

//
/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年1月9日
Function        : Transaction_Manager::get_clone_from_pool
Return          : int
Parameter List  :
  Param1: unsigned int frame_cmd     克隆事务的命令字
  Param2: unsigned int qq_uin,       使用的用户的QQ号码
  Param3: Transaction_Base*& crt_trans 取走的事务的克隆
Description     : 从池子中间根据命令得到一个Trans的克隆
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Transaction_Manager::get_clone_from_pool(unsigned int frame_cmd,
                                             unsigned int qq_uin,
                                             Transaction_Base *&crt_trans)
{
    int ret = 0;

    HASHMAP_OF_POLLREGTRANS::iterator mapiter = regtrans_pool_map_.find(frame_cmd);

    if (mapiter == regtrans_pool_map_.end())
    {
        //如果是一个rsp包，找不到是正常的 我们一般用偶数做rsp的命令字，奇数做req的命令字
        //为什么在这儿不输出日志呢，因为有些REQ中间作为事务的应答,在后面进行统一的日志输出

        return SOAR_RET::ERROR_TRANS_NO_CLONE_COMMAND;
    }

    CREATE_TRANS_RECORD &reg_ctr_trans = regtrans_pool_map_[frame_cmd];

    //如果这个命令要进行加锁
    if (reg_ctr_trans.if_auto_trans_lock_)
    {
        //进行加锁,
        ret = lock_qquin_trnas_cmd(qq_uin,
                                   reg_ctr_trans.trans_lock_cmd_,
                                   frame_cmd);

        if (0 != ret )
        {
            return ret;
        }
    }

    ZLOG_DEBUG("Get clone frame command [%u],QQUin[%u],Pool size=[%u].",
               frame_cmd,
               qq_uin,
               reg_ctr_trans.crttrs_cmd_pool_.size());

    //还有最后一个
    if (reg_ctr_trans.crttrs_cmd_pool_.size() == 1)
    {
        ZLOG_INFO("[framework] Before extend trans.");
        //取一个模型
        Transaction_Base *model_trans = NULL;
        reg_ctr_trans.crttrs_cmd_pool_.pop_front(model_trans);

        size_t capacity_of_pool = reg_ctr_trans.crttrs_cmd_pool_.capacity();
        reg_ctr_trans.crttrs_cmd_pool_.resize(capacity_of_pool + POOL_EXTEND_TRANSACTION_NUM);

        ZLOG_INFO("[framework] Pool Size=%u,  command %u, capacity = %u , resize =%u .",
                  reg_ctr_trans.crttrs_cmd_pool_.size(),
                  frame_cmd,
                  capacity_of_pool,
                  capacity_of_pool + POOL_EXTEND_TRANSACTION_NUM);

        //用模型克隆N-1个Trans
        for (size_t i = 0; i < POOL_EXTEND_TRANSACTION_NUM; ++i)
        {
            Transaction_Base *cloned_txbase = model_trans->create_self(timer_queue_, this);
            reg_ctr_trans.crttrs_cmd_pool_.push_back(cloned_txbase);
        }

        //将模型放到第N个
        reg_ctr_trans.crttrs_cmd_pool_.push_back(model_trans);
        ZLOG_INFO("[framework] After Extend trans.");
    }

    //取得一个事务
    reg_ctr_trans.crttrs_cmd_pool_.pop_front(crt_trans);
    //初始化丫的
    crt_trans->re_init();

    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年1月21日
Function        : Transaction_Manager::return_clone_to_pool
Return          : int
Parameter List  :
  Param1: unsigned int frame_cmd     克隆事务的命令字
  Param2: Transaction_Base*& rt_txbase 归还的克隆的事务
Description     : 向池子中间根据命令归还一个Trans的克隆
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Transaction_Manager::return_clone_to_pool(unsigned int frame_cmd, Transaction_Base *&rt_txbase)
{
#if defined DEBUG || defined _DEBUG
    HASHMAP_OF_POLLREGTRANS::iterator mapiter = regtrans_pool_map_.find(frame_cmd);

    if (mapiter == regtrans_pool_map_.end())
    {
        return SOAR_RET::ERROR_TRANS_NO_CLONE_COMMAND;
    }

#endif //defined DEBUG || defined _DEBUG
    //
    CREATE_TRANS_RECORD &reg_ctr_trans  = regtrans_pool_map_[frame_cmd];
    ZLOG_DEBUG("[framework] Return clone frame command %u,Pool size=%u .",
               frame_cmd,
               reg_ctr_trans.crttrs_cmd_pool_.size());

    //如果原来要求加锁，现在就解锁
    if (reg_ctr_trans.if_auto_trans_lock_)
    {
        unlock_qquin_trans_cmd(rt_txbase->req_qq_uin_,
                               reg_ctr_trans.trans_lock_cmd_);
    }

    //用于资源的回收
    rt_txbase->finish();

    //
    reg_ctr_trans.crttrs_cmd_pool_.push_back(rt_txbase);
    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008年1月21日
Function        : Transaction_Manager::recycle_clone_from_pool
Return          : void
Parameter List  : NULL
Description     : 从池子中间回收多于的事务的克隆，如果克隆过多会占用过多的内存，安装限制的事务模型，
Calls           :
Called By       :
Other           : 克隆的事务应该不用太多
Modify Record   :
******************************************************************************************/
void Transaction_Manager::recycle_clone_from_pool()
{
    //
    HASHMAP_OF_POLLREGTRANS::iterator iter_tmp = regtrans_pool_map_.begin();
    HASHMAP_OF_POLLREGTRANS::iterator iter_end = regtrans_pool_map_.end();
    //因为比较关键，用了RS_INFO
    ZLOG_INFO("[framework] Recycle trans,transaction manager are processing pool number [%d] . ",
              regtrans_pool_map_.size());

    for (unsigned int i = 1; iter_tmp != iter_end ; ++iter_tmp, ++i)
    {
        unsigned int frame_command = iter_tmp->first;
        POOL_OF_REGISTERTRANS &pool_regtrans = (iter_tmp->second).crttrs_cmd_pool_;
        ZLOG_INFO("[framework] %u.Pool porcess command:%u,capacity:%u,size:%u,use:%u.",
                  i,
                  frame_command,
                  pool_regtrans.capacity(),
                  pool_regtrans.size(),
                  pool_regtrans.capacity() - pool_regtrans.size()
                 );

        //如果池子的空间 > RECYCLE_POOL_THRESHOLD_VALUE 并且正在使用的事务克隆很少
        //则调整池子，回收部分数据，减少浪费
        const size_t USE_TRANS_THRESHOLD = RECYCLE_POOL_THRESHOLD_VALUE / 8;

        if (pool_regtrans.capacity() - pool_regtrans.size() < USE_TRANS_THRESHOLD &&
            pool_regtrans.size() > RECYCLE_POOL_THRESHOLD_VALUE )
        {
            const size_t RECYCLE_TRANS_NUM = RECYCLE_POOL_THRESHOLD_VALUE / 2;
            size_t pool_capacity = pool_regtrans.capacity();

            //循环取得一个事务，删除掉
            for (size_t i = 0; i < RECYCLE_TRANS_NUM; ++i)
            {
                Transaction_Base *recycle_txbase = NULL;
                pool_regtrans.pop_front(recycle_txbase);
                delete recycle_txbase;
            }

            //调整池子的容量
            pool_regtrans.resize(pool_capacity - RECYCLE_TRANS_NUM);
        }
    }

    //
    return ;
}

/******************************************************************************************
Author          : Sail(ZENGXING)  Date Of Creation: 2009年3月16日
Function        : Transaction_Manager::lock_qquin_trnas_cmd
Return          : int
Parameter List  :
  Param1: unsigned int qq_uin        QQUIN
  Param2: unsigned int trnas_lock_id 加锁的ID,可以和命令字相同，或者不同
  Param3: unsigned int frame_cmd     事务的命令，仅仅用于日志输出
Description     : 对某一个用户的一个命令的事务进行加锁
Calls           :
Called By       : 事务锁的意思是保证一个时刻，只能一个这样的事务,事务锁不阻塞
Other           :
Modify Record   :
******************************************************************************************/
int Transaction_Manager::lock_qquin_trnas_cmd(unsigned int qq_uin,
                                              unsigned int trnas_lock_id,
                                              unsigned int frame_cmd)
{
    TRANS_LOCK_RECORD lock_rec(qq_uin, trnas_lock_id);
    std::pair <INNER_TRANS_LOCK_POOL::iterator, bool> iter_tmp = trans_lock_pool_.insert(lock_rec);

    //如果已经有一个锁了，那么加锁失败
    if (false == iter_tmp.second )
    {
        ZLOG_ERROR("[framework] [LOCK]Oh!Transaction lock fail.QQUin[%u] trans lock id[%u] trans cmd[%u].",
                   qq_uin,
                   trnas_lock_id,
                   frame_cmd);
        return -1;
    }

    return 0;
}

//对某一个用户的一个命令的事务进行加锁
void Transaction_Manager::unlock_qquin_trans_cmd(unsigned int qq_uin, unsigned int lock_trnas_id)
{
    TRANS_LOCK_RECORD lock_rec(qq_uin, lock_trnas_id);
    trans_lock_pool_.erase(lock_rec);
    return;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2006年4月22日
Function        : Transaction_Manager::process_appframe
Return          : int
Parameter List  :
  Param1: const Zerg_App_Frame* ppetappframe 处理的事务的帧数据，ppetappframe帧的生命周期由process_appframe函数管理
  Param3: bool& bcrttx                是否创建事务
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Transaction_Manager::process_appframe( Zerg_App_Frame *ppetappframe, bool &bcrttx)
{
    bcrttx = false;
    int ret = 0;

    //如果是跟踪命令，打印出来
    if (ppetappframe->frame_option_ & Zerg_App_Frame::DESC_MONITOR_TRACK)
    {
        Zerg_App_Frame::dumpoutput_framehead(ppetappframe, "[TRACK MONITOR][TRANS PROCESS]", RS_INFO);
    }

    Transaction_Base *crt_trans = NULL;
    ret = get_clone_from_pool(ppetappframe->frame_command_,
                              ppetappframe->frame_uid_,
                              crt_trans);

    //是一个激活事务的命令
    if (ret == 0)
    {

        //跳过0，认为错误的Transaction ID默认为0,所以这样有好处,容易发现错误.
        if (++trans_id_builder_ == 0)
        {
            ++trans_id_builder_;
        }

        ret = regiester_trans_id(trans_id_builder_, ppetappframe->frame_command_, crt_trans);

        //创建了一个事务
        if (ret != 0)
        {
            //
            return_clone_to_pool(ppetappframe->frame_command_, crt_trans);
            return ret;
        }

        ret = crt_trans->initialize_trans(ppetappframe, trans_id_builder_);

        //注意这儿销毁用的是handle_close
        if (ret != 0)
        {
            crt_trans->handle_close();
            return ret;
        }

        bcrttx = true;

        //统计技术器
        ++gen_trans_counter_;
        ++cycle_gentrans_counter_;

        ZLOG_DEBUG("Create Trascation ,Command:%u Transaction ID:%u .",
                   ppetappframe->frame_command_, trans_id_builder_);
    }
    else
    {
        //根据事务ID寻找事务,改成用回填的事务ID查询
        Transaction_Base *run_tans = NULL;
        ret = get_handler_by_transid(ppetappframe->backfill_trans_id_,
                                     ppetappframe->frame_command_,
                                     run_tans);

        if (ret != 0 )
        {
            return ret;
        }

        ZLOG_DEBUG("Find raw Transaction ID: %u. ", ppetappframe->backfill_trans_id_);

        //检查受到的FRAME的数据和信息
        ret = run_tans->check_receive_frame(ppetappframe);

        if (ret != 0 )
        {
            return ret;
        }

        //收到一个数据，进行处理。
        ret = run_tans->receive_trans_msg(ppetappframe);

        if (ret != 0 )
        {
            run_tans->handle_close();
            return ret;
        }
    }

    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2006年4月3日
Function        : Transaction_Manager::get_handler_by_transid
Return          : int
Parameter List  :
  Param1: unsigned int transid     事务ID
  Param1: unsigned int trans_cmd   按照inmore的要求，增加了一个CMD，用于无法发现时答应
  Param2: Transaction_Base*& ptxbase 返回的Handler指针
Description     : 根据事务ID寻找事务
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Transaction_Manager::get_handler_by_transid(unsigned int transid, unsigned int trans_cmd, Transaction_Base *&ptxbase)
{
    //根据事务ID寻找事务
    HASHMAP_OF_TRANSACTION::iterator mapiter = transc_map_.find(transid);

    if (mapiter == transc_map_.end())
    {
        ZLOG_INFO("[framework] get_handler_by_transid,Can't Find Transaction ID:%u,Command:%u From MAP.",
                  transid,
                  trans_cmd);
        return SOAR_RET::ERROR_CANNOT_FIND_TRANSACTION_ID;
    }

    ptxbase = mapiter->second;

    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2007年11月14日
Function        : Transaction_Manager::dump_all_trans_info
Return          : void
Parameter List  : NULL
Description     : Dump得到实例
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
void Transaction_Manager::dump_all_trans_info() const
{
    //
    HASHMAP_OF_TRANSACTION::const_iterator iter_tmp = transc_map_.begin();
    HASHMAP_OF_TRANSACTION::const_iterator iter_end = transc_map_.end();
    //因为比较关键，用了RS_INFO
    ZLOG_INFO("[framework] Transaction Manager are processing [%d] transactions. ", transc_map_.size());

    for (unsigned int i = 1; iter_tmp != iter_end ; ++iter_tmp, ++i)
    {
        Transaction_Base *pbase = iter_tmp->second;
        ZLOG_INFO("[framework] %u.Transaction ID:%u,Request UIN:%u,Command:%u,State:%u.",
                  i,
                  (pbase)->req_qq_uin_,
                  (pbase)->req_qq_uin_,
                  (pbase)->trans_command_,
                  (pbase)->trans_phase_,
                  (pbase)->transaction_id_ );
    }

    //
    return ;
}

void Transaction_Manager::dump_trans_pool_info() const
{
    //
    HASHMAP_OF_POLLREGTRANS::const_iterator iter_tmp = regtrans_pool_map_.begin();
    HASHMAP_OF_POLLREGTRANS::const_iterator iter_end = regtrans_pool_map_.end();
    //因为比较关键，用了RS_INFO
    ZLOG_INFO("[framework] Transaction Manager are processing pool number [%d] . ", regtrans_pool_map_.size());

    for (unsigned int i = 1; iter_tmp != iter_end ; ++iter_tmp, ++i)
    {
        unsigned int frame_command = iter_tmp->first;
        const POOL_OF_REGISTERTRANS &pool_regtrans = (iter_tmp->second).crttrs_cmd_pool_;
        ZLOG_INFO("[framework] %u.Pool porcess command:%u,capacity:%u,size:%u.",
                  i,
                  frame_command,
                  pool_regtrans.capacity(),
                  pool_regtrans.size()
                 );
    }

    //
    return ;
}

//DUMP所有的统计信息
void Transaction_Manager::dump_statistics_info() const
{
    //
    HASHMAP_OF_POLLREGTRANS::const_iterator iter_tmp = regtrans_pool_map_.begin();
    HASHMAP_OF_POLLREGTRANS::const_iterator iter_end = regtrans_pool_map_.end();

    ZLOG_INFO("[framework] [TRANS INFO] All generate transaction counter [%llu] ,previous cycle generate transaction number[%llu].",
              gen_trans_counter_,
              cycle_gentrans_counter_);

    //因为比较关键，用了RS_INFO
    ZLOG_INFO("[framework] Transaction Manager are processing [%d] transactions. ", transc_map_.size());

    for (unsigned int i = 1; iter_tmp != iter_end ; ++iter_tmp, ++i)
    {
        ZLOG_INFO("[framework] [TRANS INFO]%u.Transaction command ID [%u],create [%llu], destroy right[%llu], destroy timeout[%llu],destroy exception[%llu],consume seconds[%llu]",
                  i,
                  (iter_tmp->second).trans_command_,
                  (iter_tmp->second).create_trans_num_,
                  (iter_tmp->second).destroy_right_num_,
                  (iter_tmp->second).destroy_timeout_num_,
                  (iter_tmp->second).destroy_exception_num_,
                  (iter_tmp->second).trans_consume_time_
                 );
    }
}

void Transaction_Manager::dump_all_debug_info() const
{
    dump_trans_pool_info();
    dump_statistics_info();
    dump_all_trans_info();
}

//管理器发送一消息头给一个服务器,_表示他是一个内部函数，不提供给非相关人士使用
int Transaction_Manager::mgr_sendmsghead_to_service(unsigned int cmd,
                                                    unsigned int qquin,
                                                    const SERVICES_ID &rcvsvc,
                                                    const SERVICES_ID &proxysvc,
                                                    unsigned int backfill_trans_id,
                                                    unsigned int app_id,
                                                    unsigned int option)
{
    //
    Zerg_App_Frame *rsp_msg = reinterpret_cast<Zerg_App_Frame *>(trans_send_buffer_);
    rsp_msg->init_framehead(Zerg_App_Frame::MAX_LEN_OF_APPFRAME);

    rsp_msg->frame_length_ = Zerg_App_Frame::LEN_OF_APPFRAME_HEAD;
    rsp_msg->frame_command_ = cmd;
    rsp_msg->frame_uid_ = qquin;

    rsp_msg->transaction_id_ = 0;
    rsp_msg->recv_service_ = rcvsvc;
    rsp_msg->proxy_service_ = proxysvc;
    rsp_msg->send_service_ =  this->self_svc_id_;
    rsp_msg->frame_option_ = option;

    //回填事务ID
    rsp_msg->backfill_trans_id_ = backfill_trans_id;
    rsp_msg->app_id_ = app_id;

    return push_back_sendpipe(rsp_msg);
}

//打开性能统计
void Transaction_Manager::enable_trans_statistics (const ZCE_Time_Value *stat_clock)
{
    statistics_clock_ = stat_clock;
}

int Transaction_Manager::mgr_postframe_to_msgqueue(Zerg_App_Frame *post_frame)
{
    int ret = 0;
    Zerg_App_Frame *tmp_frame = NULL;

    //如果是从池子中间取出的FRAME，就什么都不做

    inner_frame_malloc_->clone_appframe(post_frame, tmp_frame);

    //理论上不用等待任何时间
    ret = inner_message_queue_->enqueue(tmp_frame);

    //返回值小于0表示失败
    if (ret < 0)
    {
        ZLOG_DEBUG("Post message to send queue fail.ret =%d"
                   "Send queue message_count:%u message_bytes:%u. ",
                   ret,
                   inner_message_queue_->size(),
                   inner_message_queue_->size() * sizeof(Zerg_App_Frame *));
        //出错了以后还回去
        inner_frame_malloc_->free_appframe(tmp_frame);

        return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_ENQUEUE_FAIL;
    }

    return 0;
}

//处理从接收队列取出的FRAME
int Transaction_Manager::process_queue_frame(size_t &proc_frame, size_t &create_trans)
{
    int ret = 0;
    create_trans = 0;

    //处理队列
    for (proc_frame = 0; inner_message_queue_->empty() == false && proc_frame < MAX_ONCE_PROCESS_FRAME ;  ++proc_frame)
    {

        Zerg_App_Frame *tmp_frame = NULL;
        //
        ret = inner_message_queue_->dequeue(tmp_frame);

        //如果小于0表示错误，到这个地方应该是一个错误，因为上面还有一个判断
        if (ret < 0)
        {
            ZLOG_ERROR("[framework] Recv queue dequeue fail ,ret=%u,", ret);
            return 0;
        }

        DEBUGDUMP_FRAME_HEAD(tmp_frame, "FROM RECV QUEUE FRAME:", RS_DEBUG);

        //是否创建一个事务，
        bool bcrtcx = false;

        //tmp_frame  马上回收
        ret = process_appframe(tmp_frame, bcrtcx);
        //释放内存
        inner_frame_malloc_->free_appframe(tmp_frame);

        //
        if (ret !=  0)
        {
            continue;
        }

        //创建了一个事务
        if ( true == bcrtcx )
        {
            ++create_trans;
        }
    }

    //
    return 0;
}

//得到管理器的负载参数
void Transaction_Manager::get_manager_load_foctor(unsigned int &load_max, unsigned int &load_cur)
{
    load_max = static_cast<unsigned int>(max_trans_);
    load_cur = static_cast<unsigned int>( transc_map_.size());

    //负载人数必须大于1
    if (load_cur == 0)
    {
        load_cur = 1;
    }
}

//得到管理器的负载参数
//有一些服务器，没有阶段性的事务，用上面的函数不是特别理想，
void Transaction_Manager::get_manager_load_foctor2(unsigned int &load_max, unsigned int &load_cur)
{
    const unsigned int ONE_CYCLE_GENERATE_TRANS = 30000;

    //得到负载的数量
    load_max = ONE_CYCLE_GENERATE_TRANS;

    if (cycle_gentrans_counter_ > ONE_CYCLE_GENERATE_TRANS)
    {
        load_cur = ONE_CYCLE_GENERATE_TRANS;
    }
    else
    {
        load_cur = cycle_gentrans_counter_;
    }

    //周期计数器清零
    cycle_gentrans_counter_ = 0;

    //负载人数必须大于1
    if (load_cur == 0)
    {
        load_cur = 1;
    }
}

//得到实例
Transaction_Manager *Transaction_Manager::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Transaction_Manager();
    }

    return instance_;
}

//实例赋值
void Transaction_Manager::instance(Transaction_Manager *pinstatnce)
{
    clean_instance();
    instance_ = pinstatnce;
    return;
}

//清除实例
void Transaction_Manager::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }

    return;
}

//直接发送一个buffer to services。
int Transaction_Manager::mgr_sendbuf_to_service(unsigned int cmd,
                                                unsigned int qquin,
                                                unsigned int trans_id,
                                                unsigned int backfill_trans_id,
                                                const SERVICES_ID &rcvsvc,
                                                const SERVICES_ID &proxysvc,
                                                const SERVICES_ID &sndsvc,
                                                const unsigned char *buf,
                                                size_t buf_len,
                                                unsigned int app_id,
                                                unsigned int option )
{
    return zerg_mmap_pipe_->pipe_sendbuf_to_service(cmd,
                                                    qquin,
                                                    trans_id,
                                                    backfill_trans_id,
                                                    rcvsvc,
                                                    proxysvc,
                                                    sndsvc,
                                                    buf,
                                                    buf_len,
                                                    app_id,
                                                    option);
}

