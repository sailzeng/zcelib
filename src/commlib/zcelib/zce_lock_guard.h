/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_guard.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��1��14��
* @brief      GURAD�����ù��캯�������������Զ��������������࣬
*             �ڸ��������涼��typdef����ʹ��
*             ����ZCE_Thread_Light_Mutex
*
*/

#ifndef ZCE_LIB_LOCK_GUARD_H_
#define ZCE_LIB_LOCK_GUARD_H_

#include "zce_boost_non_copyable.h"

/*!
* @brief      ��GUARD�����ù�����޸Ľ����Զ��������Զ�������������
* @tparam     ZCE_LOCK �������ͣ�������ZCE_Null_Mutex��ZCE_Process_Mutex��
*             ZCE_Thread_Light_Mutex��ZCE_Thread_Recursive_Mutex
*             ZCE_Process_Semaphore,ZCE_Thread_NONR_Mutex,��
*/
template <typename ZCE_LOCK>
class ZCE_Lock_Guard : public ZCE_NON_Copyable
{
public:

    ///���죬�õ�������������
    ZCE_Lock_Guard (ZCE_LOCK &lock):
        lock_(&lock)
    {
        lock_->lock();
    }

    //���죬�õ���������Ҫ������Ƿ������������
    ZCE_Lock_Guard (ZCE_LOCK &lock, bool block):
        lock_(&lock)
    {
        if (block)
        {
            lock_->lock();
        }
    }

    ///������ͬʱ���������ͷŲ���
    ~ZCE_Lock_Guard (void)
    {
        lock_->unlock();
    };

    ///��������
    void lock (void)
    {
        return lock_->lock();
    }

    ///������������
    bool try_lock (void)
    {
        return lock_->try_lock();
    }

    ///��������
    void unlock (void)
    {
        return lock_->unlock();
    }

protected:

    ///����GUARD��������
    ZCE_LOCK *lock_;

};

/*!
* @brief      ��д���Ĺ���������ȡ��������GUARD�����ù�����޸Ľ����Զ��������Զ�������������
* @tparam     ZCE_LOCK ��������,�����ǣ�ZCE_Null_Mutex, ZCE_File_Lock ZCE_Thread_RW_Mutex
*/
template <class ZCE_LOCK>
class ZCE_Read_Guard : public ZCE_NON_Copyable
{
public:
    ///���죬�õ���д�������ж�����
    ZCE_Read_Guard (ZCE_LOCK &lock):
        lock_(&lock)
    {
        lock_->lock_read();
    }

    ///���죬�õ���д�������ݲ���ȷ���Ƿ���ж�����
    ZCE_Read_Guard (ZCE_LOCK &lock, bool block):
        lock_(&lock)
    {
        if (block)
        {
            lock_->lock_read();
        }
    }

    ///���������н�������
    ~ZCE_Read_Guard()
    {
        lock_->unlock();
    }

    ///��ȡ��
    void lock_read()
    {
        return lock_->lock_read();
    }
    ///���Զ�ȡ��
    bool try_lock_read()
    {
        return lock_->try_lock_read();
    }

    ///����,����Ƕ�д��Ҳֻ��Ҫ��һ������
    void unlock_read()
    {
        return lock_->unlock_read();
    }

protected:

    ///����GUARD��������
    ZCE_LOCK *lock_;

};

/*!
* @brief      ��д����д������GUARD�����ù�����޸Ľ����Զ��������Զ�������������
* @tparam     ZCE_LOCK ��������,������ ZCE_Null_Mutex ZCE_File_Lock ZCE_Thread_RW_Mutex
*/
template <class ZCE_LOCK>
class ZCE_Write_Guard : public ZCE_NON_Copyable
{
public:
    ///���죬�õ���д�������ж�����
    ZCE_Write_Guard (ZCE_LOCK &lock):
        lock_(&lock)
    {
        lock_->lock_write();
    }

    ///���죬�õ���д�������ݲ���ȷ���Ƿ���ж�����
    ZCE_Write_Guard (ZCE_LOCK &lock, bool block):
        lock_(&lock)
    {
        if (block)
        {
            lock_->lock_write();
        }
    }

    ///���������н�������
    ~ZCE_Write_Guard()
    {
        lock_->unlock();
    }

    ///��ȡ��
    void lock_write()
    {
        return lock_->lock_write();
    }
    ///���Զ�ȡ��
    bool try_lock_write()
    {
        return lock_->try_lock_write();
    }

    ///����,����Ƕ�д��Ҳֻ��Ҫ��һ������
    void unlock_write()
    {
        return lock_->unlock_write();
    }

protected:

    ///����GUARD��������
    ZCE_LOCK *lock_;

};

#endif //ZCE_LIB_LOCK_GUARD_H_

