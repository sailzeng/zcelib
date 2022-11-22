/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/thread_rw_mutex.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年9月13日
* @brief      线程的读写锁，Thread_RW_Mutex
*             在WINDOWS下，内部是使用的适配层的代码，清参考 @ref zce/os_adapt/rwlock.h
*             在不支持WINSERVER 2008的环境用的是条件变量和MUTEX的模拟的作品，
*             在支持WINSERVER 2008的环境（包括服务器和编译），会使用WINDOWS自己的读写锁封装
*
* @detail     读写锁的解锁都是unlock，如果是自己模拟实现的化，分开读写的unlock当然
*             是有利于实现的，BOOST就是这样,
*             pthread_rwlock的函数是没有区分的，我的模拟也是遵循了pthread_rwlock
*             的接口
*
*             相对与BOOST的shared的共享锁的叫法，我还是倾向读写锁
*
*             有些鸟儿是关不住的。他们的羽毛太鲜亮了。当它们飞走的时候，你心底里知道把他们关起来
*             是一种罪恶，你会因此而振奋。不过，他们一走,你住的地方也就更加灰暗空虚。我觉得我真是
*             怀念我的朋友。——《肖申克的救赎》
*/

#include "zce/lock/lock_base.h"
#include "zce/lock/lock_guard.h"
#include "zce/time/time_value.h"

#pragma once

namespace zce
{
/*!
@brief      线程的读写锁

*/
class thread_rw_mutex
{
public:
    //!读锁的GUARD
    typedef zce::shared_guard<thread_rw_mutex>  LOCK_SHARED_GUARD;
    //!写锁的GUARD
    typedef zce::unique_guard<thread_rw_mutex> LOCK_UNIQUE_GUARD;

public:
    //!构造函数
    thread_rw_mutex();
    virtual ~thread_rw_mutex();

public:
    //!读取锁
    virtual void lock_shared() noexcept;
    //尝试读取锁
    virtual bool try_lock_shared() noexcept;
    //!绝对时间
    bool try_lock_shared_until(const zce::time_value& abs_time) noexcept;
    //!相对时间
    bool try_lock_shared_for(const zce::time_value& relative_time) noexcept;
    //!解读锁
    void unlock_shared() noexcept;

    //!写锁定
    void lock() noexcept;
    //!尝试读取锁
    bool try_lock() noexcept;
    //!写锁定超时，绝对时间
    bool try_lock_until(const zce::time_value& abs_time) noexcept;
    //!写锁定超时，相对时间
    bool try_lock_for(const zce::time_value& relative_time) noexcept;
    //!解写锁
    void unlock() noexcept;

    //!取出内部的锁的指针
    pthread_rwlock_t* get_lock();

protected:

    //!线程锁
    pthread_rwlock_t  rw_lock_;
};

#if defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008

/*!
@brief      轻量级的线程的读写锁，没有超时处理，
            主要是为了适配Windows SVR 2008以后的读写锁实现，
            如果环境允许，推荐使用这个，速度比模拟的估计快很多，
*/
class thread_win_rw_mutex
{
public:
    //!读锁的GUARD
    typedef zce::shared_guard<thread_win_rw_mutex>  LOCK_SHARED_GUARD;
    //!写锁的GUARD
    typedef zce::unique_guard<thread_win_rw_mutex> LOCK_UNIQUE_GUARD;

public:
    //构造函数
    thread_win_rw_mutex();
    virtual ~thread_win_rw_mutex();

public:
    //!读取锁
    void lock_shared() noexcept;
    //!尝试读取锁
    bool try_lock_shared() noexcept;
    //!解读锁
    void unlock_shared() noexcept;

    //!写锁定
    void lock() noexcept;
    //!尝试读取锁
    bool try_lock() noexcept;
    //!解写锁
    void unlock() noexcept;

    //!取出内部的锁的指针
    SRWLOCK* get_lock();

protected:

    //!WINSVR 2008以后，WINDOWS自己实现的读写锁
    SRWLOCK                rwlock_slim_;
};

#endif
}
