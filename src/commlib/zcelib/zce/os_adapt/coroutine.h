/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年11月27日
* @brief      协程的OS适配层，
*
* @details    一个简单的协程的封装
*             最开始参考的文章是这个，
*             http://www.codeproject.com/Articles/4225/Unix-ucontext_t-Operations-on-Windows-Platforms
*             但发现其实他并不正确，
*             1.CONTEXT根据CPU结构所不同的，
*             2.Windows 下的API GetThreadContext,SetThreadContext ，在64位的环境
*               下是没法用的。后面Windows 增加了Wow64GetThreadContext ，
*             3.GetThreadContext对当前运行线程是无效的，因为当前线程是在运行的。
*             这点最讨厌，你就不能切换会主线程了，
*
*             第二次，我希望用Windows 的Fibers来模拟Linux下的getcontext等函数，但发现
*             其实Fibers和Context是有本质不同的，Fibers更像线程的对象（过程），Context
*             更像一个堆栈（点），比如
*             0.context API实现起来更像goto，Fibers的API更像是线程。注意：context的
*             swapcontext会给你错觉。但其实swapcontext是先保存当前的context到第一个参数，
*             1.Fibers是无法实现类似SwitchToFiber( GetCurrentFiber() );的调用，其
*               只能跳入另外一个Fibers，
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
*
* @note       关于Fibers函数的说明，清参考如下文档，作者写的非常清楚。
*             ConvertFiberToThread
*             http://www.cnblogs.com/wz19860913/archive/2008/08/26/1276816.html
*
*/

#ifndef ZCE_LIB_OS_ADAPT_CORROUTINE_H_
#define ZCE_LIB_OS_ADAPT_CORROUTINE_H_

#include "zce/os_adapt/common.h"

#if defined ZCE_OS_WINDOWS

///coroutine_t 内部保存协程句柄的数据，包括MAIN和COROUTINE的数据
struct  coroutine_t
{
    coroutine_t()
    {
        main_ = NULL;
        coroutine_ = NULL;
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

//为什么最后选择3个参数的函数作为支持的类型，大概是因为维基的例子，（我本来一直认为2个参数足够了）
typedef   void(*ZCE_COROUTINE_3PARA) (void* para1,
                                      void* para2,
                                      void* para3);

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
* @param      fun_ptr       函数指针，接受3个指针参数
* @param      para1         指针参数1
* @param      para2         指针参数2
* @param      para3         指针参数3
*/
int make_coroutine(coroutine_t* coroutine_hdl,
                   size_t stack_size,
                   bool back_main,
                   ZCE_COROUTINE_3PARA fun_ptr,
                   void* para1,
                   void* para2,
                   void* para3);

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

#endif //ZCE_LIB_OS_ADAPT_CORROUTINE_H_
