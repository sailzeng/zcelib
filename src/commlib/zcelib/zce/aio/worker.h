

#pragma once

#include "zce/pool/multiobjs_pool.h"
#include "zce/thread/msgque_condi.h"
#include "zce/aio/caller.h"

namespace zce::aio
{

class Worker
{
public:

    Worker() = default;
    ~Worker() = default;

    //!初始化
    int initialize(size_t work_thread_num,
                   size_t work_queue_len);

    //!销毁
    void terminate();

    //! 根据AIO_TYPE，请求一个AIO_Handle
    AIO_Handle* alloc_handle(AIO_TYPE aio_type);
    //! 释放一个AIO_Handle，根据AIO_TYPE归还到不同的池子里面。
    void free_handle(zce::aio::AIO_Handle* base);

    //!在请求队列放入一个请求
    bool request(zce::aio::AIO_Handle* base);

    //! 处理请求
    void process_request();

    //! 处理应答
    void process_response(size_t& num_rsp,
                          zce::Time_Value* wait_time);

    //!处理AIO操作，会根据type分解工作到下面这些函数
    void process_aio(zce::aio::AIO_Handle* base);

    //!处理文件操作
    void process_fs(zce::aio::FS_Handle* base);
    //!处理MySQL
    void process_mysql(zce::aio::MySQL_Handle* base);

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
    zce::msgring_condi<zce::aio::AIO_Handle*>* requst_queue_ = nullptr;
    zce::msgring_condi<zce::aio::AIO_Handle*>* response_queue_ = nullptr;
    //! 对象池子，用于分配对象
    zce::multiobjs_pool<std::mutex,
        zce::aio::FS_Handle,
        zce::aio::MySQL_Handle> aio_obj_pool_;
};
}