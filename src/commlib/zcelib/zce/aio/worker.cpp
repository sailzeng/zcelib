#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/file.h"
#include "zce/os_adapt/dirent.h"
#include "zce/os_adapt/socket.h"
#include "zce/os_adapt/time.h"
#include "zce/mysql/execute.h"

#include "zce/aio/worker.h"

namespace zce::aio
{
worker::~worker()
{
    terminate();
}
//!
int worker::initialize(size_t work_thread_num,
                       size_t work_queue_len,
                       size_t max_event_num)
{
    worker_running_ = true;
    work_thread_num_ = work_thread_num;
    work_thread_ = new std::thread * [work_thread_num_];
    for (size_t i = 0; i < work_thread_num_; ++i)
    {
        work_thread_[i] = new std::thread(&zce::aio::worker::process_request,
                                          this);
    }

    requst_queue_ = new zce::msgring_condi<zce::aio::AIO_ATOM*>(work_queue_len);
    response_queue_ = new zce::msgring_condi<zce::aio::AIO_ATOM*>(work_queue_len);
    aio_obj_pool_.initialize<zce::aio::FS_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::DIR_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::MYSQL_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::HOST_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::SOCKET_TIMEOUT_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::EVENT_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::TIMER_ATOM>(128, 256);
    reactor_ = new reactor_mini();
    reactor_->initialize(max_event_num, 1024, true);
    return 0;
}

//!
void worker::terminate()
{
    worker_running_ = false;

    for (size_t i = 0; i < work_thread_num_; ++i)
    {
        work_thread_[i]->join();
        delete work_thread_[i];
    }
    delete[] work_thread_;
    work_thread_ = nullptr;
    if (reactor_)
    {
        reactor_->close();
        delete reactor_;
        reactor_ = nullptr;
    }
}

AIO_ATOM* worker::alloc_handle(AIO_TYPE aio_type)
{
    AIO_ATOM* handle = nullptr;
    if (aio_type > AIO_TYPE::FS_BEGIN &&
        aio_type < AIO_TYPE::FS_END)
    {
        handle = aio_obj_pool_.alloc_object<FS_ATOM>();
    }
    else if (aio_type > AIO_TYPE::DIR_BEGIN &&
             aio_type < AIO_TYPE::DIR_END)
    {
        handle = aio_obj_pool_.alloc_object<DIR_ATOM>();
    }
    else if (aio_type > AIO_TYPE::MYSQL_BEGIN &&
             aio_type < AIO_TYPE::MYSQL_END)
    {
        handle = aio_obj_pool_.alloc_object<MYSQL_ATOM>();
    }
    else if (aio_type > AIO_TYPE::HOST_BEGIN &&
             aio_type < AIO_TYPE::HOST_END)
    {
        handle = aio_obj_pool_.alloc_object<HOST_ATOM>();
    }
    else if (aio_type > AIO_TYPE::SOCKET_BEGIN &&
             aio_type < AIO_TYPE::SOCKET_END)
    {
        handle = aio_obj_pool_.alloc_object<SOCKET_TIMEOUT_ATOM>();
    }
    else if (aio_type > AIO_TYPE::EVENT_BEGIN &&
             aio_type < AIO_TYPE::EVENT_END)
    {
        handle = aio_obj_pool_.alloc_object<EVENT_ATOM>();
    }
    else if (aio_type > AIO_TYPE::TIMER_BEGIN &&
             aio_type < AIO_TYPE::TIMER_END)
    {
        handle = aio_obj_pool_.alloc_object<TIMER_ATOM>();
    }
    else
    {
        return nullptr;
    }
    handle->aio_type_ = aio_type;
    handle->id_ = caller_id_builder_++;
    return handle;
}

void worker::free_handle(zce::aio::AIO_ATOM* base)
{
    base->clear();
    if (base->aio_type_ > AIO_TYPE::FS_BEGIN &&
        base->aio_type_ < AIO_TYPE::FS_END)
    {
        aio_obj_pool_.free_object<FS_ATOM>(static_cast<FS_ATOM*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::DIR_BEGIN &&
             base->aio_type_ < AIO_TYPE::DIR_END)
    {
        aio_obj_pool_.free_object<DIR_ATOM>(static_cast<DIR_ATOM*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::MYSQL_BEGIN &&
             base->aio_type_ < AIO_TYPE::MYSQL_END)
    {
        aio_obj_pool_.free_object<MYSQL_ATOM>(static_cast<MYSQL_ATOM*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::HOST_BEGIN &&
             base->aio_type_ < AIO_TYPE::HOST_END)
    {
        aio_obj_pool_.free_object<HOST_ATOM>(static_cast<HOST_ATOM*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::SOCKET_BEGIN &&
             base->aio_type_ < AIO_TYPE::SOCKET_END)
    {
        aio_obj_pool_.free_object<SOCKET_TIMEOUT_ATOM>(static_cast<SOCKET_TIMEOUT_ATOM*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::EVENT_BEGIN &&
             base->aio_type_ < AIO_TYPE::EVENT_END)
    {
        aio_obj_pool_.free_object<EVENT_ATOM>(static_cast<EVENT_ATOM*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::TIMER_BEGIN &&
             base->aio_type_ < AIO_TYPE::TIMER_END)
    {
        aio_obj_pool_.free_object<TIMER_ATOM>(static_cast<TIMER_ATOM*>(base));
    }
    else
    {
    }
}

bool worker::request(AIO_ATOM* base)
{
    return requst_queue_->try_enqueue(base);
}

//!处理请求
void worker::process_request()
{
    size_t num_req = 0;
    bool go = false;
    do
    {
        AIO_ATOM* base = nullptr;
        zce::time_value tv(0, 2000);
        go = requst_queue_->dequeue_wait(base, tv);
        if (go)
        {
            ++num_req;
            thread_aio(base);
        }
        else
        {
            continue;
        }
    } while (worker_running_);
}

//!处理应答
void worker::process_response(zce::time_value* wait_time,
                              size_t& num_rsp,
                              size_t& num_event)
{
    num_rsp = 0;
    bool go = false;
    do
    {
        reactor_->tiggers_events(wait_time, num_event);
        AIO_ATOM* base = nullptr;
        if (wait_time)
        {
            go = response_queue_->dequeue_wait(base,
                                               *wait_time);
        }
        else
        {
            go = response_queue_->try_dequeue(base);
        }
        if (go)
        {
            //调用回调函数
            base->call_back_(base);
            ++num_rsp;
        }
    } while (go);
}

//!
void worker::thread_aio(zce::aio::AIO_ATOM* base)
{
    if (base->aio_type_ > AIO_TYPE::FS_BEGIN &&
        base->aio_type_ < AIO_TYPE::FS_END)
    {
        thread_fs(static_cast<zce::aio::FS_ATOM*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::DIR_BEGIN &&
             base->aio_type_ < AIO_TYPE::DIR_END)
    {
        thread_dir(static_cast<zce::aio::DIR_ATOM*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::MYSQL_BEGIN &&
             base->aio_type_ < AIO_TYPE::MYSQL_END)
    {
        thread_mysql(static_cast<zce::aio::MYSQL_ATOM*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::HOST_BEGIN &&
             base->aio_type_ < AIO_TYPE::HOST_END)
    {
        thread_host(static_cast<zce::aio::HOST_ATOM*>(base));
    }
    else if (base->aio_type_ > AIO_TYPE::SOCKET_BEGIN &&
             base->aio_type_ < AIO_TYPE::SOCKET_END)
    {
        thread_socket_timeout(static_cast<zce::aio::SOCKET_TIMEOUT_ATOM*>(base));
    }
    else
    {
    }
    //放入应答队列
    response_queue_->enqueue(base);
}
//!在线程中处理文件
void worker::thread_fs(zce::aio::FS_ATOM* atom)
{
    switch (atom->aio_type_)
    {
    case FS_OPEN:
        atom->result_ = zce::open2(atom->handle_,
                                   atom->path_,
                                   atom->mode_,
                                   atom->flags_);
        break;
    case FS_CLOSE:
        atom->result_ = zce::close(atom->handle_);
        break;
    case FS_LSEEK:
        off_t off;
        atom->result_ = zce::lseek(atom->handle_,
                                   (off_t)atom->offset_,
                                   atom->whence_,
                                   off);
        break;

    case FS_READ:

        atom->result_ = zce::read(atom->handle_,
                                  atom->read_bufs_,
                                  atom->bufs_len_,
                                  atom->result_len_,
                                  (off_t)atom->offset_,
                                  atom->whence_);
        break;
    case FS_WRITE:
        atom->result_ = zce::write(atom->handle_,
                                   atom->write_bufs_,
                                   atom->bufs_len_,
                                   atom->result_len_,
                                   (off_t)atom->offset_,
                                   atom->whence_);
        break;
    case FS_READFILE:
        atom->result_ = zce::read_file(atom->path_,
                                       atom->read_bufs_,
                                       atom->bufs_len_,
                                       atom->result_len_,
                                       (off_t)atom->offset_);
        break;
    case FS_WRITEFILE:
        atom->result_ = zce::write_file(atom->path_,
                                        atom->write_bufs_,
                                        atom->bufs_len_,
                                        atom->result_len_,
                                        (off_t)atom->offset_);
        break;
    case FS_STAT:
        atom->result_ = zce::fstat(atom->handle_,
                                   atom->file_stat_);
        break;

    default:
        break;
    }
}

//!
void worker::thread_dir(zce::aio::DIR_ATOM* atom)
{
    switch (atom->aio_type_)
    {
    case DIR_MKDIR:
        atom->result_ = zce::mkdir(atom->dirname_,
                                   atom->mode_);
        break;
    case DIR_RMDIR:
        atom->result_ = zce::rmdir(atom->dirname_);
        break;
    case DIR_SCANDIR:
        atom->result_ = zce::scandir(atom->dirname_,
                                     atom->namelist_,
                                     NULL,
                                     NULL);
        break;
    default:
        break;
    }
}

//在线程处理MySQL操作请求
void worker::thread_mysql(zce::aio::MYSQL_ATOM* atom)
{
    switch (atom->aio_type_)
    {
    case MYSQL_CONNECT:
        atom->result_ = zce::mysql::exe::connect(
            atom->db_connect_,
            atom->host_name_,
            atom->user_,
            atom->pwd_,
            atom->port_);
        break;
    case MYSQL_DISCONNECT:
        atom->result_ = 0;
        zce::mysql::exe::disconnect(
            atom->db_connect_);
        break;
    case MYSQL_QUERY_NOSELECT:
        atom->result_ = zce::mysql::exe::query(
            atom->db_connect_,
            atom->sql_,
            atom->sql_len_,
            atom->num_affect_,
            atom->insert_id_);
        break;
    case MYSQL_QUERY_SELECT:
        atom->result_ = zce::mysql::exe::query(
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
void worker::thread_host(zce::aio::HOST_ATOM* atom)
{
    switch (atom->aio_type_)
    {
    case HOST_GETADDRINFO_ARY:
        atom->result_ = zce::getaddrinfo_to_addrary(
            atom->hostname_,
            atom->service_,
            atom->ary_addr_num_,
            atom->ary_addr_,
            atom->ary_addr6_num_,
            atom->ary_addr6_);
        break;
    case HOST_GETADDRINFO_ONE:
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

//在线程中处理Socket请求
void worker::thread_socket_timeout(zce::aio::SOCKET_TIMEOUT_ATOM* atom)
{
    ssize_t len = 0;
    switch (atom->aio_type_)
    {
    case SOCKET_CONNECT_ADDR:
        atom->result_ = zce::connect_timeout(
            atom->handle_,
            atom->addr_,
            atom->addr_len_,
            *atom->timeout_tv_);
        break;
    case SOCKET_CONNECT_HOST:
        atom->result_ = zce::connect_timeout(
            atom->handle_,
            atom->host_name_,
            atom->host_port_,
            atom->host_addr_,
            atom->addr_len_,
            *atom->timeout_tv_);
        break;
    case SOCKET_ACCEPT:
        atom->result_ = zce::accept_timeout(
            atom->handle_,
            atom->accept_hdl_,
            atom->from_,
            atom->from_len_,
            *atom->timeout_tv_);
        if (*(atom->accept_hdl_) == ZCE_INVALID_SOCKET)
        {
            atom->result_ = -1;
        }
        else
        {
            atom->result_ = 0;
        }
        break;
    case SOCKET_RECV:
        len = zce::recvn_timeout(
            atom->handle_,
            atom->rcv_buf_,
            atom->len_,
            *atom->timeout_tv_,
            atom->flags_,
            true);
        if (len > 0)
        {
            atom->result_ = 0;
            *(atom->result_len_) = len;
        }
        else
        {
            atom->result_ = -1;
            *(atom->result_len_) = 0;
        }
        break;
    case SOCKET_SEND:
        len = zce::sendn_timeout(
            atom->handle_,
            atom->snd_buf_,
            atom->len_,
            *atom->timeout_tv_,
            atom->flags_);
        if (len > 0)
        {
            atom->result_ = 0;
            *(atom->result_len_) = len;
        }
        else
        {
            atom->result_ = -1;
            *(atom->result_len_) = 0;
        }
        break;
    case SOCKET_RECVFROM:
        len = zce::recvfrom_timeout(
            atom->handle_,
            atom->rcv_buf_,
            atom->len_,
            atom->from_,
            atom->from_len_,
            *atom->timeout_tv_,
            atom->flags_);
        if (len > 0)
        {
            atom->result_ = 0;
            *(atom->result_len_) = len;
        }
        else
        {
            atom->result_ = -1;
            *(atom->result_len_) = 0;
        }
        break;
    default:
        break;
    }
}

int worker::reg_event(ZCE_HANDLE handle,
                      RECTOR_EVENT event_todo,
                      event_callback_t call_back)
{
    return reactor_->register_event(handle,
                                    event_todo,
                                    call_back);
}
}