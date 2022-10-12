#include "zce/predefine.h"
#include "zce/aio/awaiter.h"

namespace zce::aio
{
//============================================================================

//AIO 协程的co_await 函数
awaiter_aio co_read_file(zce::aio::worker* worker,
                         const char* path,
                         char* read_bufs,
                         size_t nbufs,
                         ssize_t offset)
{
    zce::aio::FS_ATOM* aio_atom = (FS_ATOM*)
        worker->alloc_handle(AIO_TYPE::FS_READFILE);
    aio_atom->path_ = path;
    aio_atom->read_bufs_ = read_bufs;
    aio_atom->bufs_count_ = nbufs;
    aio_atom->offset_ = offset;

    return awaiter_aio(worker, aio_atom);
}

awaiter_aio co_write_file(zce::aio::worker* worker,
                          const char* path,
                          const char* write_bufs,
                          size_t nbufs,
                          ssize_t offset)
{
    zce::aio::FS_ATOM* aio_atom = (FS_ATOM*)
        worker->alloc_handle(AIO_TYPE::FS_WRITEFILE);
    aio_atom->path_ = path;
    aio_atom->write_bufs_ = write_bufs;
    aio_atom->bufs_count_ = nbufs;
    aio_atom->offset_ = offset;

    return awaiter_aio(worker, aio_atom);
}

//============================================================================
//! 异步scandir,参数参考scandir，namelist可以用free_scandir_list要释放
awaiter_aio co_dir_scandir(zce::aio::worker* worker,
                           const char* dirname,
                           struct dirent*** namelist)
{
    zce::aio::DIR_ATOM* aio_atom = (DIR_ATOM*)
        worker->alloc_handle(AIO_TYPE::DIR_SCANDIR);
    aio_atom->dirname_ = dirname;
    aio_atom->namelist_ = namelist;

    return awaiter_aio(worker, aio_atom);
}

//!异步建立dir
awaiter_aio co_dir_mkdir(zce::aio::worker* worker,
                         const char* dirname,
                         int mode)
{
    zce::aio::DIR_ATOM* aio_atom = (DIR_ATOM*)
        worker->alloc_handle(AIO_TYPE::DIR_SCANDIR);
    aio_atom->dirname_ = dirname;
    aio_atom->mode_ = mode;
    return awaiter_aio(worker, aio_atom);
}

//!异步删除dir
awaiter_aio co_dir_rmdir(zce::aio::worker* worker,
                         const char* dirname)
{
    zce::aio::DIR_ATOM* aio_atom = (DIR_ATOM*)
        worker->alloc_handle(AIO_TYPE::DIR_SCANDIR);
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
    zce::aio::MYSQL_ATOM* aio_atom = (MYSQL_ATOM*)
        worker->alloc_handle(AIO_TYPE::MYSQL_CONNECT);
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
    zce::aio::MYSQL_ATOM* aio_atom = (MYSQL_ATOM*)
        worker->alloc_handle(AIO_TYPE::MYSQL_DISCONNECT);
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
    zce::aio::MYSQL_ATOM* aio_atom = (MYSQL_ATOM*)
        worker->alloc_handle(AIO_TYPE::MYSQL_QUERY_NOSELECT);
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
    zce::aio::MYSQL_ATOM* aio_atom = (MYSQL_ATOM*)
        worker->alloc_handle(AIO_TYPE::MYSQL_QUERY_SELECT);
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
    zce::aio::HOST_ATOM* aio_atom = (HOST_ATOM*)
        worker->alloc_handle(AIO_TYPE::HOST_GETADDRINFO_ARY);
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
    zce::aio::HOST_ATOM* aio_atom = (HOST_ATOM*)
        worker->alloc_handle(AIO_TYPE::HOST_GETADDRINFO_ONE);
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
    zce::aio::SOCKET_ATOM* aio_atom = (SOCKET_ATOM*)
        worker->alloc_handle(AIO_TYPE::SOCKET_CONNECT_ADDR);
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
    zce::aio::SOCKET_ATOM* aio_atom = (SOCKET_ATOM*)
        worker->alloc_handle(AIO_TYPE::SOCKET_CONNECT_HOST);
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
                         sockaddr* from,
                         socklen_t* from_len,
                         zce::time_value* timeout_tv)
{
    zce::aio::SOCKET_ATOM* aio_atom = (SOCKET_ATOM*)
        worker->alloc_handle(AIO_TYPE::SOCKET_ACCEPT);
    aio_atom->handle_ = handle;
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
    zce::aio::SOCKET_ATOM* aio_atom = (SOCKET_ATOM*)
        worker->alloc_handle(AIO_TYPE::SOCKET_RECV);
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
    zce::aio::SOCKET_ATOM* aio_atom = (SOCKET_ATOM*)
        worker->alloc_handle(AIO_TYPE::SOCKET_SEND);
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
    zce::aio::SOCKET_ATOM* aio_atom = (SOCKET_ATOM*)
        worker->alloc_handle(AIO_TYPE::SOCKET_RECVFROM);
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

int co_er_connect(zce::aio::worker* worker,
                  ZCE_SOCKET handle,
                  const sockaddr* addr,
                  socklen_t addr_len)
{
    return 0;
}

int co_er_accept(zce::aio::worker* worker,
                 ZCE_SOCKET handle,
                 ZCE_SOCKET *accept_hdl,
                 sockaddr* from,
                 socklen_t* from_len)
{
    return 0;
}
}