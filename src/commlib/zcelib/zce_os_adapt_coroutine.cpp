
#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_coroutine.h"


#if defined ZCE_OS_WINDOWS

///Windows的Fiber实现(CreateFiber)函数指针对应的参数只有一个，而且需要的的函数纸质是WINAPI的，
///就是__stdcall的，而且Fiber没有返回的context指定,所以做一个转换，
struct _FIBERS_3PARAFUN_ADAPT
{
    ///
    coroutine_t          *handle_;
    ///是否在退出的时候返回主协程，
    bool                  exit_back_main_;
    ///函数指针
    ZCE_COROUTINE_3PARA   fun_ptr_;

    //函数的第1个参数，
    void                 *para1_;
    //函数的第2个参数
    void                 *para2_;
    //函数的第3个参数
    void                 *para3_;
};

//帮助完成函数适配适配
VOID  WINAPI _fibers_adapt_fun (VOID *fun_para)
{



    _FIBERS_3PARAFUN_ADAPT *fun_adapt = (_FIBERS_3PARAFUN_ADAPT *)fun_para;

    coroutine_t *handle = fun_adapt->handle_;
    bool exit_back_main = fun_adapt->exit_back_main_;

    ZCE_COROUTINE_3PARA treepara_fun = fun_adapt->fun_ptr_;
    void *para1 = fun_adapt->para1_;
    void *para2 = fun_adapt->para2_;
    void *para3 = fun_adapt->para3_;

    //这个函数是堆分配的，要清理掉释放
    delete fun_adapt;

    treepara_fun(para1, para2, para3);

    if (exit_back_main)
    {
        ::SwitchToFiber(handle->main_);
    }
}

#endif



//兼容封装的makecontext，非标准函数，可以使用2个参数的函数指针
int ZCE_LIB::make_coroutine(coroutine_t *coroutine_hdl,
                           size_t stack_size,
                           bool exit_back_main,
                           ZCE_COROUTINE_3PARA fun_ptr,
                           void *para1,
                           void *para2,
                           void *para3)
{
#if defined ZCE_OS_WINDOWS

    coroutine_hdl->main_ = NULL;
    coroutine_hdl->coroutine_ = NULL;

    //如果当前还不是纤程，进行转换，同时也到当前的纤程标识
    if (FALSE == ::IsThreadAFiber())
    {
        //FIBER_FLAG_FLOAT_SWITCH XP不支持，
        coroutine_hdl->main_ = ::ConvertThreadToFiberEx(NULL,
                                                        FIBER_FLAG_FLOAT_SWITCH);
        if (NULL == coroutine_hdl->main_)
        {
            return -1;
        }
    }
    //如果已经是纤程了，得到当前纤程的标识
    else
    {
        coroutine_hdl->main_ = ::GetCurrentFiber();
        if (NULL == coroutine_hdl->main_)
        {
            return -1;
        }
    }


    //使用这个结构完成函数适配
    struct _FIBERS_3PARAFUN_ADAPT *fibers_adapt = new _FIBERS_3PARAFUN_ADAPT();
    fibers_adapt->exit_back_main_ = exit_back_main;
    fibers_adapt->fun_ptr_ = fun_ptr;
    fibers_adapt->para1_ = para1;
    fibers_adapt->para2_ = para2;
    fibers_adapt->para3_ = para3;


    //注意FIBER_FLAG_FLOAT_SWITCH 在XP是不被支持的，
    coroutine_hdl->coroutine_ = ::CreateFiberEx(stack_size,
                                                stack_size,
                                                FIBER_FLAG_FLOAT_SWITCH,
                                                _fibers_adapt_fun,
                                                fibers_adapt);

    if (NULL == coroutine_hdl->coroutine_)
    {
        return -1;
    }

    fibers_adapt->handle_ = coroutine_hdl;

    return 0;
#elif defined ZCE_OS_LINUX

    //
    int ret = ::getcontext(&(coroutine_hdl->main_));
    if (0 != ret)
    {
        return ret;
    }
    ret = ::getcontext(&(coroutine_hdl->coroutine_));
    if (0 != ret)
    {
        return ret;
    }

    //只使用一个参数，不允许使用变参，Windwos不支持
    const int ONLY_3_ARG_COUNT = 3;
    if (exit_back_main)
    {
        coroutine_hdl->coroutine_.uc_link = &(coroutine_hdl->main_);
    }
    else
    {
        coroutine_hdl->coroutine_.uc_link = NULL;
    }
    coroutine_hdl->coroutine_.uc_stack.ss_sp = new char[stack_size];
    coroutine_hdl->coroutine_.uc_stack.ss_size = stack_size;

    ::makecontext(coroutine_hdl->coroutine_,
                  (void( *)(void)) fun_ptr,
                  ONLY_3_ARG_COUNT,
                  para1,
                  para2,
                  para3
                 );
    return 0;
#endif
}

//非标准函数，
void ZCE_LIB::delete_coroutine(coroutine_t *coroutine_hdl)
{
#if defined ZCE_OS_WINDOWS
    //
    if (coroutine_hdl->coroutine_)
    {
        ::DeleteFiber(coroutine_hdl->coroutine_);
    }
    return ;
#elif defined ZCE_OS_LINUX

    //释放ZCE_LIB::makecontext申请的空间
    delete[](char *) coroutine_hdl->coroutine_.uc_stack.ss_sp;
    coroutine_hdl->coroutine_.uc_stack.ss_sp = NULL;
    coroutine_hdl->coroutine_.uc_stack.ss_size = 0;
#endif
}


//切换到协程
int ZCE_LIB::yeild_coroutine(coroutine_t *coroutine_hdl)
{
#if defined ZCE_OS_WINDOWS
    int ret = 0;

    if ((coroutine_hdl == NULL) )
    {
        errno = EINVAL;
        return -1;
    }

    ::SwitchToFiber(coroutine_hdl->coroutine_);
    return ret;

#elif defined ZCE_OS_LINUX
    return ::swapcontext(coroutine_hdl->main_,
                         coroutine_hdl->coroutine_);
#endif
}


//切换到Main
int ZCE_LIB::yeild_main(coroutine_t *coroutine_hdl)
{
#if defined ZCE_OS_WINDOWS
    int ret = 0;

    if ((coroutine_hdl == NULL))
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

int ZCE_LIB::exchage_coroutine(coroutine_t *save_hdl,
                              coroutine_t *goto_hdl)
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

