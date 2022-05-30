/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/null_lock.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年9月13日
* @brief      为了纪念我用ACE这么多年，我还是倾向用ACE的一些方式解决问题。
*             虽然我也发现我还是会学习一些BOOST的特此
*             ACE有专门的一篇文章是讲模板策略锁模式的，
*             我也对比了过模板策略锁和多态策略锁的优缺点，我觉得各有优劣，
*             但如果希望不过过度模板化，还是多态好一点。
*
*             请参考
*             http://www.cnblogs.com/fullsail/archive/2012/07/31/2616106.html
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_LOCK_NULL_LOCK_H_
#define ZCE_LIB_LOCK_NULL_LOCK_H_

#include "zce/lock/lock_guard.h"
#include "zce/lock/lock_base.h"
#include "zce/util/non_copyable.h"

class zce::Time_Value;

namespace zce
{
/*!
* @brief      空锁，也是一种模式，用于某些情况灵活的使用是否加锁的方式,
*
*/
class Null_Mutex : public zce::Lock_Base
{
public:
    //NULL锁的GUARD
    typedef Lock_Guard<zce::Null_Mutex>      LOCK_GUARD;

    typedef Read_Guard<zce::Null_Mutex>      LOCK_READ_GUARD;

    typedef Write_Guard<zce::Null_Mutex>     LOCK_WRITE_GUARD;

public:
    ///构造函数
    Null_Mutex(const char* ptr = NULL);
    ///析构函数
    virtual ~Null_Mutex(void);

public:
    ///锁定
    virtual void lock();

    ///尝试锁定
    virtual bool try_lock();

    ///解锁,
    virtual void unlock();

    ///绝对时间超时的的锁定，超时后解锁，返回是否超时
    virtual bool lock(const zce::Time_Value& /*abs_time*/);
    ///相对时间
    virtual bool lock_for(const zce::Time_Value& /*relative_time*/);

    //相对与BOOST的shared的共享-独占锁的叫法，我还是倾向读写锁

    ///读取锁
    virtual void lock_read();
    ///尝试读取锁
    virtual bool try_lock_read();

    ///绝对时间超时的读取锁，
    virtual bool timed_lock_read(const zce::Time_Value& /*abs_time*/);
    ///相对时间超时的读取锁，
    virtual bool duration_lock_read(const zce::Time_Value& /*relative_time*/);

    ///写锁定
    virtual void lock_write();
    ///尝试读取锁
    virtual bool try_lock_write();
    ///写锁定超时
    virtual bool timed_lock_write(const zce::Time_Value& /*abs_time*/);
    //相对时间
    virtual bool duration_lock_write(const zce::Time_Value& /*relative_time*/);

protected:
    // A dummy lock.
    int    lock_;
};

/*!
* @brief      空锁，也是一种模式，用于某些情况灵活的使用是否加锁的方式,
*             整体的接口类似于BOOST的接口，比如不控制返回值，也参考过一些ACE
*/
class Null_Condition : public Condition_Base
{
public:
    //
    Null_Condition();
    virtual ~Null_Condition();

private:

    ///等待
    virtual void wait(zce::Null_Mutex* /*external_mutex*/);

    ///绝对时间超时的的等待，超时后解锁
    virtual bool systime_wait(zce::Null_Mutex* /*external_mutex*/, const zce::Time_Value& /*abs_time*/);

    ///相对时间的超时锁定等待，超时后，解锁
    virtual bool duration_wait(zce::Null_Mutex* /*external_mutex*/, const zce::Time_Value& /*relative_time*/);

    /// 给一个等待线程发送信号 Signal one waiting thread.
    virtual void signal(void);

    ///给所有的等待线程广播信号 Signal *all* waiting threads.
    virtual void broadcast(void);

protected:
    // A dummy lock.
    int    lock_;
};
}

#endif //ZCE_LIB_LOCK_NULL_LOCK_H_
