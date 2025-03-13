/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/condi.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年10月2日
*/

#pragma once

#if defined (ZCE_OS_WINDOWS)

#include "zce/os_adapt/define.h"

namespace zce
{
//====================================================================================================
/*!
* @brief      这个代码只是在2010年以前，用于在WINDOWS下模拟条件变量的，现在已经不推荐使用了，
*             放在这儿直至为了怀念一下过往的青春。
*             条件变量模拟封装，在WINDOWS下是模拟的，而且只能在线程中使用，
*             在WIN SVR 2008和VISTA之前，Windows没有提供条件变量，采用的方法是用信号灯，临界区
*             模拟的封装。WINDOWS在VISTA，SERVER2008后也支持这个条件变量这个互斥方式了。
*             对于模拟的实现，虽然我也测试过。但我实在不敢保证什么，拿来当学术研究可以，真用？还是算
*             了把。你老上信号灯把。
*
*             另外，条件变量其实一个毕竟难以解释的互斥方式，我觉得真能明白的人并不多，
*             如果你真懂了，请回答这样几个问题，
*             0.条件的判断是用if还是where？
*             1.你讲讲pthread_cond_wait的参数的外部锁传递进去的目的是什么？
*             2.你讲讲调用signal，broadcast的时候，外部的锁是否应该加上？
*             3.如果有一个线程broadcast了，部分线程激活，部分还在等待，另外一个线程broadcast，
*               这时这些等待的线程会被激活几次？
*             以上答案分别是：
*             1.WHERE，因为可能发生虚假的唤醒
*             2.不是加锁，恰恰是解开，让其他人通行。
*             3.答案是看情况，系统的实现理论上如果没有处理共享的数据，是不用加的，而且理论上不加应该更快，
*             如果有共享数据，（建议参考steven 的UNP V2 或者POSIX）
*             系统默认实现指PTHRAD默认的实现和WINDOWS 2008的新同步原语。
*             而对于模拟实现，目前这里面的模拟实现实现和ACE一样，要求在signal，broadcast的时候，外部
*             的锁应该加上。但PTHREAD WIN32没有改出明确的说法，甚至说过行为未定义，所以还是加吧。
*
*             4.天知道。如果是系统实现，按边缘触发的道理，应该是还在等待的部分。
*             如果是模拟实现，由于要求外部锁要求加上，所以应该不会出现这种情况。
*
* @details    别小瞧这个倒霉的condi模拟,我在有ACE,BOOST,PTHREAD WIN32的基础上参考下，仍然了折腾了3天
*             这还不算我自己用Event模拟失败的一个，我只能给这些设计者跪了，的确不容易，一个细节没有想到就
*             可能有问题，真不容易。
*             按道理来说，用MUTEX和信号量模拟条件变量，不难实现。用一个互斥量，一个信号量，一个计数器。
*             但是在WINDOWS下，我发现问题要复杂不少，ACE和PTHREAD WIN32的实现都用了2个信号量，但他
*             们的实现又都有所不同。
*             为什么要用2个信号量呢，看实现，一个用于阻塞等待线程。一个用于广播过程中等待所有线程都被激活。
*             就是说，他们的实现都在等待所有的线程被激活，这个和LINUX的条件变量的行为不一样，但为什么他们
*             要这样做，在10多年后和chatGPT的交流，我才明白了。这是因为Windows 的 ReleaseSemaphore()
*             不能保证所有线程一次性被唤醒，而 WaitForMultipleObjects() 也不能完美支持广播唤醒，有数量
*             限制。所以ACE等的实现，用另外一个信号量来进行等待。
*
*             仍然选择了参考ACE的算法。实在没辙，PTHREAD WIN32的我看不懂，
*
*             好吧，还是讲讲历史把，其实这些实现其实都是参考ACE的鼻祖的论文，
*             http://www.cse.wustl.edu/~schmidt/win32-cv-1.html
*             作者Douglas C. Schmidt and Irfan Pyarali
*
*             PTHREAD WIN32，应该是参考过ACE的实现的，但他认为ACE的实现存在问题，他给过一个证明，
*             但那个证明在条件变量WHERE条件里面套用条件变量，算了，不口水了。
*             BOOST的设计是参考PTHREAD WIN32的一个较早一点的版本，大体大家的渊源如此。
*
*             最开始我看的就是ACE的版本，我在参考了ACE 代码数个小时无法证明后，因为ACE默认实现有
*             EVENT,我把目光投向了PTHREAD WIN32，其给出了一个伪代码供参考，以为伪代码是帮助理解
*             的好东东。我折腾了1天半看这段伪代码，发现他除了比ACE更加难以理解以外，没有什么变化，吐血，
*             BOOST的实现也是参考PTHREAD WIN32，
*             于是我又回头参考ACE,见鬼，ACE的代码至少在思路上没有什么偏激的地方，你能明白他为啥是这样的，
*             而PTHREAD的实现总计有3个变量绕来绕去,有一个信号灯很无厘头，当然BOOST的代码也好不到哪儿去，
*
*             ACE的实现：好吧，我基本大约已经明白ACE的实现，（我还是无法给出证明他是完美的）但我感觉他只能
*             同时支持一种唤醒方式，broadcast或者singal一种，如果两周同时支持，肯定会导致麻烦，
*             他的broadcast的实现是基于发出广播后，必须等待所有等待者收到广播，
*             ACE的broadcast函数也明确要求，调用时，外部锁是必须锁上的，
*             BTW:我到现在不能完全证明这个东东是完美的，有点小晕，比如有的地方是否要换成EVENT？有的地方
*             是否要用SignalObjectAndWait?
*
*             PTHREAD WIN32 和BOOST的实现，他们应该支持2种一起调用。但你如果希望解释他们的实现，这是一
*             件很难的事情，他利用3个变量记录进入wait，被激活退出wait，超时，取消退出wait的数量，（但他
*             算法对这些数量的改写，清理，可以让你吐血的）
*             其的广播实现是基于broadcast或者singal后，再进行一次singal或者broadcast时会先等待上一次
*             的等待者是否都已经被全部激活了。
*
* @note       ACE的模拟是否完美？我不确认，我至少知道ACE的实现是你调用broadcast和signal时，外部锁必须加上！！！
*             看LINUX手册，signal 和 broadcast 并不明确是否要加锁。
*
*             pthread_condxxx_xxxx的函数也是在返回值中记录错误ID的，处理时注意
*
*/
struct ws_cv_t
{
    ///
    int  outer_lock_type_ = PTHREAD_MUTEX_RECURSIVE;
    /// 等待者的数量
    int  waiters_ = 0;

    /// 保存进行的解锁操作是broadcast进行的还是signal进行
    bool was_broadcast_ = false;

    /// waiters 的计数的保护锁
    pthread_mutex_t      waiters_lock_;

    /// 信号灯，阻塞排队等待的线程直到 signaled.
    sem_t* block_sema_ = nullptr;

    ///完成广播后的通知，这个条件变量看起来多余
    ///因为Windows 的 ReleaseSemaphore()，不能保证所有线程一次性被唤醒
    ///而 WaitForMultipleObjects() 也不能完美支持广播唤醒，有数量限制。
    ///所以ACE等的实现，用另外一个信号量来进行等待所有通知完成。
    sem_t* finish_broadcast_ = nullptr;
};

/*!
 @brief      销毁条件变量对象
 @return     int   等于0标识成功,
 @param      cond  条件变量对象
*/
int ws_cond_destroy(ws_cv_t* cond);

/*!
* @brief      初始化条件变量对象
* @return     int     等于0标识成功,
* @param      cond    条件变量对象
* @param      attr    条件变量属性
*/
int ws_cond_init(ws_cv_t* cond);

/*!
* @brief      初始化条件变量对象，不同的平台给不同的默认定义
*             非标准，但是建议你使用，简单多了,如果要多进程共享，麻烦你老给个名字，否则没法搞
* @return     int            等于0标识成功
* @param      cond              CV对象，条件变量对象
* @param      win_mutex_or_sema 外部等待的锁，是否有句柄，如果是MUTEX，信号灯就有，如果是临界区就没有
*/
int ws_cond_initex(ws_cv_t* cond,
                   bool win_mutex_or_sema = false);

/*!
* @brief      条件变量等待，一致等待
* @return     int            等于0标识成功,-1标识失败
* @param      cond           条件变量对象
* @param      external_mutex 外部的MUTEX对象，进入wait前应该是锁定的
*/
int ws_cond_wait(ws_cv_t* cond,
                 pthread_mutex_t* external_mutex);

/*!
* @brief      条件变量等待一段时间，超时后继续
* @return     int               等于0标识成功,-1标识失败
* @param      cond              条件变量对象
* @param      external_mutex    外部的MUTEX对象
* @param      abs_timespec_out  超时的时间，绝对值时间，timespec类型
* @note
*/
int ws_cond_timedwait(ws_cv_t* cond,
                      pthread_mutex_t* external_mutex,
                      const ::timespec* abs_timespec_out);

/*!
* @brief      非标准函数，条件变量等待一段时间，超时后继续,时间变量用我内部统一的timeval
* @return     int               等于0标识成功,-1标识失败
* @param      cond              条件变量对象
* @param      external_mutex    外部的MUTEX对象
* @param      abs_timeval_out   超时的时间，绝对值时间，timeval类型
*/
int ws_cond_timedwait(ws_cv_t* cond,
                      pthread_mutex_t* external_mutex,
                      const timeval* abs_timeval_out);

/*!
* @brief      条件变量解锁广播
* @return     int
* @param      cond
* @note       调用前，外部的锁是否要加上?，(POSIX对这个问题表示含混，建议参考steven 的UNP V2 或者POSIX)
*             如果是WIN2008以下版本模拟实现，建议加上，否则会有问题，
*             如果是真实的条件变量，那么应该是可以不加外部锁，但如果修改的条件是一个共享数据，要加锁
*/
int ws_cond_broadcast(ws_cv_t* cond) noexcept;

/*!
* @brief      条件变量解锁发信号。
* @return     int  返回值为0表示成功
* @param      cond 条件变量对象
* @note       建议参考pthread_cond_broadcast说明段。
*/
int ws_cond_signal(ws_cv_t* cond) noexcept;

//====================================================================================================

/**
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

///读写锁的对象结构，利用互斥量，条件变量实现的读写锁
struct ws_rwlock_t
{
    ///模拟的
    ///是否是唤醒写入优先，（是就是写入优先，否则读取优先）这是一个问题，我把抉择权利给你
    bool            priority_to_write_ = true;

    ///保护这个结构在多线程中读写的互斥量，主要下面那些整数的修改
    pthread_mutex_t rw_mutex_;

    /// 读者等待的条件变量
    pthread_cond_t  rw_condreaders_;
    /// 写入等待的条件变量
    pthread_cond_t  rw_condwriters_;

    ///等待读的线程数量
    int             rw_nwaitreaders_ = 0;
    ///等待写的线程数量
    int             rw_nwaitwriters_ = 0;

    ///锁的持有状态，如果有一个写者持有锁-1 如果>0表示多少个读者持有这个锁
    int             rw_refcount_ = 0;
};

/*!
* @brief      读写锁的对象的初始化
* @return     int
* @param      rwlock
* @param      attr
* @param      priority_to_write 写入优先还是读取优先，（只对Windows的模拟有用，
*                               完全是为了自己好玩,不会快什么，MS一个类似测试）
* @note
*/
int ws_rwlock_init(ws_rwlock_t* rwlock,
                   bool priority_to_write);

/*!
* @brief      销毁读写锁的对象
* @return     int
* @param      rwlock
*/
int ws_rwlock_destroy(ws_rwlock_t* rwlock);

/*!
* @brief      获取读取锁，如果获取不到进行等待
* @return     int     0成功，，返回值表示错误ID
* @param      rwlock  读写锁对象
*/
int ws_rwlock_rdlock(ws_rwlock_t* rwlock);

/*!
* @brief      尝试获取读锁，如果获取不到，则返回错误，不阻塞
* @return     int     0成功，否则失败
* @param      rwlock  读写锁对象
* @note
*/
int ws_rwlock_tryrdlock(ws_rwlock_t* rwlock);

/*!
* @brief      获取读取锁的，同时等待直至超时
* @return     int              0成功，否则失败，返回值表示错误ID
* @param      rwlock           读写锁对象
* @param      abs_timeout_spec 等待的时间点（时间点）
* @note
*/
int ws_rwlock_timedrdlock(ws_rwlock_t* rwlock,
                          const ::timespec* abs_timeout_spec);

/*!
* @brief      非标准，读取锁的超时锁定，时间参数调整成timeval，
* @return     int              0成功，否则失败，返回值表示错误ID
* @param      rwlock           读写锁对象
* @param      abs_timeout_val  等待的时间点（时间点）
*/
int ws_rwlock_timedrdlock(ws_rwlock_t* rwlock,
                          const timeval* abs_timeout_val);

/*!
* @brief      获取写锁（独占锁），如果获取不到，会一直等待
* @return     int     0成功，否则失败，返回值表示错误ID
* @param      rwlock  读写锁对象
*/
int ws_rwlock_wrlock(ws_rwlock_t* rwlock);

/*!
* @brief      尝试获取写锁（独占锁），如果获取不到，返回错误，不阻塞
* @return     int     0成功，否则失败，返回值表示错误ID
* @param      rwlock  读写锁对象
*/
int ws_rwlock_trywrlock(ws_rwlock_t* rwlock);

/*!
* @brief      获取写锁（独占锁），如果获取不到，会等待一段时间
* @return     int         0成功，否则失败，返回值表示错误ID
* @param      rwlock           读写锁对象
* @param      abs_timeout_spec 超时时间点，绝对时间,timespec类型的，和标准一致
*/
int ws_rwlock_timedwrlock(ws_rwlock_t* rwlock,
                          const ::timespec* abs_timeout_spec);

/*!
@param      abs_timeout_val 超时时间点，绝对时间,timeval类型的,如果希望这个值填写nullptr，这个函数可能和上面的函数冲突，
* @brief      非标准，写取锁的超时锁定，时间参数调整成timeval，
* @return     int         0成功，否则失败
* @param      rwlock          读写锁对象
*                             请这样操作 timeval*time_out =nullptr,将这个time_out作为参数，
*/
int ws_rwlock_timedwrlock(ws_rwlock_t* rwlock,
                          const timeval* abs_timeout_val);

/*!
* @brief      解锁，对读锁和写锁都是一个函数，这个地方和WINDOWS SVR 20008以后实现的读写锁实现不一样
* @return     int     0成功，否则失败
* @param      rwlock  读写锁对象
*/
int ws_rwlock_unlock(ws_rwlock_t* rwlock);
};

#endif //#if defined (ZCE_OS_WINDOWS)
