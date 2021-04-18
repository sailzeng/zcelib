

#include "soar/predefine.h"
#include "soar/zerg/frame_zerg.h"
#include "soar/svrd/mmap_buspipe.h"

Soar_MMAP_BusPipe *Soar_MMAP_BusPipe::zerg_bus_instance_ = NULL;

char Soar_MMAP_BusPipe::send_buffer_[soar::Zerg_Frame_Head::MAX_LEN_OF_APPFRAME];

Soar_MMAP_BusPipe::Soar_MMAP_BusPipe():
    ZCE_BusPipe_TwoWay()
{
}

Soar_MMAP_BusPipe::~Soar_MMAP_BusPipe()
{

}

//初始化
int Soar_MMAP_BusPipe::initialize(soar::SERVICES_INFO &svr_info,
                                  size_t size_recv_pipe,
                                  size_t size_send_pipe,
                                  size_t max_frame_len,
                                  bool if_restore)
{

    monitor_ = Soar_Stat_Monitor::instance();

    zerg_svr_info_ = svr_info;

    char bus_mmap_name[MAX_PATH + 1];
    get_mmapfile_name(bus_mmap_name, MAX_PATH);

    return ZCE_BusPipe_TwoWay::initialize(bus_mmap_name,
                                          size_recv_pipe,
                                          size_send_pipe,
                                          max_frame_len,
                                          if_restore);
}

//根据SVR INFO得到MMAP文件名称
void Soar_MMAP_BusPipe::get_mmapfile_name(char *mmapfile, size_t buflen)
{
    snprintf(mmapfile, buflen, "./ZERGPIPE.%u.%u.MMAP", 
             zerg_svr_info_.svc_id_.services_type_, 
             zerg_svr_info_.svc_id_.services_id_);
}

//得到唯一的单子实例
Soar_MMAP_BusPipe *Soar_MMAP_BusPipe::instance()
{
    if (zerg_bus_instance_ == NULL)
    {
        zerg_bus_instance_ = new Soar_MMAP_BusPipe();
    }

    return zerg_bus_instance_;
}

//赋值唯一的单子实例
void Soar_MMAP_BusPipe::instance(Soar_MMAP_BusPipe *pinstatnce)
{
    clean_instance();
    zerg_bus_instance_ = pinstatnce;
    return;
}

//清除单子实例
void Soar_MMAP_BusPipe::clean_instance()
{
    if (zerg_bus_instance_)
    {
        delete zerg_bus_instance_;
    }

    zerg_bus_instance_ = NULL;
    return;
}

int
Soar_MMAP_BusPipe::pipe_sendbuf_to_service(unsigned int cmd,
                                           unsigned int user_id,
                                           unsigned int transaction_id,
                                           unsigned int backfill_trans_id,
                                           const soar::SERVICES_ID &rcvsvc,
                                           const soar::SERVICES_ID &proxysvc,
                                           const soar::SERVICES_ID &sendsvc,
                                           const unsigned char *buf,
                                           size_t buf_len,
                                           unsigned int option /*= 0*/)
{
    soar::Zerg_Frame_Head *send_frame = reinterpret_cast<soar::Zerg_Frame_Head *>(send_buffer_);

    send_frame->init_framehead(soar::Zerg_Frame_Head::MAX_LEN_OF_APPFRAME, option, cmd);
    send_frame->frame_userid_ = user_id;

    send_frame->send_service_ = sendsvc;
    send_frame->proxy_service_ = proxysvc;
    send_frame->recv_service_ = rcvsvc;

    //填写事务ID和回填事务ID
    send_frame->transaction_id_ = transaction_id;
    send_frame->backfill_trans_id_ = backfill_trans_id;

    int ret = send_frame->fill_appdata(buf_len, (const char *)buf);

    if (ret != 0 )
    {
        ZCE_LOG(RS_ERROR, "[framework] [%s]TDR encode fail.ret =%d,Please check your code and buffer len.",
                __ZCE_FUNC__,
                ret);
        return ret;
    }

    //
    return push_back_sendpipe(send_frame);
}


//从RECV管道读取帧，
int Soar_MMAP_BusPipe::pop_front_recvpipe(soar::Zerg_Frame_Head *&proc_frame)
{
    int ret = pop_front_bus(RECV_PIPE_ID,reinterpret_cast<zce::lockfree::dequechunk_node *&>(proc_frame));

    // 加监控数据
    if (ret == 0)
    {
        monitor_->increase_once(COMM_STAT_RECV_PKG_COUNT,
                                zerg_svr_info_.business_id_,
                                proc_frame->frame_command_);
        monitor_->increase_by_statid(COMM_STAT_RECV_PKG_BYTES,
                                     zerg_svr_info_.business_id_,
                                     proc_frame->frame_command_,
                                     proc_frame->frame_length_);


        //如果是跟踪命令，把数据包打印出来，会非常耗时，少用
        if (proc_frame->frame_option_.option_ & soar::Zerg_Frame_Head::DESC_MONITOR_TRACK)
        {
            soar::Zerg_Frame_Head::dumpoutput_frameinfo(RS_ERROR,"[TRACK MONITOR][Recv]",proc_frame);
        }
    }

    return ret;
}

//向SEND管道写入帧，
int Soar_MMAP_BusPipe::push_back_sendpipe(soar::Zerg_Frame_Head *proc_frame)
{
    DEBUGDUMP_FRAME_HEAD_DBG(RS_DEBUG,"TO SEND PIPE FRAME:",proc_frame);

    if (proc_frame->frame_length_ > soar::Zerg_Frame_Head::MAX_LEN_OF_APPFRAME || proc_frame->frame_length_ < soar::Zerg_Frame_Head::LEN_OF_APPFRAME_HEAD)
    {
        ZCE_LOG(RS_ERROR,"[framework] Frame Len is error ,frame length :%u ,Please check your code.",
                proc_frame->frame_length_);
        return SOAR_RET::ERROR_FRAME_DATA_IS_ERROR;
    }

    //如果是跟踪命令，把数据包打印出来，会非常耗时，少用
    if (proc_frame->frame_option_.option_ & soar::Zerg_Frame_Head::DESC_MONITOR_TRACK)
    {
        soar::Zerg_Frame_Head::dumpoutput_frameinfo(RS_ERROR,"[TRACK MONITOR][Send]",proc_frame);
    }

    int ret = push_back_bus(SEND_PIPE_ID,
                            reinterpret_cast<const zce::lockfree::dequechunk_node *>(proc_frame));

    if (ret != 0)
    {
        // 发送失败, 管道满了
        monitor_->increase_once(COMM_STAT_SEND_PKG_FAIL,
                                zerg_svr_info_.business_id_,
                                proc_frame->frame_command_);
        return SOAR_RET::ERROR_PIPE_IS_FULL;
    }
    else
    {
        monitor_->increase_once(COMM_STAT_SEND_PKG_SUCC,
                                zerg_svr_info_.business_id_,
                                proc_frame->frame_command_);
        monitor_->increase_by_statid(COMM_STAT_SEND_PKG_BYTES,
                                     zerg_svr_info_.business_id_,
                                     proc_frame->frame_command_,
                                     proc_frame->frame_length_);
    }

    // 返回成功
    return 0;
}
