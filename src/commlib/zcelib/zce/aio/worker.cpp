#include "zce/predefine.h"
#include "zce/os_adapt/file.h"
#include "zce/os_adapt/dirent.h"
#include "zce/os_adapt/socket.h"
#include "zce/mysql/execute.h"
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

    requst_queue_ = new zce::msgring_condi<zce::aio::AIO_Atom*>(work_queue_len);
    response_queue_ = new zce::msgring_condi<zce::aio::AIO_Atom*>(work_queue_len);
    aio_obj_pool_.initialize<zce::aio::FS_Atom>(16, 16);
    aio_obj_pool_.initialize<zce::aio::MySQL_Atom>(16, 16);
    aio_obj_pool_.initialize<zce::aio::Host_Atom>(16, 16);
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

AIO_Atom* Worker::alloc_handle(AIO_TYPE aio_type)
{
    AIO_Atom* handle = nullptr;
    if (aio_type > AIO_TYPE::FS_BEGIN &&
        aio_type < AIO_TYPE::FS_END)
    {
        handle = aio_obj_pool_.alloc_object<FS_Atom>();
    }
    else if (aio_type > AIO_TYPE::MYSQL_BEGIN &&
             aio_type < AIO_TYPE::MYSQL_END)
    {
        handle = aio_obj_pool_.alloc_object<MySQL_Atom>();
    }
    else if (aio_type > AIO_TYPE::HOST_BEGIN &&
             aio_type < AIO_TYPE::HOST_END)
    {
        handle = aio_obj_pool_.alloc_object<Host_Atom>();
    }
    else
    {
        return nullptr;
    }
    handle->aio_type_ = aio_type;
    handle->id_ = caller_id_builder_++;
    return handle;
}

void Worker::free_handle(zce::aio::AIO_Atom* base)
{
    base->clear();
    if (base->aio_type_ > AIO_TYPE::FS_BEGIN &&
        base->aio_type_ < AIO_TYPE::FS_END)
    {
        aio_obj_pool_.free_object<FS_Atom>(static_cast<FS_Atom*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::MYSQL_BEGIN &&
             base->aio_type_ < AIO_TYPE::MYSQL_END)
    {
        aio_obj_pool_.free_object<MySQL_Atom>(static_cast<MySQL_Atom*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::HOST_BEGIN &&
             base->aio_type_ < AIO_TYPE::HOST_END)
    {
        aio_obj_pool_.free_object<Host_Atom>(static_cast<Host_Atom*>(base));
    }
    else
    {
    }
}

bool Worker::request(AIO_Atom* base)
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
        AIO_Atom* base = nullptr;
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
void Worker::process_response(size_t& num_rsp, zce::Time_Value* wait_time)
{
    num_rsp = 0;
    bool go = false;
    do
    {
        AIO_Atom* base = nullptr;
        if (wait_time)
        {
            go = response_queue_->dequeue_wait(base, *wait_time);
        }
        else
        {
            go = response_queue_->try_dequeue(base);
        }
        if (go)
        {
            base->call_back_(base);
            free_handle(base);
            ++num_rsp;
        }
    } while (go);
}

//!
void Worker::process_aio(zce::aio::AIO_Atom* base)
{
    if (base->aio_type_ > AIO_TYPE::FS_BEGIN &&
        base->aio_type_ < AIO_TYPE::FS_END)
    {
        process_fs(static_cast<zce::aio::FS_Atom*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::MYSQL_BEGIN &&
             base->aio_type_ < AIO_TYPE::MYSQL_END)
    {
        process_mysql(static_cast<zce::aio::MySQL_Atom*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::HOST_BEGIN &&
             base->aio_type_ < AIO_TYPE::HOST_END)
    {
        process_host(static_cast<zce::aio::Host_Atom*>(base));
    }
    else
    {
    }
    //放入应答队列
    response_queue_->enqueue(base);
}
//!在线程中处理文件
void Worker::process_fs(zce::aio::FS_Atom* hdl)
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
    case FS_READFILE:
        hdl->result_ = zce::read_file(hdl->path_,
                                      hdl->read_bufs_,
                                      hdl->bufs_count_,
                                      &hdl->result_count_,
                                      (off_t)hdl->offset_);
        break;
    case FS_WRITEFILE:
        hdl->result_ = zce::write_file(hdl->path_,
                                       hdl->write_bufs_,
                                       hdl->bufs_count_,
                                       &hdl->result_count_,
                                       (off_t)hdl->offset_);
        break;
    case FS_STAT:
        hdl->result_ = zce::fstat(hdl->handle_,
                                  hdl->file_stat_);
        break;
    case FS_SCANDIR:
        hdl->result_ = zce::scandir(hdl->dirname_,
                                    hdl->namelist_,
                                    NULL,
                                    NULL);
    default:
        break;
    }
}
//在线程处理MySQL操作请求
void Worker::process_mysql(zce::aio::MySQL_Atom* atom)
{
    switch (atom->aio_type_)
    {
    case MYSQL_CONNECT:
        atom->result_ = zce::mysql::execute::connect(
            atom->db_connect_,
            atom->host_name_,
            atom->user_,
            atom->pwd_,
            atom->port_);
        break;
    case MYSQL_DISCONNECT:
        atom->result_ = 0;
        zce::mysql::execute::disconnect(
            atom->db_connect_);
        break;
    case MYSQL_QUERY_NOSELECT:
        atom->result_ = zce::mysql::execute::query(
            atom->db_connect_,
            atom->sql_,
            atom->sql_len_,
            atom->num_affect_,
            atom->insert_id_);
        break;
    case MYSQL_QUERY_SELECT:
        atom->result_ = zce::mysql::execute::query(
            atom->db_connect_,
            atom->sql_,
            atom->sql_len_,
            atom->num_affect_,
            atom->db_result_);
        break;
    default:
        break;
    }
}

//在线程中处理Gat Host Addr请求
void Worker::process_host(zce::aio::Host_Atom* atom)
{
    switch (atom->aio_type_)
    {
    case GETADDRINFO_ARY:
        atom->result_ = zce::getaddrinfo_to_addrary(
            atom->hostname_,
            atom->service_,
            atom->ary_addr_num_,
            atom->ary_addr_,
            atom->ary_addr6_num_,
            atom->ary_addr6_);
        break;
    case GETADDRINFO_ONE:
        atom->result_ = zce::getaddrinfo_to_addr(
            atom->hostname_,
            atom->service_,
            atom->addr_,
            atom->addr_len_);
        break;
    default:
        break;
    }
}
}