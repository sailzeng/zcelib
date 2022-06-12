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
    typedef Thread_Light_Mutex                  MUTEX;
    typedef Null_Lock                           NULL_MUTEX;
    typedef Process_Mutex                       PROCESS_MUTEX;
    typedef Thread_Recursive_Mutex              RECURSIVE_MUTEX;
    typedef Thread_RW_Mutex                     RW_MUTEX;
    typedef Thread_Condition                    CONDITION;
    typedef Thread_Recursive_Condition          RECURSIVE_CONDITION;
    typedef Thread_Semaphore                    SEMAPHORE;
    typedef Null_Lock                           NULL_SEMAPHORE;
};

/*!
* @brief      ZCE_NULL_SYNCH 不加锁环境的锁类型萃取器
*/
class ZCE_NULL_SYNCH
{
public:
    typedef Null_Lock               MUTEX;
    typedef Null_Lock               NULL_MUTEX;
    typedef Null_Lock               PROCESS_MUTEX;
    typedef Null_Lock               RECURSIVE_MUTEX;
    typedef Null_Lock               RW_MUTEX;
    typedef Null_Condition          CONDITION;
    typedef Null_Condition          RECURSIVE_CONDITION;
    typedef Null_Lock               SEMAPHORE;
    typedef Null_Lock               NULL_SEMAPHORE;
};
}
