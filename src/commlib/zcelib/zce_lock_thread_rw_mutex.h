/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_thread_rw_mutex.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��9��13��
* @brief      �̵߳Ķ�д����ZCE_Thread_RW_Mutex
*             ��WINDOWS�£��ڲ���ʹ�õ������Ĵ��룬��ο� @ref zce_os_adapt_rwlock.h
*             �ڲ�֧��WINSERVER 2008�Ļ����õ�������������MUTEX��ģ�����Ʒ��
*             ��֧��WINSERVER 2008�Ļ����������������ͱ��룩����ʹ��WINDOWS�Լ��Ķ�д����װ
*
* @detail     ��д���Ľ�������unlock��������Լ�ģ��ʵ�ֵĻ����ֿ���д��unlock��Ȼ
*             ��������ʵ�ֵģ�BOOST��������,
*             pthread_rwlock�ĺ�����û�����ֵģ��ҵ�ģ��Ҳ����ѭ��pthread_rwlock
*             �Ľӿ�
*
*             �����BOOST��shared�Ĺ������Ľз����һ��������д��
*
*             ��Щ����ǹز�ס�ġ����ǵ���ë̫�����ˡ������Ƿ��ߵ�ʱ�����ĵ���֪�������ǹ�����
*             ��һ����������˶���ܡ�����������һ��,��ס�ĵط�Ҳ�͸��ӻҰ����顣�Ҿ���������
*             �����ҵ����ѡ�������Ф��˵ľ��꡷
*/

#ifndef ZCE_LIB_LOCK_THREAD_RW_MUTEX_H_
#define ZCE_LIB_LOCK_THREAD_RW_MUTEX_H_

#include "zce_lock_base.h"
#include "zce_lock_guard.h"
#include "zce_time_value.h"

/*!
@brief      �̵߳Ķ�д��

*/
class ZCE_Thread_RW_Mutex : public ZCE_Lock_Base
{

public:
    ///������GUARD
    typedef ZCE_Read_Guard<ZCE_Thread_RW_Mutex>  LOCK_READ_GUARD;
    ///д����GUARD
    typedef ZCE_Write_Guard<ZCE_Thread_RW_Mutex> LOCK_WRITE_GUARD;

public:
    //���캯��
    ZCE_Thread_RW_Mutex();
    virtual ~ZCE_Thread_RW_Mutex();

public:
    //��ȡ��
    virtual void lock_read();
    //���Զ�ȡ��
    virtual bool try_lock_read();

    //����ʱ��
    virtual bool systime_lock_read(const ZCE_Time_Value &abs_time);
    //���ʱ��
    virtual bool duration_lock_read(const ZCE_Time_Value &relative_time);

    //д����
    virtual void lock_write();
    //���Զ�ȡ��
    virtual bool try_lock_write();
    //д������ʱ������ʱ��
    virtual bool systime_lock_write(const ZCE_Time_Value &abs_time);
    //д������ʱ�����ʱ��
    virtual bool duration_lock_write(const ZCE_Time_Value &relative_time);

    ///��д��
    virtual void unlock_write();
    ///�����
    virtual void unlock_read();

    ///ȡ���ڲ�������ָ��
    pthread_rwlock_t *get_lock();

protected:

    //�߳���
    pthread_rwlock_t  rw_lock_;

};


#if defined ZCE_SUPPORT_WINSVR2008 && ZCE_SUPPORT_WINSVR2008 == 1

/*!
@brief      ���������̵߳Ķ�д����û�г�ʱ����
            ��Ҫ��Ϊ������Windows SVR 2008�Ժ�Ķ�д��ʵ�֣�
            ������������Ƽ�ʹ��������ٶȱ�ģ��Ĺ��ƿ�ܶ࣬
*/
class ZCE_Thread_Win_RW_Mutex : public ZCE_Lock_Base
{

public:
    ///������GUARD
    typedef ZCE_Read_Guard<ZCE_Thread_Win_RW_Mutex>  LOCK_READ_GUARD;
    ///д����GUARD
    typedef ZCE_Write_Guard<ZCE_Thread_Win_RW_Mutex> LOCK_WRITE_GUARD;

public:
    //���캯��
    ZCE_Thread_Win_RW_Mutex();
    virtual ~ZCE_Thread_Win_RW_Mutex();

public:
    //��ȡ��
    virtual void lock_read();
    //���Զ�ȡ��
    virtual bool try_lock_read();

    //д����
    virtual void lock_write();
    //���Զ�ȡ��
    virtual bool try_lock_write();

    ///�����
    virtual void unlock_read();

    ///��д��
    virtual void unlock_write();

    ///ȡ���ڲ�������ָ��
    SRWLOCK *get_lock();

protected:

    ///WINSVR 2008�Ժ�WINDOWS�Լ�ʵ�ֵĶ�д��
    SRWLOCK                rwlock_slim_;

};

#endif

#endif //ZCE_LIB_LOCK_THREAD_MUTEX_H_

