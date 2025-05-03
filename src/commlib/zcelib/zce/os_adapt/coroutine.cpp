#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/coroutine.h"

#if defined ZCE_OS_WINDOWS

VOID  WINAPI _FIBERS_FUN_ADAPT::adapt_fun(VOID* param)
{
    _FIBERS_FUN_ADAPT* fun_adapt = (_FIBERS_FUN_ADAPT*)param;

    //coroutine_t* handle = fun_adapt->handle_;
    //bool exit_back_main = fun_adapt->exit_back_main_;
    std::function<void()> fun_call(std::move(fun_adapt->fun_));
    //this parameter is heap allocated, need to delete it
    delete fun_adapt;

    fun_call();

    //Fiber function exits, switch to SwitchToFiber main fiber
}

#elif defined ZCE_OS_LINUX

void  _FIBERS_FUN_ADAPT::adapt_fun(void* param)
{
    _FIBERS_FUN_ADAPT* fun_adapt = (_FIBERS_FUN_ADAPT*)param;
    coroutine_t* handle = fun_adapt->handle_;
    bool exit_back_main = fun_adapt->exit_back_main_;
    std::function<void()> fun_call(std::move(fun_adapt->fun_));
    //this adapt function is heap allocated, need to delete it
    delete fun_adapt;
}
#endif

//非标准函数，
void zce::delete_coroutine(coroutine_t* coroutine_hdl)
{
#if defined ZCE_OS_WINDOWS
    //
    if (coroutine_hdl->coroutine_)
    {
        ::DeleteFiber(coroutine_hdl->coroutine_);
    }
    return;
#elif defined ZCE_OS_LINUX

    //释放zce::makecontext申请的空间
    delete[](char*) coroutine_hdl->coroutine_.uc_stack.ss_sp;
    coroutine_hdl->coroutine_.uc_stack.ss_sp = nullptr;
    coroutine_hdl->coroutine_.uc_stack.ss_size = 0;
#endif
}

//切换到协程
int zce::resume_coroutine(coroutine_t* coroutine_hdl)
{
#if defined ZCE_OS_WINDOWS
    int ret = 0;

    if ((coroutine_hdl == nullptr))
    {
        errno = EINVAL;
        return -1;
    }

    ::SwitchToFiber(coroutine_hdl->coroutine_);
    return ret;

#elif defined ZCE_OS_LINUX
    //注意呀，swapcontext给你一个错觉，会让你怀疑，没有getcontext怎么跳回来？
    //其实swapcontext是保存了参数1的context，然后才跳入参数2的context的，
    return ::swapcontext(&coroutine_hdl->main_,
                         &coroutine_hdl->coroutine_);
#endif
}

//切换到Main
int zce::yeild_coroutine(coroutine_t* coroutine_hdl)
{
#if defined ZCE_OS_WINDOWS
    int ret = 0;

    if ((coroutine_hdl == nullptr))
    {
        errno = EINVAL;
        return -1;
    }

    ::SwitchToFiber(coroutine_hdl->main_);
    return ret;

#elif defined ZCE_OS_LINUX

    return ::swapcontext(&coroutine_hdl->coroutine_,
                         &coroutine_hdl->main_);
#endif
}

int zce::exchage_coroutine(coroutine_t* save_hdl,
                           coroutine_t* goto_hdl)
{
#if defined ZCE_OS_WINDOWS
    //
    ZCE_UNUSED_ARG(save_hdl);
    ::SwitchToFiber(goto_hdl->coroutine_);
    return 0;

#elif defined ZCE_OS_LINUX
    return ::swapcontext(&save_hdl->coroutine_,
                         &goto_hdl->coroutine_);
#endif
}
