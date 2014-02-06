/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年11月27日
* @brief      协程的OS适配层，
* 
* @details    最开始参考的文章是这个，
*             http://www.codeproject.com/Articles/4225/Unix-ucontext_t-Operations-on-Windows-Platforms
*             但发现其实他并不正确，
*             1.CONTEXT是更加CPU结构所不同的，
*             2.Windows 下的API GetThreadContext,SetThreadContext ，在64位的环境下是没法用的。
*             3.GetThreadContext对当前线程是无效的，
*             所以，
* 
*             所以只能用Windows 的Fibers来模拟Linux下的getcontext等函数，但这些函数接口是存在
*             无法完全融合的差异的，所以只能改变了一些接口的特质来适配，和原生解耦有不一致的地方，
*             关于Fibers函数的说明，清参考如下文档，作者写的非常清楚。
*             http://www.cnblogs.com/wz19860913/archive/2008/08/26/1276816.html
* @note       ConvertFiberToThread
* 
* 
* 
*/

#ifndef ZCE_LIB_OS_ADAPT_CORROUTINE_H_
#define ZCE_LIB_OS_ADAPT_CORROUTINE_H_

#include "zce_os_adapt_predefine.h"

#if defined ZCE_OS_WINDOWS

typedef  void * ucontext_t;

#endif 

//为什么最后选择3个参数的函数作为支持的类型，大概是因为维基的例子，（我本来一直认为2个参数足够了）
typedef   void(*ZCE_COROUTINE_3PARA) (void *para1,
    void *para2,
    void *para3);


namespace ZCE_OS
{


/*!
* @brief      非标准函数，和LINUX下的makecontext类似，但区别也不少，这样做的主要原
*             因是兼容Windows 的Fibers实现。区别如下：
*             1.函数指针标识的函数只允许使用2个参数，不能和LINUX一样使用变参
*             2.LINUX原生的::makecontext的ucontext_t参数要先用::getcontext，
*             获取的，  但使用此函数不用，内部集成了，
*             3.第二个参数是堆栈大小，我会根据你的指定分配堆空间，作为你的COROUTINE
*             的栈，而LINUX原生的makecontext 的ucontext_t参数使用前要初始化堆指
*             针和长度，
*             4.结束后，必须调用deletecontext清理ucontext_t，deletecontext在
*             WINDOWS下调用DeleteFiber函数，在Linux负责清理ucontext_t里面的栈
*             指针（从堆上分配的空间）
*             5.LINUX原生的::makecontext没有返回值，而此封装有，因为Windows下我
*             们用了CreateFiberEx，Linux下我们内部调用了::getcontext，这些函数
*             都可能失败
*             6.LINUX的::makecontext可以使用变参，这个函数没有考虑参数，当然这个变
*             也是有风险，但其实使用参数是有移植风险的，相见下面两个文档的说明，
*             http://en.wikipedia.org/wiki/Setcontext   
*             http://pubs.opengroup.org/onlinepubs/009695399/functions/makecontext.html
*             所以如果考虑兼容性，还是使用前面那个函数比较好
*             另外，为了包装参数传递，在Windows下，这个函数会new一个结构
* @return     int 返回0标识成功，
* @param      uctt         ucontext_t，生成的CONTEXT句柄， 
* @param      stack_size   栈大小
* @param      back_uc      ucontext_t，函数返回，回到的ucontext_t
* @param      fun_ptr      函数指针，接受3个指针参数
* @param      para1        指针参数1
* @param      para2        指针参数2
* @param      para3        指针参数3
* @note       
*/
    int makecontext(ucontext_t *uctt,
    size_t stack_size,
    ucontext_t *back_uc,
    ZCE_COROUTINE_3PARA fun_ptr,
    void *para1,
    void *para2,
    void *para3);


/*!
* @brief      非标准函数，LINUX下的会分配对的空间
* @return     void
* @param      ucct  要进行释放的CONTEXT句柄，
* @note       LINUX下释放了自动分配的ucct的空间，Windows下调用的是DeleteFiber
*/
void deletecontext(ucontext_t *ucct);


/*!
* @brief      取得当前栈的的CONTEXT,
* @return     int 返回0标识成功
* @param      ucct CONTEXT句柄，保存获得的CONTEXT
* @note       WINDOWS下集成了，ConvertThreadToFiberEx和GetCurrentFiber
*/
int getcontext(ucontext_t *ucct);

/*!
* @brief      设置当前栈的的CONTEXT,
* @return     int   返回0标识成功
* @param      ucct  CONTEXT句柄，

*/
int setcontext(const ucontext_t *ucct);

/*!
* @brief      交互CONTEXT
* @return     int 返回0标识成功
* @param      get_uctt 取得的CONTEXT
* @param      set_uctt 设置的CONTEXT
* @note       
*/
int swapcontext(ucontext_t *get_uctt, const ucontext_t *set_uctt);

};

#endif //ZCE_LIB_OS_ADAPT_CORROUTINE_H_

