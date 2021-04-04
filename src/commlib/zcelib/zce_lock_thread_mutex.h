/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_thread_mutex.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��9��12��
* @brief      �̵߳Ļ�������װ���ֳ�3�ַ�ʽ
*             �̵߳Ļ��������������������ݹ������ǵݹ�������д��
*             ZCE_Thread_Light_Mutex  ���������߳�����WINDODOWS�����ٽ���ģ�⣬LINUX��pthread_mutex, �ɵݹ飬����û�г�ʱ����
*             ZCE_Thread_Recursive_Mutex �ݹ���߳��� WINDODOWS����MUTEXģ�⣬�ɵݹ飬�г�ʱ����
*             ZCE_Thread_NONR_Mutex �����Եݹ���߳��� WINDODOWS���źŵ�ģ���MUTEX�������Եݹ飬�г�ʱ����
*
*             ����������ٽ��������󣬶���ϣ�����죬���Կ���SPIN LOCK��
*/

#ifndef ZCE_LIB_LOCK_THREAD_MUTEX_H_
#define ZCE_LIB_LOCK_THREAD_MUTEX_H_

#include "zce_lock_base.h"
#include "zce_lock_guard.h"
#include "zce_time_value.h"


class ZCE_Time_Value;


/*!
* @brief      ���������̻߳�������һ��������Ƽ�ʹ�ã�����Ҫ������Windowsƽ̨�����ٽ���
*             Windows�������ٽ���ʵ�ֵģ������ͬʱע���ٽ����ǿɵݹ��
*             ��Ϊ�ٽ���û�г�ʱ������¼�������װҲ�Ͳ��ṩ�ˡ����ٽ�������Ҳ��Ӧ���г�ʱ��
*/
class ZCE_Thread_Light_Mutex : public ZCE_Lock_Base
{

public:
    ///�߳�����GUARD
    typedef ZCE_Lock_Guard<ZCE_Thread_Light_Mutex> LOCK_GUARD;

public:

    //���캯��
    ZCE_Thread_Light_Mutex ();
    ///��������
    virtual ~ZCE_Thread_Light_Mutex (void);

    ///����
    virtual void lock();

    ///��������
    virtual bool try_lock();

    ///����,
    virtual void unlock();

    ///ȡ���ڲ�������ָ��
    pthread_mutex_t *get_lock();

protected:
    //�߳���
    pthread_mutex_t  lock_;


};



/*!
* @brief      �̵߳ݹ�����Windows����Mutex��Linux��pthread_mutex
*             ��Զ��ԣ��Ƽ�ʹ�õݹ���,
*/
class ZCE_Thread_Recursive_Mutex : public ZCE_Lock_Base
{
public:

    ///�ݹ�����GUARD
    typedef ZCE_Lock_Guard<ZCE_Thread_Recursive_Mutex> LOCK_GUARD;

public:

    ///���캯��
    ZCE_Thread_Recursive_Mutex ();
    virtual ~ZCE_Thread_Recursive_Mutex (void);

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
    ///�߳���
    pthread_mutex_t  lock_;


};


/*!
* @brief      �̵߳ķǵݹ������ṩ�����ȫ��Ϊ������һЩ���ã���Ϊ�ݹ�������������ǵݹ������
*             Windows�µĻ��������ٽ�����ʵ�ǿ��Եݹ�ģ�����ֻ���źŵ���ģ��ǵݹ���
*             201209 pthread_mutex�ڲ����Ѿ������˷ǵݹ������ܣ����źŵ�ģ��
*/
class ZCE_Thread_NONR_Mutex : public ZCE_Lock_Base
{



public:
    ///����GUARD
    typedef ZCE_Lock_Guard<ZCE_Thread_NONR_Mutex> LOCK_GUARD;

public:

    ///���캯��
    ZCE_Thread_NONR_Mutex ();
    ///��������
    virtual ~ZCE_Thread_NONR_Mutex (void);
public:
    ///����
    virtual void lock();

    ///��������
    virtual bool try_lock();

    ///����
    virtual void unlock();


    ///����ʱ�䳬ʱ�ĵ���������ʱ�����
    virtual bool systime_lock(const ZCE_Time_Value &abs_time);

    ///���ʱ��ĳ�ʱ��������ʱ�󣬽���
    virtual bool duration_lock(const ZCE_Time_Value &relative_time);

protected:


    ///�߳���,��Windows����Ҳ�Ѿ�ģ��ķǵݹ����
    pthread_mutex_t  lock_;



};



#endif //ZCE_LIB_LOCK_THREAD_MUTEX_H_

