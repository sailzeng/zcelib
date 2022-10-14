#include "zce/predefine.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"
#include "zce/async/lua_thread.h"

namespace zce
{
//========================================================================================

async_luathead::async_luathead(zce::async_obj_mgr* async_mgr,
                               uint32_t reg_cmd) :
    zce::async_object(async_mgr, reg_cmd)
{
}

async_luathead::~async_luathead()
{
}

//初始化协程的对象
int async_luathead::initialize()
{
    zce::async_object::initialize();
    auto luathread_mgr = static_cast <async_luathead_mgr*>(async_mgr_);
    mgr_lua_tie_ = luathread_mgr->mgr_lua_tie();
    mgr_lua_tie_->new_thread(&lua_thread_);

    //理论下面你需要打开lua脚本
    return 0;
}

//清理协程对象
void async_luathead::terminate()
{
    zce::async_object::terminate();
    mgr_lua_tie_->del_thread(&lua_thread_);
    return;
}

//调用协程
void async_luathead::on_run(bool first_run, bool& continue_run)
{
    continue_run = true;
    if (first_run)
    {
        int ret = luathread_run();
        if (ret != 0)
        {
            continue_run = false;
        }
    }
    else
    {
        int state = mgr_lua_tie_->resume_thread(&lua_thread_, 0);
        //根据调用返回的函数记录的状态值得到当前的状态
        if (state == LUA_YIELD)
        {
            continue_run = true;
        }
        else if (state == 0)
        {
            continue_run = false;
        }
        else
        {
            ZCE_ASSERT_ALL(false);
        }
    }
}

//调用协程
void async_luathead::on_timeout(const zce::time_value& /*now_time*/,
                                bool& running)
{
    running = false;
    //int state = COROUTINE_STATE::TIMEOUT;
    int state = mgr_lua_tie_->resume_thread(&lua_thread_, 0);

    //根据调用返回的函数记录的状态值得到当前的状态
    if (state == LUA_YIELD)
    {
        running = true;
    }
    else if (state == 0)
    {
        running = false;
    }
    else
    {
        ZCE_ASSERT_ALL(false);
    }
}

//=====================================================================================

//携程主控管理类
async_luathead_mgr::async_luathead_mgr() :
    zce::async_obj_mgr()
{
    pool_init_size_ = COROUTINE_POOL_INIT_SIZE;
    pool_extend_size_ = COROUTINE_POOL_EXTEND_SIZE;
}

async_luathead_mgr::~async_luathead_mgr()
{
}
}