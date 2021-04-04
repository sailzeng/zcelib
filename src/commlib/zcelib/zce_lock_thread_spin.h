/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_thread_spin.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��1��31��
* @brief      SPIN �������ķ�װ��
*             WINDOWS����SPIN + �ٽ�����װ
*             LINUX����SPIN MUTEX��װ
*
* @details    �������һЩ�ٽ��������������������װʵ��һ�£�
*             �󲿷��������ݣ�����ʾ�󲿷ֲ��Խ���ķ����ǻ�����ġ�
*
*             SPIN �ڳ�ײ�ر��������ʹ��Ҫ����һ�㡣
*             WINDOPWS ��ʵ�ֿ��ܸ�������һ�㣬������SPIN�ﵽһ���������������
*             ����ȴ���
*
*/

#ifndef ZCE_LIB_LOCK_THREAD_SPIN_H_
#define ZCE_LIB_LOCK_THREAD_SPIN_H_

#include "zce_lock_base.h"
#include "zce_lock_guard.h"
#include "zce_time_value.h"

/*!
* @brief      SPIN���������̻߳�������
*             ��������Ϊ�˱���һ�δ��룬ĳ�����ݡ����������ٽ���������ʹ��
*             ���������
*             Windows�������ٽ���+SPINʵ�ֵģ������ͬʱע���ٽ����ǿɵݹ��
*             LINUX���õľ���SPIN ��
*/
class ZCE_Thread_Spin_Mutex : public ZCE_Lock_Base
{

public:
    ///�߳�����GUARD
    typedef ZCE_Lock_Guard<ZCE_Thread_Spin_Mutex> LOCK_GUARD;

public:

    //���캯��
    ZCE_Thread_Spin_Mutex ();
    ///��������
    virtual ~ZCE_Thread_Spin_Mutex (void);

    ///����
    virtual void lock() override;

    ///��������
    virtual bool try_lock() override;

    ///����,
    virtual void unlock() override;

    ///ȡ���ڲ�������ָ��
    pthread_spinlock_t *get_lock();

protected:

    ///SPIN�߳���
    pthread_spinlock_t  lock_;

};

#endif //ZCE_LIB_LOCK_THREAD_SPIN_H_

