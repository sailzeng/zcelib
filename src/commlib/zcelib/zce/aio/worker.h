

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

    //!初始化
    int initialize(size_t work_thread_num,
                   size_t work_queue_len);

    //!
    void terminate();

    //!
    AIO_Handle* alloc_handle(AIO_TYPE aio_type);

    //!
    void free_handle(zce::aio::AIO_Handle* base);

    bool request(zce::aio::AIO_Handle* base);

    //!
    void process_request();

    //!
    void process_response(size_t& num_rsp);

    //!
    void process_aio(zce::aio::AIO_Handle* base);
    //!
    void process_fs(zce::aio::FS_Handle* base);
    //!
    void process_mysql(zce::aio::MySQL_Handle* base);

protected:
    //!
    uint32_t caller_id_builder_ = 1;

    //! 工作线程
    size_t work_thread_num_ = 0;
    //! 
    std::thread** work_thread_ = nullptr;
    //!
    bool worker_running_ = true;
    //!
    zce::msgring_condi<zce::aio::AIO_Handle*>* requst_queue_ = nullptr;
    zce::msgring_condi<zce::aio::AIO_Handle*>* response_queue_ = nullptr;
    //!
    zce::multiobjs_pool<std::mutex,
        zce::aio::FS_Handle,
        zce::aio::MySQL_Handle> aio_obj_pool_;
};
}