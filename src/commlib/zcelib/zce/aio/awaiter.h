/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   zce/aio/awaiter.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2022年9月9日
* @brief
* @details
*
*
*
* @note
*            英女王去世，我对腐国并无什么好感。
*            但女王这人，在二战时，拒绝前往加拿大躲避战火，还曾经参军当汽车兵，
*            她自己一生也一直在为她的联邦努力。
*            这代人多少在践行他们的追求。
*            而今天，普金这种流氓，打着俄罗斯民族主义的旗号入侵乌克兰。
*            而自己却让自己的小三，儿子，躲在北欧。什么玩意。
*            一个时代过去了。比烂的时代开场。
*
*
*/

#pragma once

#include "zce/aio/caller.h"
#include "zce/aio/worker.h"

namespace zce::aio
{
struct AIO_ATOM;
//========================================================================================
//
//AIO FS文件处理相关的awaiter等待体
struct awaiter_aio
{
    awaiter_aio(zce::aio::worker* worker,
                AIO_ATOM* request_atom) :
        worker_(worker),
        request_atom_(request_atom)
    {
    }
    ~awaiter_aio()
    {
        if (request_atom_)
        {
            worker_->free_handle(request_atom_);
            request_atom_ = nullptr;
        }
    }

    //请求进行AIO操作，如果请求成功.return false挂起协程
    bool await_ready();

    //挂起操作
    void await_suspend(std::coroutine_handle<> awaiting)
    {
        awaiting_ = awaiting;
    }
    //!回复后的操作。恢复后返回结果
    int await_resume()
    {
        return ret_result_;
    }
    //!回调函数，AIO操作完成后恢复时调用
    void resume(AIO_ATOM* return_hdl)
    {
        ret_result_ = return_hdl->result_;
        awaiting_.resume();

        return;
    }

protected:
    //请求进行多线程的AIO操作
    bool aio_thread_await_ready();
    //请求进行事件的AIO操作
    bool event_await_ready();

protected:

    //!工作者，具有请求，应答管道，处理IO多线程的管理者
    zce::aio::worker* worker_ = nullptr;
    //!请求的文件操作句柄
    AIO_ATOM* request_atom_ = nullptr;

    //!完成后返回的句柄
    int ret_result_ = -1;
    //!协程的句柄（调用者）
    std::coroutine_handle<> awaiting_;
};

//========================================================================================
//AIO 协程的co_await 函数

//!协程co_await AIO读取文件
awaiter_aio co_read_file(zce::aio::worker* worker,
                         const char* path,
                         char* read_bufs,
                         size_t nbufs,
                         size_t *result_len,
                         ssize_t offset = 0);
//!协程co_await AIO写入文件
awaiter_aio co_write_file(zce::aio::worker* worker,
                          const char* path,
                          const char* write_bufs,
                          size_t nbufs,
                          size_t *result_len,
                          ssize_t offset = 0);

//========================================================================================
//!协程co_await 链接数据
awaiter_aio co_mysql_connect(zce::aio::worker* worker,
                             zce::mysql::connect* db_connect,
                             const char* host_name,
                             const char* user,
                             const char* pwd,
                             unsigned int port);

//!协程co_await 断开数据库链接
awaiter_aio co_mysql_disconnect(zce::aio::worker* worker,
                                zce::mysql::connect* db_connect);

//!协程co_await 查询，非SELECT语句
awaiter_aio co_mysql_query(zce::aio::worker* worker,
                           zce::mysql::connect* db_connect,
                           const char* sql,
                           size_t sql_len,
                           uint64_t* num_affect,
                           uint64_t* insert_id);

//!协程co_await 查询，SELECT语句
awaiter_aio co_mysql_query(zce::aio::worker* worker,
                           zce::mysql::connect* db_connect,
                           const char* sql,
                           size_t sql_len,
                           uint64_t* num_affect,
                           zce::mysql::result* db_result);

//========================================================================================
//! 异步scandir,参数参考scandir，namelist可以用free_scandir_list要释放
awaiter_aio co_dir_scandir(zce::aio::worker* worker,
                           const char* dirname,
                           struct dirent*** namelist);

//!异步建立dir
awaiter_aio co_dir_mkdir(zce::aio::worker* worker,
                         const char* dirname,
                         int mode);

//!异步删除dir
awaiter_aio co_dir_rmdir(zce::aio::worker* worker,
                         const char* dirname);

//========================================================================================
//!协程co_await getaddrinfo_ary
awaiter_aio co_host_getaddr_ary(zce::aio::worker* worker,
                                const char* hostname,
                                const char* service,
                                size_t* ary_addr_num,
                                sockaddr_in* ary_addr,
                                size_t* ary_addr6_num,
                                sockaddr_in6* ary_addr6);

//!协程co_await ，类似getaddrinfo_one
awaiter_aio co_host_getaddr_one(zce::aio::worker* worker,
                                const char* hostname,
                                const char* service,
                                sockaddr* addr,
                                socklen_t addr_len);

//========================================================================================

//! 等待若干时间进行connect，使用地址参数,直至超时
awaiter_aio co_st_connect(zce::aio::worker* worker,
                          ZCE_SOCKET handle,
                          const sockaddr* addr,
                          socklen_t addr_len,
                          zce::time_value* timeout_tv);

//! 等待若干时间进行connect，使用hostname参数,直至超时
awaiter_aio co_st_connect(zce::aio::worker* worker,
                          ZCE_SOCKET handle,
                          const char* host_name,
                          uint16_t host_port,
                          sockaddr* host_addr,
                          socklen_t addr_len,
                          zce::time_value& timeout_tv);

//! 等待若干时间进行accept，直至超时
awaiter_aio co_st_accept(zce::aio::worker* worker,
                         ZCE_SOCKET handle,
                         ZCE_SOCKET *accept_hdl,
                         sockaddr* addr,
                         socklen_t* addr_len,
                         zce::time_value* timeout_tv);

//! 等待若干时间进行recv，直至超时
awaiter_aio co_st_recv(zce::aio::worker* worker,
                       ZCE_SOCKET handle,
                       void* buf,
                       size_t len,
                       zce::time_value* timeout_tv,
                       int flags = 0);

//!等待若干时间进行send，直至超时
awaiter_aio co_st_send(zce::aio::worker* worker,
                       ZCE_SOCKET handle,
                       const void* buf,
                       size_t len,
                       zce::time_value* timeout_tv,
                       int flags = 0);

//!等待若干时间进行recv数据，直至超时
awaiter_aio co_st_recvfrom(zce::aio::worker* worker,
                           ZCE_SOCKET handle,
                           void* buf,
                           size_t len,
                           sockaddr* from,
                           socklen_t* from_len,
                           zce::time_value* timeout_tv,
                           int flags = 0);

//========================================================================================
//!异步进行CONNECT
awaiter_aio co_er_connect(zce::aio::worker* worker,
                          ZCE_SOCKET handle,
                          const sockaddr* addr,
                          socklen_t addr_len);

//!异步进行accept，直至超时
awaiter_aio co_er_accept(zce::aio::worker* worker,
                         ZCE_SOCKET handle,
                         ZCE_SOCKET *accept_hdl,
                         sockaddr* from,
                         socklen_t* from_len);

//!异步进行recv，
awaiter_aio co_er_recv(zce::aio::worker* worker,
                       ZCE_SOCKET handle,
                       void* rcv_buf,
                       size_t len,
                       size_t *result_len);

//!异步进行send，
awaiter_aio co_er_send(zce::aio::worker* worker,
                       ZCE_SOCKET handle,
                       const void* snd_buf,
                       size_t len,
                       size_t *result_len);

//!异步进行recv数据，
awaiter_aio co_er_recvfrom(zce::aio::worker* worker,
                           ZCE_SOCKET handle,
                           void* rcv_buf,
                           size_t len,
                           size_t *result_len,
                           sockaddr* from,
                           socklen_t* from_len);

//========================================================================================

//!
awaiter_aio  co_schedule_timeout(zce::aio::worker* worker,
                                 const zce::time_value* timeout_tv,
                                 int *timer_id);

//!
awaiter_aio  co_cancel_timeout(int timer_id);
}