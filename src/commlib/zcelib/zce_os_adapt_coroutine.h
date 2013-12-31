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


typedef   void(*ZCE_COROUTINE_FUN_PTR) (void *left_para,void *right_para);


namespace ZCE_OS
{


/*!
* @brief      非标准函数，和LINUX下的makecontext类似，但第二个参数是堆栈大小，
*             函数指针标识的函数只允许使用2个参数，不能和LINUX一样使用变参
*             LINUX原生的makecontext 的ucct 使用前要初始化堆指针和长度，
*             我的实现内部会自动分配一个堆数据，作为ucct的堆栈，（所以你不用自己分配） 
*             所以必须调用deletecontext（WINDOWS下这个参数在Fibers有用）
*             这样做的主要原因是兼容Windows 的Fibers实现。
* @return     int 返回0标识成功，
* @param      ucct         ucontext_t，CONTEXT句柄，
* @param      slack_size   堆栈大小
* @param      fun_ptr      函数指针，函数只接受两个void *的指针。
* @param      left_para    左参数
* @param      right_para   右参数
* @note       
*/
int makecontext(ucontext_t *ucct,
    size_t slack_size, 
    ZCE_COROUTINE_FUN_PTR fun_ptr,
    void *left_para,
    void *right_para = NULL);


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

