

#include "soar_predefine.h"
#include "soar_zerg_frame.h"
#include "soar_mmap_buspipe.h"

Soar_MMAP_BusPipe *Soar_MMAP_BusPipe::zerg_bus_instance_ = NULL;

char Soar_MMAP_BusPipe::send_buffer_[Zerg_App_Frame::MAX_LEN_OF_APPFRAME];

Soar_MMAP_BusPipe::Soar_MMAP_BusPipe():
    ZCE_BusPipe_TwoWay()
{
}

Soar_MMAP_BusPipe::~Soar_MMAP_BusPipe()
{

}

//初始化
int Soar_MMAP_BusPipe::initialize(SERVICES_ID &svrinfo,
                                  size_t size_recv_pipe,
                                  size_t size_send_pipe,
                                  size_t max_frame_len,
                                  bool if_restore)
{

    monitor_ = Soar_Stat_Monitor::instance();

    zerg_svr_info_ = svrinfo;

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
    snprintf(mmapfile, buflen, "./ZERGPIPE.%u.%u.MMAP", zerg_svr_info_.services_type_, zerg_svr_info_.services_id_);
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
                                           unsigned int qquin,
                                           unsigned int transaction_id,
                                           unsigned int backfill_trans_id,
                                           const SERVICES_ID &rcvsvc,
                                           const SERVICES_ID &proxysvc,
                                           const SERVICES_ID &sendsvc,
                                           const unsigned char *buf,
                                           size_t buf_len,
                                           unsigned int app_id /*= 0*/,
                                           unsigned int option /*= 0*/)
{
    Zerg_App_Frame *send_frame = reinterpret_cast<Zerg_App_Frame *>(send_buffer_);

    send_frame->init_framehead(Zerg_App_Frame::MAX_LEN_OF_APPFRAME, option, cmd);
    send_frame->frame_uid_ = qquin;
    send_frame->app_id_ = app_id;

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

