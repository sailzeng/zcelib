

#pragma once

#include "zce/pool/multiobjs_pool.h"
#include "zce/thread/msgque_condi.h"
#include "zce/aio/caller.h"

namespace zce::aio
{

class Worker
{
public:

    Worker() = default;
    ~Worker() = default;

    //!
    int initialize(size_t work_thread_num,
                   size_t work_queue_len);

    int fs_work(Caller* caller,
                const AIO_FS& aio_fs);

    AIO_FS* alloc_aio_fs();
    AIO_MySQL* alloc_aio_mysql();

    bool request(AIO_base* base);

    bool response(AIO_base* &base);

protected:
    //!
    size_t work_thread_num_ = 0;
    //!
    std::thread* work_thread_ = nullptr;
    //!
    zce::msgring_condi<zce::aio::AIO_base *>* requst_queue_ = nullptr;
    zce::msgring_condi<zce::aio::AIO_base *>* response_queue_ = nullptr;
    //!
    zce::multiobjs_pool<std::mutex,
        zce::aio::AIO_FS,
        zce::aio::AIO_MySQL> aio_obj_pool_;
};
}