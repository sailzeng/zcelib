/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/thread_semaphore.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年1月14日
* @brief      线程的信号灯封装
*/

#ifndef ZCE_LIB_LOCK_THREAD_SEMAPHORE_H_
#define ZCE_LIB_LOCK_THREAD_SEMAPHORE_H_

#include "zce/lock/lock_base.h"
#include "zce/lock/lock_guard.h"
#include "zce/time/time_value.h"

/*!
@brief      线程的信号灯封装，使用的是无名的信号灯，
*/
class ZCE_Thread_Semaphore  : public ZCE_Lock_Base
{

public:
    //线程锁的GUARD
    typedef ZCE_Lock_Guard<ZCE_Thread_Semaphore> LOCK_GUARD;

public:

    //构造函数,默认创建匿名信号灯，线程下一般用匿名信号灯就足够了,sem_name一般用NULL就足够了
    //匿名信号灯==无名信号灯
    ZCE_Thread_Semaphore (unsigned int init_value);
    virtual ~ZCE_Thread_Semaphore (void);

    //锁定
    virtual void lock();

    //尝试锁定
    virtual bool try_lock();

    //解锁,
    virtual void unlock();

    //绝对时间超时的的锁定，超时后解锁
    virtual bool systime_lock(const zce::Time_Value &abs_time);

    //相对时间的超时锁定，超时后，解锁
    virtual bool duration_lock(const zce::Time_Value &relative_time);

protected:
    //线程锁
    sem_t            *lock_;

};

#endif //ZCE_LIB_LOCK_THREAD_SEMAPHORE_H_

