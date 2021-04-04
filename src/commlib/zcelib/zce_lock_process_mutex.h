/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_process_mutex.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013��1��15��
* @brief      ���̵�MUTEX�����������֣�
*
*/
#ifndef ZCE_LIB_LOCK_PROCESS_MUTEX_H_
#define ZCE_LIB_LOCK_PROCESS_MUTEX_H_

#include "zce_lock_base.h"
#include "zce_lock_guard.h"
#include "zce_share_mem_posix.h"

/*!
* @brief      ����Mutex��,���ڶ���̼�Ļ���ͬ������
*             ��WINDOWS�£��ݹ�������MUTEXģ��ģ����������֣����ǵݹ��������źŵ�ģ�⣬
*             ��LINUX�£�������pthread_mutexʵ�ֵģ������ù����ڴ��ţ��������ڹ���
*             �ڴ�����
*/
class ZCE_Process_Mutex : public ZCE_Lock_Base
{

public:

    //��������GUARD
    typedef ZCE_Lock_Guard<ZCE_Process_Mutex> LOCK_GUARD;

public:
    ///���캯��
    ZCE_Process_Mutex (const char *mutex_name, bool recursive = true);
    ///��������
    virtual ~ZCE_Process_Mutex (void);

    ///����
    virtual void lock();

    ///��������
    virtual bool try_lock();

    ///����,
    virtual void unlock();

    ///����ʱ�䳬ʱ�ĵ���������ʱ�����
    virtual bool systime_lock(const ZCE_Time_Value &abs_time);

    ///���ʱ��ĳ�ʱ��������ʱ�󣬽���
    virtual bool duration_lock(const ZCE_Time_Value &relative_time);

    ///ȡ���ڲ�������ָ��
    pthread_mutex_t *get_lock();

protected:

    ///��������ע�⣬ע�⣬ע�⣬����ط��õ���һ��ָ�룬
    pthread_mutex_t    *lock_;

#if defined ZCE_OS_LINUX
    //�����ڴ棬LINUX��Ҫ�����ڴ�
    ZCE_ShareMem_Posix posix_sharemem_;
#endif

};

#endif //ZCE_LIB_LOCK_PROCESS_MUTEX_H_

