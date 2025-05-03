/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年11月27日
* @brief      协程的OS适配层，
*
* @details    一个简单的协程的封装, 其实与其说更像ucontext，不如说更像是一个Fiber，
*
*             第一次，最开始参考的文章是这个，
*             http://www.codeproject.com/Articles/4225/Unix-ucontext_t-Operations-on-Windows-Platforms
*             但发现其实他并不正确，Windows 的 Thread CONTEXT API 和ucontext不类似。
*             1.Windows 的 Thread CONTEXT API 是一个关于线程的上下文的，和协程没有关系，
*             你可以把它理解Windows 底层获取，设置一个线程的寄存器的快照，和协程没有关系。
*             用于调试器使用（查看断点、栈帧、指令指针），注入器使用：修改线程执行位置（比如
*             DLL 注入、劫持线程），异常处理/恢复上下文
*             2.Windows 下的API GetThreadContext,SetThreadContext ，在64位的环境
*               下是没法用的。后面Windows 增加了Wow64GetThreadContext ，Wow64SetThreadContext
*             3.GetThreadContext对当前运行线程是无效的，因为当前线程是在运行的。
*             这点最讨厌，你就不能切换会主线程了，
*
*             第二次，我希望用Windows 的Fibers来模拟Linux下的getcontext等函数，但发现
*             其实Fibers和Context是有本质不同的，Fibers更像线程的对象（过程），Context
*             更像一个堆栈（点），比如
*             0.ucontext是一个上下文，Fibers是一个异步执行单元
*               ucontext API实现起来更像goto，Fibers的API更像是线程。注意：context的
*               swapcontext会给你错觉。但其实swapcontext是先保存当前的context到第一个参数，
*             1.Fibers是无法实现类似SwitchToFiber( GetCurrentFiber());的调
*               用，其只能跳入另外一个Fibers，
*             2.Fibers除了启动阶段和SwitchToFiber 点，不存在一个类似getcontext的点能
*               切换过去，（getcontext和GetCurrentFiber不是一个东东）
*             3.无法完全融合的差异的，比如CreateFiber 是自己构造堆栈的，而makecontext
*               不处理这些事情，要你自己在参数里面决定。
*             所以……
*
*             所以只能实现一个COROUTINE的封装，我的设计希望是这样的
*             在main里面，在里面用make_coroutine生成协程，
*             在main里面，resume_coroutine恢复到协程，
*             在coroutine里面，使用yeild_coroutine切换到主协程，
*
*             我在等待C++ 20的协程，最后我干掉这些代码。
*             C++ 20出来了，结果是无栈协程
*
* @note       关于Fibers函数的说明，清参考如下文档，作者写的非常清楚。
*             ConvertFiberToThread
*             http://www.cnblogs.com/wz19860913/archive/2008/08/26/1276816.html
*
*/

#pragma once

#include "zce/os_adapt/define.h"

//!
typedef   void(*ZCE_COROUTINE_FUN) (void* para1);

#if defined ZCE_OS_WINDOWS

///coroutine_t 内部保存协程句柄的数据，包括MAIN和COROUTINE的数据
struct  coroutine_t
{
    coroutine_t()
    {
        main_ = nullptr;
        coroutine_ = nullptr;
    }

    void* main_;
    void* coroutine_;
};

#elif defined ZCE_OS_LINUX

struct  coroutine_t
{
    ucontext_t         main_;
    ucontext_t         coroutine_;
};

#endif

///Windows的Fiber实现(CreateFiber)函数指针对应的参数只有一个，而且需要的的函数纸质是WINAPI的，
///就是__stdcall的，而且Fiber没有返回的context指定,所以做一个转换，
struct _FIBERS_FUN_ADAPT
{
    //help adapt function to call the function object
#if defined ZCE_OS_WINDOWS
    static VOID  WINAPI adapt_fun(VOID* param);
#elif defined ZCE_OS_LINUX
    void  adapt_fun(void* param);
#endif
    ///
    coroutine_t* handle_ = nullptr;
    ///是否在退出的时候返回主协程，
    bool                exit_back_main_ = true;
    ///函数指针
    std::function<void()>   fun_;
};

namespace zce
{
/*!
* @brief      非标准函数，生成协程句柄，
*             借用LINUX下的makecontext，Windows下的CreateFiberEx实
*             现，但注意这儿不是生成context,而是协程，注意
*             1.函数指针标识的函数只允许使用2个参数，不能和LINUX一样使用变参
*             2.LINUX原生的::makecontext的ucontext_t参数要先用::getcontext，
*             获取的，  但使用此函数不用，内部集成了，
*             3.第二个参数是堆栈大小，我会根据你的指定分配堆空间，作为你的COROUTINE
*             的栈，而LINUX原生的makecontext 的ucontext_t参数使用前要初始化堆指
*             针和长度，
*             4.结束后，必须调用delete_coroutine清理coroutine_t，
*             delete_coroutine在WINDOWS下调用DeleteFiber函数，在Linux负责清
*             理ucontext_t里面的栈指针（从堆上分配的空间）
*             5.LINUX原生的::makecontext没有返回值，而此封装有，因为Windows下我
*             们用了CreateFiberEx，Linux下我们内部调用了::getcontext，这些函数
*             都可能失败
*             6.LINUX的::makecontext可以使用变参，这个函数没有考虑参数，当然这个变
*             也是有风险，但其实使用参数是有移植风险的，相见下面两个文档的说明，
*             http://en.wikipedia.org/wiki/Setcontext
*             http://pubs.opengroup.org/onlinepubs/009695399/functions/makecontext.html
*             另外，为了包装参数传递，在Windows下，这个函数会new一个结构
* @return     int 返回0标识成功，
* @param      coroutine_hdl ucontext_t，生成的CONTEXT句柄，
* @param      stack_size    栈大小
* @param      back_main     携程最后是否返回main函数
* @param      fp            函数对象，
* @param      args          函数参数，变参
*/
template <class Call, class... Args >
int make_coroutine(coroutine_t* coroutine_hdl,
                   size_t stack_size,
                   bool exit_back_main,
                   Call&& fp,
                   Args&&... args)
{
#if defined ZCE_OS_WINDOWS

    coroutine_hdl->main_ = nullptr;
    coroutine_hdl->coroutine_ = nullptr;

    //如果当前还不是纤程，进行转换，同时也到当前的纤程标识
    if (FALSE == ::IsThreadAFiber())
    {
        //FIBER_FLAG_FLOAT_SWITCH XP不支持，浮点环境切换应该会耗时，有一些简化去掉了
        coroutine_hdl->main_ = ::ConvertThreadToFiberEx(nullptr,
                                                        FIBER_FLAG_FLOAT_SWITCH);
        if (nullptr == coroutine_hdl->main_)
        {
            return -1;
        }
    }
    //如果已经是纤程了，得到当前纤程的标识
    else
    {
        coroutine_hdl->main_ = ::GetCurrentFiber();
        if (nullptr == coroutine_hdl->main_)
        {
            return -1;
        }
    }

    //使用这个结构完成函数适配
    struct _FIBERS_FUN_ADAPT* fibers_adapt = new _FIBERS_FUN_ADAPT();
    fibers_adapt->exit_back_main_ = exit_back_main;
    fibers_adapt->fun_ = std::bind(std::forward<Call>(fp), std::forward<Args>(args)...);

    //caution: flag FIBER_FLAG_FLOAT_SWITCH don't support at Windows XP and earlier versions
    coroutine_hdl->coroutine_ = ::CreateFiberEx(stack_size,
                                                stack_size,
                                                FIBER_FLAG_FLOAT_SWITCH,
                                                _FIBERS_FUN_ADAPT::adapt_fun,
                                                fibers_adapt);

    if (nullptr == coroutine_hdl->coroutine_)
    {
        return -1;
    }

    fibers_adapt->handle_ = coroutine_hdl;

    return 0;
#elif defined ZCE_OS_LINUX

    //必须先getcontext才能makecontext
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
    const int ONLY_1_ARG_COUNT = 1;
    if (exit_back_main)
    {
        coroutine_hdl->coroutine_.uc_link = &(coroutine_hdl->main_);
    }
    else
    {
        coroutine_hdl->coroutine_.uc_link = nullptr;
    }
    struct _FIBERS_FUN_ADAPT* fibers_adapt = new _FIBERS_FUN_ADAPT();
    fibers_adapt->exit_back_main_ = exit_back_main;
    fibers_adapt->fun_ = std::bind(std::forward<Call>(fp), std::forward<Args>(args)...);

    coroutine_hdl->coroutine_.uc_stack.ss_sp = new char[stack_size];
    coroutine_hdl->coroutine_.uc_stack.ss_size = stack_size;

    ::makecontext(&coroutine_hdl->coroutine_,
                  (void(*)(void)) _fibers_adapt_fun,
                  ONLY_1_ARG_COUNT,
                  func_obj);
    return 0;
#endif
}

/*!
* @brief      非标准函数，LINUX下的会分配对的空间
* @return     void
* @param      coroutine_hdl  要进行释放coroutine的句柄，
* @note       LINUX下释放了自动分配的ucct的空间，Windows下调用的是DeleteFiber
*/
void delete_coroutine(coroutine_t* coroutine_hdl);

/*!
* @brief      从Main切换到协程，
* @return     int 返回0标识成功
* @param      coroutine_hdl
*/
int resume_coroutine(coroutine_t* coroutine_hdl);

/*!
* @brief      从coroutine切换到Main
* @return     int 返回0标识成功
* @param      coroutine_hdl
*/
int yeild_coroutine(coroutine_t* coroutine_hdl);

/*!
* @brief      从一个coroutine切换到另外一个coroutine
* @return     int 返回0标识成功
* @param      save_hdl
* @param      goto_hdl
*/
int exchage_coroutine(coroutine_t* save_hdl,
                      coroutine_t* goto_hdl);
};
