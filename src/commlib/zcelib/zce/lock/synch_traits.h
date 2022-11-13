/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/synch_traits.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年11月15日
* @brief      策略锁模式的类萃取 traits，用于模版代码中的typedef，
*             比如一个代码可能需要加锁和不加锁两种策略的时候，而且内部还有
*             互斥量，信号灯等多种同步对象时，
*             可以用ZCE_MT_SYNCH或者ZCE_NULL_SYNCH类型作为模版参数，模版通过
*             type::MUTEX这样的方式获得不同加锁策略以适配多线程，或者
*             单线程环境
*/

#pragma once

//用于某些策略处理的类
#include "zce/lock/process_mutex.h"
#include "zce/lock/thread_mutex.h"
#include "zce/lock/thread_rw_mutex.h"
#include "zce/lock/thread_semaphore.h"
#include "zce/lock/thread_condi.h"
#include "zce/lock/null_lock.h"

namespace zce
{
/*!
* @brief      线程加锁环境的锁类型萃取器
*/
class MT_SYNCH
{
public:
    typedef thread_mutex                  MUTEX;
    typedef null_lock                           NULL_MUTEX;
    typedef process_mutex                       PROCESS_MUTEX;
    typedef thread_recursive_mutex              RECURSIVE_MUTEX;
    typedef thread_rw_mutex                     RW_MUTEX;
    typedef thread_condition                    CONDITION;
    typedef thread_recursive_condition          RECURSIVE_CONDITION;
    typedef thread_semaphore                    SEMAPHORE;
    typedef null_lock                           NULL_SEMAPHORE;
};

/*!
* @brief      ZCE_NULL_SYNCH 不加锁环境的锁类型萃取器
*/
class ZCE_NULL_SYNCH
{
public:
    typedef null_lock               MUTEX;
    typedef null_lock               NULL_MUTEX;
    typedef null_lock               PROCESS_MUTEX;
    typedef null_lock               RECURSIVE_MUTEX;
    typedef null_lock               RW_MUTEX;
    typedef null_condition          CONDITION;
    typedef null_condition          RECURSIVE_CONDITION;
    typedef null_lock               SEMAPHORE;
    typedef null_lock               NULL_SEMAPHORE;
};
}
