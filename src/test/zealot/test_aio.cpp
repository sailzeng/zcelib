#include "predefine.h"

ZCE_HANDLE file_handle = ZCE_INVALID_HANDLE;

void on_writefile(zce::aio::AIO_ATOM* ahdl)
{
    auto fhdl = (zce::aio::FS_ATOM*)(ahdl);
    std::cout << "on_writefile,reuslt:" << fhdl->result_ << " count:" << fhdl->result_len_ << std::endl;
}

void on_readfile(zce::aio::AIO_ATOM* ahdl)
{
    auto fhdl = (zce::aio::FS_ATOM*)(ahdl);
    std::cout << "on_readfile,reuslt:" << fhdl->result_ << " count:" << fhdl->result_len_ << std::endl;
    if (fhdl->result_ == 0 && *fhdl->result_len_ > 0)
    {
        if (*fhdl->result_len_ < 1024)
        {
            fhdl->read_bufs_[*fhdl->result_len_] = '\0';
        }
        std::cout << "context:" << std::endl;
        std::cout << fhdl->read_bufs_ << std::endl;
    }
}

int test_aio1(int /*argc*/, char* /*argv*/[])
{
    int ret = 0;
    zce::aio::worker aio_worker;
    ret = aio_worker.initialize(5, 2048, 2048);
    if (ret)
    {
        return ret;
    }
    ret = zce::aio::fs_writefile(&aio_worker,
                                 "E:/TEST001/aio_test_001.txt",
                                 "01234567890123456789",
                                 20,
                                 on_writefile);
    if (ret)
    {
        return ret;
    }
    char read_buf[1024];
    ret = zce::aio::fs_readfile(&aio_worker,
                                "E:/TEST001/aio_test_002.txt",
                                read_buf,
                                1024,
                                on_readfile);
    if (ret)
    {
        return ret;
    }
    size_t count = 0;
    do
    {
        size_t num_event = 0, num_rsp = 0;
        zce::time_value tv(1, 0);
        aio_worker.process_response(&tv, num_event, num_rsp);
        if (num_rsp > 0)
        {
            count += num_rsp;
        }
    } while (count < 2);
    aio_worker.terminate();
    return 0;
}

template <typename T>
struct coro_ret
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro_handle_;

    coro_ret(handle_type h)
        : coro_handle_(h)
    {
    }
    coro_ret(const coro_ret&) = delete;
    coro_ret(coro_ret&& s)
        : coro_handle_(s.coro_)
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

    T get()
    {
        return coro_handle_.promise().return_data_;
    }

    struct promise_type
    {
        promise_type() = default;
        ~promise_type() = default;

        auto get_return_object()
        {
            return coro_ret<T>{handle_type::from_promise(*this)};
        }
        //注意这个函数,如果返回std::suspend_never{}，就不挂起，
        //返回std::suspend_always{} 挂起
        auto initial_suspend()
        {
            return std::suspend_never{};
            //return std::suspend_always{};
        }
        //co_return 后这个函数会被调用
        void return_value(T v)
        {
            return_data_ = v;
            return;
        }
        auto yield_value(T v)
        {
            std::cout << "yield_value invoked." << std::endl;
            return_data_ = v;
            return std::suspend_always{};
        }
        auto final_suspend() noexcept
        {
            std::cout << "final_suspend invoked." << std::endl;
            return std::suspend_always{};
        }
        void unhandled_exception()
        {
            std::exit(1);
        }
        //返回值
        T return_data_;
    };
};

coro_ret<int> coroutine_aio(zce::aio::worker* worker)
{
    char read_buf[1024];
    std::cout << "Coroutine co_await co_read_file" << std::endl;
    size_t read_size = 0;
    auto r_ret = co_await zce::aio::co_fs_readfile(worker,
                                                   "E:/TEST001/aio_test_002.txt",
                                                   read_buf,
                                                   1024,
                                                   &read_size);

    std::cout << "co_read_file,reuslt:" << r_ret << " count:" << read_size << std::endl;
    if (r_ret == 0 && read_size > 0)
    {
        if (read_size < 1024)
        {
            read_buf[read_size] = '\0';
        }
        std::cout << "context:" << std::endl;
        std::cout << read_size << std::endl;
    }

    size_t write_size = 0;
    std::cout << "Coroutine co_await co_write_file" << std::endl;
    auto w_ret = co_await zce::aio::co_fs_writefile(worker,
                                                    "E:/TEST001/aio_test_001.txt",
                                                    "01234567890123456789",
                                                    20,
                                                    &write_size);
    std::cout << "co_write_file,reuslt:" << w_ret << " count:" << write_size << std::endl;
    char tm_stt[128];
    size_t sz_buf = 0;
    zce::time_value tmo_tv(5, 0);
    int time_id;
    zce::time_value tigger_tv;
    auto tmo_ret = co_await zce::aio::co_timeout_schedule(worker,
                                                          tmo_tv,
                                                          &time_id,
                                                          &tigger_tv);
    std::cout << "co_write_file,reuslt:" << tmo_ret << " count:" <<
        tigger_tv.to_str(tm_stt, 128, sz_buf) << std::endl;

    co_return 0;
}

int test_aio3(int /*argc*/, char* /*argv*/[])
{
    int ret = 0;
    zce::aio::worker aio_worker;
    ret = aio_worker.initialize(5, 2048, 2048);
    if (ret)
    {
        return 101;
    }

    std::cout << "Start coroutine_aio()\n";
    auto c_r = coroutine_aio(&aio_worker);
    std::cout << "exit coroutine_aio ()\n";
    std::cout << "coroutine done ?:" <<
        (c_r.done() ? "true" : "false") << std::endl;
    //第一次停止因为initial_suspend 返回的是suspend_always
    //此时没有进入Stage 1
    size_t count = 0;
    do
    {
        size_t num_rsp = 0, num_event = 0;
        zce::time_value tv(1, 0);
        aio_worker.process_response(&tv, num_rsp, num_event);
        if (num_rsp > 0)
        {
            count += num_rsp;
        }
        std::cout << "coroutine done ?:" <<
            (c_r.done() ? "true" : "false") << std::endl;
    } while (count < 3);
    aio_worker.terminate();
    return 0;
}

void on_timeout(zce::aio::AIO_ATOM* ahdl)
{
    auto tom = (zce::aio::TIMER_ATOM*)(ahdl);
    char tv_str[128];
    size_t use_len = 0;
    std::cout << "on_timeout:" << tom->timer_id_ << " timevalue:" << tom->trigger_tv_->to_str(tv_str, 128, use_len) << std::endl;
}

int test_aio4(int /*argc*/, char* /*argv*/[])
{
    int ret = 0;
    zce::aio::worker aio_worker;
    ret = aio_worker.initialize(5, 2048, 2048);
    if (ret)
    {
        return 101;
    }
    int timer_id = 0;
    zce::time_value tv_to(5, 0);
    zce::time_value trigger_tv;
    ret = zce::aio::tmo_schedule(&aio_worker,
                                 tv_to,
                                 &timer_id,
                                 &trigger_tv,
                                 on_timeout);
    if (ret)
    {
        return ret;
    }

    //此时没有进入Stage 1
    size_t count = 0;
    do
    {
        size_t num_event = 0, num_rsp = 0;
        zce::time_value tv(1, 0);
        aio_worker.process_response(&tv, num_event, num_rsp);
        if (num_rsp > 0)
        {
            count += num_rsp;
        }
    } while (true);
    aio_worker.terminate();

    return 0;
}