#include "zce/predefine.h"
#include "zce/aio/worker.h"
#include "zce/aio/caller.h"

namespace zce::aio
{

void AIO_Handle::clear()
{
    aio_type_ = AIO_INVALID;
    id_ = 0;
}

void FS_Handle::clear()
{
    AIO_Handle::clear();
    result_ = 0;
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
    dirname_ = nullptr;
    namelist_ = nullptr;
}

void MySQL_Handle::clear()
{
    AIO_Handle::clear();
    result_ = 0;
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

//====================================================

//!异步打开某个文件，完成后回调函数call_back
int fs_open(zce::aio::Worker* worker,
            const char* path,
            int flags,
            int mode,
            std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* hdl = (FS_Handle*)
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
             std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* hdl = (FS_Handle*)
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
             std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* hdl = (FS_Handle*)
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
            std::function<void(AIO_Handle*)> call_back,
            ssize_t offset,
            int whence)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_READ);
    aio_hdl->handle_ = handle;
    aio_hdl->read_bufs_ = read_bufs;
    aio_hdl->bufs_count_ = nbufs;
    aio_hdl->offset_ = offset;
    aio_hdl->whence_ = whence;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
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
             std::function<void(AIO_Handle*)> call_back,
             ssize_t offset,
             int whence)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_WRITE);
    aio_hdl->handle_ = handle;
    aio_hdl->write_bufs_ = write_bufs;
    aio_hdl->bufs_count_ = nbufs;
    aio_hdl->offset_ = offset;
    aio_hdl->whence_ = whence;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
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
                 std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_FTRUNCATE);
    aio_hdl->handle_ = handle;
    aio_hdl->offset_ = offset;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
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
                 std::function<void(AIO_Handle*)> call_back,
                 ssize_t offset)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_READFILE);
    aio_hdl->path_ = path;
    aio_hdl->read_bufs_ = read_bufs;
    aio_hdl->bufs_count_ = nbufs;
    aio_hdl->offset_ = offset;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
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
                  std::function<void(AIO_Handle*)> call_back,
                  ssize_t offset)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_WRITEFILE);
    aio_hdl->path_ = path;
    aio_hdl->write_bufs_ = write_bufs;
    aio_hdl->bufs_count_ = nbufs;
    aio_hdl->offset_ = offset;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步删除文件
int fs_unlink(zce::aio::Worker* worker,
              const char* path,
              std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_UNLINK);
    aio_hdl->path_ = path;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
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
              std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_RENAME);
    aio_hdl->path_ = path;
    aio_hdl->new_path_ = new_path;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
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
            std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_STAT);
    aio_hdl->path_ = path;
    aio_hdl->file_stat_ = file_stat;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步scandir
int fs_scandir(zce::aio::Worker* worker,
               const char* dirname,
               struct dirent*** namelist,
               std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_SCANDIR);
    aio_hdl->dirname_ = dirname;
    aio_hdl->namelist_ = namelist;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步建立dir
int fs_mkdir(zce::aio::Worker* worker,
             const char* dirname,
             int mode,
             std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_MKDIR);
    aio_hdl->dirname_ = dirname;
    aio_hdl->mode_ = mode;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步删除dir
int fs_rmdir(zce::aio::Worker* worker,
             const char* dirname,
             std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_RMDIR);
    aio_hdl->dirname_ = dirname;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
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
                  std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::MySQL_Handle* aio_hdl = (MySQL_Handle*)
        worker->alloc_handle(AIO_TYPE::MYSQL_CONNECT);
    aio_hdl->db_connect_ = db_connect;
    aio_hdl->host_name_ = host_name;
    aio_hdl->user_ = user;
    aio_hdl->pwd_ = pwd;
    aio_hdl->port_ = port;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!断开数据库链接
int mysql_disconnect(zce::aio::Worker* worker,
                     zce::mysql::Connect* db_connect,
                     std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::MySQL_Handle* aio_hdl = (MySQL_Handle*)
        worker->alloc_handle(AIO_TYPE::MYSQL_DISCONNECT);
    aio_hdl->db_connect_ = db_connect;
    aio_hdl->call_back_ = call_back;
    auto succ_req = worker->request(aio_hdl);
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
                std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::MySQL_Handle* aio_hdl = (MySQL_Handle*)
        worker->alloc_handle(AIO_TYPE::MYSQL_QUERY_NOSELECT);
    aio_hdl->db_connect_ = db_connect;
    aio_hdl->sql_ = sql;
    aio_hdl->sql_len_ = sql_len;
    aio_hdl->num_affect_ = num_affect;
    aio_hdl->insert_id_ = insert_id;
    aio_hdl->call_back_ = call_back;

    auto succ_req = worker->request(aio_hdl);
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
                std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::MySQL_Handle* aio_hdl = (MySQL_Handle*)
        worker->alloc_handle(AIO_TYPE::MYSQL_QUERY_SELECT);
    aio_hdl->db_connect_ = db_connect;
    aio_hdl->sql_ = sql;
    aio_hdl->sql_len_ = sql_len;
    aio_hdl->num_affect_ = num_affect;
    aio_hdl->db_result_ = db_result;
    aio_hdl->call_back_ = call_back;

    auto succ_req = worker->request(aio_hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//====================================================
//await_aiofs是co_wait 文件操作的等待体
await_aiofs::await_aiofs(zce::aio::Worker* worker,
                         zce::aio::FS_Handle* fs_hdl) :
    worker_(worker),
    fs_hdl_(fs_hdl)
{
}
//请求进行AIO操作，如果请求成功.return false挂起协程
bool await_aiofs::await_ready()
{
    //绑定回调函数
    fs_hdl_->call_back_ = std::bind(&await_aiofs::resume,
                                    this,
                                    std::placeholders::_1);
    //将一个文件操作句柄放入请求队列
    bool succ_req = worker_->request(fs_hdl_);
    if (succ_req)
    {
        return false;
    }
    else
    {
        return true;
    }
}
//挂起操作
void await_aiofs::await_suspend(std::coroutine_handle<> awaiting)
{
    awaiting_ = awaiting;
}
//!回复后的操作。返回结果
FS_Handle await_aiofs::await_resume()
{
    return return_hdl_;
}
//!回调函数
void await_aiofs::resume(AIO_Handle* return_hdl)
{
    FS_Handle* fs_hdl = (FS_Handle*)return_hdl;
    return_hdl_ = *fs_hdl;
    awaiting_.resume();
    return;
}

//await_aiomysql是co_wait MySQL操作的等待体
await_aiomysql::await_aiomysql(zce::aio::Worker* worker,
                               zce::aio::MySQL_Handle* mysql_hdl) :
    worker_(worker),
    mysql_hdl_(mysql_hdl)
{
}
//请求进行AIO操作，如果请求成功.return false挂起协程
bool await_aiomysql::await_ready()
{
    //绑定回调函数
    mysql_hdl_->call_back_ = std::bind(&await_aiomysql::resume,
                                       this,
                                       std::placeholders::_1);
    //将一个文件操作句柄放入请求队列
    bool succ_req = worker_->request(mysql_hdl_);
    if (succ_req)
    {
        return false;
    }
    else
    {
        return true;
    }
}
//挂起操作
void await_aiomysql::await_suspend(std::coroutine_handle<> awaiting)
{
    awaiting_ = awaiting;
}
//!回复后的操作。返回结果
MySQL_Handle await_aiomysql::await_resume()
{
    return return_hdl_;
}
//!回调函数
void await_aiomysql::resume(AIO_Handle* return_hdl)
{
    MySQL_Handle* mysql_hdl = (MySQL_Handle*)return_hdl;
    return_hdl_ = *mysql_hdl;
    awaiting_.resume();
    return;
}


//============================================================================

//AIO 协程的co_await 函数
await_aiofs co_read_file(zce::aio::Worker* worker,
                         const char* path,
                         char* read_bufs,
                         size_t nbufs,
                         ssize_t offset)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_READFILE);
    aio_hdl->path_ = path;
    aio_hdl->read_bufs_ = read_bufs;
    aio_hdl->bufs_count_ = nbufs;
    aio_hdl->offset_ = offset;

    return await_aiofs(worker, aio_hdl);
}

await_aiofs co_write_file(zce::aio::Worker* worker,
                          const char* path,
                          const char* write_bufs,
                          size_t nbufs,
                          ssize_t offset)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_WRITEFILE);
    aio_hdl->path_ = path;
    aio_hdl->write_bufs_ = write_bufs;
    aio_hdl->bufs_count_ = nbufs;
    aio_hdl->offset_ = offset;

    return await_aiofs(worker, aio_hdl);
}

//!链接数据
await_aiomysql co_mysql_connect(zce::aio::Worker* worker,
                                zce::mysql::Connect* db_connect,
                                const char* host_name,
                                const char* user,
                                const char* pwd,
                                unsigned int port)
{
    zce::aio::MySQL_Handle* aio_hdl = (MySQL_Handle*)
        worker->alloc_handle(AIO_TYPE::MYSQL_CONNECT);
    aio_hdl->db_connect_ = db_connect;
    aio_hdl->host_name_ = host_name;
    aio_hdl->user_ = user;
    aio_hdl->pwd_ = pwd;
    aio_hdl->port_ = port;

    return await_aiomysql(worker, aio_hdl);
}

//!断开数据库链接
await_aiomysql co_mysql_disconnect(zce::aio::Worker* worker,
                                   zce::mysql::Connect* db_connect)
{
    zce::aio::MySQL_Handle* aio_hdl = (MySQL_Handle*)
        worker->alloc_handle(AIO_TYPE::MYSQL_DISCONNECT);
    aio_hdl->db_connect_ = db_connect;
    return await_aiomysql(worker, aio_hdl);
}


//!查询，非SELECT语句
await_aiomysql co_mysql_query(zce::aio::Worker* worker,
                              zce::mysql::Connect* db_connect,
                              const char* sql,
                              size_t sql_len,
                              uint64_t* num_affect,
                              uint64_t* insert_id)
{
    zce::aio::MySQL_Handle* aio_hdl = (MySQL_Handle*)
        worker->alloc_handle(AIO_TYPE::MYSQL_QUERY_NOSELECT);
    aio_hdl->db_connect_ = db_connect;
    aio_hdl->sql_ = sql;
    aio_hdl->sql_len_ = sql_len;
    aio_hdl->num_affect_ = num_affect;
    aio_hdl->insert_id_ = insert_id;

    return await_aiomysql(worker, aio_hdl);
}

//!查询，SELECT语句
await_aiomysql co_mysql_query(zce::aio::Worker* worker,
                              zce::mysql::Connect* db_connect,
                              const char* sql,
                              size_t sql_len,
                              uint64_t* num_affect,
                              zce::mysql::Result* db_result)
{
    zce::aio::MySQL_Handle* aio_hdl = (MySQL_Handle*)
        worker->alloc_handle(AIO_TYPE::MYSQL_QUERY_SELECT);
    aio_hdl->db_connect_ = db_connect;
    aio_hdl->sql_ = sql;
    aio_hdl->sql_len_ = sql_len;
    aio_hdl->num_affect_ = num_affect;
    aio_hdl->db_result_ = db_result;

    return await_aiomysql(worker, aio_hdl);
}

}

