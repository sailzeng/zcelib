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
worker::worker()
{
}
worker::~worker()
{
    terminate();
}
//!
int worker::initialize(size_t work_thread_num,
                       size_t work_queue_len,
                       size_t max_event_num)
{
    int ret = 0;
    worker_running_ = true;
    work_thread_num_ = work_thread_num;
    work_thread_ = new std::thread * [work_thread_num_];
    for (size_t i = 0; i < work_thread_num_; ++i)
    {
        work_thread_[i] = new std::thread(&zce::aio::worker::process_request,
                                          this);
    }

    requst_queue_ = new zce::msgring_condi<std::shared_ptr<void>>(work_queue_len);
    response_queue_ = new zce::msgring_condi<std::shared_ptr<void>>(work_queue_len);
    aio_obj_pool_.initialize<zce::aio::FS_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::DIR_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::MYSQL_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::HOST_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::SOCKET_TIMEOUT_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::EVENT_ATOM>(128, 256);
    aio_obj_pool_.initialize<zce::aio::TIMER_ATOM>(128, 256);
    reactor_ = new reactor_mini();
    ret = reactor_->initialize(max_event_num, 1024, true);
    if (ret != 0)
    {
        return ret;
    }

    timer_queue_ = new zce::timer_wheel();
    ret = timer_queue_->initialize(1024);
    if (ret != 0)
    {
        return ret;
    }
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

//!处理请求
void worker::process_request()
{
    size_t num_req = 0;
    bool go = false;
    do
    {
        std::shared_ptr<void> base;
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
        std::shared_ptr<void> base;
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
            AIO_ATOM *atom_base = (AIO_ATOM *)base.get();
            //调用回调函数
            atom_base->call_back_(atom_base);
            ++num_rsp;
        }
        timer_queue_->expire();
    } while (go);
}

//!
void worker::thread_aio(const std::shared_ptr<void> &atom)
{
    AIO_ATOM *base = (AIO_ATOM *)atom.get();
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
    response_queue_->enqueue(atom);
}
//!在线程中处理文件
void worker::thread_fs(zce::aio::FS_ATOM* atom)
{
    switch (atom->aio_type_)
    {
    case AIO_TYPE::FS_OPEN:
        atom->result_ = zce::open2(atom->handle_,
                                   atom->path_,
                                   atom->mode_,
                                   atom->flags_);
        break;
    case AIO_TYPE::FS_CLOSE:
        atom->result_ = zce::close(atom->handle_);
        break;
    case AIO_TYPE::FS_LSEEK:
        off_t off;
        atom->result_ = zce::lseek(atom->handle_,
                                   (off_t)atom->offset_,
                                   atom->whence_,
                                   off);
        break;

    case AIO_TYPE::FS_READ:
        atom->result_ = zce::read(atom->handle_,
                                  atom->read_bufs_,
                                  atom->bufs_len_,
                                  atom->result_len_,
                                  (off_t)atom->offset_,
                                  atom->whence_);
        break;
    case AIO_TYPE::FS_WRITE:
        atom->result_ = zce::write(atom->handle_,
                                   atom->write_bufs_,
                                   atom->bufs_len_,
                                   atom->result_len_,
                                   (off_t)atom->offset_,
                                   atom->whence_);
        break;
    case AIO_TYPE::FS_READFILE:
        atom->result_ = zce::read_file(atom->path_,
                                       atom->read_bufs_,
                                       atom->bufs_len_,
                                       atom->result_len_,
                                       (off_t)atom->offset_);
        break;
    case AIO_TYPE::FS_WRITEFILE:
        atom->result_ = zce::write_file(atom->path_,
                                        atom->write_bufs_,
                                        atom->bufs_len_,
                                        atom->result_len_,
                                        (off_t)atom->offset_);
        break;
    case AIO_TYPE::FS_STAT:
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
    case AIO_TYPE::DIR_MKDIR:
        atom->result_ = zce::mkdir(atom->dirname_,
                                   atom->mode_);
        break;
    case AIO_TYPE::DIR_RMDIR:
        atom->result_ = zce::rmdir(atom->dirname_);
        break;
    case AIO_TYPE::DIR_SCANDIR:
        atom->result_ = zce::scandir(atom->dirname_,
                                     atom->namelist_,
                                     nullptr,
                                     nullptr);
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
    case AIO_TYPE::MYSQL_CONNECT:
        atom->result_ = zce::mysql::exe::connect(
            atom->db_connect_,
            atom->host_name_,
            atom->user_,
            atom->pwd_,
            atom->port_);
        break;
    case AIO_TYPE::MYSQL_DISCONNECT:
        atom->result_ = 0;
        zce::mysql::exe::disconnect(
            atom->db_connect_);
        break;
    case AIO_TYPE::MYSQL_QUERY_NOSELECT:
        atom->result_ = zce::mysql::exe::query(
            atom->db_connect_,
            atom->sql_,
            atom->sql_len_,
            atom->num_affect_,
            atom->insert_id_);
        break;
    case AIO_TYPE::MYSQL_QUERY_SELECT:
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
    case AIO_TYPE::HOST_GETADDRINFO_ARY:
        atom->result_ = zce::getaddrinfo_to_addrary(
            atom->hostname_,
            atom->service_,
            atom->ary_addr_num_,
            atom->ary_addr_,
            atom->ary_addr6_num_,
            atom->ary_addr6_);
        break;
    case AIO_TYPE::HOST_GETADDRINFO_ONE:
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
    case AIO_TYPE::SOCKET_CONNECT_ADDR:
        atom->result_ = zce::connect_timeout(
            atom->handle_,
            atom->addr_,
            atom->addr_len_,
            *atom->timeout_tv_);
        break;
    case AIO_TYPE::SOCKET_CONNECT_HOST:
        atom->result_ = zce::connect_timeout(
            atom->handle_,
            atom->host_name_,
            atom->host_port_,
            atom->host_addr_,
            atom->addr_len_,
            *atom->timeout_tv_);
        break;
    case AIO_TYPE::SOCKET_ACCEPT:
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
    case AIO_TYPE::SOCKET_RECV:
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
    case AIO_TYPE::SOCKET_SEND:
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
    case AIO_TYPE::SOCKET_RECVFROM:
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

int worker::schedule_timer(timeout_callback_t call_fun,
                           int &time_id,
                           const zce::time_value& delay_time)
{
    timer_queue_->schedule_timer(call_fun,
                                 time_id,
                                 delay_time);
    return 0;
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