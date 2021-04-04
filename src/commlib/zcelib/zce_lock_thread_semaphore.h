/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_thread_semaphore.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��1��14��
* @brief      �̵߳��źŵƷ�װ
*/

#ifndef ZCE_LIB_LOCK_THREAD_SEMAPHORE_H_
#define ZCE_LIB_LOCK_THREAD_SEMAPHORE_H_

#include "zce_lock_base.h"
#include "zce_lock_guard.h"
#include "zce_time_value.h"

/*!
@brief      �̵߳��źŵƷ�װ��ʹ�õ����������źŵƣ�
*/
class ZCE_Thread_Semaphore  : public ZCE_Lock_Base
{

public:
    //�߳�����GUARD
    typedef ZCE_Lock_Guard<ZCE_Thread_Semaphore> LOCK_GUARD;

public:

    //���캯��,Ĭ�ϴ��������źŵƣ��߳���һ���������źŵƾ��㹻��,sem_nameһ����NULL���㹻��
    //�����źŵ�==�����źŵ�
    ZCE_Thread_Semaphore (unsigned int init_value);
    virtual ~ZCE_Thread_Semaphore (void);

    //����
    virtual void lock();

    //��������
    virtual bool try_lock();

    //����,
    virtual void unlock();

    //����ʱ�䳬ʱ�ĵ���������ʱ�����
    virtual bool systime_lock(const ZCE_Time_Value &abs_time);

    //���ʱ��ĳ�ʱ��������ʱ�󣬽���
    virtual bool duration_lock(const ZCE_Time_Value &relative_time);

protected:
    //�߳���
    sem_t            *lock_;

};

#endif //ZCE_LIB_LOCK_THREAD_SEMAPHORE_H_

