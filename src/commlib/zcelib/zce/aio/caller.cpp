#include "zce/predefine.h"
#include "zce/aio/worker.h"
#include "zce/aio/caller.h"

namespace zce::aio
{
void AIO_Atom::clear()
{
    aio_type_ = AIO_INVALID;
    id_ = 0;
}

void FS_Atom::clear()
{
    AIO_Atom::clear();
    result_ = -1;
    path_ = nullptr;
    flags_ = 0;
    mode_ = 0;
    handle_ = ZCE_INVALID_HANDLE;
    offset_ = 0;
    whence_ = SEEK_CUR;
    read_bufs_ = nullptr;
    write_bufs_ = nullptr;
    bufs_count_ = 0;
    result_count_ = 0;
    new_path_ = nullptr;
    file_stat_ = nullptr;
}

void Dir_Atom::clear()
{
    AIO_Atom::clear();
    result_ = -1;
    dirname_ = nullptr;
    namelist_ = nullptr;
    mode_ = 0;
}

void MySQL_Atom::clear()
{
    AIO_Atom::clear();
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

void Host_Atom::clear()
{
    AIO_Atom::clear();
    hostname_ = nullptr;
    service_ = nullptr;
    ary_addr_num_ = nullptr;
    ary_addr_ = nullptr;
    ary_addr6_num_ = nullptr;
    ary_addr6_ = nullptr;
}

//====================================================

//!异步打开某个文件，完成后回调函数call_back
int fs_open(zce::aio::Worker* worker,
            const char* path,
            int flags,
            int mode,
            std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::FS_Atom* hdl = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_OPEN);
    hdl->path_ = path;
    hdl->flags_ = flags;
    hdl->mode_ = mode;
    hdl->call_back_ = call_back;
    auto succ_req = worker->request(hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!关闭某个文件
int fs_close(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::FS_Atom* hdl = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_CLOSE);
    hdl->handle_ = handle;
    hdl->call_back_ = call_back;
    auto succ_req = worker->request(hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!移动文件的读写位置
int fs_lseek(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             off_t offset,
             int whence,
             std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::FS_Atom* hdl = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_LSEEK);
    hdl->handle_ = handle;
    hdl->offset_ = offset;
    hdl->whence_ = whence;
    hdl->call_back_ = call_back;
    auto succ_req = worker->request(hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步读取文件内容
int fs_read(zce::aio::Worker* worker,
            ZCE_HANDLE handle,
            char* read_bufs,
            size_t nbufs,
            std::function<void(AIO_Atom*)> call_back,
            ssize_t offset,
            int whence)
{
    zce::aio::FS_Atom* aio_atom = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_READ);
    aio_atom->handle_ = handle;
    aio_atom->read_bufs_ = read_bufs;
    aio_atom->bufs_count_ = nbufs;
    aio_atom->offset_ = offset;
    aio_atom->whence_ = whence;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步写入文件内容
int fs_write(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             const char* write_bufs,
             size_t nbufs,
             std::function<void(AIO_Atom*)> call_back,
             ssize_t offset,
             int whence)
{
    zce::aio::FS_Atom* aio_atom = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_WRITE);
    aio_atom->handle_ = handle;
    aio_atom->write_bufs_ = write_bufs;
    aio_atom->bufs_count_ = nbufs;
    aio_atom->offset_ = offset;
    aio_atom->whence_ = whence;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步截断文件
int fs_ftruncate(zce::aio::Worker* worker,
                 ZCE_HANDLE handle,
                 size_t offset,
                 std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::FS_Atom* aio_atom = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_FTRUNCATE);
    aio_atom->handle_ = handle;
    aio_atom->offset_ = offset;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步打开文件，读取文件内容，然后关闭
int fs_read_file(zce::aio::Worker* worker,
                 const char* path,
                 char* read_bufs,
                 size_t nbufs,
                 std::function<void(AIO_Atom*)> call_back,
                 ssize_t offset)
{
    zce::aio::FS_Atom* aio_atom = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_READFILE);
    aio_atom->path_ = path;
    aio_atom->read_bufs_ = read_bufs;
    aio_atom->bufs_count_ = nbufs;
    aio_atom->offset_ = offset;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步打开文件，写入文件内容，然后关闭
int fs_write_file(zce::aio::Worker* worker,
                  const char* path,
                  const char* write_bufs,
                  size_t nbufs,
                  std::function<void(AIO_Atom*)> call_back,
                  ssize_t offset)
{
    zce::aio::FS_Atom* aio_atom = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_WRITEFILE);
    aio_atom->path_ = path;
    aio_atom->write_bufs_ = write_bufs;
    aio_atom->bufs_count_ = nbufs;
    aio_atom->offset_ = offset;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步删除文件
int fs_unlink(zce::aio::Worker* worker,
              const char* path,
              std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::FS_Atom* aio_atom = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_UNLINK);
    aio_atom->path_ = path;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!
int fs_rename(zce::aio::Worker* worker,
              const char* path,
              const char* new_path,
              std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::FS_Atom* aio_atom = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_RENAME);
    aio_atom->path_ = path;
    aio_atom->new_path_ = new_path;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步获取stat
int fs_stat(zce::aio::Worker* worker,
            const char* path,
            struct stat* file_stat,
            std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::FS_Atom* aio_atom = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_STAT);
    aio_atom->path_ = path;
    aio_atom->file_stat_ = file_stat;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步scandir
int dir_scandir(zce::aio::Worker* worker,
                const char* dirname,
                struct dirent*** namelist,
                std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::Dir_Atom* aio_atom = (Dir_Atom*)
        worker->alloc_handle(AIO_TYPE::DIR_SCANDIR);
    aio_atom->dirname_ = dirname;
    aio_atom->namelist_ = namelist;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步建立dir
int dir_mkdir(zce::aio::Worker* worker,
              const char* dirname,
              int mode,
              std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::Dir_Atom* aio_atom = (Dir_Atom*)
        worker->alloc_handle(AIO_TYPE::DIR_MKDIR);
    aio_atom->dirname_ = dirname;
    aio_atom->mode_ = mode;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步删除dir
int dir_rmdir(zce::aio::Worker* worker,
              const char* dirname,
              std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::Dir_Atom* aio_atom = (Dir_Atom*)
        worker->alloc_handle(AIO_TYPE::DIR_RMDIR);
    aio_atom->dirname_ = dirname;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!链接数据
int mysql_connect(zce::aio::Worker* worker,
                  zce::mysql::Connect* db_connect,
                  const char* host_name,
                  const char* user,
                  const char* pwd,
                  unsigned int port,
                  std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::MySQL_Atom* aio_atom = (MySQL_Atom*)
        worker->alloc_handle(AIO_TYPE::MYSQL_CONNECT);
    aio_atom->db_connect_ = db_connect;
    aio_atom->host_name_ = host_name;
    aio_atom->user_ = user;
    aio_atom->pwd_ = pwd;
    aio_atom->port_ = port;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!断开数据库链接
int mysql_disconnect(zce::aio::Worker* worker,
                     zce::mysql::Connect* db_connect,
                     std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::MySQL_Atom* aio_atom = (MySQL_Atom*)
        worker->alloc_handle(AIO_TYPE::MYSQL_DISCONNECT);
    aio_atom->db_connect_ = db_connect;
    aio_atom->call_back_ = call_back;
    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!查询，非SELECT语句
int mysql_query(zce::aio::Worker* worker,
                zce::mysql::Connect* db_connect,
                const char* sql,
                size_t sql_len,
                uint64_t* num_affect,
                uint64_t* insert_id,
                std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::MySQL_Atom* aio_atom = (MySQL_Atom*)
        worker->alloc_handle(AIO_TYPE::MYSQL_QUERY_NOSELECT);
    aio_atom->db_connect_ = db_connect;
    aio_atom->sql_ = sql;
    aio_atom->sql_len_ = sql_len;
    aio_atom->num_affect_ = num_affect;
    aio_atom->insert_id_ = insert_id;
    aio_atom->call_back_ = call_back;

    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!查询，SELECT语句
int mysql_query(zce::aio::Worker* worker,
                zce::mysql::Connect* db_connect,
                const char* sql,
                size_t sql_len,
                uint64_t* num_affect,
                zce::mysql::Result* db_result,
                std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::MySQL_Atom* aio_atom = (MySQL_Atom*)
        worker->alloc_handle(AIO_TYPE::MYSQL_QUERY_SELECT);
    aio_atom->db_connect_ = db_connect;
    aio_atom->sql_ = sql;
    aio_atom->sql_len_ = sql_len;
    aio_atom->num_affect_ = num_affect;
    aio_atom->db_result_ = db_result;
    aio_atom->call_back_ = call_back;

    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!
int host_getaddr_ary(zce::aio::Worker* worker,
                     const char* hostname,
                     const char* service,
                     size_t* ary_addr_num,
                     sockaddr_in* ary_addr,
                     size_t* ary_addr6_num,
                     sockaddr_in6* ary_addr6,
                     std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::Host_Atom* aio_atom = (Host_Atom*)
        worker->alloc_handle(AIO_TYPE::HOST_GETADDRINFO_ARY);
    aio_atom->hostname_ = hostname;
    aio_atom->service_ = service;
    aio_atom->ary_addr_num_ = ary_addr_num;
    aio_atom->ary_addr_ = ary_addr;
    aio_atom->ary_addr6_num_ = ary_addr6_num;
    aio_atom->ary_addr6_ = ary_addr6;
    aio_atom->call_back_ = call_back;

    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

int host_getaddr_one(zce::aio::Worker* worker,
                     const char* hostname,
                     const char* service,
                     sockaddr* addr,
                     socklen_t addr_len,
                     std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::Host_Atom* aio_atom = (Host_Atom*)
        worker->alloc_handle(AIO_TYPE::HOST_GETADDRINFO_ONE);
    aio_atom->hostname_ = hostname;
    aio_atom->service_ = service;
    aio_atom->addr_ = addr;
    aio_atom->addr_len_ = addr_len;
    aio_atom->call_back_ = call_back;

    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!链接数据
int socket_connect(zce::aio::Worker* worker,
                   ZCE_SOCKET handle,
                   const sockaddr* addr,
                   socklen_t addrlen,
                   std::function<void(AIO_Atom*)> call_back)
{
    zce::aio::Socket_Atom* aio_atom = (Socket_Atom*)
        worker->alloc_handle(AIO_TYPE::SOCKET_CONNECT);

    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!
int socket_recv(zce::aio::Worker* worker,
                ZCE_SOCKET handle,
                void* buf,
                size_t len,
                std::function<void(AIO_Atom*)> call_back,
                int flags)
{
    zce::aio::Socket_Atom* aio_atom = (Socket_Atom*)
        worker->alloc_handle(AIO_TYPE::SOCKET_RECV);

    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!
int socket_send(zce::aio::Worker* worker,
                ZCE_SOCKET handle,
                const void* buf,
                size_t len,
                std::function<void(AIO_Atom*)> call_back,
                int flags)
{
    zce::aio::Socket_Atom* aio_atom = (Socket_Atom*)
        worker->alloc_handle(AIO_TYPE::SOCKET_SEND);

    auto succ_req = worker->request(aio_atom);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}
}