/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_synch_traits.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年11月15日
* @brief      策略锁模式的类萃取 traits，用于模版代码中的typedef，
*             比如一个代码可能需要加锁和不加锁两种策略的时候，而且内部还有
*             互斥量，信号灯等多种同步对象时，
*             可以用ZCE_MT_SYNCH或者ZCE_NULL_SYNCH类型作为模版参数，模版通过
*             type::MUTEX这样的方式获得不同加锁策略以适配多线程，或者
*             单线程环境
*/

#ifndef ZCE_LIB_LOCK_SYNCH_TRAITS_H_
#define ZCE_LIB_LOCK_SYNCH_TRAITS_H_

//用于某些策略处理的类
#include "zce_lock_process_mutex.h"
#include "zce_lock_thread_mutex.h"
#include "zce_lock_thread_rw_mutex.h"
#include "zce_lock_thread_semaphore.h"
#include "zce_lock_thread_condi.h"
#include "zce_lock_null_lock.h"

/*!
* @brief      线程加锁环境的锁类型萃取器
*/
class ZCELIB_EXPORT ZCE_MT_SYNCH
{
public:
    typedef ZCE_Thread_Light_Mutex                        MUTEX;
    typedef ZCE_Null_Mutex                          NULL_MUTEX;
    typedef ZCE_Process_Mutex                       PROCESS_MUTEX;
    typedef ZCE_Thread_Recursive_Mutex              RECURSIVE_MUTEX;
    typedef ZCE_Thread_RW_Mutex                     RW_MUTEX;
    typedef ZCE_Thread_Condition_Mutex              CONDITION;
    typedef ZCE_Thread_Condition_Recursive_Mutex    RECURSIVE_CONDITION;
    typedef ZCE_Thread_Semaphore                    SEMAPHORE;
    typedef ZCE_Null_Mutex                          NULL_SEMAPHORE;
};

/*!
* @brief      ZCE_NULL_SYNCH 不加锁环境的锁类型萃取器
*/
class ZCELIB_EXPORT ZCE_NULL_SYNCH
{
public:
    typedef ZCE_Null_Mutex               MUTEX;
    typedef ZCE_Null_Mutex               NULL_MUTEX;
    typedef ZCE_Null_Mutex               PROCESS_MUTEX;
    typedef ZCE_Null_Mutex               RECURSIVE_MUTEX;
    typedef ZCE_Null_Mutex               RW_MUTEX;
    typedef ZCE_Null_Condition           CONDITION;
    typedef ZCE_Null_Condition           RECURSIVE_CONDITION;
    typedef ZCE_Null_Mutex               SEMAPHORE;
    typedef ZCE_Null_Mutex               NULL_SEMAPHORE;
};

#endif //ZCE_LIB_LOCK_SYNCH_TRAITS_H_

