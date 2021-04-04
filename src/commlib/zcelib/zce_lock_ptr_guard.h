/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_ptr_guard.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012��9��21��
* @brief      ���ö�̬ʵ�ֵ���GUARD��
*
*             ����㻹�ܱ���ҭ�ţ��ͱ�ʾ�㻹����ǿ��
*
*/

#ifndef ZCE_LIB_LOCK_PTR_GUARD_H_
#define ZCE_LIB_LOCK_PTR_GUARD_H_

#include "zce_boost_non_copyable.h"
#include "zce_lock_base.h"

/*!
* @brief      ��GUARD�����ù�����޸Ľ��м�����������������
*             ֻҪ�Ǵ�ZCE_Lock_Base�̳еļһӦ�ö�����ʹ�������
*             ZCE_LockPtr_Guard�����ö�̬����ʵ�ֵģ�������ģ������
*/
class ZCE_Lock_Ptr_Guard : public ZCE_NON_Copyable
{
public:

    //���죬�õ�������������
    ZCE_Lock_Ptr_Guard (ZCE_Lock_Base *lock_ptr):
        lock_ptr_(lock_ptr)
    {
        lock_ptr_->lock();
    }

    ///���죬�õ���������Ҫ������Ƿ������������
    ZCE_Lock_Ptr_Guard (ZCE_Lock_Base *lock_ptr, bool block):
        lock_ptr_(lock_ptr)
    {
        if (block)
        {
            lock_ptr_->lock();
        }
    }

    ///������ͬʱ���������ͷŲ���
    ~ZCE_Lock_Ptr_Guard (void)
    {
        lock_ptr_->unlock();
    };

    ///��������
    void lock (void)
    {
        return lock_ptr_->lock();
    }

    ///������������
    bool try_lock (void)
    {
        return lock_ptr_->try_lock();
    }

    ///��������
    void unlock (void)
    {
        return lock_ptr_->unlock();
    }

protected:

    ///����GUARD��������,����C++����ʵ��������
    ZCE_Lock_Base    *lock_ptr_;

};

#endif //ZCE_LIB_LOCK_PTR_GUARD_H_

