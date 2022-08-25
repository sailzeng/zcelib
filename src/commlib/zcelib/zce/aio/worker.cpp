#include "zce/predefine.h"
#include "zce/aio/worker.h"

namespace zce::aio
{


//!
int Worker::initialize(size_t work_thread_num,
                       size_t work_queue_len)
{
    work_thread_num_ = work_thread_num;
    work_thread_ = new std::thread[work_thread_num_];
    requst_queue_ = new zce::msgring_condi<zce::aio::AIO_base>(work_queue_len);
    response_queue_ = new zce::msgring_condi<zce::aio::AIO_base>(work_queue_len);
    aio_obj_pool_.initialize<AIO_FS>(16, 16);
    aio_obj_pool_.initialize<AIO_MySQL>(16, 16);

    return 0;
}

}