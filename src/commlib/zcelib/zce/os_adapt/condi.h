/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/condi.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年10月2日
* @brief      条件变量快平台的封装，在WINDOWS下是模拟的，而且只能在线程中使用，
*             推荐在在WIN SVR 2008和VISTA以后使用这个封装，这时，系统会用操作系统Windows新增的同
*             步原语Cond，否则是用信号灯，临界区模拟的封装，
*             对于模拟的实现，虽然我也测试过。但我实在不敢保证什么，拿来当学术研究可以，真用？还是算了把。
*             你老上信号灯把。
*
*             条件变量其实一个毕竟难以解释的互斥方式，我觉得真能明白的人不多，
*             如果你真懂了，请回答这样几个问题，
*             0.条件的判断是用if还是where？
*             1.你讲讲pthread_cond_wait的参数的外部锁传递进去的目的是什么？
*             2.你讲讲调用signal，broadcast的时候，外部的锁是否应该加上？
*             3.如果有一个线程broadcast了，部分线程激活，部分还在等待，另外一个线程broadcast，
*               这时这些等待的线程会被激活几次？
*
* @details    WINDOWS在VISTA，SERVER2008后也支持这个条件变量这个互斥方式了。
*             前面的版本只能用模拟的了，仍然选择了ACE的算法。实在没辙，PTHREAD WIN32的我看不懂，
*
*             所以在XP，NT4，WINSERVER2003的服务上，我们用临界区（互斥量）+信号灯，模拟
*             在VISTA以后，Visual studio 2008以后的环境（注意他的外部互斥量只能临界区只能用！），
*             我们直接用WINDOLWS的临界区，
*
*             别小瞧这个倒霉的condi模拟,我在有ACE,BOOST,PTHREAD WIN32的基础上参考下，仍然了折腾了3天
*             这还不算我自己用Event模拟失败的一个，我只能给这些设计者跪了，的确不容易，一个细节没有想到就可
*             能有问题，真不容易。
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
*             后面我自己搞了一个简化的用事件模拟的版本，（同时只支持一种激发方式，而且外部锁要用MUTEX）
*             结果在测试中惨遭失败，
*
*             以上答案分别是：
*             1.WHERE，因为可能发生虚假的唤醒
*             2.不是加锁，恰恰是解开，让其他人通行。
*             3.答案是看情况，系统的实现理论上如果没有处理共享的数据，是不用加的，而且理论上不加应该更快，
*             如果有共享数据，（建议参考steven 的UNP V2 或者POSIX）
*             系统默认实现指PTHRAD默认的实现和WINDOWS 2008的新同步原语。
*             而对于模拟实现，目前这里面的模拟实现实现和ACE一样，要求在signal，broadcast的时候，外部的锁
*             应该加上。但PTHREAD WIN32没有改出明确的说法，甚至说过行为未定义，所以还是加吧。
*
*             4.天知道。如果是系统实现，按边缘触发的道理，应该是还在等待的部分。
*             如果是模拟实现，由于要求外部锁要求加上，所以应该不会出现这种情况。
*
* @note       在WINDOWS平台，在WIN SERVER2008后，在多线程下，外部互斥量用临界区的情况下，使用条件变量，
*
*             ACE的模拟是否完美？我不确认，我至少知道ACE的实现是你调用broadcast和signal时，外部锁必须加上！！！
*             看LINUX手册，signal 和 broadcast 并不明确是否要加锁。
*
*             pthread_condxxx_xxxx的函数也是在返回值中记录错误ID的，处理时注意
*
*/

#pragma once

#include "zce/os_adapt/common.h"

namespace zce
{
//====================================================================================================

/*!
@brief      销毁条件变量属性
@return     int    等于0标识成功
@param      attr   销毁condi的属性变量
*/
int pthread_condattr_destroy(pthread_condattr_t* attr);

/*!
@brief      初始化条件变量属性
@return     int    等于0标识成功
@param      attr   初始化condi的属性变量
*/
int pthread_condattr_init(pthread_condattr_t* attr);

/*!
 @brief      销毁条件变量对象
 @return     int   等于0标识成功,
 @param      cond  条件变量对象
*/
int pthread_cond_destroy(pthread_cond_t* cond);

/*!
* @brief      初始化条件变量对象
* @return     int     等于0标识成功,
* @param      cond    条件变量对象
* @param      attr    条件变量属性
*/
int pthread_cond_init(pthread_cond_t* cond,
                      const pthread_condattr_t* attr);

/*!
* @brief      初始化条件变量对象，不同的平台给不同的默认定义
*             非标准，但是建议你使用，简单多了,如果要多进程共享，麻烦你老给个名字，否则没法搞
* @return     int            等于0标识成功
* @param      cond              CV对象，条件变量对象
* @param      win_mutex_or_sema 外部等待的锁，是否有句柄，如果是MUTEX，信号灯就有，如果是临界区就没有
*/
int pthread_cond_initex(pthread_cond_t* cond,
                        bool win_mutex_or_sema = false);

/*!
* @brief      条件变量等待，一致等待
* @return     int            等于0标识成功,-1标识失败
* @param      cond           条件变量对象
* @param      external_mutex 外部的MUTEX对象，进入wait前应该是锁定的
*/
int pthread_cond_wait(pthread_cond_t* cond,
                      pthread_mutex_t* external_mutex);

/*!
* @brief      条件变量等待一段时间，超时后继续
* @return     int               等于0标识成功,-1标识失败
* @param      cond              条件变量对象
* @param      external_mutex    外部的MUTEX对象
* @param      abs_timespec_out  超时的时间，绝对值时间，timespec类型
* @note
*/
int pthread_cond_timedwait(pthread_cond_t* cond,
                           pthread_mutex_t* external_mutex,
                           const ::timespec* abs_timespec_out);

/*!
* @brief      非标准函数，条件变量等待一段时间，超时后继续,时间变量用我内部统一的timeval
* @return     int               等于0标识成功,-1标识失败
* @param      cond              条件变量对象
* @param      external_mutex    外部的MUTEX对象
* @param      abs_timeval_out   超时的时间，绝对值时间，timeval类型
*/
int pthread_cond_timedwait(pthread_cond_t* cond,
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
int pthread_cond_broadcast(pthread_cond_t* cond);

/*!
* @brief      条件变量解锁发信号。
* @return     int  返回值为0表示成功
* @param      cond 条件变量对象
* @note       建议参考pthread_cond_broadcast说明段。
*/
int pthread_cond_signal(pthread_cond_t* cond);

//====================================================================================================
};
