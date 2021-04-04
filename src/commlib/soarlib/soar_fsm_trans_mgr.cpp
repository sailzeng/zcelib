#include "soar_predefine.h"
#include "soar_zerg_frame.h"
#include "soar_fsm_trans_base.h"
#include "soar_services_info.h"
#include "soar_zerg_frame_malloc.h"
#include "soar_fsm_trans_mgr.h"

/******************************************************************************************
struct TRANS_LOCK_RECORD �����ļ�¼��Ԫ
******************************************************************************************/
TRANS_LOCK_RECORD::TRANS_LOCK_RECORD(unsigned int lock_qq_uin, unsigned int lock_trans_cmd):
    lock_user_id_(lock_qq_uin),
    lock_trans_cmd_(lock_trans_cmd)
{
}

TRANS_LOCK_RECORD::TRANS_LOCK_RECORD():
    lock_user_id_(0),
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
    , self_svc_id_(0, 0)
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

//�������������������
Transaction_Manager::~Transaction_Manager()
{
}

int Transaction_Manager::initialize(ZCE_Timer_Queue_Base *timer_queue,
                                    size_t  szregtrans,
                                    size_t sztransmap,
                                    const SERVICES_ID &selfsvr,
                                    Soar_MMAP_BusPipe *zerg_mmap_pipe,
                                    unsigned int max_frame_len,
                                    bool init_inner_queue,
                                    bool init_lock_pool)
{
    ZCE_TRACE_FILELINE(RS_INFO);

    ZCE_ASSERT(timer_queue != NULL);
    ZCE_ASSERT(zerg_mmap_pipe != NULL);

    int ret = 0;
    ret = ZCE_Async_FSMMgr::initialize(timer_queue, szregtrans, sztransmap);
    if (ret != 0)
    {
        return ret;
    }

    max_trans_ = sztransmap;
    self_svc_id_ = selfsvr;
    zerg_mmap_pipe_ = zerg_mmap_pipe;

    trans_send_buffer_ = Zerg_App_Frame::new_frame(max_frame_len + 32);
    trans_send_buffer_->init_framehead(max_frame_len, CMD_INVALID_CMD);
    trans_recv_buffer_ = Zerg_App_Frame::new_frame(max_frame_len + 32);
    trans_recv_buffer_->init_framehead(max_frame_len, CMD_INVALID_CMD);
    fake_recv_buffer_ = Zerg_App_Frame::new_frame(max_frame_len + 32);
    fake_recv_buffer_->init_framehead(max_frame_len, CMD_INVALID_CMD);

    //�����ȷҪ���ʼ���ڲ���QUEUE,
    if (init_inner_queue)
    {
        inner_frame_malloc_ = new INNER_APPFRAME_MALLOCOR();
        inner_frame_malloc_->initialize(INIT_FRAME_MALLOC_NUMBER);

        inner_message_queue_ = new INNER_FRAME_MESSAGE_QUEUE(INNER_QUEUE_WATER_MARK);
        //inner_message_queue_->open(,INNER_QUEUE_WATER_MARK);
    }

    //��ʼ������
    if (init_lock_pool)
    {
        //��������ߴ��һ���ʼ����������
        trans_lock_pool_.rehash(sztransmap / 2);
    }
    return 0;
}

//
void Transaction_Manager::finish()
{
    //�����ڴ������
    if (inner_frame_malloc_)
    {
        delete inner_frame_malloc_;
        inner_frame_malloc_ = NULL;
    }
    //������Ϣ����
    if (inner_message_queue_)
    {
        delete inner_message_queue_;
        inner_message_queue_ = NULL;
    }
    if (trans_send_buffer_)
    {
        Zerg_App_Frame::delete_frame(trans_send_buffer_);
        trans_send_buffer_ = NULL;
    }
    if (trans_recv_buffer_)
    {
        Zerg_App_Frame::delete_frame(trans_recv_buffer_);
        trans_recv_buffer_ = NULL;
    }
    if (fake_recv_buffer_)
    {
        Zerg_App_Frame::delete_frame(fake_recv_buffer_);
        fake_recv_buffer_ = NULL;
    }
    ZCE_Async_FSMMgr::finish();

    ZCE_TRACE_FILELINE(RS_INFO);
}


/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2008��1��9��
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

    for (proc_frame = 0; zerg_mmap_pipe_->is_empty_bus(Soar_MMAP_BusPipe::RECV_PIPE_ID) == false && proc_frame < MAX_ONCE_PROCESS_FRAME ;  ++proc_frame)
    {
        //
        ret = zerg_mmap_pipe_->pop_front_recvpipe(tmp_frame);

        if (ret !=  0)
        {
            return 0;
        }

        DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "FROM RECV PIPE FRAME", tmp_frame );

        //�Ƿ񴴽�һ������
        bool bcrtcx = false;
        //tmp_frame���û���
        ret = process_appframe(tmp_frame, bcrtcx);

        //
        if (ret !=  0)
        {
            continue;
        }

        //������һ������
        if ( true == bcrtcx )
        {
            ++create_trans;
        }
    }

    //
    return 0;
}

//�����ݷ��뷢�͹ܵ�
int Transaction_Manager::push_back_sendpipe(Zerg_App_Frame *proc_frame)
{
    //Soar_MMAP_BusPipe�����ȳ�ʼ��....
    return zerg_mmap_pipe_->push_back_sendpipe(proc_frame);
}





/******************************************************************************************
Author          : Sail(ZENGXING)  Date Of Creation: 2009��3��16��
Function        : Transaction_Manager::lock_qquin_trnas_cmd
Return          : int
Parameter List  :
  Param1: unsigned int user_id        USER ID
  Param2: unsigned int trnas_lock_id ������ID,���Ժ���������ͬ�����߲�ͬ
  Param3: unsigned int frame_cmd     ������������������־���
Description     : ��ĳһ���û���һ�������������м���
Calls           :
Called By       : ����������˼�Ǳ�֤һ��ʱ�̣�ֻ��һ������������,������������
Other           :
Modify Record   :
******************************************************************************************/
int Transaction_Manager::lock_qquin_trnas_cmd(unsigned int user_id,
                                              unsigned int trnas_lock_id,
                                              unsigned int frame_cmd)
{
    TRANS_LOCK_RECORD lock_rec(user_id, trnas_lock_id);
    std::pair <INNER_TRANS_LOCK_POOL::iterator, bool> iter_tmp = trans_lock_pool_.insert(lock_rec);

    //����Ѿ���һ�����ˣ���ô����ʧ��
    if (false == iter_tmp.second )
    {
        ZCE_LOG(RS_ERROR, "[framework] [LOCK]Oh!Transaction lock fail.QQUin[%u] trans lock id[%u] trans cmd[%u].",
                user_id,
                trnas_lock_id,
                frame_cmd);
        return -1;
    }

    return 0;
}

//��ĳһ���û���һ�������������м���
void Transaction_Manager::unlock_qquin_trans_cmd(unsigned int user_id, unsigned int lock_trnas_id)
{
    TRANS_LOCK_RECORD lock_rec(user_id, lock_trnas_id);
    trans_lock_pool_.erase(lock_rec);
    return;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2006��4��22��
Function        : Transaction_Manager::process_appframe
Return          : int
Parameter List  :
  Param1: const Zerg_App_Frame* ppetappframe ����������֡���ݣ�ppetappframe֡������������process_appframe��������
  Param3: bool& bcrttx                �Ƿ񴴽�����
Description     :
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Transaction_Manager::process_appframe(Zerg_App_Frame *app_frame, bool &bcrttx)
{
    bcrttx = false;
    int ret = 0;

    //����Ǹ��������ӡ����
    if (app_frame->frame_option_ & Zerg_App_Frame::DESC_MONITOR_TRACK)
    {
        Zerg_App_Frame::dumpoutput_framehead(RS_INFO, "[TRACK MONITOR][TRANS PROCESS]", app_frame);
    }


    bool is_reg_cmd = is_register_cmd(app_frame->frame_command_);

    //��һ���������������
    if (is_reg_cmd)
    {

        unsigned int id = 0;
        ret = create_asyncobj(app_frame->frame_command_, app_frame, &id);

        bcrttx = true;

        //ͳ�Ƽ�����
        ++gen_trans_counter_;
        ++cycle_gentrans_counter_;

        ZCE_LOG(RS_DEBUG, "Create Trascation ,Command:%u Transaction ID:%u .",
                app_frame->frame_command_, id);
    }
    else
    {

        ret = active_asyncobj(app_frame->backfill_trans_id_, app_frame);
        if (ret != 0 )
        {
            DEBUGDUMP_FRAME_HEAD(RS_ERROR, "No use frame:", app_frame );
            return ret;
        }

        ZCE_LOG(RS_DEBUG, "Find raw Transaction ID: %u. ", app_frame->backfill_trans_id_);
    }

    return 0;
}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2006��4��3��
Function        : Transaction_Manager::get_handler_by_transid
Return          : int
Parameter List  :
  Param1: unsigned int transid     ����ID
  Param1: unsigned int trans_cmd   ����inmore��Ҫ��������һ��CMD�������޷�����ʱ��Ӧ
  Param2: Transaction_Base*& ptxbase ���ص�Handlerָ��
Description     : ��������IDѰ������
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
//int Transaction_Manager::get_handler_by_transid(unsigned int transid, unsigned int trans_cmd, Transaction_Base *&ptxbase)
//{
//    //��������IDѰ������
//    HASHMAP_OF_TRANSACTION::iterator mapiter = transc_map_.find(transid);
//
//    if (mapiter == transc_map_.end())
//    {
//        ZCE_LOG(RS_INFO,"[framework] get_handler_by_transid,Can't Find Transaction ID:%u,Command:%u From MAP.",
//                  transid,
//                  trans_cmd);
//        return SOAR_RET::ERROR_CANNOT_FIND_TRANSACTION_ID;
//    }
//
//    ptxbase = mapiter->second;
//
//    return 0;
//}
//
///******************************************************************************************
//Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2007��11��14��
//Function        : Transaction_Manager::dump_all_trans_info
//Return          : void
//Parameter List  : NULL
//Description     : Dump�õ�ʵ��
//Calls           :
//Called By       :
//Other           :
//Modify Record   :
//******************************************************************************************/
//void Transaction_Manager::dump_all_trans_info() const
//{
//    //
//    HASHMAP_OF_TRANSACTION::const_iterator iter_tmp = transc_map_.begin();
//    HASHMAP_OF_TRANSACTION::const_iterator iter_end = transc_map_.end();
//    //��Ϊ�ȽϹؼ�������RS_INFO
//    ZCE_LOG(RS_INFO,"[framework] Transaction Manager are processing [%d] transactions. ", transc_map_.size());
//
//    for (unsigned int i = 1; iter_tmp != iter_end ; ++iter_tmp, ++i)
//    {
//        Transaction_Base *pbase = iter_tmp->second;
//        ZCE_LOG(RS_INFO,"[framework] %u.Transaction ID:%u,Request UIN:%u,Command:%u,State:%u.",
//                  i,
//                  (pbase)->req_qq_uin_,
//                  (pbase)->req_qq_uin_,
//                  (pbase)->req_command_,
//                  (pbase)->fsm_stage_,
//                  (pbase)->transaction_id_ );
//    }
//
//    //
//    return ;
//}
//
//void Transaction_Manager::dump_trans_pool_info() const
//{
//    //
//    HASHMAP_OF_POLLREGTRANS::const_iterator iter_tmp = regtrans_pool_map_.begin();
//    HASHMAP_OF_POLLREGTRANS::const_iterator iter_end = regtrans_pool_map_.end();
//    //��Ϊ�ȽϹؼ�������RS_INFO
//    ZCE_LOG(RS_INFO,"[framework] Transaction Manager are processing pool number [%d] . ", regtrans_pool_map_.size());
//
//    for (unsigned int i = 1; iter_tmp != iter_end ; ++iter_tmp, ++i)
//    {
//        unsigned int frame_command = iter_tmp->first;
//        const POOL_OF_REGISTERTRANS &pool_regtrans = (iter_tmp->second).crttrs_cmd_pool_;
//        ZCE_LOG(RS_INFO,"[framework] %u.Pool porcess command:%u,capacity:%u,size:%u.",
//                  i,
//                  frame_command,
//                  pool_regtrans.capacity(),
//                  pool_regtrans.size()
//                 );
//    }
//
//    //
//    return ;
//}
//
////DUMP���е�ͳ����Ϣ
//void Transaction_Manager::dump_statistics_info() const
//{
//    //
//    HASHMAP_OF_POLLREGTRANS::const_iterator iter_tmp = regtrans_pool_map_.begin();
//    HASHMAP_OF_POLLREGTRANS::const_iterator iter_end = regtrans_pool_map_.end();
//
//    ZCE_LOG(RS_INFO,"[framework] [TRANS INFO] All generate transaction counter [%llu] ,previous cycle generate transaction number[%llu].",
//              gen_trans_counter_,
//              cycle_gentrans_counter_);
//
//    //��Ϊ�ȽϹؼ�������RS_INFO
//    ZCE_LOG(RS_INFO,"[framework] Transaction Manager are processing [%d] transactions. ", transc_map_.size());
//
//    for (unsigned int i = 1; iter_tmp != iter_end ; ++iter_tmp, ++i)
//    {
//        ZCE_LOG(RS_INFO,"[framework] [TRANS INFO]%u.Transaction command ID [%u],create [%llu], destroy right[%llu], destroy timeout[%llu],destroy exception[%llu],consume seconds[%llu]",
//                  i,
//                  (iter_tmp->second).req_command_,
//                  (iter_tmp->second).create_trans_num_,
//                  (iter_tmp->second).destroy_right_num_,
//                  (iter_tmp->second).destroy_timeout_num_,
//                  (iter_tmp->second).destroy_exception_num_,
//                  (iter_tmp->second).trans_consume_time_
//                 );
//    }
//}
//
//void Transaction_Manager::dump_all_debug_info() const
//{
//    dump_trans_pool_info();
//    dump_statistics_info();
//    dump_all_trans_info();
//}

//����������һ��Ϣͷ��һ��������,_��ʾ����һ���ڲ����������ṩ���������ʿʹ��
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

    //��������ID
    rsp_msg->backfill_trans_id_ = backfill_trans_id;
    rsp_msg->app_id_ = app_id;

    return push_back_sendpipe(rsp_msg);
}

//������ͳ��
void Transaction_Manager::enable_trans_statistics (const ZCE_Time_Value *stat_clock)
{
    statistics_clock_ = stat_clock;
}

int Transaction_Manager::mgr_postframe_to_msgqueue(Zerg_App_Frame *post_frame)
{
    int ret = 0;
    Zerg_App_Frame *tmp_frame = NULL;

    //����Ǵӳ����м�ȡ����FRAME����ʲô������
    inner_frame_malloc_->clone_appframe(post_frame, tmp_frame);

    //�����ϲ��õȴ��κ�ʱ��
    ret = inner_message_queue_->enqueue(tmp_frame);

    //����ֵС��0��ʾʧ��
    if (ret < 0)
    {
        ZCE_LOG(RS_DEBUG, "Post message to send queue fail.ret =%d"
                "Send queue message_count:%u message_bytes:%u. ",
                ret,
                inner_message_queue_->size(),
                inner_message_queue_->size() * sizeof(Zerg_App_Frame *));
        //�������Ժ󻹻�ȥ
        inner_frame_malloc_->free_appframe(tmp_frame);

        return SOAR_RET::ERROR_NOTIFY_SEND_QUEUE_ENQUEUE_FAIL;
    }

    return 0;
}

//����ӽ��ն���ȡ����FRAME
int Transaction_Manager::process_queue_frame(size_t &proc_frame, size_t &create_trans)
{
    int ret = 0;
    create_trans = 0;

    //�������
    for (proc_frame = 0; inner_message_queue_->empty() == false && proc_frame < MAX_ONCE_PROCESS_FRAME ;  ++proc_frame)
    {

        Zerg_App_Frame *tmp_frame = NULL;
        //
        ret = inner_message_queue_->dequeue(tmp_frame);

        //���С��0��ʾ���󣬵�����ط�Ӧ����һ��������Ϊ���滹��һ���ж�
        if (ret < 0)
        {
            ZCE_LOG(RS_ERROR, "[framework] Recv queue dequeue fail ,ret=%u,", ret);
            return 0;
        }

        DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG, "FROM RECV QUEUE FRAME:", tmp_frame );

        //�Ƿ񴴽�һ������
        bool bcrtcx = false;

        //tmp_frame  ���ϻ���
        ret = process_appframe(tmp_frame, bcrtcx);
        //�ͷ��ڴ�
        inner_frame_malloc_->free_appframe(tmp_frame);

        //
        if (ret !=  0)
        {
            continue;
        }

        //������һ������
        if ( true == bcrtcx )
        {
            ++create_trans;
        }
    }

    //
    return 0;
}

//�õ��������ĸ��ز���
//void Transaction_Manager::get_manager_load_foctor(unsigned int &load_max, unsigned int &load_cur)
//{
//    load_max = static_cast<unsigned int>(max_trans_);
//    load_cur = static_cast<unsigned int>( transc_map_.size());
//
//    //���������������1
//    if (load_cur == 0)
//    {
//        load_cur = 1;
//    }
//}

//�õ��������ĸ��ز���
//��һЩ��������û�н׶��Ե�����������ĺ��������ر����룬
void Transaction_Manager::get_manager_load_foctor2(unsigned int &load_max, unsigned int &load_cur)
{
    const unsigned int ONE_CYCLE_GENERATE_TRANS = 30000;

    //�õ����ص�����
    load_max = ONE_CYCLE_GENERATE_TRANS;

    if (cycle_gentrans_counter_ > ONE_CYCLE_GENERATE_TRANS)
    {
        load_cur = ONE_CYCLE_GENERATE_TRANS;
    }
    else
    {
        load_cur = cycle_gentrans_counter_;
    }

    //���ڼ���������
    cycle_gentrans_counter_ = 0;

    //���������������1
    if (load_cur == 0)
    {
        load_cur = 1;
    }
}

//�õ�ʵ��
Transaction_Manager *Transaction_Manager::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new Transaction_Manager();
    }

    return instance_;
}

//ʵ����ֵ
void Transaction_Manager::instance(Transaction_Manager *pinstatnce)
{
    clean_instance();
    instance_ = pinstatnce;
    return;
}

//���ʵ��
void Transaction_Manager::clean_instance()
{
    if (instance_)
    {
        delete instance_;
        instance_ = NULL;
    }

    return;
}

//ֱ�ӷ���һ��buffer to services��
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

