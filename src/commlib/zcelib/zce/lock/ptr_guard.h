/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/ptr_guard.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2012年9月21日
* @brief      利用多态实现的锁GUARD，
*
*             如果你还能被人噎着，就表示你还不够强大，
*
*/

#pragma once

#include "zce/util/non_copyable.h"
#include "zce/lock/lock_base.h"

/*!
* @brief      锁GUARD，利用构造和析构进行加锁解锁操作方法，利用多态兼容变化
*             多态的好处是，你可以动态的决定使用什么锁，而不是在编译的时候。
*             只要是从ZCE_Lock_Base继承的家伙，应该都可以使用这个，
*             ZCE_LockPtr_Guard是利用多态特性实现的，而不是模版特性
*/
class ZCE_Lock_Ptr_Guard : public zce::NON_Copyable
{
public:

    //构造，得到锁，进行锁定
    ZCE_Lock_Ptr_Guard(ZCE_Lock_Base* lock_ptr) :
        lock_ptr_(lock_ptr)
    {
        lock_ptr_->lock();
    }

    ///构造，得到锁，根据要求决定是否进行锁定操作
    ZCE_Lock_Ptr_Guard(ZCE_Lock_Base* lock_ptr, bool block) :
        lock_ptr_(lock_ptr)
    {
        if (block)
        {
            lock_ptr_->lock();
        }
    }

    ///析构，同时对锁进行释放操作
    ~ZCE_Lock_Ptr_Guard(void)
    {
        lock_ptr_->unlock();
    };

    ///锁定操作
    void lock(void)
    {
        return lock_ptr_->lock();
    }

    ///尝试锁定操作
    bool try_lock(void)
    {
        return lock_ptr_->try_lock();
    }

    ///解锁操作
    void unlock(void)
    {
        return lock_ptr_->unlock();
    }

protected:

    ///用来GUARD保护的锁,利用C++特性实现锁差异
    ZCE_Lock_Base* lock_ptr_;
};
