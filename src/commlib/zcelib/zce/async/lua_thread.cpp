#include "zce/predefine.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"
#include "zce/async/lua_thread.h"

namespace zce
{
//========================================================================================

Async_LuaThead::Async_LuaThead(zce::Async_Obj_Mgr* async_mgr,
                               uint32_t reg_cmd) :
    zce::Async_Object(async_mgr, reg_cmd)
{
}

Async_LuaThead::~Async_LuaThead()
{
}

//初始化协程的对象
int Async_LuaThead::initialize()
{
    zce::Async_Object::initialize();
    auto luathread_mgr = static_cast <Async_LuaTheadMgr*>(async_mgr_);
    mgr_lua_tie_ = luathread_mgr->mgr_lua_tie();
    mgr_lua_tie_->new_thread(&lua_thread_);

    //理论下面你需要打开lua脚本
    return 0;
}

//清理协程对象
void Async_LuaThead::terminate()
{
    zce::Async_Object::terminate();
    mgr_lua_tie_->del_thread(&lua_thread_);
    return;
}

//调用协程
void Async_LuaThead::on_run(bool first_run, bool& continue_run)
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
        else if (state == LUA_OK)
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
void Async_LuaThead::on_timeout(const zce::time_value& /*now_time*/,
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
    else if (state == LUA_OK)
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
Async_LuaTheadMgr::Async_LuaTheadMgr() :
    zce::Async_Obj_Mgr()
{
    pool_init_size_ = COROUTINE_POOL_INIT_SIZE;
    pool_extend_size_ = COROUTINE_POOL_EXTEND_SIZE;
}

Async_LuaTheadMgr::~Async_LuaTheadMgr()
{
}
}