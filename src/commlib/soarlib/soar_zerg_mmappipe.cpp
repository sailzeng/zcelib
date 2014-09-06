

#include "soar_predefine.h"
#include "soar_zerg_frame.h"
#include "soar_zerg_mmappipe.h"

Zerg_MMAP_BusPipe *Zerg_MMAP_BusPipe::zerg_bus_instance_ = NULL;

char Zerg_MMAP_BusPipe::send_buffer_[Zerg_App_Frame::MAX_LEN_OF_APPFRAME];

Zerg_MMAP_BusPipe::Zerg_MMAP_BusPipe():
    ZCE_BusPipe_TwoWay(),
    zerg_svr_info_(0, 0),
    monitor_(NULL)
{
}

Zerg_MMAP_BusPipe::~Zerg_MMAP_BusPipe()
{

}

/******************************************************************************************
Author          : Sailzeng <sailerzeng@gmail.com>  Date Of Creation: 2006年7月12日
Function        : Zerg_MMAP_BusPipe::initialize
Return          : void
Parameter List  :
  Param1: SERVICES_INFO& svrinfo
  Param2: size_t szrecv
  Param3: size_t szsend
  Param4: size_t szerror
  Param5: bool block
Description     : 初始化
Calls           :
Called By       :
Other           :
Modify Record   :
******************************************************************************************/
int Zerg_MMAP_BusPipe::initialize(SERVICES_ID &svrinfo,
                                  size_t size_recv_pipe,
                                  size_t size_send_pipe,
                                  size_t max_frame_len,
                                  bool if_restore)
{

    monitor_ = Comm_Stat_Monitor::instance();

    zerg_svr_info_ = svrinfo;

    char bus_mmap_name[MAX_PATH + 1];
    get_mmapfile_name(bus_mmap_name, MAX_PATH);

    return ZCE_BusPipe_TwoWay::initialize(bus_mmap_name,
                                          size_recv_pipe,
                                          size_send_pipe,
                                          max_frame_len,
                                          if_restore);
}

//int Zerg_MMAP_BusPipe::getpara_from_zergcfg(const conf_zerg::ZERG_CONFIG &zerg_config)
//{
//    int ret = 0;
//    bus_head_.size_of_pipe_[RECV_PIPE_ID] = zerg_config.soar_cfg.recv_pipe_len_;
//    TESTCONFIG(( ret == 0 && bus_head_.size_of_pipe_[RECV_PIPE_ID] > 2 * 1024 * 1024 && bus_head_.size_of_pipe_[RECV_PIPE_ID] < 800 * 1024 * 1024), "COMMCFG|RECVPIPELEN key error.");
//
//    bus_head_.size_of_pipe_[SEND_PIPE_ID] = zerg_config.soar_cfg.send_pipe_len_;
//    TESTCONFIG(ret == 0 && bus_head_.size_of_pipe_[SEND_PIPE_ID] > 2 * 1024 * 1024 && bus_head_.size_of_pipe_[SEND_PIPE_ID] < 800 * 1024 * 1024, "COMMCFG|SENDPIPELEN key error.");
//
//    //读取自己的配置
//    unsigned short svrtype = 0;
//    svrtype = zerg_config.self_cfg.self_svr_info.svr_type;
//    TESTCONFIG((ret == 0 && svrtype != 0), "SELFCFG|SELFSVRTYPE key error.");
//
//    unsigned int svrid = 0;
//    svrid = zerg_config.self_cfg.self_svr_info.svr_id;
//    TESTCONFIG((ret == 0 && svrid != 0), "SELFCFG|SELFSVRID key error.");
//
//    zerg_svr_info_.set_serviceid(svrtype, svrid);
//
//
//    return SOAR_RET::SOAR_RET_SUCC;
//}

//int Zerg_MMAP_BusPipe::getpara_from_zergcfg(const ZCE_INI_PropertyTree &zerglingcfg)
//{
//
//    int ret = 0;
//
//    uint32_t tmp_value = 0;
//    ret = zerglingcfg.get_uint32_value("COMMCFG", "RECVPIPELEN", tmp_value);
//    bus_head_.size_of_pipe_[RECV_PIPE_ID] = tmp_value;
//    TESTCONFIG(( ret == 0 && bus_head_.size_of_pipe_[RECV_PIPE_ID] > 2 * 1024 * 1024 && bus_head_.size_of_pipe_[RECV_PIPE_ID] < 800 * 1024 * 1024), "COMMCFG|RECVPIPELEN key error.");
//
//    ret = zerglingcfg.get_uint32_value("COMMCFG", "SENDPIPELEN", tmp_value);
//    bus_head_.size_of_pipe_[SEND_PIPE_ID] = tmp_value;
//    TESTCONFIG(ret == 0 && bus_head_.size_of_pipe_[SEND_PIPE_ID] > 2 * 1024 * 1024 && bus_head_.size_of_pipe_[SEND_PIPE_ID] < 800 * 1024 * 1024, "COMMCFG|SENDPIPELEN key error.");
//
//    uint32_t tmpuint = 0;
//
//    //读取自己的配置
//    unsigned short svrtype = 0;
//
//    ret = zerglingcfg.get_uint32_value("SELFCFG", "SELFSVRTYPE", tmpuint);
//    svrtype = static_cast<unsigned short>(tmpuint);
//    TESTCONFIG((ret == 0 && svrtype != 0), "SELFCFG|SELFSVRTYPE key error.");
//
//    unsigned int svrid = 0;
//    ret = zerglingcfg.get_uint32_value("SELFCFG", "SELFSVRID", tmpuint);
//    svrid = static_cast<unsigned int>(tmpuint);
//    TESTCONFIG((ret == 0 && svrid != 0), "SELFCFG|SELFSVRID key error.");
//
//    zerg_svr_info_.set_serviceid(svrtype, svrid);
//
//    return SOAR_RET::SOAR_RET_SUCC;
//}

//通讯服务器初始化内存管道
int Zerg_MMAP_BusPipe::init_after_getcfg(size_t max_frame_len,
                                         bool if_restore)
{
    return initialize(zerg_svr_info_,
                      bus_head_.size_of_pipe_[RECV_PIPE_ID],
                      bus_head_.size_of_pipe_[SEND_PIPE_ID],
                      max_frame_len,
                      if_restore);
}

//根据SVR INFO得到MMAP文件名称
void Zerg_MMAP_BusPipe::get_mmapfile_name(char *mmapfile, size_t buflen)
{
    snprintf(mmapfile, buflen, "./ZERGPIPE.%u.%u.MMAP", zerg_svr_info_.services_type_, zerg_svr_info_.services_id_);
}

//得到唯一的单子实例
Zerg_MMAP_BusPipe *Zerg_MMAP_BusPipe::instance()
{
    if (zerg_bus_instance_ == NULL)
    {
        zerg_bus_instance_ = new Zerg_MMAP_BusPipe();
    }

    return zerg_bus_instance_;
}

//赋值唯一的单子实例
void Zerg_MMAP_BusPipe::instance(Zerg_MMAP_BusPipe *pinstatnce)
{
    clean_instance();
    zerg_bus_instance_ = pinstatnce;
    return;
}

//清除单子实例
void Zerg_MMAP_BusPipe::clean_instance()
{
    if (zerg_bus_instance_)
    {
        delete zerg_bus_instance_;
    }

    zerg_bus_instance_ = NULL;
    return;
}

int
Zerg_MMAP_BusPipe::pipe_sendbuf_to_service(unsigned int cmd,
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

    if (ret != SOAR_RET::SOAR_RET_SUCC )
    {
        ZLOG_ERROR("[framework] [%s]TDR encode fail.ret =%d,Please check your code and buffer len.",
                   __ZCE_FUNCTION__,
                   ret);
        return ret;
    }

    //
    return push_back_sendpipe(send_frame);
}

