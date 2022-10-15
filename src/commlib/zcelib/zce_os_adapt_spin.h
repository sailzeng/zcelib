/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_spin.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012年9月6日
* @brief      pthread spin函数的封装，
*             SPIN LOCK，自旋锁，避免CPU切换的同步对象，
*             我的实现，在WINDOWS下用临界区（进程内）+SPIN 的实现 或者Mutex（进程间）模
*             拟SPIN lock， 当然后者其实没有真正实现SPIN LOCK的效果，
*
* @details    WINDOWS下和LINUX下其实都实现了Spin lock，但WINDOWS下，有两种SPIN LOCK
*             真正的Spin lock只能在驱动层使用，用户层其实无法使用，
*             但还有一种临界区使用SPIN锁机制，（临界区我的最爱………………）
*             而WINDOWS的实现比较有意思，其允许你自己定义一个SPIN Count，在没有达到这个值
*             时，锁自加，不释放CPU，一旦到达这个值，还没有获得临界区，那么就等待，
*             如果只有一个核心的CPU，那么他退化为我们常见的临界区。
*
*             LINUX下，如果冲撞较低，SPIN也好过MUTEX，(某种程度上你可以认为SPIN是LINUX的临界区)
*
*             spin毕竟讲的人少，还是放几个文章出来，大家共赏一下，
*             看雪的文章
*             http://bbs.pediy.com/showthread.php?t=74502
*
*             LINUX SPIN的性能测试
*             http://www.parallellabs.com/2010/01/31/pthreads-programming-spin-lock-vs-mutex-performance-analysis/
*
*             最后还是说一下改进的历史，本来写SPIN纯属好玩，特别是我一直认为WINDOWS 在用户态上
*             没有SPIN可言，后来发现了有这两个东东，也看了一下MSDN，完善了这个封装，（也就加了2
*             行代码，呵呵）
*
* @note       SPIN的运用场景应该是就是类似临界区的地方，同步保护一段代码，一些数据
*             SPIN的运用场景应该是冲撞少，不需要长时间等待的地方，（也就是为啥SPIN不提供超时）
*
*             关于Spin lock，我们能说他是一个好东东，但并不见得是所有人都合适使用的东东。
*             可以说，除非你知道你在做什么，为什么要用spin lock，否则还是用Mutex，临界区吧。
*             举个例子，比如我们要等待某个多线程的消息队列里面有数据再取，用Mutex比Spin合适很多。
*
*/
#ifndef ZCE_LIB_OS_ADAPT_SPIN_H_
#define ZCE_LIB_OS_ADAPT_SPIN_H_

#include "zce_os_adapt_predefine.h"

namespace zce
{

/*!
* @brief      SPIN 锁的初始化
* @return     int        0标识成功，非0标识失败以及错误原因
* @param[in]  lock       SPIN对象
* @param[in]  pshared    共享的访问，是多线程共享PTHREAD_PROCESS_PRIVATE，还是多进程共享
*/
int pthread_spin_init(pthread_spinlock_t *lock, int pshared);

/*!
* @brief      SPIN 锁的初始化扩展版本，非标准函数，但在WIN下多进程间使用，使用这个才行，（必须有名字）
* @return     int           0表示成功，非0表示失败以及错误原因
* @param[in]  lock          SPIN对象
* @param[in]  process_share 是否需要多进程间共享
* @param[in]  spin_name     在WIN下，如果要多进程间共享，要名字
*/
int pthread_spin_initex(pthread_spinlock_t *lock,
                        bool process_share = false,
                        const char *spin_name = NULL);

/*!
* @brief      SPIN 锁的销毁
* @return     int    0表示成功，非0表示失败以及错误原因
* @param[in]  lock   SPIN对象
*/
int pthread_spin_destroy(pthread_spinlock_t *lock);

/*!
* @brief      SPIN 锁的加锁
* @return     int   0表示成功，非0表示失败以及错误原因
* @param[in]  lock  SPIN对象
*/
int pthread_spin_lock(pthread_spinlock_t *lock);

/*!
* @brief      SPIN 锁的尝试加锁
* @return     int   0表示成功，非0表示失败以及错误原因
* @param[in]  lock  SPIN对象
*/
int pthread_spin_trylock(pthread_spinlock_t *lock);

/*!
* @brief      SPIN 锁的解锁
* @return     int   0表示成功，非0表示失败以及错误原因
* @param[in]  lock  SPIN对象
*/
int pthread_spin_unlock(pthread_spinlock_t *lock);

};

#endif //ZCE_LIB_OS_ADAPT_SPIN_H_

