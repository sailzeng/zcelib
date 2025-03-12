/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/rwlock.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年4月19日
* @brief      [尽量不要使用这个代码了，C++17 已经有了shared_mutex]
*             [尽量不要使用这个代码了，C++17 已经有了shared_mutex]
*             [尽量不要使用这个代码了，C++17 已经有了shared_mutex]
*
*             使用的是Windows 2008以后的Slim RWlock 模拟的，不再兼容旧有的版本。
*             更早版本的模拟代码，已经移除到lock/win_simulate.h中，
*             在WINDOWS下的模拟只支持线程版本，不打算考虑多进程，
*
*             我自己的测试，WIN下，模拟的RW速度大约比真实的RW LOCK慢9倍。所以
*             真心不推荐用模拟的，（2读2写，这个测试数据和MS的测试结果类似）
*
* @details
*
*/

#ifndef ZCE_LIB_OS_ADAPT_RWLOCK_H_
#define ZCE_LIB_OS_ADAPT_RWLOCK_H_

#include "zce/os_adapt/define.h"
#include "zce/os_adapt/mutex.h"
#include "zce/os_adapt/condi.h"

namespace zce
{
/*!
* @brief      读写锁的对象的初始化
* @return     int
* @param      rwlock
* @param      attr
* @note
*/
int pthread_rwlock_init(pthread_rwlock_t* rwlock,
                        const pthread_rwlockattr_t* attr);

/*!
* @brief      销毁读写锁的对象
* @return     int
* @param      rwlock
*/
int pthread_rwlock_destroy(pthread_rwlock_t* rwlock);

/*!
* @brief      获取读取锁，如果获取不到进行等待
* @return     int     0成功，，返回值表示错误ID
* @param      rwlock  读写锁对象
*/
int pthread_rwlock_rdlock(pthread_rwlock_t* rwlock);

/*!
* @brief      尝试获取读锁，如果获取不到，则返回错误，不阻塞
* @return     int     0成功，否则失败
* @param      rwlock  读写锁对象
* @note
*/
int pthread_rwlock_tryrdlock(pthread_rwlock_t* rwlock);

/*!
* @brief      获取读取锁的，同时等待直至超时
* @return     int              0成功，否则失败，返回值表示错误ID
* @param      rwlock           读写锁对象
* @param      abs_timeout_spec 等待的时间点（时间点）
* @note
*/
int pthread_rwlock_timedrdlock(pthread_rwlock_t* rwlock,
                               const ::timespec* abs_timeout_spec);

/*!
* @brief      非标准，读取锁的超时锁定，时间参数调整成timeval，
* @return     int              0成功，否则失败，返回值表示错误ID
* @param      rwlock           读写锁对象
* @param      abs_timeout_val  等待的时间点（时间点）
*/
int pthread_rwlock_timedrdlock(pthread_rwlock_t* rwlock,
                               const timeval* abs_timeout_val);

/*!
* @brief      获取写锁（独占锁），如果获取不到，会一直等待
* @return     int     0成功，否则失败，返回值表示错误ID
* @param      rwlock  读写锁对象
*/
int pthread_rwlock_wrlock(pthread_rwlock_t* rwlock);

/*!
* @brief      尝试获取写锁（独占锁），如果获取不到，返回错误，不阻塞
* @return     int     0成功，否则失败，返回值表示错误ID
* @param      rwlock  读写锁对象
*/
int pthread_rwlock_trywrlock(pthread_rwlock_t* rwlock);

/*!
* @brief      获取写锁（独占锁），如果获取不到，会等待一段时间
* @return     int         0成功，否则失败，返回值表示错误ID
* @param      rwlock           读写锁对象
* @param      abs_timeout_spec 超时时间点，绝对时间,timespec类型的，和标准一致
*/
int pthread_rwlock_timedwrlock(pthread_rwlock_t* rwlock,
                               const ::timespec* abs_timeout_spec);

/*!
@param      abs_timeout_val 超时时间点，绝对时间,timeval类型的,如果希望这个值填写nullptr，这个函数可能和上面的函数冲突，
* @brief      非标准，写取锁的超时锁定，时间参数调整成timeval，
* @return     int         0成功，否则失败
* @param      rwlock          读写锁对象
*                             请这样操作 timeval*time_out =nullptr,将这个time_out作为参数，
*/
int pthread_rwlock_timedwrlock(pthread_rwlock_t* rwlock,
                               const timeval* abs_timeout_val);

/*!
* @brief      解锁，对读锁和写锁都是一个函数，这个地方和WINDOWS SVR 20008以后实现的读写锁实现不一样
* @return     int     0成功，否则失败
* @param      rwlock  读写锁对象
*/
int pthread_rwlock_unlock(pthread_rwlock_t* rwlock);
};

#endif //ZCE_LIB_OS_ADAPT_RWLOCK_H_
