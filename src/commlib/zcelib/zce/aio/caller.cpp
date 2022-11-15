#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/socket.h"
#include "zce/event/handle_base.h"
#include "zce/event/reactor_base.h"
#include "zce/aio/worker.h"
#include "zce/aio/caller.h"

namespace zce::aio
{
void AIO_ATOM::clear()
{
    aio_type_ = AIO_TYPE::AIO_INVALID;
    id_ = 0;
    std::function<void(AIO_ATOM*)> tmp;
    call_back_.swap(tmp);
    result_ = -1;
}

void FS_ATOM::clear()
{
    AIO_ATOM::clear();
    path_ = nullptr;
    flags_ = 0;
    mode_ = 0;
    handle_ = ZCE_INVALID_HANDLE;
    offset_ = 0;
    whence_ = SEEK_CUR;
    read_bufs_ = nullptr;
    write_bufs_ = nullptr;
    bufs_len_ = 0;
    result_len_ = 0;
    new_path_ = nullptr;
    file_stat_ = nullptr;
}

//====================================================

//!异步打开某个文件，完成后回调函数call_back
int fs_open(zce::aio::worker* worker,
            const char* path,
            int flags,
            int mode,
            std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_OPEN;
    aio_atom->path_ = path;
    aio_atom->flags_ = flags;
    aio_atom->mode_ = mode;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!关闭某个文件
int fs_close(zce::aio::worker* worker,
             ZCE_HANDLE handle,
             std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_CLOSE;
    aio_atom->handle_ = handle;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!移动文件的读写位置
int fs_lseek(zce::aio::worker* worker,
             ZCE_HANDLE handle,
             off_t offset,
             int whence,
             std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_LSEEK;
    aio_atom->handle_ = handle;
    aio_atom->offset_ = offset;
    aio_atom->whence_ = whence;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步读取文件内容
int fs_read(zce::aio::worker* worker,
            ZCE_HANDLE handle,
            char* read_bufs,
            size_t nbufs,
            size_t *result_len,
            std::function<void(AIO_ATOM*)> call_back,
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
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步写入文件内容
int fs_write(zce::aio::worker* worker,
             ZCE_HANDLE handle,
             const char* write_bufs,
             size_t nbufs,
             size_t *result_len,
             std::function<void(AIO_ATOM*)> call_back,
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
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步截断文件
int fs_ftruncate(zce::aio::worker* worker,
                 ZCE_HANDLE handle,
                 size_t offset,
                 std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_FTRUNCATE;
    aio_atom->handle_ = handle;
    aio_atom->offset_ = offset;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步打开文件，读取文件内容，然后关闭
int fs_readfile(zce::aio::worker* worker,
                const char* path,
                char* read_bufs,
                size_t nbufs,
                std::function<void(AIO_ATOM*)> call_back,
                ssize_t offset)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_READFILE;
    aio_atom->path_ = path;
    aio_atom->read_bufs_ = read_bufs;
    aio_atom->bufs_len_ = nbufs;
    aio_atom->offset_ = offset;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步打开文件，写入文件内容，然后关闭
int fs_writefile(zce::aio::worker* worker,
                 const char* path,
                 const char* write_bufs,
                 size_t nbufs,
                 std::function<void(AIO_ATOM*)> call_back,
                 ssize_t offset)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_WRITEFILE;
    aio_atom->path_ = path;
    aio_atom->write_bufs_ = write_bufs;
    aio_atom->bufs_len_ = nbufs;
    aio_atom->offset_ = offset;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步删除文件
int fs_unlink(zce::aio::worker* worker,
              const char* path,
              std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_UNLINK;
    aio_atom->path_ = path;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//! 文件重命名
int fs_rename(zce::aio::worker* worker,
              const char* path,
              const char* new_path,
              std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_RENAME;
    aio_atom->path_ = path;
    aio_atom->new_path_ = new_path;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步获取stat
int fs_stat(zce::aio::worker* worker,
            const char* path,
            struct stat* file_stat,
            std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<FS_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::FS_STAT;
    aio_atom->path_ = path;
    aio_atom->file_stat_ = file_stat;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//=======================================================================
void DIR_ATOM::clear()
{
    AIO_ATOM::clear();

    dirname_ = nullptr;
    namelist_ = nullptr;
    mode_ = 0;
}
//!异步scandir
int dir_scandir(zce::aio::worker* worker,
                const char* dirname,
                struct dirent*** namelist,
                std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<DIR_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::DIR_SCANDIR;
    aio_atom->dirname_ = dirname;
    aio_atom->namelist_ = namelist;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步建立dir
int dir_mkdir(zce::aio::worker* worker,
              const char* dirname,
              int mode,
              std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<DIR_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::DIR_MKDIR;
    aio_atom->dirname_ = dirname;
    aio_atom->mode_ = mode;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步删除dir
int dir_rmdir(zce::aio::worker* worker,
              const char* dirname,
              std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<DIR_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::DIR_RMDIR;
    aio_atom->dirname_ = dirname;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//=======================================================================
void MYSQL_ATOM::clear()
{
    AIO_ATOM::clear();
    result_ = -1;
    db_connect_ = nullptr;
    host_name_ = nullptr;
    user_ = nullptr;
    pwd_ = nullptr;
    port_ = MYSQL_PORT;
    sql_ = nullptr;
    sql_len_ = 0;
    num_affect_ = nullptr;
    insert_id_ = nullptr;
    db_result_ = nullptr;
}

//!链接数据
int mysql_connect(zce::aio::worker* worker,
                  zce::mysql::connect* db_connect,
                  const char* host_name,
                  const char* user,
                  const char* pwd,
                  unsigned int port,
                  std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<MYSQL_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::MYSQL_CONNECT;
    aio_atom->db_connect_ = db_connect;
    aio_atom->host_name_ = host_name;
    aio_atom->user_ = user;
    aio_atom->pwd_ = pwd;
    aio_atom->port_ = port;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!断开数据库链接
int mysql_disconnect(zce::aio::worker* worker,
                     zce::mysql::connect* db_connect,
                     std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<MYSQL_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::MYSQL_DISCONNECT;
    aio_atom->db_connect_ = db_connect;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!查询，非SELECT语句
int mysql_query(zce::aio::worker* worker,
                zce::mysql::connect* db_connect,
                const char* sql,
                size_t sql_len,
                uint64_t* num_affect,
                uint64_t* insert_id,
                std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<MYSQL_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::MYSQL_QUERY_NOSELECT;
    aio_atom->db_connect_ = db_connect;
    aio_atom->sql_ = sql;
    aio_atom->sql_len_ = sql_len;
    aio_atom->num_affect_ = num_affect;
    aio_atom->insert_id_ = insert_id;
    aio_atom->call_back_ = std::move(call_back);

    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!查询，SELECT语句
int mysql_query(zce::aio::worker* worker,
                zce::mysql::connect* db_connect,
                const char* sql,
                size_t sql_len,
                uint64_t* num_affect,
                zce::mysql::result* db_result,
                std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<MYSQL_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::MYSQL_QUERY_SELECT;
    aio_atom->db_connect_ = db_connect;
    aio_atom->sql_ = sql;
    aio_atom->sql_len_ = sql_len;
    aio_atom->num_affect_ = num_affect;
    aio_atom->db_result_ = db_result;
    aio_atom->call_back_ = std::move(call_back);

    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//=======================================================================
void HOST_ATOM::clear()
{
    AIO_ATOM::clear();
    hostname_ = nullptr;
    service_ = nullptr;
    ary_addr_num_ = nullptr;
    ary_addr_ = nullptr;
    ary_addr6_num_ = nullptr;
    ary_addr6_ = nullptr;
}
//!
int host_getaddr_ary(zce::aio::worker* worker,
                     const char* hostname,
                     const char* service,
                     size_t* ary_addr_num,
                     ::sockaddr_in* ary_addr,
                     size_t* ary_addr6_num,
                     ::sockaddr_in6* ary_addr6,
                     std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<HOST_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::HOST_GETADDRINFO_ARY;
    aio_atom->hostname_ = hostname;
    aio_atom->service_ = service;
    aio_atom->ary_addr_num_ = ary_addr_num;
    aio_atom->ary_addr_ = ary_addr;
    aio_atom->ary_addr6_num_ = ary_addr6_num;
    aio_atom->ary_addr6_ = ary_addr6;
    aio_atom->call_back_ = std::move(call_back);

    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

int host_getaddr_one(zce::aio::worker* worker,
                     const char* hostname,
                     const char* service,
                     sockaddr* addr,
                     socklen_t addr_len,
                     std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<HOST_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::HOST_GETADDRINFO_ONE;
    aio_atom->hostname_ = hostname;
    aio_atom->service_ = service;
    aio_atom->addr_ = addr;
    aio_atom->addr_len_ = addr_len;
    aio_atom->call_back_ = std::move(call_back);

    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//=======================================================================
void SOCKET_TIMEOUT_ATOM::clear()
{
    AIO_ATOM::clear();
    //!参数
    result_ = -1;
    handle_ = ZCE_INVALID_SOCKET;
    addr_ = nullptr;
    addr_len_ = 0;
    snd_buf_ = nullptr;
    rcv_buf_ = nullptr;
    len_ = 0;
    timeout_tv_ = nullptr;
    result_len_ = nullptr;
    flags_ = 0;
    from_ = nullptr;
    from_len_ = nullptr;
    accept_hdl_ = nullptr;
    host_name_ = nullptr;
    host_port_ = 0;
    host_addr_ = nullptr;
}
//!超时链接数据
int st_connect(zce::aio::worker* worker,
               ZCE_SOCKET handle,
               const sockaddr* addr,
               socklen_t addr_len,
               zce::time_value* timeout_tv,
               std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_CONNECT_ADDR;
    aio_atom->handle_ = handle;
    aio_atom->addr_ = addr;
    aio_atom->addr_len_ = addr_len;
    aio_atom->timeout_tv_ = timeout_tv;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

int st_connect(zce::aio::worker* worker,
               ZCE_SOCKET handle,
               const char* host_name,
               uint16_t host_port,
               sockaddr* host_addr,
               socklen_t addr_len,
               zce::time_value& timeout_tv,
               std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_CONNECT_HOST;
    aio_atom->handle_ = handle;
    aio_atom->host_name_ = host_name;
    aio_atom->host_port_ = host_port;
    aio_atom->host_addr_ = host_addr;
    aio_atom->addr_len_ = addr_len;
    aio_atom->timeout_tv_ = &timeout_tv;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

int st_accept(zce::aio::worker* worker,
              ZCE_SOCKET handle,
              ZCE_SOCKET *accept_hdl,
              sockaddr* from,
              socklen_t* from_len,
              zce::time_value* timeout_tv,
              std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_ACCEPT;
    aio_atom->handle_ = handle;
    aio_atom->accept_hdl_ = accept_hdl;
    aio_atom->from_ = from;
    aio_atom->from_len_ = from_len;
    aio_atom->timeout_tv_ = timeout_tv;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!
int st_recv(zce::aio::worker* worker,
            ZCE_SOCKET handle,
            void* buf,
            size_t len,
            size_t *result_len,
            zce::time_value* timeout_tv,
            std::function<void(AIO_ATOM*)> call_back,
            int flags)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_RECV;
    aio_atom->handle_ = handle;
    aio_atom->rcv_buf_ = buf;
    aio_atom->len_ = len;
    aio_atom->result_len_ = result_len;
    aio_atom->flags_ = flags;
    aio_atom->timeout_tv_ = timeout_tv;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!
int st_send(zce::aio::worker* worker,
            ZCE_SOCKET handle,
            const void* buf,
            size_t len,
            size_t *result_len,
            zce::time_value* timeout_tv,
            std::function<void(AIO_ATOM*)> call_back,
            int flags)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_SEND;
    aio_atom->handle_ = handle;
    aio_atom->snd_buf_ = buf;
    aio_atom->len_ = len;
    aio_atom->result_len_ = result_len;
    aio_atom->flags_ = flags;
    aio_atom->timeout_tv_ = timeout_tv;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

int st_recvfrom(zce::aio::worker* worker,
                ZCE_SOCKET handle,
                void* buf,
                size_t len,
                size_t *result_len,
                sockaddr* from,
                socklen_t* from_len,
                zce::time_value* timeout_tv,
                std::function<void(AIO_ATOM*)> call_back,
                int flags)
{
    auto aio_atom = worker->alloc_handle<SOCKET_TIMEOUT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::SOCKET_RECVFROM;
    aio_atom->handle_ = handle;
    aio_atom->rcv_buf_ = buf;
    aio_atom->len_ = len;
    aio_atom->result_len_ = result_len;
    aio_atom->flags_ = flags;
    aio_atom->from_ = from;
    aio_atom->from_len_ = from_len;
    aio_atom->timeout_tv_ = timeout_tv;
    aio_atom->call_back_ = std::move(call_back);
    auto succ_req = worker->request(std::move(aio_atom));
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//=======================================================================

int EVENT_ATOM::event_do(std::shared_ptr<void> &atom,
                         ZCE_HANDLE socket,
                         RECTOR_EVENT event,
                         bool connect_succ)
{
    auto eva = (EVENT_ATOM *)(atom.get());
    assert((ZCE_HANDLE)eva->handle_ == socket);
    if ((ZCE_HANDLE)eva->handle_ != socket)
    {
        return -1;
    }
    if (event == RECTOR_EVENT::READ_MASK)
    {
        ssize_t sz_rcv = 0;
        if (AIO_TYPE::EVENT_RECV == eva->aio_type_)
        {
            sz_rcv = zce::recv(eva->handle_,
                               eva->rcv_buf_,
                               eva->len_);
        }
        else if (AIO_TYPE::EVENT_RECVFROM == eva->aio_type_)
        {
            sz_rcv = zce::recvfrom(eva->handle_,
                                   eva->rcv_buf_,
                                   eva->len_,
                                   0,
                                   eva->from_,
                                   eva->from_len_);
        }

        if (sz_rcv >= 0)
        {
            eva->result_ = 0;
            *eva->result_len_ = sz_rcv;
        }
        else
        {
            eva->result_ = -1;
            *eva->result_len_ = 0;
        }
    }
    else if (event == RECTOR_EVENT::WRITE_MASK)
    {
        ssize_t sz_rcv = zce::recv(eva->handle_,
                                   eva->rcv_buf_,
                                   eva->len_);
        if (sz_rcv >= 0)
        {
            eva->result_ = 0;
            *eva->result_len_ = sz_rcv;
        }
        else
        {
            eva->result_ = -1;
            *eva->result_len_ = 0;
        }
    }
    else if (event == RECTOR_EVENT::CONNECT_MASK)
    {
        if (connect_succ)
        {
            eva->result_ = 0;
        }
        else
        {
            eva->result_ = -1;
        }
    }
    else if (event == RECTOR_EVENT::ACCEPT_MASK)
    {
        *eva->accept_hdl_ = zce::accept(eva->handle_,
                                        eva->from_,
                                        eva->from_len_);
        if (*eva->accept_hdl_ != ZCE_INVALID_SOCKET)
        {
            eva->result_ = 0;
        }
        else
        {
            eva->result_ = -1;
        }
    }
    else if (event == RECTOR_EVENT::EXCEPTION_MASK)
    {
        //暂时没哟
    }
    else if (event == RECTOR_EVENT::INOTIFY_MASK)
    {
        //暂时没哟
    }
    else
    {
        assert(false);
    }
    eva->call_back_(eva);
    return 0;
}

//!清理
void EVENT_ATOM::clear()
{
    result_len_ = nullptr;
    //
    handle_ = ZCE_INVALID_SOCKET;
    addr_ = nullptr;
    addr_len_ = 0;
    snd_buf_ = nullptr;
    rcv_buf_ = nullptr;
    len_ = 0;

    flags_ = 0;
    from_ = nullptr;
    from_len_ = nullptr;
    host_name_ = nullptr;
    host_port_ = 0;
    host_addr_ = nullptr;
    accept_hdl_ = nullptr;
}

//事件模式的函数如下N个

int er_connect(zce::aio::worker* worker,
               ZCE_SOCKET handle,
               const sockaddr* addr,
               socklen_t addr_len,
               bool *alread_do,
               std::function<void(AIO_ATOM*)> call_back)
{
    int ret = 0;
    *alread_do = false;
    //使用非阻塞的方式搞一次
    ret = zce::connect(handle, addr, addr_len);
    if (ret == 0)
    {
        *alread_do = true;
        return 0;
    }
    else
    {
        if (zce::last_error() != EWOULDBLOCK)
        {
            return -1;
        }
    }
    auto aio_atom = worker->alloc_handle<EVENT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::EVENT_CONNECT;
    aio_atom->handle_ = handle;
    aio_atom->call_back_ = std::move(call_back);
    ret = worker->reg_event(
        (ZCE_HANDLE)handle,
        RECTOR_EVENT::CONNECT_MASK,
        std::bind(&EVENT_ATOM::event_do,
        std::shared_ptr<void>(aio_atom),
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3));
    if (ret != 0)
    {
        return 0;
    }
    return 0;
}

int er_accept(zce::aio::worker* worker,
              ZCE_SOCKET handle,
              ZCE_SOCKET *accept_hdl,
              sockaddr* from,
              socklen_t* from_len,
              bool *alread_do,
              std::function<void(AIO_ATOM*)> call_back)
{
    int ret = 0;
    *alread_do = false;
    //使用非阻塞的方式搞一次
    *accept_hdl = zce::accept(handle, from, from_len);
    if (*accept_hdl != ZCE_INVALID_SOCKET)
    {
        *alread_do = true;
        return 0;
    }
    else
    {
        if (zce::last_error() != EWOULDBLOCK)
        {
            return -1;
        }
    }
    auto aio_atom = worker->alloc_handle<EVENT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::EVENT_ACCEPT;
    aio_atom->handle_ = handle;
    aio_atom->call_back_ = std::move(call_back);
    aio_atom->accept_hdl_ = accept_hdl;
    aio_atom->from_ = from;
    aio_atom->from_len_ = from_len;
    ret = worker->reg_event(
        (ZCE_HANDLE)handle,
        RECTOR_EVENT::ACCEPT_MASK,
        std::bind(&EVENT_ATOM::event_do,
        std::shared_ptr<void>(aio_atom),
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3));
    if (ret != 0)
    {
        return 0;
    }
    return 0;
}

int er_recv(zce::aio::worker* worker,
            ZCE_SOCKET handle,
            void* rcv_buf,
            size_t len,
            size_t *result_len,
            bool *alread_do,
            std::function<void(AIO_ATOM*)> call_back)
{
    int ret = 0;
    *alread_do = false;
    //使用非阻塞的方式搞一次
    ssize_t sz_rcv = zce::recv(handle, rcv_buf, len);
    if (sz_rcv > 0)
    {
        *alread_do = true;
        *result_len = sz_rcv;
        return 0;
    }
    else
    {
        if (zce::last_error() != EWOULDBLOCK)
        {
            return -1;
        }
    }
    auto aio_atom = worker->alloc_handle<EVENT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::EVENT_RECV;
    aio_atom->handle_ = handle;
    aio_atom->rcv_buf_ = rcv_buf;
    aio_atom->len_ = len;
    aio_atom->result_len_ = result_len;
    aio_atom->call_back_ = std::move(call_back);
    ret = worker->reg_event(
        (ZCE_HANDLE)handle,
        RECTOR_EVENT::READ_MASK,
        std::bind(&EVENT_ATOM::event_do,
        std::shared_ptr<void>(aio_atom),
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3));
    if (ret != 0)
    {
        return 0;
    }
    return 0;
}

int er_send(zce::aio::worker* worker,
            ZCE_SOCKET handle,
            const void* snd_buf,
            size_t len,
            size_t *result_len,
            bool *alread_do,
            std::function<void(AIO_ATOM*)> call_back)
{
    int ret = 0;
    *alread_do = false;
    //使用非阻塞的方式搞一次
    ssize_t sz_rcv = zce::send(handle, snd_buf, len);
    if (sz_rcv > 0)
    {
        *alread_do = true;
        *result_len = sz_rcv;
        return 0;
    }
    else
    {
        if (zce::last_error() != EWOULDBLOCK)
        {
            return -1;
        }
    }
    auto aio_atom = worker->alloc_handle<EVENT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::EVENT_SEND;
    aio_atom->handle_ = handle;
    aio_atom->snd_buf_ = snd_buf;
    aio_atom->len_ = len;
    aio_atom->result_len_ = result_len;
    aio_atom->call_back_ = std::move(call_back);
    ret = worker->reg_event(
        (ZCE_HANDLE)handle,
        RECTOR_EVENT::WRITE_MASK,
        std::bind(&EVENT_ATOM::event_do,
        std::shared_ptr<void>(aio_atom),
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3));
    if (ret != 0)
    {
        return 0;
    }
    return 0;
}

int er_recvfrom(zce::aio::worker* worker,
                ZCE_SOCKET handle,
                void* rcv_buf,
                size_t len,
                size_t *result_len,
                sockaddr* from,
                socklen_t* from_len,
                bool *alread_do,
                std::function<void(AIO_ATOM*)> call_back)
{
    int ret = 0;
    *alread_do = false;
    //使用非阻塞的方式搞一次
    ssize_t sz_rcv = zce::recvfrom(handle,
                                   rcv_buf,
                                   len,
                                   0,
                                   from,
                                   from_len);
    if (sz_rcv > 0)
    {
        *alread_do = true;
        *result_len = sz_rcv;
        return 0;
    }
    else
    {
        if (zce::last_error() != EWOULDBLOCK)
        {
            return -1;
        }
    }
    auto aio_atom = worker->alloc_handle<EVENT_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::EVENT_RECVFROM;
    aio_atom->handle_ = handle;
    aio_atom->rcv_buf_ = rcv_buf;
    aio_atom->len_ = len;
    aio_atom->result_len_ = result_len;
    aio_atom->from_ = from;
    aio_atom->from_len_ = from_len;
    aio_atom->call_back_ = std::move(call_back);
    ret = worker->reg_event(
        (ZCE_HANDLE)handle,
        RECTOR_EVENT::READ_MASK,
        std::bind(&EVENT_ATOM::event_do,
        std::shared_ptr<void>(aio_atom),
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3));
    if (ret != 0)
    {
        return 0;
    }
    return 0;
}

//=======================================================================
void TIMER_ATOM::clear()
{
    trigger_tv_ = nullptr;
    timer_id_ = nullptr;
}

int TIMER_ATOM::time_out(std::shared_ptr<void> &atom,
                         const zce::time_value &trigger_tv,
                         int timer_id)
{
    auto toa = (TIMER_ATOM *)(atom.get());
    *(toa->trigger_tv_) = trigger_tv;
    *(toa->timer_id_) = timer_id;
    toa->call_back_(toa);
    return 0;
}

int tmo_schedule(zce::aio::worker* worker,
                 const zce::time_value& timeout_tv,
                 int *timer_id,
                 zce::time_value *trigger_tv,
                 std::function<void(AIO_ATOM*)> call_back)
{
    auto aio_atom = worker->alloc_handle<TIMER_ATOM>();
    aio_atom->aio_type_ = AIO_TYPE::TIMER_SCHEDULE;
    aio_atom->timeout_tv_ = timeout_tv;
    aio_atom->timer_id_ = timer_id;
    aio_atom->trigger_tv_ = trigger_tv;
    aio_atom->call_back_ = std::move(call_back);
    std::shared_ptr<void> void_ptr = std::move(aio_atom);
    int t_id = -1;
    int ret = worker->schedule_timer(std::bind(&TIMER_ATOM::time_out,
                                     void_ptr,
                                     std::placeholders::_1,
                                     std::placeholders::_2),
                                     t_id,
                                     timeout_tv);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}
}