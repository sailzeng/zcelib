#include "zce/predefine.h"
#include "zce/lockfree/kfifo.h"
#include "zce/os_adapt/file.h"
#include "zce/shared_mem/mmap.h"
#include "zce/logger/logging.h"
#include "zce/bus/mmap_pipe.h"
#include "zce/bus/twoway_pipe.h"

namespace zce
{

TwoWay_BusPipe *TwoWay_BusPipe::two_way_instance_ = NULL;

const char  TwoWay_BusPipe::BUS_PIPE_NAME[NUM_OF_PIPE][16] =
{
    "RECV_PIPE",
    "SEND_PIPE",
};

//构造函数
TwoWay_BusPipe::TwoWay_BusPipe()
{
}

TwoWay_BusPipe::~TwoWay_BusPipe()
{
}

int TwoWay_BusPipe::initialize(const char *bus_mmap_name,
                                   size_t size_recv_pipe,
                                   size_t size_send_pipe,
                                   size_t max_frame_len,
                                   bool if_restore)
{
    int ret = 0;
    size_t size_of_pipe[NUM_OF_PIPE];
    size_of_pipe[RECV_PIPE_ID] = size_recv_pipe;
    size_of_pipe[SEND_PIPE_ID] = size_send_pipe;

    ret = MMAP_BusPipe::initialize(bus_mmap_name,
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
TwoWay_BusPipe *TwoWay_BusPipe::instance()
{
    if (two_way_instance_ == NULL)
    {
        two_way_instance_ = new TwoWay_BusPipe();
    }

    return two_way_instance_;
}

//赋值唯一的单子实例
void TwoWay_BusPipe::instance(TwoWay_BusPipe *pinstatnce)
{
    clean_instance();
    two_way_instance_ = pinstatnce;
    return;
}

//清除单子实例
void TwoWay_BusPipe::clean_instance()
{
    if (two_way_instance_)
    {
        delete two_way_instance_;
    }

    two_way_instance_ = NULL;
    return;
}

}