/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_mutex.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年8月13日
* @brief      跨平台互斥量部分的代码，接口基本模拟了pthread的函数，
* 
* @details    在Windows下我们分成几种模式,不用TIMEOUT的锁，递归锁，非递归锁
*             不用TIMEOUT的MUTEX，我们用临界区模拟实现的，（临界区是递归的呀）
*             递归锁，WINDOWS自己的MUTEX是递归的，
*             非递归锁，喔喔喔，这个是用信号灯模拟
*             这些东东都有完整的封装，在@ref zce_lock_thread_mutex.h
*             @ref zce_lock_process_mutex.h,
*             所以这儿的东东看看就行了，写代码可以必用。除非你是C爱好狂，
*             pthread_mutex内部我已经合入了非递归锁功能，用信号灯模拟
* 
* @note       pthread_mutex_XXX函数的返回值大都是返回0表示成功，否则是错误ID
*             这个和POSIX大部分函数不太一样，大部分其他函数是return -1;然后错误
*             保存在errno中，
*             我小小的揣测，大概是因为这可能因为是在多线程环境，返回值比保存在errno
*             反而容易处理。
*             这个地方，我向pthread_mutex_XXX函数靠齐，return的数值里面保存错误
*             ID，
*             请你注意返回值的处理，
*/

#ifndef ZCE_LIB_OS_ADAPT_MUTEX_H_
#define ZCE_LIB_OS_ADAPT_MUTEX_H_

#include "zce_os_adapt_predefine.h"

namespace ZCE_OS
{

//Pthread Mutex的属性部分操作……，有点裹脚布的感觉

/*!
* @brief      初始化MUTEX的属性参数
* @return     int  等于0标识成功，其实我实在看不出这种函数怎么可能失败，
* @param      attr 初始化的属性
*/
int pthread_mutexattr_init (pthread_mutexattr_t *attr);

/*!
* @brief      互斥量属性销毁
* @return     int  等于0标识成功
* @param      attr 销毁的属性
*/
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

/*!
* @brief      互斥量属性设置共享属性
* @return     int      等于0标识成功
* @param      attr     操作的属性
* @param      pshared  共享特效，PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE，如果想多进程间共享，要设置PTHREAD_PROCESS_SHARED
*/
int pthread_mutexattr_setpshared (pthread_mutexattr_t *attr,
                                  int pshared);
//
/*!
* @brief      取得线程的共享属性
* @return     int      等于0标识成功
* @param[in]  attr
* @param[out] pshared 返回共享属性
*/
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr,
                                 int *pshared);

/*!
* @brief      设置线程的属性
* @return     int    等于0标识成功
* @param      attr   设置的属性
* @param      type   PTHREAD_MUTEX_XXXX的几个值，可以用|一次设置多个属性
*/
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);

/*!
* @brief      取得线程的属性
* @return     int    等于0标识成功
* @param      attr   取的熟悉
* @param      type   返回的type
* @note
*/
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr,
                              int *type);

/*!
* @brief      pthread mutex 初始化
* @return     int    等于0表示成功,否则返回错误ID
* @param      mutex  MUTEX对象
* @param      attr   MUTEX的属性
*/
int pthread_mutex_init (pthread_mutex_t *mutex,
                        const pthread_mutexattr_t *attr);

#if defined (ZCE_OS_WINDOWS)
/*!
* @brief      初始化设置线程的属性，不同的平台给不同的默认定义，
* @return     int             等于0表示成功
* @param      mutex           PTHREAD MUTEX 对象
* @param      process_share   是否需要进程间共享，默认都FALSE
* @param      recursive       是否需要递归，默认WIN，LINUX都TURE
* @param      need_timeout    是否需要超时处理，WIN下默认为FALSE，因为这样可以用成本更低的临界区，
* @param      mutex_name      互斥量的名字，WIN下如果要多进程共享，就必须有个名字，如果要多进程共享，麻烦你老给个名字，否则没法搞
* @note       非标准，但是建议你使用，简单多了,
*/
int pthread_mutex_initex(pthread_mutex_t *mutex,
                         bool process_share = false,
                         bool recursive = true,
                         bool need_timeout = false,
                         const char *mutex_name = NULL);
#elif defined (ZCE_OS_LINUX)
int pthread_mutex_initex(pthread_mutex_t *mutex,
                         bool process_share = false,
                         bool recursive = true,
                         bool need_timeout = true,
                         const char *mutex_name = NULL);
#endif

/*!
* @brief      pthread mutex 销毁
* @return     int   等于0表示成功
* @param      mutex MUTEX对象
*/
int pthread_mutex_destroy (pthread_mutex_t *mutex);

/*!
* @brief      pthread mutex 锁定
* @return     int     等于0表示成功
* @param      mutex   MUTEX对象
*/
int pthread_mutex_lock (pthread_mutex_t *mutex);

/*!
* @brief      pthread mutex 超时锁定，
* @return     int              等于0表示成功，否则返回错误ID
* @param      mutex            MUTEX对象
* @param      abs_timeout_spec 超时的时间，为timespec类型，而且是绝对时间，
*/
int pthread_mutex_timedlock (pthread_mutex_t *mutex,
                             const ::timespec *abs_timeout_spec);

/*!
* @brief      pthread mutex 超时锁定，非标准实现,是用内部比较通用的时间变量timeval
* @return     int             等于0表示成功，否则返回错误ID
* @param      mutex           MUTEX对象
* @param      abs_timeout_val 超时的时间，timeval类型，绝对时间，
*                             如果希望这个值填写NULL，这个函数可能和上面的函数冲突，
*                             请这样操作 timeval*time_out =NULL,将这个time_out作为参数，
*/
int pthread_mutex_timedlock (pthread_mutex_t *mutex,
                             const timeval *abs_timeout_val);

/*!
* @brief      pthread mutex 尝试加锁
* @return     int
* @param      mutex
*/
int pthread_mutex_trylock (pthread_mutex_t *mutex);

/*!
* @brief      pthread mutex 解锁
* @return     int    等于0表示成功
* @param      mutex  MUTEX对象
*/
int pthread_mutex_unlock (pthread_mutex_t *mutex);

};

#endif //ZCE_LIB_OS_ADAPT_TIME_H_

