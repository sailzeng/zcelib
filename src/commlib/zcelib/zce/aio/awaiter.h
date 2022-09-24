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
//========================================================================================
//
//AIO FS文件处理相关的awaiter等待体
template <typename RA>
struct awaiter
{
    typedef awaiter<RA> self;

    awaiter(zce::aio::worker* worker,
            RA* request_atom) :
        worker_(worker),
        request_atom_(request_atom)
    {
    }
    ~awaiter() = default;

    //请求进行AIO操作，如果请求成功.return false挂起协程
    bool await_ready()
    {
        //绑定回调函数
        request_atom_->call_back_ = std::bind(&self::resume,
                                              this,
                                              std::placeholders::_1);
        //将一个文件操作句柄放入请求队列
        bool succ_req = worker_->request(request_atom_);
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
    void await_suspend(std::coroutine_handle<> awaiting)
    {
        awaiting_ = awaiting;
    }
    //!回复后的操作。恢复后返回结果
    RA await_resume()
    {
        return return_atom_;
    }
    //!回调函数，AIO操作完成后恢复时调用
    void resume(AIO_Atom* return_hdl)
    {
        RA* fs_hdl = (RA*)return_hdl;
        return_atom_ = *fs_hdl;
        awaiting_.resume();
        return;
    }

    //!工作者，具有请求，应答管道，处理IO多线程的管理者
    zce::aio::worker* worker_ = nullptr;
    //!请求的文件操作句柄
    RA* request_atom_ = nullptr;
    //!完成后返回的句柄
    RA return_atom_;
    //!协程的句柄（调用者）
    std::coroutine_handle<> awaiting_;
};

typedef zce::aio::awaiter<zce::aio::FS_Atom> awaiter_fs;
typedef zce::aio::awaiter<zce::aio::Dir_Atom> awaiter_dir;
typedef zce::aio::awaiter<zce::aio::MySQL_Atom> awaiter_mysql;
typedef zce::aio::awaiter<zce::aio::Host_Atom> awaiter_host;

//========================================================================================
//AIO 协程的co_await 函数

//!协程co_await AIO读取文件
awaiter_fs co_read_file(zce::aio::worker* worker,
                        const char* path,
                        char* read_bufs,
                        size_t nbufs,
                        ssize_t offset = 0);
//!协程co_await AIO写入文件
awaiter_fs co_write_file(zce::aio::worker* worker,
                         const char* path,
                         const char* write_bufs,
                         size_t nbufs,
                         ssize_t offset = 0);

//!协程co_await 链接数据
awaiter_mysql co_mysql_connect(zce::aio::worker* worker,
                               zce::mysql::connect* db_connect,
                               const char* host_name,
                               const char* user,
                               const char* pwd,
                               unsigned int port);

//!协程co_await 断开数据库链接
awaiter_mysql co_mysql_disconnect(zce::aio::worker* worker,
                                  zce::mysql::connect* db_connect);

//!协程co_await 查询，非SELECT语句
awaiter_mysql co_mysql_query(zce::aio::worker* worker,
                             zce::mysql::connect* db_connect,
                             const char* sql,
                             size_t sql_len,
                             uint64_t* num_affect,
                             uint64_t* insert_id);

//! 异步scandir,参数参考scandir，namelist可以用free_scandir_list要释放
awaiter_dir co_dir_scandir(zce::aio::worker* worker,
                           const char* dirname,
                           struct dirent*** namelist);

//!异步建立dir
awaiter_dir co_dir_mkdir(zce::aio::worker* worker,
                         const char* dirname,
                         int mode);

//!异步删除dir
awaiter_dir co_dir_rmdir(zce::aio::worker* worker,
                         const char* dirname);

//!协程co_await 查询，SELECT语句
awaiter_mysql co_mysql_query(zce::aio::worker* worker,
                             zce::mysql::connect* db_connect,
                             const char* sql,
                             size_t sql_len,
                             uint64_t* num_affect,
                             zce::mysql::result* db_result);

//!协程co_await getaddrinfo_ary
awaiter_host co_host_getaddr_ary(zce::aio::worker* worker,
                                 const char* hostname,
                                 const char* service,
                                 size_t* ary_addr_num,
                                 sockaddr_in* ary_addr,
                                 size_t* ary_addr6_num,
                                 sockaddr_in6* ary_addr6);

//!协程co_await ，类似getaddrinfo_one
awaiter_host co_host_getaddr_one(zce::aio::worker* worker,
                                 const char* hostname,
                                 const char* service,
                                 sockaddr* addr,
                                 socklen_t addr_len);
}