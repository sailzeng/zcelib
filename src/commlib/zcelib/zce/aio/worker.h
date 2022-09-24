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
*            人生是一场旅程。我们经历了几次轮回，才换来这个旅程。
*            而这个旅程很短，因此不妨大胆一些，
*            不妨大胆一些去爱一个人，去攀一座山，去追一个梦……
*            有很多事我都不明白。但我相信一件事。
*            上天让我们来到这个世上，就是为了让我们创造奇迹。
*
*
*/
#pragma once

#include "zce/pool/multiobjs_pool.h"
#include "zce/thread/msgque_condi.h"
#include "zce/aio/caller.h"

namespace zce::aio
{
//!
class worker
{
public:

    worker() = default;
    ~worker() = default;

    //!初始化
    int initialize(size_t work_thread_num,
                   size_t work_queue_len);

    //!销毁
    void terminate();

    //! 根据AIO_TYPE，请求一个AIO_Atom
    AIO_Atom* alloc_handle(AIO_TYPE aio_type);
    //! 释放一个AIO_Atom，根据AIO_TYPE归还到不同的池子里面。
    void free_handle(zce::aio::AIO_Atom* base);

    //!在请求队列放入一个请求
    bool request(zce::aio::AIO_Atom* base);

    //! 处理请求
    void process_request();

    //! 处理应答
    void process_response(size_t& num_rsp,
                          zce::time_value* wait_time);

    //!在线程中处理AIO操作，会根据type分解工作到下面这些函数
    void process_aio(zce::aio::AIO_Atom* base);

    //!在线程中处理文件操作
    void process_fs(zce::aio::FS_Atom* base);
    //!在线程中处理目录操作
    void process_dir(zce::aio::Dir_Atom* hdl);
    //!在线程中处理MySQL操作请求
    void process_mysql(zce::aio::MySQL_Atom* base);
    //!在线程中处理Gat Host Addr请求
    void process_host(zce::aio::Host_Atom* base);
    //!在线程中处理Socket请求
    void process_socket(zce::aio::Socket_Atom* base);

protected:

    //! Caller的ID生成器
    uint32_t caller_id_builder_ = 1;

    //! 工作线程
    size_t work_thread_num_ = 0;
    //! 线程队列
    std::thread** work_thread_ = nullptr;
    //! 线程是否继续干活
    bool worker_running_ = true;

    //! 请求，应答队列，用于Caller 和Worker 线程交互
    zce::msgring_condi<zce::aio::AIO_Atom*>* requst_queue_ = nullptr;
    zce::msgring_condi<zce::aio::AIO_Atom*>* response_queue_ = nullptr;

    //! 对象池子，用于分配对象
    zce::multiobjs_pool<std::mutex,
        zce::aio::FS_Atom,
        zce::aio::Dir_Atom,
        zce::aio::MySQL_Atom,
        zce::aio::Host_Atom,
        zce::aio::Socket_Atom> aio_obj_pool_;
};
}