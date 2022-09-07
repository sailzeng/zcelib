#pragma once

#include "zce/async/async_base.h"

namespace zce
{
//====================================================================================

struct coro_ret
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        promise_type() = default;
        ~promise_type() = default;

        auto get_return_object()
        {
            return coro_ret{ handle_type::from_promise(*this) };
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
    coro_ret()
    {
    }
    coro_ret(handle_type h)
        : coro_handle_(h)
    {
    }
    coro_ret(const coro_ret&) = delete;
    coro_ret(coro_ret&& s)
        : coro_handle_(s.coro_handle_)
    {
        s.coro_handle_ = nullptr;
    }
    ~coro_ret()
    {
        if (coro_handle_)
            coro_handle_.destroy();
    }
    coro_ret& operator=(const coro_ret&) = delete;
    coro_ret& operator=(coro_ret&& s)
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
*             注意，为了避免一些无意义的暴漏，我这儿选择的继承方式是private
*/
class Async_Coroutine : public zce::Async_Object
{
    friend class Async_CoroutineMgr;

    //
    enum class COROUTINE_STATE
    {
        //
        INVALID = 0x0,
        //携程传递给管理器的状态值，
        CONTINUE = 0x10001,
        EXIT = 0x10002,

        //超时后，管理器通知携程的状态值
        TIMEOUT = 0x20002,
    };

public:
    /*!
    * @brief      构造函数，
    * @param      async_mgr ,协程异步管理器的指针
    */
    Async_Coroutine(zce::Async_Obj_Mgr* async_mgr,
                    uint32_t reg_cmd);
protected:
    /*!
    * @brief      析构函数
    */
    ~Async_Coroutine();

public:

    /*!
    * @brief      初始化函数，在构造函数后调用，在放入池子前执行一次，
    * @return     int 0标识成功
    */
    virtual int initialize();

    /*!
    * @brief      结束销毁函数，在析构前的调用
    * @return     int
    */
    virtual void terminate();

protected:

    //!协程运行,你要重载的函数
    virtual coro_ret coroutine_run() = 0;

    /*!
    * @brief      等待time_out 时间后超时，设置定时器后，切换协程到main
    * @return     int
    * @param      time_out
    */
    int waitfor_timeout(const zce::Time_Value& time_out);

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
    virtual void on_timeout(const zce::Time_Value& now_time,
                            bool& continued) override;


protected:
    //!
    coro_ret         coroutine_ret_;
    //!协程的状态
    COROUTINE_STATE  coroutine_state_ = COROUTINE_STATE::INVALID;
};

//====================================================================================

/*!
* @brief      协程对象主控管理类
*
*/
class Async_CoroutineMgr : public zce::Async_Obj_Mgr
{
public:
    //
    Async_CoroutineMgr();
    virtual ~Async_CoroutineMgr();

protected:

    ///默认异步对象池子的初始化的数量
    static const size_t COROUTINE_POOL_INIT_SIZE = 1;
    ///默认池子扩展的时候，扩展的异步对象的数量
    static const size_t COROUTINE_POOL_EXTEND_SIZE = 32;
};
}