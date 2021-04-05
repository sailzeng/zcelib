/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_synch_traits.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��11��15��
* @brief      ������ģʽ������ȡ traits������ģ������е�typedef��
*             ����һ�����������Ҫ�����Ͳ��������ֲ��Ե�ʱ�򣬶����ڲ�����
*             ���������źŵƵȶ���ͬ������ʱ��
*             ������ZCE_MT_SYNCH����ZCE_NULL_SYNCH������Ϊģ�������ģ��ͨ��
*             type::MUTEX�����ķ�ʽ��ò�ͬ����������������̣߳�����
*             ���̻߳���
*/

#ifndef ZCE_LIB_LOCK_SYNCH_TRAITS_H_
#define ZCE_LIB_LOCK_SYNCH_TRAITS_H_

//����ĳЩ���Դ������
#include "zce_lock_process_mutex.h"
#include "zce_lock_thread_mutex.h"
#include "zce_lock_thread_rw_mutex.h"
#include "zce_lock_thread_semaphore.h"
#include "zce_lock_thread_condi.h"
#include "zce_lock_null_lock.h"

/*!
* @brief      �̼߳�����������������ȡ��
*/
class ZCE_MT_SYNCH
{
public:
    typedef ZCE_Thread_Light_Mutex                        MUTEX;
    typedef ZCE_Null_Mutex                          NULL_MUTEX;
    typedef ZCE_Process_Mutex                       PROCESS_MUTEX;
    typedef ZCE_Thread_Recursive_Mutex              RECURSIVE_MUTEX;
    typedef ZCE_Thread_RW_Mutex                     RW_MUTEX;
    typedef ZCE_Thread_Condition_Mutex              CONDITION;
    typedef ZCE_Thread_Condition_Recursive_Mutex    RECURSIVE_CONDITION;
    typedef ZCE_Thread_Semaphore                    SEMAPHORE;
    typedef ZCE_Null_Mutex                          NULL_SEMAPHORE;
};

/*!
* @brief      ZCE_NULL_SYNCH ��������������������ȡ��
*/
class ZCE_NULL_SYNCH
{
public:
    typedef ZCE_Null_Mutex               MUTEX;
    typedef ZCE_Null_Mutex               NULL_MUTEX;
    typedef ZCE_Null_Mutex               PROCESS_MUTEX;
    typedef ZCE_Null_Mutex               RECURSIVE_MUTEX;
    typedef ZCE_Null_Mutex               RW_MUTEX;
    typedef ZCE_Null_Condition           CONDITION;
    typedef ZCE_Null_Condition           RECURSIVE_CONDITION;
    typedef ZCE_Null_Mutex               SEMAPHORE;
    typedef ZCE_Null_Mutex               NULL_SEMAPHORE;
};

#endif //ZCE_LIB_LOCK_SYNCH_TRAITS_H_

