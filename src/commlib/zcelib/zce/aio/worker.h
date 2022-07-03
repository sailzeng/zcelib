

#pragma once

#include "zce/pool/multiobjs_pool.h"
#include "zce/thread/msgque_condi.h"
#include "zce/aio/caller.h"

namespace zce::aio
{

class Worker
{
public:

    int fs_work(Caller* caller,
                const AIO_FS& aio_fs);

protected:
    //!
    size_t work_thread_num_ = 0;
    //!
    std::thread** work_thread_ = nullptr;
    //!
    zce::msgring_condi<zce::aio::AIO_base> work_queue_;
    //!
    zce::multiobjs_pool<std::mutex,
        zce::aio::AIO_FS,
        zce::aio::AIO_MySQL> aio_obj_pool_;
};
}