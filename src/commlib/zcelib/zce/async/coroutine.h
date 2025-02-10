#pragma once

#include "zce/async/actor.h"

namespace zce
{
//====================================================================================

struct aco_ret
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        promise_type() = default;
        ~promise_type() = default;

        auto get_return_object()
        {
            return aco_ret{ handle_type::from_promise(*this) };
        }
        //注意这个函数,如果返回std::suspend_never{}，就不挂起，
        //返回std::suspend_always{} 挂起
        auto initial_suspend()
        {
            return std::suspend_never{};
            //return std::suspend_always{};
        }
        //co_return 后这个函数会被调用
        void return_value(int v)
        {
            return_data_ = v;
            return;
        }
        auto yield_value(int v)
        {
            return_data_ = v;
            return std::suspend_always{};
        }
        auto final_suspend() noexcept
        {
            return std::suspend_never{};
        }
        void unhandled_exception()
        {
            std::exit(1);
        }
        //返回值
        int return_data_;
    };
    aco_ret()
    {
    }
    aco_ret(handle_type h)
        : coro_handle_(h)
    {
    }
    aco_ret(const aco_ret&) = delete;
    aco_ret(aco_ret&& s) noexcept
        : coro_handle_(s.coro_handle_)
    {
        s.coro_handle_ = nullptr;
    }
    ~aco_ret()
    {
        if (coro_handle_)
            coro_handle_.destroy();
    }
    aco_ret& operator=(const aco_ret&) = delete;
    aco_ret& operator=(aco_ret&& s) noexcept
    {
        coro_handle_ = s.coro_handle_;
        s.coro_handle_ = nullptr;
        return *this;
    }

    bool done()
    {
        return coro_handle_.done();
    }

    bool move_next()
    {
        coro_handle_.resume();
        return coro_handle_.done();
    }

    int get()
    {
        return coro_handle_.promise().return_data_;
    }

    handle_type coro_handle_;
};
/*!
* @brief      协程对象
*
*/
class coro : public zce::async::actor
{
    friend class coro_mgr;

public:
    /*!
    * @brief      构造函数，
    * @param      async_mgr ,协程异步管理器的指针
    */
    coro(zce::async::manager* async_mgr,
         uint32_t reg_cmd);
protected:
    /*!
    * @brief      析构函数
    */
    ~coro();

public:

    /*!
    * @brief      初始化函数，在构造函数后调用，在放入池子前执行一次，
    * @return     int 0标识成功
    */
    virtual int initialize();

    /*!
    * @brief      结束销毁函数，在析构前的调用
    */
    virtual void terminate();

protected:

    //!协程运行,你要重载的函数
    virtual aco_ret coro_run() = 0;

    /*!
    * @brief      等待time_out 时间后超时，设置定时器后，切换协程到main
    * @return     int
    * @param      time_out
    */
    int waitfor_timeout(const zce::time_value& time_out);

    /*!
    * @brief      继承zce::Async_Object的函数，
    *             协程对象的运行处理
    */
    virtual void on_run(bool first_run,
                        bool& continue_run) override;

    /*!
    * @brief      异步对象超时处理
    * @param[in]  now_time  发生超时的时间，
    * @param[out] continued 异步对象是否继续运行,
    */
    virtual void on_timeout(const zce::time_value& now_time,
                            bool& continue_run) override;

protected:
    //!
    aco_ret  coroutine_ret_;
    //!
    bool    occur_timeout_ = false;
};

//====================================================================================

/*!
* @brief      协程对象主控管理类
*
*/
class coro_mgr : public zce::async::manager
{
public:
    //
    coro_mgr();
    virtual ~coro_mgr();

protected:

    ///默认异步对象池子的初始化的数量
    static const size_t COROUTINE_POOL_INIT_SIZE = 1;
    ///默认池子扩展的时候，扩展的异步对象的数量
    static const size_t COROUTINE_POOL_EXTEND_SIZE = 32;
};
}