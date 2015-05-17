/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_guard.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年1月14日
* @brief      GURAD。利用构造函数和析构函数自动加锁，解锁的类，
*             在各个类里面都有typdef帮助使用
*             比如ZCE_Thread_Light_Mutex
*
*/

#ifndef ZCE_LIB_LOCK_GUARD_H_
#define ZCE_LIB_LOCK_GUARD_H_

#include "zce_boost_non_copyable.h"

/*!
* @brief      锁GUARD，利用构造和修改进行自动加锁，自动解锁操作方法
* @tparam     ZCE_LOCK 锁的类型，可以是ZCE_Null_Mutex，ZCE_Process_Mutex，
*             ZCE_Thread_Light_Mutex，ZCE_Thread_Recursive_Mutex
*             ZCE_Process_Semaphore,ZCE_Thread_NONR_Mutex,等
*/
template <typename ZCE_LOCK>
class ZCE_Lock_Guard : public ZCE_NON_Copyable
{
public:

    ///构造，得到锁，进行锁定
    ZCE_Lock_Guard (ZCE_LOCK &lock):
        lock_(&lock)
    {
        lock_->lock();
    }

    //构造，得到锁，根据要求决定是否进行锁定操作
    ZCE_Lock_Guard (ZCE_LOCK &lock, bool block):
        lock_(&lock)
    {
        if (block)
        {
            lock_->lock();
        }
    }

    ///析构，同时对锁进行释放操作
    ~ZCE_Lock_Guard (void)
    {
        lock_->unlock();
    };

    ///锁定操作
    void lock (void)
    {
        return lock_->lock();
    }

    ///尝试锁定操作
    bool try_lock (void)
    {
        return lock_->try_lock();
    }

    ///解锁操作
    void unlock (void)
    {
        return lock_->unlock();
    }

protected:

    ///用来GUARD保护的锁
    ZCE_LOCK *lock_;

};

/*!
* @brief      读写锁的共享锁（读取）锁定的GUARD，利用构造和修改进行自动加锁，自动解锁操作方法
* @tparam     ZCE_LOCK 锁的类型,可以是，ZCE_Null_Mutex, ZCE_File_Lock ZCE_Thread_RW_Mutex
*/
template <class ZCE_LOCK>
class ZCE_Read_Guard : public ZCE_NON_Copyable
{
public:
    ///构造，得到读写锁，进行读锁定
    ZCE_Read_Guard (ZCE_LOCK &lock):
        lock_(&lock)
    {
        lock_->lock_read();
    }

    ///构造，得到读写锁，根据参数确定是否进行读锁定
    ZCE_Read_Guard (ZCE_LOCK &lock, bool block):
        lock_(&lock)
    {
        if (block)
        {
            lock_->lock_read();
        }
    }

    ///析构，进行解锁操作
    ~ZCE_Read_Guard()
    {
        lock_->unlock();
    }

    ///读取锁
    void lock_read()
    {
        return lock_->lock_read();
    }
    ///尝试读取锁
    bool try_lock_read()
    {
        return lock_->try_lock_read();
    }

    ///解锁,如果是读写锁也只需要这一个函数
    void unlock()
    {
        return lock_->unlock();
    }

protected:

    ///用来GUARD保护的锁
    ZCE_LOCK *lock_;

};

/*!
* @brief      读写锁，写锁定的GUARD，利用构造和修改进行自动加锁，自动解锁操作方法
* @tparam     ZCE_LOCK 锁的类型,可以是 ZCE_Null_Mutex ZCE_File_Lock ZCE_Thread_RW_Mutex
*/
template <class ZCE_LOCK>
class ZCE_Write_Guard : public ZCE_NON_Copyable
{
public:
    ///构造，得到读写锁，进行读锁定
    ZCE_Write_Guard (ZCE_LOCK &lock):
        lock_(&lock)
    {
        lock_->lock_write();
    }

    ///构造，得到读写锁，根据参数确定是否进行读锁定
    ZCE_Write_Guard (ZCE_LOCK &lock, bool block):
        lock_(&lock)
    {
        if (block)
        {
            lock_->lock_write();
        }
    }

    ///析构，进行解锁操作
    ~ZCE_Write_Guard()
    {
        lock_->unlock();
    }

    ///读取锁
    void lock_write()
    {
        return lock_->lock_write();
    }
    ///尝试读取锁
    bool try_lock_write()
    {
        return lock_->try_lock_write();
    }

    ///解锁,如果是读写锁也只需要这一个函数
    void unlock()
    {
        return lock_->unlock();
    }

protected:

    ///用来GUARD保护的锁
    ZCE_LOCK *lock_;

};

#endif //ZCE_LIB_LOCK_GUARD_H_

