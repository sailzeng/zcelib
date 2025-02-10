#include "zce/predefine.h"
#include "zce/async/coroutine.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"

namespace zce
{
//========================================================================================

coro::coro(zce::async::manager* async_mgr,
           uint32_t reg_cmd) :
    zce::async::actor(async_mgr, reg_cmd)
{
    //堆栈大小默认选择最小的，
}

coro::~coro()
{
}

//调用协程
void coro::on_run(bool first_run,
                  bool& continue_run)
{
    continue_run = true;
    int ret = 0;
    if (first_run)
    {
        ret = coroutine_ret_.get();
        coroutine_ret_ = coro_run();
        if (ret != 0)
        {
            ZCE_TRACE_FAIL_RETURN(RS_ERROR, "coroutine_run return fail.",
                                  ret);
            return;
        }
        //!
        if (coroutine_ret_.done())
        {
            continue_run = false;
        }
        return;
    }
    else
    {
        bool done = coroutine_ret_.move_next();
        if (done)
        {
            continue_run = false;
        }
        return;
    }
}

//调用协程
void coro::on_timeout(const zce::time_value& /*now_time*/,
                      bool& continue_run)
{
    occur_timeout_ = true;
    continue_run = true;
    //根据调用返回的函数记录的状态值得到当前的状态
    bool done = coroutine_ret_.move_next();
    if (done)
    {
        continue_run = false;
    }
    return;
}

//等待time_out 时间后超时，设置定时器后，
int coro::waitfor_timeout(const zce::time_value& time_out)
{
    int ret = 0;
    occur_timeout_ = false;
    ret = set_timeout(time_out);
    if (0 != ret)
    {
        return ret;
    }
    return 0;
}

int coro::initialize()
{
    occur_timeout_ = false;
    return 0;
}
void coro::terminate()
{
    return;
}
//=====================================================================================

//携程主控管理类
coro_mgr::coro_mgr() :
    zce::async::manager()
{
    pool_init_size_ = COROUTINE_POOL_INIT_SIZE;
    pool_extend_size_ = COROUTINE_POOL_EXTEND_SIZE;
}

coro_mgr::~coro_mgr()
{
}
}