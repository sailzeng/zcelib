#include "soar/predefine.h"
#include "soar/zerg/frame_zerg.h"
#include "soar/fsm/fsm_base.h"
#include "soar/zerg/services_info.h"
#include "soar/zerg/frame_malloc.h"
#include "soar/fsm/fsm_mgr.h"


/******************************************************************************************
class Transaction_Manager
******************************************************************************************/
FSM_Manager *FSM_Manager::instance_ = NULL;

FSM_Manager::FSM_Manager()
{
}

//事务管理器的析构函数
FSM_Manager::~FSM_Manager()
{
}

int FSM_Manager::initialize(zce::Timer_Queue_Base *timer_queue,
                            size_t  reg_fsm_num,
                            size_t running_fsm_num,
                            const soar::SERVICES_INFO &selfsvr,
                            Soar_MMAP_BusPipe *zerg_mmap_pipe,
                            size_t max_frame_len,
                            bool init_inner_queue,
                            bool init_lock_pool)
{
    ZCE_TRACE_FILELINE(RS_INFO);

    ZCE_ASSERT(timer_queue != NULL);
    ZCE_ASSERT(zerg_mmap_pipe != NULL);

    int ret = 0;
    ret = ZCE_Async_FSMMgr::initialize(timer_queue,
                                       reg_fsm_num,
                                       running_fsm_num);
    if (ret != 0)
    {
        return ret;
    }

    self_svc_info_ = selfsvr;
    zerg_mmap_pipe_ = zerg_mmap_pipe;

    trans_send_buffer_ = soar::Zerg_Frame::new_frame(max_frame_len + 32);
    trans_send_buffer_->init_head(max_frame_len,CMD_INVALID_CMD);
    trans_recv_buffer_ = soar::Zerg_Frame::new_frame(max_frame_len + 32);
    trans_recv_buffer_->init_head(max_frame_len,CMD_INVALID_CMD);
    fake_recv_buffer_ = soar::Zerg_Frame::new_frame(max_frame_len + 32);
    fake_recv_buffer_->init_head(max_frame_len,CMD_INVALID_CMD);

    //如果明确要求初始化内部的QUEUE,
    if (init_inner_queue)
    {
        inner_frame_mallocor_ = new Inner_Frame_Mallocor();
        inner_frame_mallocor_->initialize(INIT_FRAME_MALLOC_NUMBER);

        message_queue_ = new Inner_Frame_Queue(MAX_QUEUE_NODE_NUMBER);
        //inner_message_queue_->open(,INNER_QUEUE_WATER_MARK);
    }
    if (init_lock_pool)
    {
        //按照事务尺寸进行初始化，
        only_one_lock_pool_ = new ONLY_ONE_LOCK_POOL();
        only_one_lock_pool_->rehash(running_fsm_num / 5 + 32);
    }
    return 0;
}

//
void FSM_Manager::finish()
{
    //销毁内存分配器
    if (inner_frame_mallocor_)
    {
        delete inner_frame_mallocor_;
        inner_frame_mallocor_ = NULL;
    }
    //销毁消息队列
    if (message_queue_)
    {
        delete message_queue_;
        message_queue_ = NULL;
    }
    if (trans_send_buffer_)
    {
        soar::Zerg_Frame::delete_frame(trans_send_buffer_);
        trans_send_buffer_ = NULL;
    }
    if (trans_recv_buffer_)
    {
        soar::Zerg_Frame::delete_frame(trans_recv_buffer_);
        trans_recv_buffer_ = NULL;
    }
    if (fake_recv_buffer_)
    {
        soar::Zerg_Frame::delete_frame(fake_recv_buffer_);
        fake_recv_buffer_ = NULL;
    }

    ZCE_Async_FSMMgr::finish();

    ZCE_TRACE_FILELINE(RS_INFO);
}


/******************************************************************************************
Author          : Sailzeng <sailzeng.cn@gmail.com>  Date Of Creation: 2008年1月9日
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
int FSM_Manager::process_pipe_frame(size_t &proc_frame,size_t &create_trans)
{
    int ret = 0;
    create_trans = 0;

    soar::Zerg_Frame *tmp_frame = reinterpret_cast<soar::Zerg_Frame *>(trans_recv_buffer_);

    for (proc_frame = 0; zerg_mmap_pipe_->is_empty_bus(Soar_MMAP_BusPipe::RECV_PIPE_ID) == false && proc_frame < MAX_ONCE_PROCESS_FRAME; ++proc_frame)
    {
        //
        ret = zerg_mmap_pipe_->pop_front_recvpipe(tmp_frame);

        if (ret != 0)
        {
            return 0;
        }

        DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG,"FROM RECV PIPE FRAME",tmp_frame);

        //是否创建一个事务，
        bool bcrtcx = false;
        //tmp_frame不用回收
        ret = process_appframe(tmp_frame,bcrtcx);

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



//处理一个收到的命令
int FSM_Manager::process_appframe(soar::Zerg_Frame *zerg_frame,bool &bcrttx)
{
    bcrttx = false;
    int ret = 0;

    //如果是跟踪命令，打印出来
    if (zerg_frame->frame_option_.option_ & soar::Zerg_Frame::DESC_MONITOR_TRACK)
    {
        DUMP_ZERG_FRAME_HEAD(RS_INFO,"[TRACK MONITOR][TRANS PROCESS]",zerg_frame);
    }


    bool is_reg_cmd = is_register_cmd(zerg_frame->command_);

    //是一个激活事务的命令
    if (is_reg_cmd)
    {

        unsigned int id = 0;
        ret = create_asyncobj(zerg_frame->command_,zerg_frame,&id);

        bcrttx = true;

        //统计技术器
        ++gen_trans_counter_;
        ++cycle_gentrans_counter_;

        ZCE_LOG(RS_DEBUG,"Create Trascation ,Command:%u Transaction ID:%u .",
                zerg_frame->command_,id);
    }
    else
    {

        ret = active_asyncobj(zerg_frame->backfill_fsm_id_,zerg_frame);
        if (ret != 0)
        {
            DUMP_ZERG_FRAME_HEAD(RS_ERROR,"No use frame:",zerg_frame);
            return ret;
        }

        ZCE_LOG(RS_DEBUG,"Find raw Transaction ID: %u. ",zerg_frame->backfill_fsm_id_);
    }
    return 0;
}


//直接发送一个buffer to services。
int FSM_Manager::sendfame_to_pipe(const soar::Zerg_Frame* send_frame)
{
    return zerg_mmap_pipe_->push_back_sendpipe(send_frame);
}

//打开性能统计
void FSM_Manager::enable_trans_statistics(const ZCE_Time_Value *stat_clock)
{
    statistics_clock_ = stat_clock;
}


int FSM_Manager::sendbuf_to_pipe(const soar::Zerg_Head& zerg_head,
                                 const char* buf,
                                 size_t buf_len)
{
    soar::Zerg_Frame* send_frame = trans_send_buffer_;
    send_frame->init_head(soar::Zerg_Frame::MAX_LEN_OF_APPFRAME);
    ::memcpy(send_frame,&zerg_head,soar::Zerg_Frame::LEN_OF_APPFRAME_HEAD);
    if (buf_len)
    {
        ::memcpy(send_frame->frame_appdata_,
                 buf,
                 buf_len);
    }
    send_frame->length_ += buf_len;
    sendfame_to_pipe(send_frame);
    return 0;
}


int FSM_Manager::postmsg_to_queue(soar::Zerg_Frame *post_frame)
{
    int ret = 0;
    soar::Zerg_Frame *tmp_frame = NULL;

    //如果是从池子中间取出的FRAME，就什么都不做
    inner_frame_mallocor_->clone_appframe(post_frame,tmp_frame);

    //理论上不用等待任何时间
    ret = message_queue_->enqueue(tmp_frame);

    //返回值小于0表示失败
    if (ret < 0)
    {
        ZCE_LOG(RS_DEBUG,"Post message to send queue fail.ret =%d"
                "Send queue message_count:%u message_bytes:%u. ",
                ret,
                message_queue_->size(),
                message_queue_->size() * sizeof(soar::Zerg_Frame *));
        //出错了以后还回去
        inner_frame_mallocor_->free_appframe(tmp_frame);

        return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_ENQUEUE_FAIL;
    }

    return 0;
}

//处理从接收队列取出的FRAME
int FSM_Manager::process_queue_frame(size_t &proc_frame,size_t &create_trans)
{
    int ret = 0;
    create_trans = 0;

    //处理队列
    for (proc_frame = 0; message_queue_->empty() == false && proc_frame < MAX_ONCE_PROCESS_FRAME; ++proc_frame)
    {

        soar::Zerg_Frame *tmp_frame = NULL;
        //
        ret = message_queue_->dequeue(tmp_frame);

        //如果小于0表示错误，到这个地方应该是一个错误，因为上面还有一个判断
        if (ret < 0)
        {
            ZCE_LOG(RS_ERROR,"[framework] Recv queue dequeue fail ,ret=%u,",ret);
            return 0;
        }

        DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG,"FROM RECV QUEUE FRAME:",tmp_frame);

        //是否创建一个事务，
        bool bcrtcx = false;
        //tmp_frame  马上回收
        ret = process_appframe(tmp_frame,bcrtcx);
        //释放内存
        inner_frame_mallocor_->free_appframe(tmp_frame);

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

//得到管理器的负载参数
//void Transaction_Manager::get_manager_load_foctor(unsigned int &load_max, unsigned int &load_cur)
//{
//    load_max = static_cast<unsigned int>(max_trans_);
//    load_cur = static_cast<unsigned int>( transc_map_.size());
//
//    //负载人数必须大于1
//    if (load_cur == 0)
//    {
//        load_cur = 1;
//    }
//}

//得到管理器的负载参数
//有一些服务器，没有阶段性的事务，用上面的函数不是特别理想，
void FSM_Manager::get_manager_load_foctor2(unsigned int &load_max,unsigned int &load_cur)
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


// recv_svr填的是自己，就假装收到一个包，如其名fake
int FSM_Manager::fake_receive_frame(const soar::Zerg_Frame* fake_recv)
{
    int ret = 0;

    soar::Zerg_Frame* tmp_frame = reinterpret_cast<soar::Zerg_Frame*>(fake_recv_buffer_);
    size_t buff_size = fake_recv->length_;
    memcpy(tmp_frame->frame_appdata_,fake_recv,buff_size);

    bool crttx = false;
    ret = process_appframe(tmp_frame,crttx);

    if (ret != 0 && ret != SOAR_RET::ERROR_TRANS_HAS_FINISHED)
    {
        return ret;
    }
    return 0;
}

//得到实例
FSM_Manager *FSM_Manager::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new FSM_Manager();
    }

    return instance_;
}



//对某一个用户的一个命令的事务进行加锁
int FSM_Manager::lock_only_one(uint32_t cmd,
                               uint32_t lock_id)
{
    ONLYONE_LOCK lock_rec = {cmd,lock_id};
    auto iter_tmp =
        only_one_lock_pool_->insert(lock_rec);

    //如果已经有一个锁了，那么加锁失败
    if (false == iter_tmp.second)
    {
        ZCE_LOG(RS_ERROR,"[framework] [LOCK]Oh!Transaction lock fail.cmd[%u] trans lock id[%u].",
                cmd,
                lock_id);
        return -1;
    }

    return 0;
}

//对某一个用户的一个命令的事务进行加锁
void FSM_Manager::unlock_only_one(uint32_t cmd,
                                  uint32_t lock_id)
{
    ONLYONE_LOCK lock_rec = {cmd,lock_id};
    only_one_lock_pool_->erase(lock_rec);
    return;
}


//实例赋值
void FSM_Manager::instance(FSM_Manager *pinstatnce)
{
    clean_instance();
    instance_ = pinstatnce;
    return;
}

//清除实例
void FSM_Manager::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }

    return;
}