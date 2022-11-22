/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/thread.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年9月16日
* @brief
*
* @details    目的是站在ACE巨人的肩膀上，得到一个更加简单的封装
*             ACE的线程是依据与SUN平台的基础上实现的，和pthread还真有区别。
*             另外部分函数我没有保留，因为为了兼容，而且很多代码在两个平台不同，
*             有些是为了REAL TIME系统准备的
*
* @note       注意WINDOWS下的内核其实使用HANDLE标识一个线程，而不是线程ID，
*
*/

#ifndef ZCE_LIB_OS_ADAPT_THREAD_H_
#define ZCE_LIB_OS_ADAPT_THREAD_H_

#include "zce/os_adapt/define.h"

//由于
namespace zce
{
//------------------------------------------------------------------------------------------------------

/*!
* @brief      初始化线程属性
* @return     int       0成功，-1失败
* @param[in]  attr
*/
int pthread_attr_init(pthread_attr_t* attr);

/*!
* @brief      销毁线程属性
* @return     int       0成功，-1失败
* @param[in]  attr
*/
int pthread_attr_destroy(pthread_attr_t* attr);

/*!
* @brief      非标准函数，设置线程属性变量属性，你可以设置，线程的的分离，JOIN属性，堆栈大小，线程的调度优先级
* @return     int                0成功，-1失败
* @param[out] attr
* @param[in]  detachstate  分离的属性 PTHREAD_CREATE_DETACHED PTHREAD_CREATE_JOINABLE
* @param[in]  stacksize    堆栈的大小
* @param[in]  policy       调度策略，SCHED_FIFO or SCHED_RR 不建议您使用这个值，（0是默认值）
* @param[in]  priority     线程优先级，不建议你用这个东东，（0是默认值）
*                          Linux，下 1-99，SCHED_FIFO 和 SCHED_RR 有优先级，看调度
*                          调整优先级要先设置，PTHREAD_EXPLICIT_SCHED
*                          Windows 下，-15 到，15
* @note       WINDOWS 和LINUX共有的属性并不多
*/
int pthread_attr_setex(pthread_attr_t* attr,
                       int detachstate = PTHREAD_CREATE_DETACHED,
                       size_t stacksize = 0,
                       int policy = 0,
                       int threadpriority = 0);

/*!
* @brief      非标准函数，获得线程几个属性
* @return     int           0成功，-1失败
* @param      attr
* @param      detachstate
* @param      stacksize
* @param      threadpriority
*/
int pthread_attr_getex(const pthread_attr_t* attr,
                       int* detachstate,
                       size_t* stacksize,
                       int* threadpriority);

/*!
* @brief      创建一个线程,调用线程函数向各个平台兼容模式靠齐，有返回值，但你在各个平台定义不同的回调函数
* @return     int           0成功，-1失败
* @param      threadid      线程ID，（在WINDOWS也是线程ID，不是线程句柄）
* @param      attr          线程属性
* @param      start_routine 运行的线程的回调函数指针,不要void *的返回值
* @param      arg           start_routine 回调函数的参数
* @note       推荐用上面的函数，下面这组提供出来主要是为了满足一些特殊爱好，但是这样写还是要在不同的平台写奇怪的代码，何必呢，
*/
int pthread_create(ZCE_THREAD_ID* threadid,
                   const pthread_attr_t* attr,
                   void *(*start_routine)(void*),
                   void* arg);

/*!
* @brief      创建线程，简单一点的封装，直接调用，不用处理pthread_attr_t
*             这个不是POSIX的封装，但推荐使用
* @return     int                0成功，-1失败
* @param[in]  start_routine 线程调用的启动，函数指针
* @param[in]  arg           start_routine函数 的参数
* @param[out] threadid      返回的线程ID
* @param[in]  detachstate   分离的属性 PTHREAD_CREATE_DETACHED PTHREAD_CREATE_JOINABLE
* @param[in]  stacksize     堆栈大小  =0 表示默认
* @param[in]  policy        调度策略 =0 表示默认
* @param[in]  priority      线程优先级 = 0 表示默认
*/
int pthread_createex(void* (*start_routine)(void*),
                     void* arg,
                     ZCE_THREAD_ID* threadid,
                     int detachstate = PTHREAD_CREATE_JOINABLE,
                     size_t stacksize = 0,
                     int policy = 0,
                     int priority = 0);

/*!
* @brief      退出线程，注意这儿没有任何参数让你作为返回值，
*             注意，WINDOWS和LINUX下返回值不同，所以我放弃不使用返回值参数，
*/
void pthread_exit(void *return_data);

/*!
* @brief      等待某个JOIN的线程结束,并且得到线程回调函数的返回值
* @return     int
* @param      threadid 等待退出的线程ID，
* @param      ret_val  线程的返回值，在LINUX和WINDOWS运行时，并不相同
*/
int pthread_join(ZCE_THREAD_ID threadid,
                 void* ret_val);

/*!
* @brief      得到当前线程ID
* @return     ZCE_THREAD_ID
*/
ZCE_THREAD_ID pthread_self(void);

/*!
* @brief      取消一个线程
* @return     int       0成功，-1失败
* @param      threadid  操作的线程ID
* @note       这个函数放在这儿，完全是为了满足我的一点点封装意愿，因为在LINUX平台，你未必能取消一个线程
*             在外WINDOWS，调用TerminateThread，这个函数也是不被推荐的
*/
int pthread_cancel(ZCE_THREAD_ID threadid);

/*!
* @brief      对一个线程进行松绑，不再需要join去等待退出
* @return     int        0成功，-1失败
* @param      threadid   操作的线程ID
*/
int pthread_detach(ZCE_THREAD_ID threadid);

#if defined ZCE_OS_WINDOWS
/*!
* @brief      将线程ID转换为HANDLE，只在WIN2000以后有用
* @return     ZCE_THREAD_HANDLE
* @param      threadid
* @note       内部调用的是OpenThread是一个WIN SERVER 2000后才有的函数 VC6应该没有
*/
ZCE_THREAD_HANDLE pthread_id2handle(ZCE_THREAD_ID threadid);
#endif //

int pthread_yield(void);

//------------------------------------------------------------------------------------------------------
//Thread Specific Data 。线程私有（专有）存储 TSS
//也叫 TLS Thread Local Storage

/*!
* @brief 创建TSS key
* @return     int
* @param      key 创建的key
* @param      (*destructor) 析构函数，此参数在Windows 下没有用处，
*            （如果跨平台）不建议使用
*/
int pthread_key_create(pthread_key_t* key,
                       void (*destructor)(void*));

/*!
* @brief 删除TSS key
* @return     int
* @param      key TSS key
*/
int pthread_key_delete(pthread_key_t key);

/*!
* @brief 设置该线程对应的关键字关联的私有数据空间
* @return     int
* @param      key TSS key
* @param      value
*/
int pthread_setspecific(pthread_key_t key,
                        const void* value);

/*!
* @brief 取得该线程对应的关键字关联的私有数据空间
* @return     void*
* @param      key TSS key
*/
void* pthread_getspecific(pthread_key_t key);
};

#endif //ZCE_LIB_OS_ADAPT_THREAD_H_
