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

#pragma once

#include "zce/lock/lock_guard.h"
#include "zce/lock/lock_base.h"
#include "zce/util/non_copyable.h"

namespace zce
{
class time_value;
/*!
* @brief      空锁，也是一种模式，用于某些情况灵活的使用是否加锁的方式,
*
*/
class null_lock : public zce::lock_base
{
public:
    //nullptr锁的GUARD
    typedef lock_guard<zce::null_lock>      LOCK_GUARD;
    typedef shared_guard<zce::null_lock>    LOCK_SHARED_GUARD;
    typedef unique_guard<zce::null_lock>    LOCK_UNIQUE_GUARD;

public:
    ///构造函数
    null_lock() = default;
    ///析构函数
    virtual ~null_lock(void) = default;

public:
    ///锁定
    void lock() noexcept override
    {
    }
    ///尝试锁定
    bool try_lock() noexcept override
    {
        return true;
    }
    ///解锁,
    void unlock() noexcept override
    {
    }
    ///尝试锁定，等待到超时时间点（绝对时间）后解锁，返回是否锁定
    bool try_lock_until(const zce::time_value&) noexcept override
    {
        return true;
    }
    ///相对时间
    bool try_lock_for(const zce::time_value&) noexcept override
    {
        return true;
    }
    //相对与BOOST的shared的共享-独占锁的叫法，我还是倾向读写锁

    ///读取锁
    void lock_shared() noexcept override
    {
    }
    ///解锁读取锁
    void unlock_shared() noexcept override
    {
    }
    ///尝试读取锁
    bool try_lock_shared() noexcept override
    {
        return true;
    }
    ///绝对时间超时的读取锁，
    bool try_lock_shared_until(
        const zce::time_value& /*abs_time*/) noexcept override
    {
        return true;
    }
    ///相对时间超时的读取锁，
    bool try_lock_shared_for(
        const zce::time_value& /*relative_time*/) noexcept override
    {
        return true;
    }
};

//!为了C++标准定义一个
typedef null_lock null_lock;

//=====================================================================
class null_semaphore : public semaphore_base
{
    ///信号灯的GUARD
    typedef zce::semaphore_guard<null_semaphore> LOCK_GUARD;

public:

    null_semaphore() = default;
    virtual ~null_semaphore(void) = default;

    //!
    void acquire() noexcept override
    {
    }
    //!
    bool try_acquire() noexcept override
    {
        return true;
    }
    //!
    void release() noexcept override
    {
    }
    //!
    bool try_acquire_until(
        const zce::time_value& /*abs_time*/) noexcept override
    {
        return true;
    }
    //!
    bool try_acquire_for(
        const zce::time_value& /*relative_time*/) noexcept override
    {
        return true;
    }
};
//=====================================================================
/*!
* @brief      空锁，也是一种模式，用于某些情况灵活的使用是否加锁的方式,
*             整体的接口类似于BOOST的接口，比如不控制返回值，也参考过一些ACE
*/
class null_condition : public condition_base
{
public:
    //
    null_condition() = default;
    virtual ~null_condition() = default;

private:

    //!等待
    void wait(thread_mutex* /*external_mutex*/) noexcept override
    {
    }
    //!绝对时间超时的的等待，超时后解锁
    bool wait_until(thread_mutex* /*external_mutex*/,
                    const zce::time_value& /*abs_time*/) noexcept override
    {
        return true;
    }
    //!相对时间的超时锁定等待，超时后，解锁
    bool wait_for(thread_mutex* /*external_mutex*/,
                  const zce::time_value& /*relative_time*/) noexcept override
    {
        return true;
    }
    //!给一个等待线程发送信号 Signal one waiting thread.
    void notify_one(void) noexcept override
    {
    }
    //!给所有的等待线程广播信号 Signal *all* waiting threads.
    void notify_all(void) noexcept override
    {
    }
};
}
