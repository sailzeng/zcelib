#include "soar/predefine.h"
#include "soar/zerg/frame_zerg.h"
#include "soar/svrd/mmap_buspipe.h"

Soar_MMAP_BusPipe *Soar_MMAP_BusPipe::zerg_bus_instance_ = NULL;

char Soar_MMAP_BusPipe::send_buffer_[soar::Zerg_Frame::MAX_LEN_OF_APPFRAME];

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
    monitor_ = soar::Stat_Monitor::instance();

    zerg_svr_info_ = svr_info;

    char bus_mmap_name[MAX_PATH + 1];
    get_mmapfile_name(bus_mmap_name,MAX_PATH);

    return ZCE_BusPipe_TwoWay::initialize(bus_mmap_name,
                                          size_recv_pipe,
                                          size_send_pipe,
                                          max_frame_len,
                                          if_restore);
}

//根据SVR INFO得到MMAP文件名称
void Soar_MMAP_BusPipe::get_mmapfile_name(char *mmapfile,size_t buflen)
{
    snprintf(mmapfile,buflen,"./ZERGPIPE.%u.%u.MMAP",
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

//从RECV管道读取帧，
int Soar_MMAP_BusPipe::pop_front_recvbus(soar::Zerg_Frame *&proc_frame)
{
    int ret = pop_front_bus(RECV_PIPE_ID,
                            reinterpret_cast<zce::lockfree::dequechunk_node *&>(proc_frame));

    // 加监控数据
    if (ret == 0)
    {
        monitor_->add_one(COMM_RECV_BUS_POP_COUNT,
                          proc_frame->command_,
                          0);
        monitor_->add_number(COMM_RECV_BUS_POP_BYTES,
                             proc_frame->command_,
                             0,
                             proc_frame->length_);
        //如果是跟踪命令，把数据包打印出来，会非常耗时，少用
        if (proc_frame->frame_option_.option_ & soar::Zerg_Frame::DESC_TRACK_MONITOR)
        {
            DUMP_ZERG_FRAME_HEAD(RS_ERROR,"[TRACK MONITOR][pop recv]",proc_frame);
        }
    }
    //去不到数据并不一定是错误
    return ret;
}

//从RECV管道读取帧，
int Soar_MMAP_BusPipe::pop_front_sendbus(soar::Zerg_Frame *&proc_frame)
{
    int ret = pop_front_bus(SEND_PIPE_ID,
                            reinterpret_cast<zce::lockfree::dequechunk_node *&>(proc_frame));

    // 加监控数据
    if (ret == 0)
    {
        monitor_->add_one(COMM_SEND_BUS_POP_COUNT,
                          proc_frame->command_,
                          0);
        monitor_->add_number(COMM_SEND_BUS_POP_BYTES,
                             proc_frame->command_,
                             0,
                             proc_frame->length_);
        //如果是跟踪命令，把数据包打印出来，会非常耗时，少用
        if (proc_frame->frame_option_.option_ & soar::Zerg_Frame::DESC_TRACK_MONITOR)
        {
            DUMP_ZERG_FRAME_HEAD(RS_ERROR,"[TRACK MONITOR][send pop]",proc_frame);
        }
    }
    //取不到数据并不一定是错误
    return ret;
}

//向SEND管道写入帧，
int Soar_MMAP_BusPipe::push_back_sendpipe(const soar::Zerg_Frame *proc_frame)
{
    DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG,"TO SEND PIPE FRAME:",proc_frame);

    if (proc_frame->length_ > soar::Zerg_Frame::MAX_LEN_OF_APPFRAME || 
        proc_frame->length_ < soar::Zerg_Frame::LEN_OF_APPFRAME_HEAD)
    {
        ZCE_LOG(RS_ERROR,"[framework][send bus] Frame Len is error ,frame length :%u ,Please check your code.",
                proc_frame->length_);
        return SOAR_RET::ERROR_FRAME_DATA_IS_ERROR;
    }

    //如果是跟踪命令，把数据包打印出来，会非常耗时，少用
    if (proc_frame->frame_option_.option_ & soar::Zerg_Frame::DESC_TRACK_MONITOR)
    {
        DUMP_ZERG_FRAME_HEAD(RS_ERROR,"[TRACK MONITOR][Send]",proc_frame);
    }

    int ret = push_back_bus(SEND_PIPE_ID,
                            reinterpret_cast<const zce::lockfree::dequechunk_node *>(proc_frame));

    if (ret != 0)
    {
        // 发送失败, 管道满了
        monitor_->add_one(COMM_SEND_BUS_PUSH_FAIL,
                          proc_frame->command_,
                          0);
        return SOAR_RET::ERROR_PIPE_IS_FULL;
    }
    else
    {
        monitor_->add_one(COMM_SEND_BUS_PUSH_SUCC,
                          proc_frame->command_,
                          0);
        monitor_->add_number(COMM_SEND_BUS_PUSH_BYTES,
                             proc_frame->command_,
                             0,
                             proc_frame->length_);
    }

    // 返回成功
    return 0;
}

//向RECV管道写入帧，
int Soar_MMAP_BusPipe::push_back_recvpipe(const soar::Zerg_Frame *proc_frame)
{
    DEBUG_DUMP_ZERG_FRAME_HEAD(RS_DEBUG,"TO RECV PIPE FRAME:",proc_frame);

    if (proc_frame->length_ > soar::Zerg_Frame::MAX_LEN_OF_APPFRAME ||
        proc_frame->length_ < soar::Zerg_Frame::LEN_OF_APPFRAME_HEAD)
    {
        ZCE_LOG(RS_ERROR,"[framework][recv bus] Frame Len is error ,frame length :%u ,Please check your code.",
                proc_frame->length_);
        return SOAR_RET::ERROR_FRAME_DATA_IS_ERROR;
    }

    //如果是跟踪命令，把数据包打印出来，会非常耗时，少用
    if (proc_frame->frame_option_.option_ & soar::Zerg_Frame::DESC_TRACK_MONITOR)
    {
        DUMP_ZERG_FRAME_HEAD(RS_ERROR,"[TRACK MONITOR][Recv]",proc_frame);
    }

    int ret = push_back_bus(RECV_PIPE_ID,
                            reinterpret_cast<const zce::lockfree::dequechunk_node *>(proc_frame));

    if (ret != 0)
    {
        // 发送失败, 管道满了
        monitor_->add_one(COMM_RECV_BUS_PUSH_FAIL,
                          proc_frame->command_,
                          0);
        return SOAR_RET::ERROR_PIPE_IS_FULL;
    }
    else
    {
        monitor_->add_one(COMM_RECV_BUS_PUSH_SUCC,
                          proc_frame->command_,
                          0);
        monitor_->add_number(COMM_RECV_BUS_PUSH_BYTES,
                             proc_frame->command_,
                             0,
                             proc_frame->length_);
    }

    // 返回成功
    return 0;
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