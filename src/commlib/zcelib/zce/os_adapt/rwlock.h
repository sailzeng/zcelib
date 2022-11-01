/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/rwlock.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年4月19日
* @brief      读写锁（又名共享-独占锁）的模拟，和OS适配，WIN下使用条件变量模拟，
*             写在最开始的话，请在WIN SVR 2008（VISTA）以后的系统上使用，因为
*             模拟的东东性能其实远比不上临界区，而MS自己实现的读写锁性能其实挺好的，
*             WIN2008以后，有一个线程级别读写锁被MS自己实现了，但他的UNLOCK函数
*             是分成两个的，写锁解开，读取锁解开这也给兼容带来了一些麻烦，
*             但其他版本如果要使用只有模拟了，
*             在WINDOWS下的模拟只支持线程版本，不打算考虑多进程，
*
*             我自己的测试，WIN下，模拟的RW速度大约比真实的RW LOCK慢9倍。所以
*             真心不推荐用模拟的，（2读2写，这个测试数据和MS的测试结果类似）
*
* @details    大量的代码是从W.Richard stevens的UNP V2代码上改写的，
*             很有意思的发现这些，代码的参考的还是ACE的论文，
*             为了在WINDOWS上更加明白，我用了简化的EVENT模拟的条件变量
*
*             如果使用Windows 2008 VISTA 之后才有的那个条件变量，有一个问题要解决，
*             因为Windows的API设计的和LINUX PTHREAD的设计不一样，WINDOWS的API
*             提供了2个解锁函数，对于共享锁和排他锁分别一个，而PTHREAD值提供了一个，
*             我面临N个选择，
*             1.在unlock函数里面，把共享锁和排他锁的解锁都调用一次，祈祷Windows不会
*             因为我的多次（错误）调用，而导致什么麻烦。
*             2.提供两个解锁函数的封装，BOOST里面就是这个思路，
*             3.自己用TLS变量存下来，其到底用的是什么操作，这样后面调用的解锁函数的时候
*             可以有的放矢，
*
*/

#ifndef ZCE_LIB_OS_ADAPT_RWLOCK_H_
#define ZCE_LIB_OS_ADAPT_RWLOCK_H_

#include "zce/os_adapt/common.h"
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
* @brief      读写锁的对象的的初始化，非标准，但建议，使用
* @return     int               返回0标识成功，
* @param      rwlock            读写锁对象
* @param      use_win_slim      是否使用WIN2008以后拥有的Slim RWlock，注意
*                               Slim RWlock没有超时处理，当然前提你的系统必现支持，
* @param      priority_to_write 写入优先还是读取优先，（只对Windows的模拟有用，
*                               完全是为了自己好玩,不会快什么，MS一个类似测试）
*/
int pthread_rwlock_initex(pthread_rwlock_t* rwlock,
                          bool  priority_to_write = true);

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
