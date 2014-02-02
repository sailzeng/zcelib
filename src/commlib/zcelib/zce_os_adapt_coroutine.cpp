
#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_coroutine.h"


#if defined ZCE_OS_WINDOWS


//因为CreateFiber，需要的的函数纸质是WINAPI的，就是__stdcall的，所以做一个转换，
struct _FIBERS_FUN_ADAPT_STRUCT
{
    ///函数指针
    ZCE_COROUTINE_FUN_PTR   fun_ptr_;
    //函数的第一个参数，左参
    void                   *left_para_;
    //函数的第一个参数，右参
    void                   *right_para_;
};

//帮助完成函数适配适配
VOID  WINAPI _fibers_adapt_fun (VOID *fun_para)
{
    _FIBERS_FUN_ADAPT_STRUCT *fun_adapt = (_FIBERS_FUN_ADAPT_STRUCT *)fun_para;
    fun_adapt->fun_ptr_(fun_adapt->left_para_, fun_adapt->right_para_);
}

#endif



//非标准函数，第一个参数是堆栈大小，内部会自己分配一个堆数据，给这个堆栈，所以必须调用deletecontext
int ZCE_OS::makecontext(ucontext_t * uctt,
    size_t slack_size, 
    ZCE_COROUTINE_FUN_PTR fun_ptr,
    void *left_para, 
    void *right_para)
{
#if defined ZCE_OS_WINDOWS

    //使用这个结构完成函数适配
    struct _FIBERS_FUN_ADAPT_STRUCT fibers_adapt;
    fibers_adapt.fun_ptr_ = fun_ptr;
    fibers_adapt.left_para_ = left_para;
    fibers_adapt.right_para_ = right_para;

    //注意FIBER_FLAG_FLOAT_SWITCH 在XP是不被支持的，
    *uctt = ::CreateFiberEx(slack_size,
        slack_size,
        FIBER_FLAG_FLOAT_SWITCH,
        _fibers_adapt_fun,
        &fibers_adapt);

    if (NULL == uctt)
    {
        return -1;
    }
    return 0;
#elif defined ZCE_OS_LINUX
    //只使用一个参数，不允许使用变参，Windwos不支持
    const int ONLY_TWO_ARG_COUNT = 2;

    uctt->uc_stack.ss_sp = new char [slack_size];
    uctt->uc_stack.ss_size = slack_size;

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

    //如果已经是纤程了，得到当前纤程的标识
    if (TRUE == ::IsThreadAFiber() )
    {
        *uctt = ::GetCurrentFiber();
        if (NULL == uctt)
        {
            return -1;
        }
        return 0;
    }
    //如果当前还不是纤程，进行转换，同时也到当前的纤程标识
    else
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

