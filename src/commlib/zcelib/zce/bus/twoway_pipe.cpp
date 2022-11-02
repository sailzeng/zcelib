#include "zce/predefine.h"
#include "zce/lockfree/kfifo.h"
#include "zce/os_adapt/file.h"
#include "zce/shared_mem/mmap.h"
#include "zce/logger/logging.h"
#include "zce/bus/mmap_pipe.h"
#include "zce/bus/twoway_pipe.h"

namespace zce
{
twoway_buspipe* twoway_buspipe::two_way_instance_ = nullptr;

const char  twoway_buspipe::BUS_PIPE_NAME[NUM_OF_PIPE][16] =
{
    "RECV_PIPE",
    "SEND_PIPE",
};

//构造函数
twoway_buspipe::twoway_buspipe()
{
}

twoway_buspipe::~twoway_buspipe()
{
}

int twoway_buspipe::initialize(const char* bus_mmap_name,
                               size_t size_recv_pipe,
                               size_t size_send_pipe,
                               size_t max_frame_len,
                               bool if_restore)
{
    int ret = 0;
    size_t size_of_pipe[NUM_OF_PIPE];
    size_of_pipe[RECV_PIPE_ID] = size_recv_pipe;
    size_of_pipe[SEND_PIPE_ID] = size_send_pipe;

    ret = mmap_buspipe::initialize(bus_mmap_name,
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
twoway_buspipe* twoway_buspipe::instance()
{
    if (two_way_instance_ == nullptr)
    {
        two_way_instance_ = new twoway_buspipe();
    }

    return two_way_instance_;
}

//赋值唯一的单子实例
void twoway_buspipe::instance(twoway_buspipe* pinstatnce)
{
    clear_inst();
    two_way_instance_ = pinstatnce;
    return;
}

//清除单子实例
void twoway_buspipe::clear_inst()
{
    if (two_way_instance_)
    {
        delete two_way_instance_;
    }

    two_way_instance_ = nullptr;
    return;
}
}