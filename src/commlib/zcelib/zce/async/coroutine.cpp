#include "zce/predefine.h"
#include "zce/async/coroutine.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"

namespace zce
{
//========================================================================================

Async_Coroutine::Async_Coroutine(zce::Async_Obj_Mgr* async_mgr,
                                 uint32_t reg_cmd) :
    zce::Async_Object(async_mgr, reg_cmd)
{
    //堆栈大小默认选择最小的，
}

Async_Coroutine::~Async_Coroutine()
{
}


//调用协程
void Async_Coroutine::on_run(bool first_run,
                             bool& continue_run)
{
    continue_run = true;
    int ret = 0;
    if (first_run)
    {
        ret = coroutine_ret_.get();
        coroutine_ret_ = coroutine_run();
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
void Async_Coroutine::on_timeout(const zce::Time_Value& /*now_time*/,
                                 bool& continued)
{
    continued = false;
    coroutine_state_ = COROUTINE_STATE::TIMEOUT;
    //resume_coroutine();

    //根据调用返回的函数记录的状态值得到当前的状态
    if (coroutine_state_ == COROUTINE_STATE::CONTINUE)
    {
        continued = true;
    }
    else if (coroutine_state_ == COROUTINE_STATE::EXIT)
    {
        continued = false;
    }
    else
    {
        ZCE_ASSERT_ALL(false);
    }
}


//等待time_out 时间后超时，设置定时器后，
int Async_Coroutine::waitfor_timeout(const zce::Time_Value& time_out)
{
    int ret = 0;
    ret = set_timeout(time_out);
    if (0 != ret)
    {
        return ret;
    }
    return 0;
}

//=====================================================================================

//携程主控管理类
Async_CoroutineMgr::Async_CoroutineMgr() :
    zce::Async_Obj_Mgr()
{
    pool_init_size_ = COROUTINE_POOL_INIT_SIZE;
    pool_extend_size_ = COROUTINE_POOL_EXTEND_SIZE;
}

Async_CoroutineMgr::~Async_CoroutineMgr()
{
}
}