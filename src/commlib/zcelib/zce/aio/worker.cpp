#include "zce/predefine.h"
#include "zce/os_adapt/file.h"
#include "zce/aio/worker.h"

namespace zce::aio
{


//!
int Worker::initialize(size_t work_thread_num,
                       size_t work_queue_len)
{
    worker_running_ = true;
    work_thread_num_ = work_thread_num;
    work_thread_ = new std::thread * [work_thread_num_];
    for (size_t i = 0; i < work_thread_num_; ++i)
    {
        work_thread_[i] = new std::thread(&zce::aio::Worker::process_request,
                                          this);
    }

    requst_queue_ = new zce::msgring_condi<zce::aio::AIO_Handle*>(work_queue_len);
    response_queue_ = new zce::msgring_condi<zce::aio::AIO_Handle*>(work_queue_len);
    aio_obj_pool_.initialize<zce::aio::FS_Handle>(16, 16);
    aio_obj_pool_.initialize<zce::aio::MySQL_Handle>(16, 16);

    return 0;
}

//!
void Worker::terminate()
{
    worker_running_ = false;

    for (size_t i = 0; i < work_thread_num_; ++i)
    {
        work_thread_[i]->join();
        delete work_thread_[i];
    }
    delete[] work_thread_;
}


AIO_Handle* Worker::alloc_handle(AIO_TYPE aio_type)
{
    AIO_Handle* handle = nullptr;
    if (aio_type > AIO_TYPE::FS_BEGIN &&
        aio_type < AIO_TYPE::FS_END)
    {
        handle = aio_obj_pool_.alloc_object<FS_Handle>();
    }
    else if (aio_type > AIO_TYPE::MYSQL_BEGIN &&
             aio_type < AIO_TYPE::MYSQL_END)
    {
        handle = aio_obj_pool_.alloc_object<MySQL_Handle>();
    }
    else
    {
        return nullptr;
    }
    handle->aio_type_ = aio_type;
    handle->id_ = caller_id_builder_++;
    return handle;
}


void Worker::free_handle(zce::aio::AIO_Handle* base)
{
    if (base->aio_type_ > AIO_TYPE::FS_BEGIN &&
        base->aio_type_ < AIO_TYPE::FS_END)
    {
        aio_obj_pool_.free_object<FS_Handle>(static_cast<FS_Handle*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::MYSQL_BEGIN &&
             base->aio_type_ < AIO_TYPE::MYSQL_END)
    {
        aio_obj_pool_.free_object<MySQL_Handle>(static_cast<MySQL_Handle*>(base));
    }
    else
    {
    }
}

bool Worker::request(AIO_Handle* base)
{
    return requst_queue_->try_enqueue(base);
}

//!处理请求
void Worker::process_request()
{
    size_t num_req = 0;
    bool go = false;
    do
    {
        AIO_Handle* base = nullptr;
        zce::Time_Value tv(0, 2000);
        go = requst_queue_->dequeue_wait(base, tv);
        if (go)
        {
            ++num_req;
            process_aio(base);
        }
        else
        {
            continue;
        }
    } while (worker_running_);
}

//!处理应答
void Worker::process_response(size_t& num_rsp)
{
    num_rsp = 0;
    bool go = false;
    do
    {
        AIO_Handle* base = nullptr;
        go = response_queue_->try_dequeue(base);
        base->call_back_(base);
        free_handle(base);
        ++num_rsp;
    } while (go);
}

//!
void Worker::process_aio(zce::aio::AIO_Handle* base)
{
    if (base->aio_type_ > AIO_TYPE::FS_BEGIN &&
        base->aio_type_ < AIO_TYPE::FS_END)
    {
        process_fs(static_cast<zce::aio::FS_Handle*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::MYSQL_BEGIN &&
             base->aio_type_ < AIO_TYPE::MYSQL_END)
    {
        process_mysql(static_cast<zce::aio::MySQL_Handle*>(base));
    }
    else
    {
    }

    response_queue_->dequeue(base);
}
//!在线程种处理文件
void Worker::process_fs(zce::aio::FS_Handle* hdl)
{
    switch (hdl->aio_type_)
    {
    case FS_OPEN:
        hdl->result_ = zce::open2(hdl->handle_,
                                  hdl->path_,
                                  hdl->mode_,
                                  hdl->flags_);
        break;
    case FS_CLOSE:
        hdl->result_ = zce::close(hdl->handle_);
        break;
    case FS_LSEEK:
        off_t off;
        hdl->result_ = zce::lseek(hdl->handle_,
                                  (off_t)hdl->offset_,
                                  hdl->whence_,
                                  off);
        break;
    case FS_READ:
        hdl->result_ = zce::read(hdl->handle_,
                                 hdl->read_bufs_,
                                 hdl->bufs_count_,
                                 hdl->result_count_,
                                 (off_t)hdl->offset_,
                                 hdl->whence_);
        break;
    case FS_WRITE:
        hdl->result_ = zce::write(hdl->handle_,
                                  hdl->write_bufs_,
                                  hdl->bufs_count_,
                                  hdl->result_count_,
                                  (off_t)hdl->offset_,
                                  hdl->whence_);
        break;
    case FS_STAT:
        hdl->result_ = zce::fstat(hdl->handle_,
                                  hdl->file_stat_);
        break;
    default:
        break;
    }
}
//!
void Worker::process_mysql(zce::aio::MySQL_Handle* /*hdl*/)
{

}

}