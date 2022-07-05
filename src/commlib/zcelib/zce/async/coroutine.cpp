#include "zce/predefine.h"
#include "zce/async/coroutine.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"

namespace zce
{
//========================================================================================

Async_Coroutine::Async_Coroutine(zce::Async_Obj_Mgr* async_mgr,
                                 unsigned int reg_cmd) :
    zce::Async_Object(async_mgr, reg_cmd)
{
    //堆栈大小默认选择最小的，
}

Async_Coroutine::~Async_Coroutine()
{
}

//初始化协程的对象
int Async_Coroutine::initialize()
{
    zce::Async_Object::initialize();

    return 0;
}

//清理协程对象
void Async_Coroutine::terminate()
{
    zce::Async_Object::terminate();
    return;
}

//调用协程
void Async_Coroutine::on_run(bool& continued)
{
    continued = false;
}

//调用协程
void Async_Coroutine::on_timeout(const zce::Time_Value& /*now_time*/,
                                 bool& continued)
{
    continued = false;
    coroutine_state_ = COROUTINE_STATE::TIMEOUT;
    resume_coroutine();

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



//切换回协程，也就是切换到他自己运行
void Async_Coroutine::resume_coroutine()
{
}

//协程对象的运行函数
void Async_Coroutine::coroutine_do()
{
    //如果需要协程
    for (;;)
    {
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