/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/condi.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年10月2日
* @brief      [尽量不要使用这个代码了，C++11 已经有了condi]
*             [尽量不要使用这个代码了，C++11 已经有了condi]
*             [尽量不要使用这个代码了，C++11 已经有了condi]
*
*             条件变量快平台的封装，在WINDOWS下是模拟的，而且只能在线程中使用，
*
* @details    直接使用WINDOWS在VISTA，SERVER2008后也支持这个条件变量这个互斥方式了。
*
*             前面的版本只能用模拟的了，目前这部分代码已经移除到/lock/win_simulate.h中，
*
*
* @note       pthread_condxxx_xxxx的函数也是在返回值中记录错误ID的，处理时注意
*
*/

#pragma once

#include "zce/os_adapt/define.h"

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
int pthread_cond_broadcast(pthread_cond_t* cond) noexcept;

/*!
* @brief      条件变量解锁发信号。
* @return     int  返回值为0表示成功
* @param      cond 条件变量对象
* @note       建议参考pthread_cond_broadcast说明段。
*/
int pthread_cond_signal(pthread_cond_t* cond) noexcept;

//====================================================================================================
};
