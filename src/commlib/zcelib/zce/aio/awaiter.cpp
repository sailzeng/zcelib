#include "zce/predefine.h"
#include "zce/aio/awaiter.h"

namespace zce::aio
{
bool awaiter_aio::await_ready()
{
    AIO_ATOM *atom_base = (AIO_ATOM *)request_atom_.get();
    if (atom_base->aio_type_ >= AIO_TYPE::AIO_THREAD_BEGIN &&
        atom_base->aio_type_ <= AIO_TYPE::AIO_THREAD_END)
    {
        return aio_thread_await_ready();
    }
    else if (atom_base->aio_type_ >= AIO_TYPE::AIO_EVENT_BEGIN &&
             atom_base->aio_type_ <= AIO_TYPE::AIO_EVENT_END)
    {
        return event_await_ready();
    }
    else if (atom_base->aio_type_ >= AIO_TYPE::AIO_TIMER_BEGIN &&
             atom_base->aio_type_ <= AIO_TYPE::AIO_TIMER_END)
    {
        return timer_out_ready();
    }
    else
    {
        return true;
    }
}

//请求进行多线程的AIO操作，如果请求成功.return false挂起协程
bool awaiter_aio::aio_thread_await_ready()
{
    //绑定回调函数
    AIO_ATOM *atom_base = (AIO_ATOM *)request_atom_.get();
    atom_base->call_back_ = std::bind(&awaiter_aio::resume,
                                      this,
                                      std::placeholders::_1);
    //将一个文件操作句柄放入请求队列
    bool succ_req = worker_->request(std::move(request_atom_));
    if (succ_req)
    {
        return false;
    }
    else
    {
        ret_result_ = -1;
        atom_base->result_ = -1;
        return true;
    }
}

//请求进行事件的AIO操作
bool awaiter_aio::event_await_ready()
{
    auto ev_atom = (zce::aio::EVENT_ATOM *)request_atom_.get();
    ev_atom->call_back_ = std::move(std::bind(&awaiter_aio::resume,
                                    this,
                                    std::placeholders::_1));
    int ret = 0;
    bool alread_do = false;
    if (ev_atom->aio_type_ == AIO_TYPE::EVENT_CONNECT)
    {
        ret = er_connect(worker_,
                         ev_atom->handle_,
                         ev_atom->addr_,
                         ev_atom->addr_len_,
                         &alread_do,
                         ev_atom->call_back_);
    }
    else if (ev_atom->aio_type_ == AIO_TYPE::EVENT_ACCEPT)
    {
        ret = er_accept(worker_,
                        ev_atom->handle_,
                        ev_atom->accept_hdl_,
                        ev_atom->from_,
                        ev_atom->from_len_,
                        &alread_do,
                        ev_atom->call_back_);
    }
    else if (ev_atom->aio_type_ == AIO_TYPE::EVENT_RECV)
    {
        ret = er_recv(worker_,
                      ev_atom->handle_,
                      ev_atom->rcv_buf_,
                      ev_atom->len_,
                      ev_atom->result_len_,
                      &alread_do,
                      ev_atom->call_back_);
    }
    else if (ev_atom->aio_type_ == AIO_TYPE::EVENT_SEND)
    {
        ret = er_send(worker_,
                      ev_atom->handle_,
                      ev_atom->snd_buf_,
                      ev_atom->len_,
                      ev_atom->result_len_,
                      &alread_do,
                      ev_atom->call_back_);
    }
    else if (ev_atom->aio_type_ == AIO_TYPE::EVENT_RECVFROM)
    {
        ret = er_recvfrom(worker_,
                          ev_atom->handle_,
                          ev_atom->rcv_buf_,
                          ev_atom->len_,
                          ev_atom->result_len_,
                          ev_atom->from_,
                          ev_atom->from_len_,
                          &alread_do,
                          ev_atom->call_back_);
    }

    if (ret != 0 || (ret == 0 && alread_do == true))
    {
        ret_result_ = -1;
        ev_atom->result_ = -1;
        //返回true其实是表示已经完成，不挂起
        return true;
    }
    else
    {
        //没有完成，挂起
        return false;
    }
}

bool awaiter_aio::timer_out_ready()
{
    auto to_atom = (zce::aio::TIMER_ATOM *)request_atom_.get();
    to_atom->call_back_ = std::move(std::bind(&awaiter_aio::resume,
                                    this,
                                    std::placeholders::_1));
    int ret = 0;
    if (to_atom->aio_type_ == AIO_TYPE::TIMER_SCHEDULE)
    {
        ret = tmo_schedule(worker_,
                           to_atom->timeout_tv_,
                           to_atom->timer_id_,
                           to_atom->trigger_tv_,
                           to_atom->call_back_);
    }
    if (ret != 0)
    {
        ret_result_ = -1;
        to_atom->result_ = -1;
        //返回true其实是表示已经完成，不挂起
        return true;
    }
    else
    {
        //没有完成，挂起
        return false;
    }
}

//============================================================================
//协程co_await打开某个文件，
awaiter_aio co_fs_open(zce::aio::worker* worker,
                       const char* path,
                       int flags,
                       int mode)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_OPEN;
    aio_atom->path_ = path;
    aio_atom->flags_ = flags;
    aio_atom->mode_ = mode;
    return awaiter_aio(worker, aio_atom);
}

// 协程co_await的AIO close 函数
awaiter_aio co_fs_close(zce::aio::worker* worker,
                        ZCE_HANDLE handle)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_CLOSE;
    aio_atom->handle_ = handle;
    return awaiter_aio(worker, aio_atom);
}

//!协程co_await移动文件的当前位置,
awaiter_aio co_fs_lseek(zce::aio::worker* worker,
                        ZCE_HANDLE handle,
                        off_t offset,
                        int whence)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_LSEEK;
    aio_atom->handle_ = handle;
    aio_atom->offset_ = offset;
    aio_atom->whence_ = whence;
    return awaiter_aio(worker, aio_atom);
}

//!协程co_await读取文件内容
awaiter_aio co_fs_read(zce::aio::worker* worker,
                       ZCE_HANDLE handle,
                       char* read_bufs,
                       size_t nbufs,
                       size_t *result_len,
                       ssize_t offset,
                       int whence)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_READ;
    aio_atom->handle_ = handle;
    aio_atom->read_bufs_ = read_bufs;
    aio_atom->bufs_len_ = nbufs;
    aio_atom->result_len_ = result_len;
    aio_atom->offset_ = offset;
    aio_atom->whence_ = whence;
    return awaiter_aio(worker, aio_atom);
}

//!协程co_await写入文件内容
awaiter_aio co_fs_write(zce::aio::worker* worker,
                        ZCE_HANDLE handle,
                        const char* write_bufs,
                        size_t nbufs,
                        size_t *result_len,
                        ssize_t offset,
                        int whence)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_WRITE;
    aio_atom->handle_ = handle;
    aio_atom->write_bufs_ = write_bufs;
    aio_atom->bufs_len_ = nbufs;
    aio_atom->result_len_ = result_len;
    aio_atom->offset_ = offset;
    aio_atom->whence_ = whence;
    return awaiter_aio(worker, aio_atom);
}

//!异步截断文件
awaiter_aio co_fs_ftruncate(zce::aio::worker* worker,
                            ZCE_HANDLE handle,
                            size_t offset)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_FTRUNCATE;
    aio_atom->handle_ = handle;
    aio_atom->offset_ = offset;
    return awaiter_aio(worker, aio_atom);
}

//AIO 协程co_await的readfile 函数
awaiter_aio co_fs_readfile(zce::aio::worker* worker,
                           const char* path,
                           char* read_bufs,
                           size_t nbufs,
                           size_t *result_len,
                           ssize_t offset)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_READFILE;
    aio_atom->path_ = path;
    aio_atom->read_bufs_ = read_bufs;
    aio_atom->bufs_len_ = nbufs;
    aio_atom->offset_ = offset;
    aio_atom->result_len_ = result_len;
    return awaiter_aio(worker, aio_atom);
}

//! 协程的writefile
awaiter_aio co_fs_writefile(zce::aio::worker* worker,
                            const char* path,
                            const char* write_bufs,
                            size_t nbufs,
                            size_t *result_len,
                            ssize_t offset)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_WRITEFILE;
    aio_atom->path_ = path;
    aio_atom->write_bufs_ = write_bufs;
    aio_atom->bufs_len_ = nbufs;
    aio_atom->offset_ = offset;
    aio_atom->result_len_ = result_len;
    return awaiter_aio(worker, aio_atom);
}

//协程co_await异步删除文件
awaiter_aio co_fs_unlink(zce::aio::worker* worker,
                         const char* path)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_UNLINK;
    aio_atom->path_ = path;
    return awaiter_aio(worker, aio_atom);
}

// 协程co_await异步文件重命名
awaiter_aio co_fs_rename(zce::aio::worker* worker,
                         const char* path,
                         const char* new_path)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_RENAME;
    aio_atom->path_ = path;
    aio_atom->new_path_ = new_path;
    return awaiter_aio(worker, aio_atom);
}

//协程co_await异步获取stat
awaiter_aio co_fs_stat(zce::aio::worker* worker,
                       const char* path,
                       struct stat* file_stat)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_STAT;
    aio_atom->path_ = path;
    aio_atom->file_stat_ = file_stat;
    return awaiter_aio(worker, aio_atom);
}

//============================================================================
//! 异步scandir,参数参考scandir，namelist可以用free_scandir_list要释放
awaiter_aio co_dir_scandir(zce::aio::worker* worker,
                           const char* dirname,
                           struct dirent*** namelist)
{
    auto aio_atom = worker->alloc_handle<DIR_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::DIR_SCANDIR;
    aio_atom->dirname_ = dirname;
    aio_atom->namelist_ = namelist;

    return awaiter_aio(worker, aio_atom);
}

//!异步建立dir
awaiter_aio co_dir_mkdir(zce::aio::worker* worker,
                         const char* dirname,
                         int mode)
{
    auto aio_atom = worker->alloc_handle<DIR_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::DIR_MKDIR;
    aio_atom->dirname_ = dirname;
    aio_atom->mode_ = mode;
    return awaiter_aio(worker, aio_atom);
}

//!异步删除dir
awaiter_aio co_dir_rmdir(zce::aio::worker* worker,
                         const char* dirname)
{
    auto aio_atom = worker->alloc_handle<DIR_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::DIR_RMDIR;
    aio_atom->dirname_ = dirname;
    return awaiter_aio(worker, aio_atom);
}

//============================================================================
//!链接数据
awaiter_aio co_mysql_connect(zce::aio::worker* worker,
                             zce::mysql::connect* db_connect,
                             const char* host_name,
                             const char* user,
                             const char* pwd,
                             unsigned int port)
{
    auto aio_atom = worker->alloc_handle<MYSQL_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::MYSQL_CONNECT;
    aio_atom->db_connect_ = db_connect;
    aio_atom->host_name_ = host_name;
    aio_atom->user_ = user;
    aio_atom->pwd_ = pwd;
    aio_atom->port_ = port;

    return awaiter_aio(worker, aio_atom);
}

//!断开数据库链接
awaiter_aio co_mysql_disconnect(zce::aio::worker* worker,
                                zce::mysql::connect* db_connect)
{
    auto aio_atom = worker->alloc_handle<MYSQL_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::MYSQL_DISCONNECT;
    aio_atom->db_connect_ = db_connect;
    return awaiter_aio(worker, aio_atom);
}

//!查询，非SELECT语句
awaiter_aio co_mysql_query(zce::aio::worker* worker,
                           zce::mysql::connect* db_connect,
                           const char* sql,
                           size_t sql_len,
                           uint64_t* num_affect,
                           uint64_t* insert_id)
{
    auto aio_atom = worker->alloc_handle<MYSQL_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::MYSQL_QUERY_NOSELECT;
    aio_atom->db_connect_ = db_connect;
    aio_atom->sql_ = sql;
    aio_atom->sql_len_ = sql_len;
    aio_atom->num_affect_ = num_affect;
    aio_atom->insert_id_ = insert_id;

    return awaiter_aio(worker, aio_atom);
}

//!查询，SELECT语句
awaiter_aio co_mysql_query(zce::aio::worker* worker,
                           zce::mysql::connect* db_connect,
                           const char* sql,
                           size_t sql_len,
                           uint64_t* num_affect,
                           zce::mysql::result* db_result)
{
    auto aio_atom = worker->alloc_handle<MYSQL_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::MYSQL_QUERY_SELECT;
    aio_atom->db_connect_ = db_connect;
    aio_atom->sql_ = sql;
    aio_atom->sql_len_ = sql_len;
    aio_atom->num_affect_ = num_affect;
    aio_atom->db_result_ = db_result;

    return awaiter_aio(worker, aio_atom);
}

//============================================================================

awaiter_aio co_host_getaddr_ary(zce::aio::worker* worker,
                                const char* hostname,
                                const char* service,
                                size_t* ary_addr_num,
                                ::sockaddr_in* ary_addr,
                                size_t* ary_addr6_num,
                                ::sockaddr_in6* ary_addr6)
{
    auto aio_atom = worker->alloc_handle<HOST_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::HOST_GETADDRINFO_ARY;
    aio_atom->hostname_ = hostname;
    aio_atom->service_ = service;
    aio_atom->ary_addr_num_ = ary_addr_num;
    aio_atom->ary_addr_ = ary_addr;
    aio_atom->ary_addr6_num_ = ary_addr6_num;
    aio_atom->ary_addr6_ = ary_addr6;
    return awaiter_aio(worker, aio_atom);
}

//!获得host对应的一个地址信息，类似getaddrinfo_one
awaiter_aio co_host_getaddr_one(zce::aio::worker* worker,
                                const char* hostname,
                                const char* service,
                                sockaddr* addr,
                                socklen_t addr_len)
{
    auto aio_atom = worker->alloc_handle<HOST_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::HOST_GETADDRINFO_ONE;
    aio_atom->hostname_ = hostname;
    aio_atom->service_ = service;
    aio_atom->addr_ = addr;
    aio_atom->addr_len_ = addr_len;
    return awaiter_aio(worker, aio_atom);
}

//============================================================================
//! 等待若干时间进行connect，直至超时
awaiter_aio co_st_connect(zce::aio::worker* worker,
                          ZCE_SOCKET handle,
                          const sockaddr* addr,
                          socklen_t addr_len,
                          zce::time_value* timeout_tv)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_CONNECT_ADDR;
    aio_atom->handle_ = handle;
    aio_atom->addr_ = addr;
    aio_atom->addr_len_ = addr_len;
    aio_atom->timeout_tv_ = timeout_tv;
    return awaiter_aio(worker, aio_atom);
}

awaiter_aio co_st_connect(zce::aio::worker* worker,
                          ZCE_SOCKET handle,
                          const char* host_name,
                          uint16_t host_port,
                          sockaddr* host_addr,
                          socklen_t addr_len,
                          zce::time_value& timeout_tv)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_CONNECT_HOST;
    aio_atom->handle_ = handle;
    aio_atom->host_name_ = host_name;
    aio_atom->host_port_ = host_port;
    aio_atom->host_addr_ = host_addr;
    aio_atom->addr_len_ = addr_len;
    aio_atom->timeout_tv_ = &timeout_tv;
    return awaiter_aio(worker, aio_atom);
}

//! 等待若干时间进行accept，直至超时
awaiter_aio co_st_accept(zce::aio::worker* worker,
                         ZCE_SOCKET handle,
                         ZCE_SOCKET *accept_hdl,
                         sockaddr* from,
                         socklen_t* from_len,
                         zce::time_value* timeout_tv)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_ACCEPT;
    aio_atom->handle_ = handle;
    aio_atom->accept_hdl_ = accept_hdl;
    aio_atom->from_ = from;
    aio_atom->from_len_ = from_len;
    aio_atom->timeout_tv_ = timeout_tv;
    return awaiter_aio(worker, aio_atom);
}

//! 等待若干时间进行recv，直至超时
awaiter_aio co_st_recv(zce::aio::worker* worker,
                       ZCE_SOCKET handle,
                       void* buf,
                       size_t len,
                       zce::time_value* timeout_tv,
                       int flags)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_RECV;
    aio_atom->handle_ = handle;
    aio_atom->rcv_buf_ = buf;
    aio_atom->len_ = len;
    aio_atom->timeout_tv_ = timeout_tv;
    aio_atom->flags_ = flags;
    return awaiter_aio(worker, aio_atom);
}

//!等待若干时间进行send，直至超时
awaiter_aio co_st_send(zce::aio::worker* worker,
                       ZCE_SOCKET handle,
                       const void* buf,
                       size_t len,
                       zce::time_value* timeout_tv,
                       int flags)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_SEND;
    aio_atom->handle_ = handle;
    aio_atom->snd_buf_ = buf;
    aio_atom->len_ = len;
    aio_atom->timeout_tv_ = timeout_tv;
    aio_atom->flags_ = flags;
    return awaiter_aio(worker, aio_atom);
}

//!等待若干时间进行recv数据，直至超时
awaiter_aio co_st_recvfrom(zce::aio::worker* worker,
                           ZCE_SOCKET handle,
                           void* buf,
                           size_t len,
                           sockaddr* from,
                           socklen_t* from_len,
                           zce::time_value* timeout_tv,
                           int flags)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_RECVFROM;
    aio_atom->handle_ = handle;
    aio_atom->rcv_buf_ = buf;
    aio_atom->len_ = len;
    aio_atom->from_ = from;
    aio_atom->from_len_ = from_len;
    aio_atom->timeout_tv_ = timeout_tv;
    aio_atom->flags_ = flags;
    return awaiter_aio(worker, aio_atom);
}

//============================================================================

awaiter_aio co_er_connect(zce::aio::worker* worker,
                          ZCE_SOCKET handle,
                          const sockaddr* addr,
                          socklen_t addr_len)
{
    auto aio_atom = worker->alloc_handle<EVENT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::EVENT_CONNECT;
    aio_atom->handle_ = handle;
    aio_atom->addr_ = addr;
    aio_atom->addr_len_ = addr_len;
    return awaiter_aio(worker, aio_atom);
}

awaiter_aio co_er_accept(zce::aio::worker* worker,
                         ZCE_SOCKET handle,
                         ZCE_SOCKET *accept_hdl,
                         sockaddr* from,
                         socklen_t* from_len)
{
    auto aio_atom = worker->alloc_handle<EVENT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::EVENT_ACCEPT;
    aio_atom->handle_ = handle;
    aio_atom->accept_hdl_ = accept_hdl;
    aio_atom->from_ = from;
    aio_atom->from_len_ = from_len;
    return awaiter_aio(worker, aio_atom);
}

//! 异步进行recv，
awaiter_aio co_er_recv(zce::aio::worker* worker,
                       ZCE_SOCKET handle,
                       void* rcv_buf,
                       size_t len,
                       size_t *result_len)
{
    auto aio_atom = worker->alloc_handle<EVENT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::EVENT_RECV;
    aio_atom->handle_ = handle;
    aio_atom->rcv_buf_ = rcv_buf;
    aio_atom->len_ = len;
    aio_atom->result_len_ = result_len;
    return awaiter_aio(worker, aio_atom);
}

//! 异步进行send，
awaiter_aio co_er_send(zce::aio::worker* worker,
                       ZCE_SOCKET handle,
                       const void* snd_buf,
                       size_t len,
                       size_t *result_len)
{
    auto aio_atom = worker->alloc_handle<EVENT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::EVENT_SEND;
    aio_atom->handle_ = handle;
    aio_atom->snd_buf_ = snd_buf;
    aio_atom->len_ = len;
    aio_atom->result_len_ = result_len;
    return awaiter_aio(worker, aio_atom);
}

//! 异步进行recv数据，
awaiter_aio co_er_recvfrom(zce::aio::worker* worker,
                           ZCE_SOCKET handle,
                           void* rcv_buf,
                           size_t len,
                           size_t *result_len,
                           sockaddr* from,
                           socklen_t* from_len)
{
    auto aio_atom = worker->alloc_handle<EVENT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::EVENT_RECVFROM;
    aio_atom->handle_ = handle;
    aio_atom->rcv_buf_ = rcv_buf;
    aio_atom->len_ = len;
    aio_atom->result_len_ = result_len;
    aio_atom->from_ = from;
    aio_atom->from_len_ = from_len;
    return awaiter_aio(worker, aio_atom);
}

//============================================================================
awaiter_aio  co_timeout_schedule(zce::aio::worker* worker,
                                 const zce::time_value& timeout_tv,
                                 int *timer_id,
                                 zce::time_value* trigger_tv)
{
    auto aio_atom = worker->alloc_handle<TIMER_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::TIMER_SCHEDULE;
    aio_atom->timeout_tv_ = timeout_tv;
    aio_atom->trigger_tv_ = trigger_tv;
    aio_atom->timer_id_ = timer_id;
    return awaiter_aio(worker, aio_atom);
}
}