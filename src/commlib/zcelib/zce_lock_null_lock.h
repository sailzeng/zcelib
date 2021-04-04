/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_null_lock.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011��9��13��
* @brief      Ϊ�˼�������ACE��ô���꣬�һ���������ACE��һЩ��ʽ������⡣
*             ��Ȼ��Ҳ�����һ��ǻ�ѧϰһЩBOOST���ش�
*             ACE��ר�ŵ�һƪ�����ǽ�ģ�������ģʽ�ģ�
*             ��Ҳ�Ա��˹�ģ��������Ͷ�̬����������ȱ�㣬�Ҿ��ø������ӣ�
*             �����ϣ����������ģ�廯�����Ƕ�̬��һ�㡣
*
*             ��ο�
*             http://www.cnblogs.com/fullsail/archive/2012/07/31/2616106.html
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_LOCK_NULL_LOCK_H_
#define ZCE_LIB_LOCK_NULL_LOCK_H_

#include "zce_lock_guard.h"
#include "zce_lock_base.h"
#include "zce_boost_non_copyable.h"

class ZCE_Time_Value;

/*!
* @brief      ������Ҳ��һ��ģʽ������ĳЩ�������ʹ���Ƿ�����ķ�ʽ,
*
*/
class ZCE_Null_Mutex : public ZCE_Lock_Base
{
public:
    //NULL����GUARD
    typedef ZCE_Lock_Guard<ZCE_Null_Mutex>      LOCK_GUARD;

    typedef ZCE_Read_Guard<ZCE_Null_Mutex>      LOCK_READ_GUARD;

    typedef ZCE_Write_Guard<ZCE_Null_Mutex>     LOCK_WRITE_GUARD;

public:
    ///���캯��
    ZCE_Null_Mutex (const char * = NULL);
    ///��������
    virtual ~ZCE_Null_Mutex (void);

public:
    ///����
    virtual void lock();

    ///��������
    virtual bool try_lock();

    ///����,
    virtual void unlock();

    ///����ʱ�䳬ʱ�ĵ���������ʱ������������Ƿ�ʱ
    virtual bool systime_lock(const ZCE_Time_Value & /*abs_time*/);
    ///���ʱ��
    virtual bool duration_lock(const ZCE_Time_Value & /*relative_time*/ );

    //�����BOOST��shared�Ĺ���-��ռ���Ľз����һ��������д��

    ///��ȡ��
    virtual void lock_read();
    ///���Զ�ȡ��
    virtual bool try_lock_read();

    ///����ʱ�䳬ʱ�Ķ�ȡ����
    virtual bool timed_lock_read(const ZCE_Time_Value & /*abs_time*/);
    ///���ʱ�䳬ʱ�Ķ�ȡ����
    virtual bool duration_lock_read(const ZCE_Time_Value & /*relative_time*/);

    ///д����
    virtual void lock_write();
    ///���Զ�ȡ��
    virtual bool try_lock_write();
    ///д������ʱ
    virtual bool timed_lock_write(const ZCE_Time_Value & /*abs_time*/);
    //���ʱ��
    virtual bool duration_lock_write(const ZCE_Time_Value & /*relative_time*/);

protected:
    // A dummy lock.
    int    lock_;
};

/*!
* @brief      ������Ҳ��һ��ģʽ������ĳЩ�������ʹ���Ƿ�����ķ�ʽ,
*             ����Ľӿ�������BOOST�Ľӿڣ����粻���Ʒ���ֵ��Ҳ�ο���һЩACE
*/
class ZCE_Null_Condition : public ZCE_Condition_Base
{

public:
    //
    ZCE_Null_Condition();
    virtual ~ZCE_Null_Condition();

private:

    ///�ȴ�
    virtual void wait (ZCE_Null_Mutex * /*external_mutex*/ );

    ///����ʱ�䳬ʱ�ĵĵȴ�����ʱ�����
    virtual bool systime_wait(ZCE_Null_Mutex * /*external_mutex*/, const ZCE_Time_Value & /*abs_time*/);

    ///���ʱ��ĳ�ʱ�����ȴ�����ʱ�󣬽���
    virtual bool duration_wait(ZCE_Null_Mutex * /*external_mutex*/, const ZCE_Time_Value & /*relative_time*/);

    /// ��һ���ȴ��̷߳����ź� Signal one waiting thread.
    virtual void signal (void);

    ///�����еĵȴ��̹߳㲥�ź� Signal *all* waiting threads.
    virtual void broadcast (void);

protected:
    // A dummy lock.
    int    lock_;

};

#endif //ZCE_LIB_LOCK_NULL_LOCK_H_

