
#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_coroutine.h"

#if defined ZCE_OS_WINDOWS


//帮助完成函数适配适配
VOID  WINAPI _fibers_adapt_nonparafun(VOID *fun_para)
{
    ZCE_COROUTINE_NONPARA fun_ptr = (ZCE_COROUTINE_NONPARA) fun_para;
    fun_ptr();
}
#endif


//兼容封装的makecontext，非标准函数，可以使用2个参数的函数指针
int ZCE_OS::makecontext(ucontext_t * uctt,
    size_t stack_size,
    ZCE_COROUTINE_NONPARA nopara_fun)
{
#if defined ZCE_OS_WINDOWS

    
    //注意FIBER_FLAG_FLOAT_SWITCH 在XP是不被支持的，
    //因为CreateFiber，需要的的函数纸质是WINAPI的，就是__stdcall的，所以做一个转换，
    *uctt = ::CreateFiberEx(stack_size,
        stack_size,
        FIBER_FLAG_FLOAT_SWITCH,
        _fibers_adapt_nonparafun,
        nopara_fun);

    if (NULL == uctt)
    {
        return -1;
    }
    return 0;
#elif defined ZCE_OS_LINUX

    //
    int ret = ::getcontext(uctt);
    if (0 != ret)
    {
        return ret;
    }

    //使用0个参数
    const int ONLY_NON_ARG_COUNT = 0;

    uctt->uc_stack.ss_sp = new char[stack_size];
    uctt->uc_stack.ss_size = stack_size;

    ::makecontext(uctt,
        (void(*)(void)) nopara_fun,
        ONLY_NON_ARG_COUNT);
    return 0;
#endif
}



#if defined ZCE_OS_WINDOWS


//因为CreateFiber，需要的的函数纸质是WINAPI的，就是__stdcall的，所以做一个转换，
struct _FIBERS_TWOPARAFUN_ADAPT
{
    ///函数指针
    ZCE_COROUTINE_TWOPARA   fun_ptr_;
    //函数的第一个参数，左参
    void                   *left_para_;
    //函数的第一个参数，右参
    void                   *right_para_;
};

//帮助完成函数适配适配
VOID  WINAPI _fibers_adapt_twoparafun (VOID *fun_para)
{
    
    _FIBERS_TWOPARAFUN_ADAPT *fun_adapt = (_FIBERS_TWOPARAFUN_ADAPT *)fun_para;
    ZCE_COROUTINE_TWOPARA twopara_fun = fun_adapt->fun_ptr_;
    void *left_para = fun_adapt->left_para_;
    void *right_para = fun_adapt->right_para_;
    
    //这个函数是堆分配的，要清理掉释放
    delete fun_adapt;

    twopara_fun(left_para,right_para);


}

#endif



//兼容封装的makecontext，非标准函数，可以使用2个参数的函数指针
int ZCE_OS::makecontext(ucontext_t * uctt,
    size_t stack_size, 
    ZCE_COROUTINE_TWOPARA fun_ptr,
    void *left_para, 
    void *right_para)
{
#if defined ZCE_OS_WINDOWS

    //使用这个结构完成函数适配
    struct _FIBERS_TWOPARAFUN_ADAPT *fibers_adapt = new _FIBERS_TWOPARAFUN_ADAPT();
    fibers_adapt->fun_ptr_ = fun_ptr;
    fibers_adapt->left_para_ = left_para;
    fibers_adapt->right_para_ = right_para;

    //注意FIBER_FLAG_FLOAT_SWITCH 在XP是不被支持的，
    *uctt = ::CreateFiberEx(stack_size,
        stack_size,
        FIBER_FLAG_FLOAT_SWITCH,
        _fibers_adapt_twoparafun,
        fibers_adapt);

    if (NULL == uctt)
    {
        return -1;
    }
    return 0;
#elif defined ZCE_OS_LINUX
    
    //
    int ret = ::getcontext(uctt);
    if (0 != ret)
    {
        return ret;
    }

    //只使用一个参数，不允许使用变参，Windwos不支持
    const int ONLY_TWO_ARG_COUNT = 2;

    uctt->uc_stack.ss_sp = new char [stack_size];
    uctt->uc_stack.ss_size = stack_size;

    ::makecontext(uctt, 
        (void(*)(void)) fun_ptr,
        ONLY_TWO_ARG_COUNT,
        left_para,
        right_para);
    return 0;
#endif
}

//非标准函数，
void ZCE_OS::deletecontext(ucontext_t *uctt)
{
#if defined ZCE_OS_WINDOWS
    return ::DeleteFiber(*uctt);
#elif defined ZCE_OS_LINUX

    //释放ZCE_OS::makecontext申请的空间
    delete [] (char *) uctt->uc_stack.ss_sp ;
    uctt->uc_stack.ss_sp = NULL;
    uctt->uc_stack.ss_size = 0;
#endif
}


int ZCE_OS::getcontext(ucontext_t *uctt)
{
#if defined ZCE_OS_WINDOWS

    //如果当前还不是纤程，进行转换，同时也到当前的纤程标识
    if (FALSE == ::IsThreadAFiber() )
    {
        //FIBER_FLAG_FLOAT_SWITCH XP不支持，
        *uctt = ::ConvertThreadToFiberEx(NULL,
            FIBER_FLAG_FLOAT_SWITCH);
        if (NULL == uctt)
        {
            return -1;
        }
        return 0;
    }
    //如果已经是纤程了，得到当前纤程的标识
    else
    {
        *uctt = ::GetCurrentFiber();
        if (NULL == uctt)
        {
            return -1;
        }
        return 0;
    }
   

#elif defined ZCE_OS_LINUX
    return ::getcontext(uctt);
#endif 
}


//设置context
int ZCE_OS::setcontext(const ucontext_t *uctt)
{
#if defined ZCE_OS_WINDOWS
    ::SwitchToFiber(*uctt);
    return 0;
#elif defined ZCE_OS_LINUX
    return ::setcontext(uctt);
#endif
}


//交互context,保存当前的，设置成新的
int ZCE_OS::swapcontext(ucontext_t *get_uctt, const ucontext_t *set_uctt)
{
#if defined ZCE_OS_WINDOWS
    int ret = 0;

    if ((get_uctt == NULL) || (set_uctt == NULL)) 
    {
        errno = EINVAL;
        return -1;
    }

    ret = ZCE_OS::getcontext(get_uctt);
    if (ret == 0) 
    {
        ret = ZCE_OS::setcontext(set_uctt);
    }
    return ret;

#elif defined ZCE_OS_LINUX
    return ::swapcontext(get_uctt,set_uctt);
#endif 
}

