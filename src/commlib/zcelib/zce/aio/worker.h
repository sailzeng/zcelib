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

#include "zce/pool/shareptr_pool.h"
#include "zce/thread/msgque_condi.h"
#include "zce/event/reactor_mini.h"
#include "zce/timer/queue_wheel.h"
#include "zce/aio/caller.h"

namespace zce::aio
{
//!
class worker
{
public:

    worker() = default;
    ~worker();

    //!初始化
    int initialize(size_t work_thread_num,
                   size_t work_queue_len,
                   size_t max_event_num);

    //!销毁
    void terminate();

    //! 根据AIO_TYPE，请求一个AIO_Atom
    template<typename T>
    std::shared_ptr<T> alloc_handle()
    {
        return aio_obj_pool_.alloc_shared<T>();
    }

    //! 在请求队列放入一个请求
    template<typename T>
    bool request(std::shared_ptr<T> &&atom)
    {
        std::shared_ptr<void> shared_void = atom;
        return requst_queue_->try_enqueue(std::move(shared_void));
    }

    //! 注册事件
    int reg_event(ZCE_HANDLE handle,
                  RECTOR_EVENT event_todo,
                  event_callback_t call_back);

    //!
    int schedule_timer(timeout_callback_t call_fun,
                       int &time_id,
                       const zce::time_value& delay_time);

    //! 处理应答
    void process_response(zce::time_value* wait_time,
                          size_t& num_rsp,
                          size_t& num_event);

protected:
    //! 处理请求
    void process_request();

    //! 在线程中处理AIO操作，会根据type分解工作到下面这些函数
    void thread_aio(const std::shared_ptr<void> &base);
    //! 在线程中处理文件操作
    void thread_fs(zce::aio::FS_ATOM* base);
    //! 在线程中处理目录操作
    void thread_dir(zce::aio::DIR_ATOM* hdl);
    //! 在线程中处理MySQL操作请求
    void thread_mysql(zce::aio::MYSQL_ATOM* base);
    //! 在线程中处理Gat Host Addr请求
    void thread_host(zce::aio::HOST_ATOM* base);
    //! 在线程中处理Socket请求
    void thread_socket_timeout(zce::aio::SOCKET_TIMEOUT_ATOM* base);

protected:

    //! Caller的ID生成器
    uint32_t caller_id_builder_ = 1;

    //! 工作线程
    size_t work_thread_num_ = 0;
    //! 线程队列
    std::thread** work_thread_ = nullptr;
    //! 线程是否继续干活
    bool worker_running_ = true;

    zce::reactor_mini *reactor_ = nullptr;

    zce::timer_wheel *timer_queue_ = nullptr;

    //! 请求，应答队列，用于Caller 和Worker 线程交互
    zce::msgring_condi<std::shared_ptr<void> >* requst_queue_ = nullptr;
    zce::msgring_condi<std::shared_ptr<void> >* response_queue_ = nullptr;

    //! 对象池子，用于分配对象
    zce::multishare_pool<std::mutex,
        zce::aio::FS_ATOM,
        zce::aio::DIR_ATOM,
        zce::aio::MYSQL_ATOM,
        zce::aio::HOST_ATOM,
        zce::aio::SOCKET_TIMEOUT_ATOM,
        zce::aio::EVENT_ATOM,
        zce::aio::TIMER_ATOM > aio_obj_pool_;
};
}