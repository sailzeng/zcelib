/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/lock_base.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年9月13日
* @brief      所有锁类(记录锁除外)的基础类，让你也有多态使用（改变）加锁行为的方式
*             为了纪念我用ACE这么多年，我还是倾向用ACE的一些方式解决问题。
*             虽然我也发现我还是会学习一些BOOST的特性
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
#include "zce/util/non_copyable.h"
#include "zce/time/time_value.h"

/*!
* @brief      所有锁类(记录锁除外)的基础类，让你也有多态使用（改变）加锁行为的方式
*             整体的接口类似于BOOST的接口，比如不控制返回值，也参考过一些ACE的代码
* @detail     考虑了很久，最后还是放弃了偷懒，这个类和ZCE_Null_Mutex功能非常接近，表面看可以重用
*             但zce::Lock_Base内部的成员，都应该是private的，而ZCE_Null_Mutex的内部成员都应该是
*             public，这个矛盾是不可调和的
*/
namespace zce
{
class lock_base
{
protected:
    ///构造函数和析构函数，允许析构，不允许构造的写法
    lock_base() = default;
public:
    ///析构函数
    virtual ~lock_base(void) = default;

    ///允许Lock_Ptr_Guard使用一些函数
    friend class lock_ptr_guard<lock_base>;

    //为了避免其他人的使用，特此将这些函数隐藏起来
private:
    //!锁定
    virtual void lock() noexcept
    {
    };
    //!尝试锁定
    virtual bool try_lock() noexcept
    {
        return false;
    };
    //!解锁,
    virtual void unlock() noexcept
    {
    };
    //!尝试锁定，直至等待时长结束，若成功获得锁则为 true ，否则为 false
    virtual bool try_lock_for(const zce::time_value& /*duration*/) noexcept
    {
        return false;
    }
    //!尝试锁定， 直至绝对时间点,若成功获得锁则为 true ，否则为 false
    virtual bool try_lock_until(const zce::time_value& /*abs_time*/) noexcept
    {
        return false;
    }

    //!读取锁
    virtual void lock_shared() noexcept
    {
    }
    //!尝试读取锁
    virtual bool try_lock_shared() noexcept
    {
        return false;
    }
    //!绝对时间,获取读取（共享）锁的，等待至绝对时间超时@param  abs_time 绝对时间
    virtual bool try_lock_shared_until(const zce::time_value& /*abs_time*/) noexcept
    {
        return false;
    }
    //!相对时间,获取读取（共享）锁的，等待至相对时间超时 @param  relative_time 相对时间
    virtual bool try_lock_shared_for(const zce::time_value& /*relative_time*/) noexcept
    {
        return false;
    }
    //!解锁读取锁
    virtual void unlock_shared() noexcept
    {
    }
};

//======================================================
/*!
* @brief      Semaphore 信号灯的基类，信号灯的
*             扩展应该都是从这个基类扩展
*
*/
class semaphore_base
{
protected:
    ///构造函数,protected，允许析构，不允许构造的写法
    semaphore_base() = default;
public:
    ///析构函数，
    virtual ~semaphore_base() = default;

    virtual void acquire() noexcept
    {
    }
    virtual void release() noexcept
    {
    }
    virtual bool try_acquire() noexcept
    {
        return false;
    }
    virtual bool try_acquire_for(const zce::time_value& /*abs_time*/) noexcept
    {
        return false;
    }

    virtual bool try_acquire_until(const zce::time_value& /*abs_time*/) noexcept
    {
        return false;
    }
};

//======================================================
class thread_mutex;
class thread_recursive_mutex;
/*!
* @brief      CV,Condition Variable 条件变量的基类，条件变量的
*             扩展应该都是从这个基类扩展
*
*/
class condition_base
{
protected:
    ///构造函数,protected，允许析构，不允许构造的写法
    condition_base() = default;
public:
    ///析构函数，
    virtual ~condition_base() = default;

    //为了避免其他人的使用，特此将这些函数隐藏起来
private:

    ///等待,
    virtual void wait(thread_mutex* /*external_mutex*/) noexcept
    {
    }
    ///绝对时间超时的的等待，超时后解锁
    virtual bool wait_until(thread_mutex* /*external_mutex*/,
                            const zce::time_value& /*abs_time*/) noexcept
    {
        return false;
    }
    ///相对时间的超时锁定等待，超时后，解锁
    virtual bool wait_for(thread_mutex* /*external_mutex*/,
                          const zce::time_value& /*relative_time*/) noexcept
    {
        return false;
    }

    /// 给一个等待线程发送信号 Signal one waiting thread.
    virtual void notify_one(void) noexcept
    {
    }
    ///给所有的等待线程广播信号 Signal *all* waiting threads.
    virtual void notify_all(void) noexcept
    {
    }
};

class condition_rmutex_base
{
protected:
    ///构造函数,protected，允许析构，不允许构造的写法
    condition_rmutex_base() = default;
public:
    ///析构函数，
    virtual ~condition_rmutex_base() = default;

    //为了避免其他人的使用，特此将这些函数隐藏起来
private:

    virtual void wait(thread_recursive_mutex* /*external_mutex*/) noexcept
    {
        return;
    }

    //!绝对时间超时的的等待，超时后解锁
    virtual bool wait_until(thread_recursive_mutex* /*external_mutex*/,
                            const zce::time_value& /*abs_time*/) noexcept
    {
        return false;
    }

    //!相对时间的超时锁定等待，超时后，解锁
    virtual bool wait_for(thread_recursive_mutex* /*external_mutex*/,
                          const zce::time_value& /*relative_time*/) noexcept
    {
        return false;
    }

    /// 给一个等待线程发送信号 Signal one waiting thread.
    virtual void notify_one(void) noexcept
    {
    }
    ///给所有的等待线程广播信号 Signal *all* waiting threads.
    virtual void notify_all(void) noexcept
    {
    }
};
}
