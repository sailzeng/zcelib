

#include "zce_predefine.h"
#include "zce_lockfree_kfifo.h"
#include "zce_os_adapt_file.h"
#include "zce_share_mem_mmap.h"
#include "zce_log_logging.h"
#include "zce_bus_mmap_pipe.h"
#include "zce_bus_two_way.h"


ZCE_BusPipe_TwoWay *ZCE_BusPipe_TwoWay::two_way_instance_ = NULL;

const char  ZCE_BusPipe_TwoWay::BUS_PIPE_NAME[NUM_OF_PIPE][16] =
{
    "RECV_PIPE",
    "SEND_PIPE",
};

//构造函数
ZCE_BusPipe_TwoWay::ZCE_BusPipe_TwoWay()
{
}

ZCE_BusPipe_TwoWay::~ZCE_BusPipe_TwoWay()
{
}





int ZCE_BusPipe_TwoWay::initialize(const char *bus_mmap_name,
                                   size_t size_recv_pipe,
                                   size_t size_send_pipe,
                                   size_t max_frame_len,
                                   bool if_restore)
{
    int ret = 0;
    size_t size_of_pipe[NUM_OF_PIPE];
    size_of_pipe[RECV_PIPE_ID] = size_recv_pipe;
    size_of_pipe[SEND_PIPE_ID] = size_send_pipe;

    ret = ZCE_Bus_MMAPPipe::initialize(bus_mmap_name,
                                       NUM_OF_PIPE,
                                       size_of_pipe,
                                       max_frame_len,
                                       if_restore);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
}



//得到唯一的单子实例
ZCE_BusPipe_TwoWay *ZCE_BusPipe_TwoWay::instance()
{
    if (two_way_instance_ == NULL)
    {
        two_way_instance_ = new ZCE_BusPipe_TwoWay();
    }

    return two_way_instance_;
}

//赋值唯一的单子实例
void ZCE_BusPipe_TwoWay::instance(ZCE_BusPipe_TwoWay *pinstatnce)
{
    clean_instance();
    two_way_instance_ = pinstatnce;
    return;
}

//清除单子实例
void ZCE_BusPipe_TwoWay::clean_instance()
{
    if (two_way_instance_)
    {
        delete two_way_instance_;
    }

    two_way_instance_ = NULL;
    return;
}



