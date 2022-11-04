/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/guard.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年1月14日
* @brief      GURAD。利用构造函数和析构函数自动加锁，解锁的类，
*             在各个类里面都有typdef帮助使用
*             比如Thread_Light_Mutex
*
*             如果你还能被人噎着，就表示你还不够强大，
*
*/

#pragma once

namespace zce
{
/*!
* @brief      锁GUARD，利用构造和修改进行自动加锁，自动解锁操作方法
* @tparam     ZCE_LOCK 锁的类型，可以是Null_Mutex，Process_Mutex，
*             Thread_Light_Mutex，Thread_Recursive_Mutex
*             process_semaphore,thread_nonr_mutex,等
*/
template <typename zce_lock>
class lock_guard
{
public:

    ///构造，得到锁，进行锁定
    lock_guard(zce_lock& lock) :
        lock_(&lock)
    {
        lock_->lock();
    }

    //构造，得到锁，根据要求决定是否进行锁定操作
    lock_guard(zce_lock& lock, bool block) :
        lock_(&lock)
    {
        if (block)
        {
            lock_->lock();
        }
    }

    ///析构，同时对锁进行释放操作
    ~lock_guard(void)
    {
        lock_->unlock();
    };

    ///锁定操作
    void lock(void)
    {
        return lock_->lock();
    }

    lock_guard(const lock_guard&) = delete;
    lock_guard& operator=(const lock_guard&) = delete;

    ///尝试锁定操作
    bool try_lock(void)
    {
        return lock_->try_lock();
    }

    ///解锁操作
    void unlock(void)
    {
        return lock_->unlock();
    }

protected:

    ///用来GUARD保护的锁
    zce_lock* lock_;
};

/*!
* @brief      读写锁的共享锁（读取）锁定的GUARD，利用构造和修改进行自动加锁，自动解锁操作方法
* @tparam     ZCE_LOCK 锁的类型,可以是，ZCE_Null_Mutex, ZCE_File_Lock thread_rw_mutex
*/
template <class zce_lock>
class shared_Guard
{
public:
    ///构造，得到读写锁，进行读锁定
    shared_Guard(zce_lock& lock) :
        lock_(&lock)
    {
        lock_->lock_shared();
    }

    ///构造，得到读写锁，根据参数确定是否进行读锁定
    shared_Guard(zce_lock& lock, bool block) :
        lock_(&lock)
    {
        if (block)
        {
            lock_->lock_shared();
        }
    }

    ///析构，进行解锁操作
    ~shared_Guard()
    {
        lock_->unlock_shared();
    }

    shared_Guard(const shared_Guard&) = delete;
    shared_Guard& operator=(const shared_Guard&) = delete;

    ///读取锁
    void lock_shared()
    {
        return lock_->lock_shared();
    }
    ///尝试读取锁
    bool try_lock_read()
    {
        return lock_->try_lock_shared();
    }

    ///解锁,如果是读写锁也只需要这一个函数
    void unlock_shared()
    {
        return lock_->unlock_shared();
    }

protected:

    ///用来GUARD保护的锁
    zce_lock* lock_;
};

/*!
* @brief      读写锁中的写锁定的GUARD，利用构造和修改进行自动加锁，自动解锁操作方法
* @tparam     ZCE_LOCK 锁的类型,可以是 ZCE_Null_Mutex ZCE_File_Lock thread_rw_mutex
*/
template <class zce_lock>
class unique_guard
{
public:
    ///构造，得到读写锁，进行读锁定
    unique_guard(zce_lock& lock) :
        lock_(&lock)
    {
        lock_->lock_write();
    }

    ///构造，得到读写锁，根据参数确定是否进行读锁定
    unique_guard(zce_lock& lock, bool block) :
        lock_(&lock)
    {
        if (block)
        {
            lock_->lock_write();
        }
    }

    ///析构，进行解锁操作
    ~unique_guard()
    {
        lock_->unlock();
    }

    unique_guard(const unique_guard&) = delete;
    unique_guard& operator=(const unique_guard&) = delete;

    ///读取锁
    void lock()
    {
        return lock_->lock();
    }
    ///尝试读取锁
    bool try_lock()
    {
        return lock_->try_lock();
    }

    ///解锁,如果是读写锁也只需要这一个函数
    void unlock()
    {
        return lock_->unlock();
    }

protected:

    ///用来GUARD保护的锁
    zce_lock* lock_;
};

/*!
* @brief      读写锁的共享锁（读取）锁定的GUARD，利用构造和修改进行自动加锁，自动解锁操作方法
* @tparam     ZCE_LOCK 锁的类型,可以是，ZCE_Null_Mutex, ZCE_File_Lock thread_rw_mutex
*/
template <class zce_sema>
class semaphore_guard
{
public:
    ///构造，得到读写锁，进行读锁定
    semaphore_guard(zce_sema& lock) :
        lock_(&lock)
    {
        lock_->acquire();
    }

    ///构造，得到读写锁，根据参数确定是否进行读锁定
    semaphore_guard(zce_sema& lock, bool block) :
        lock_(&lock)
    {
        if (block)
        {
            lock_->acquire();
        }
    }

    ///析构，进行解锁操作
    ~semaphore_guard()
    {
        lock_->release();
    }

    semaphore_guard(const semaphore_guard&) = delete;
    semaphore_guard& operator=(const semaphore_guard&) = delete;

    ///读取锁
    void acquire()
    {
        return lock_->acquire();
    }
    ///解锁,如果是读写锁也只需要这一个函数
    void release()
    {
        return lock_->release();
    }
    ///尝试读取锁
    bool try_acquire()
    {
        return lock_->try_acquire();
    }

protected:

    ///用来GUARD保护的锁
    zce_sema* lock_;
};

/*!
* @brief      锁GUARD，利用构造和析构进行加锁解锁操作方法，利用多态兼容变化
*             多态的好处是，你可以动态的决定使用什么锁，而不是在编译的时候。
*
*/
template <class zce_lock>
class lock_ptr_guard
{
public:

    //构造，得到锁，进行锁定
    lock_ptr_guard(zce_lock* lock_ptr) :
        lock_ptr_(lock_ptr)
    {
        if (lock_ptr_)
        {
            lock_ptr_->lock();
        }
    }

    ///构造，得到锁，根据要求决定是否进行锁定操作
    lock_ptr_guard(zce_lock* lock_ptr, bool block) :
        lock_ptr_(lock_ptr)
    {
        if (block && lock_ptr_)
        {
            lock_ptr_->lock();
        }
    }

    ///析构，同时对锁进行释放操作
    ~lock_ptr_guard(void)
    {
        if (lock_ptr_)
        {
            lock_ptr_->unlock();
        }
    };

    lock_ptr_guard(const lock_ptr_guard&) = delete;
    lock_ptr_guard& operator=(const lock_ptr_guard&) = delete;

    ///锁定操作
    void lock(void)
    {
        if (lock_ptr_)
        {
            lock_ptr_->lock();
        }
    }

    ///尝试锁定操作
    bool try_lock(void)
    {
        if (lock_ptr_)
        {
            return lock_ptr_->try_lock();
        }
        else
        {
            return false;
        }
    }

    ///解锁操作
    void unlock(void)
    {
        if (lock_ptr_)
        {
            lock_ptr_->unlock();
        }
        return;
    }

protected:

    ///用来GUARD保护的锁,利用C++特性实现锁差异
    zce_lock* lock_ptr_;
};
}
