#include "zce/predefine.h"
#include "zce/aio/awaiter.h"

namespace zce::aio
{
//============================================================================

//AIO 协程的co_await 函数
awaiter_fs co_read_file(zce::aio::Worker* worker,
                        const char* path,
                        char* read_bufs,
                        size_t nbufs,
                        ssize_t offset)
{
    zce::aio::FS_Atom* aio_atom = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_READFILE);
    aio_atom->path_ = path;
    aio_atom->read_bufs_ = read_bufs;
    aio_atom->bufs_count_ = nbufs;
    aio_atom->offset_ = offset;

    return awaiter_fs(worker, aio_atom);
}

awaiter_fs co_write_file(zce::aio::Worker* worker,
                         const char* path,
                         const char* write_bufs,
                         size_t nbufs,
                         ssize_t offset)
{
    zce::aio::FS_Atom* aio_atom = (FS_Atom*)
        worker->alloc_handle(AIO_TYPE::FS_WRITEFILE);
    aio_atom->path_ = path;
    aio_atom->write_bufs_ = write_bufs;
    aio_atom->bufs_count_ = nbufs;
    aio_atom->offset_ = offset;

    return awaiter_fs(worker, aio_atom);
}

//! 异步scandir,参数参考scandir，namelist可以用free_scandir_list要释放
awaiter_dir co_dir_scandir(zce::aio::Worker* worker,
                           const char* dirname,
                           struct dirent*** namelist)
{
    zce::aio::Dir_Atom* aio_atom = (Dir_Atom*)
        worker->alloc_handle(AIO_TYPE::DIR_SCANDIR);
    aio_atom->dirname_ = dirname;
    aio_atom->namelist_ = namelist;

    return awaiter_dir(worker, aio_atom);
}

//!异步建立dir
awaiter_dir co_dir_mkdir(zce::aio::Worker* worker,
                         const char* dirname,
                         int mode)
{
    zce::aio::Dir_Atom* aio_atom = (Dir_Atom*)
        worker->alloc_handle(AIO_TYPE::DIR_SCANDIR);
    aio_atom->dirname_ = dirname;
    aio_atom->mode_ = mode;
    return awaiter_dir(worker, aio_atom);
}

//!异步删除dir
awaiter_dir co_dir_rmdir(zce::aio::Worker* worker,
                         const char* dirname)
{
    zce::aio::Dir_Atom* aio_atom = (Dir_Atom*)
        worker->alloc_handle(AIO_TYPE::DIR_SCANDIR);
    aio_atom->dirname_ = dirname;
    return awaiter_dir(worker, aio_atom);
}

//!链接数据
awaiter_mysql co_mysql_connect(zce::aio::Worker* worker,
                               zce::mysql::connect* db_connect,
                               const char* host_name,
                               const char* user,
                               const char* pwd,
                               unsigned int port)
{
    zce::aio::MySQL_Atom* aio_atom = (MySQL_Atom*)
        worker->alloc_handle(AIO_TYPE::MYSQL_CONNECT);
    aio_atom->db_connect_ = db_connect;
    aio_atom->host_name_ = host_name;
    aio_atom->user_ = user;
    aio_atom->pwd_ = pwd;
    aio_atom->port_ = port;

    return awaiter_mysql(worker, aio_atom);
}

//!断开数据库链接
awaiter_mysql co_mysql_disconnect(zce::aio::Worker* worker,
                                  zce::mysql::connect* db_connect)
{
    zce::aio::MySQL_Atom* aio_atom = (MySQL_Atom*)
        worker->alloc_handle(AIO_TYPE::MYSQL_DISCONNECT);
    aio_atom->db_connect_ = db_connect;
    return awaiter_mysql(worker, aio_atom);
}

//!查询，非SELECT语句
awaiter_mysql co_mysql_query(zce::aio::Worker* worker,
                             zce::mysql::connect* db_connect,
                             const char* sql,
                             size_t sql_len,
                             uint64_t* num_affect,
                             uint64_t* insert_id)
{
    zce::aio::MySQL_Atom* aio_atom = (MySQL_Atom*)
        worker->alloc_handle(AIO_TYPE::MYSQL_QUERY_NOSELECT);
    aio_atom->db_connect_ = db_connect;
    aio_atom->sql_ = sql;
    aio_atom->sql_len_ = sql_len;
    aio_atom->num_affect_ = num_affect;
    aio_atom->insert_id_ = insert_id;

    return awaiter_mysql(worker, aio_atom);
}

//!查询，SELECT语句
awaiter_mysql co_mysql_query(zce::aio::Worker* worker,
                             zce::mysql::connect* db_connect,
                             const char* sql,
                             size_t sql_len,
                             uint64_t* num_affect,
                             zce::mysql::result* db_result)
{
    zce::aio::MySQL_Atom* aio_atom = (MySQL_Atom*)
        worker->alloc_handle(AIO_TYPE::MYSQL_QUERY_SELECT);
    aio_atom->db_connect_ = db_connect;
    aio_atom->sql_ = sql;
    aio_atom->sql_len_ = sql_len;
    aio_atom->num_affect_ = num_affect;
    aio_atom->db_result_ = db_result;

    return awaiter_mysql(worker, aio_atom);
}

awaiter_host co_host_getaddr_ary(zce::aio::Worker* worker,
                                 const char* hostname,
                                 const char* service,
                                 size_t* ary_addr_num,
                                 sockaddr_in* ary_addr,
                                 size_t* ary_addr6_num,
                                 sockaddr_in6* ary_addr6)
{
    zce::aio::Host_Atom* aio_atom = (Host_Atom*)
        worker->alloc_handle(AIO_TYPE::HOST_GETADDRINFO_ARY);
    aio_atom->hostname_ = hostname;
    aio_atom->service_ = service;
    aio_atom->ary_addr_num_ = ary_addr_num;
    aio_atom->ary_addr_ = ary_addr;
    aio_atom->ary_addr6_num_ = ary_addr6_num;
    aio_atom->ary_addr6_ = ary_addr6;
    return awaiter_host(worker, aio_atom);
}

//!获得host对应的一个地址信息，类似getaddrinfo_one
awaiter_host co_host_getaddr_one(zce::aio::Worker* worker,
                                 const char* hostname,
                                 const char* service,
                                 sockaddr* addr,
                                 socklen_t addr_len)
{
    zce::aio::Host_Atom* aio_atom = (Host_Atom*)
        worker->alloc_handle(AIO_TYPE::HOST_GETADDRINFO_ONE);
    aio_atom->hostname_ = hostname;
    aio_atom->service_ = service;
    aio_atom->addr_ = addr;
    aio_atom->addr_len_ = addr_len;
    return awaiter_host(worker, aio_atom);
}
}