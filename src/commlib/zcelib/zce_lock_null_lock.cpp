#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_lock_null_lock.h"

/************************************************************************************************************
Class           : ZCE_Null_Mutex
************************************************************************************************************/
//���캯������������
ZCE_Null_Mutex::ZCE_Null_Mutex(const char * ):
    lock_(0)
{
}

ZCE_Null_Mutex::~ZCE_Null_Mutex (void)
{
}

//����
void ZCE_Null_Mutex::lock()
{
    return;
}

//��������
bool ZCE_Null_Mutex::try_lock()
{
    return true;
}

//����,
void ZCE_Null_Mutex::unlock()
{
    return;
}

//����ʱ�䳬ʱ�ĵ���������ʱ������������Ƿ�ʱ
bool ZCE_Null_Mutex::systime_lock(const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

//���ʱ�䣬�����Ƿ�ʱ
bool ZCE_Null_Mutex::duration_lock(const ZCE_Time_Value & /*relative_time*/ )
{
    return true;
}

//��ȡ��
void ZCE_Null_Mutex::lock_read()
{
    return;
}

//���Զ�ȡ��
bool ZCE_Null_Mutex::try_lock_read()
{
    return true;
}

//����ʱ��
bool ZCE_Null_Mutex::timed_lock_read(const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

//���ʱ��
bool ZCE_Null_Mutex::duration_lock_read(const ZCE_Time_Value & /*relative_time*/)
{
    return true;
}

//д����
void ZCE_Null_Mutex::lock_write()
{
    return;
}

//���Զ�ȡ��
bool ZCE_Null_Mutex::try_lock_write()
{
    return true;
}

//д������ʱ
bool ZCE_Null_Mutex::timed_lock_write(const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

//���ʱ��
bool ZCE_Null_Mutex::duration_lock_write(const ZCE_Time_Value & /*relative_time*/)
{
    return true;
}

/************************************************************************************************************
Class           : ZCE_Null_Condition
************************************************************************************************************/
ZCE_Null_Condition::ZCE_Null_Condition():
    lock_(0)
{
}

ZCE_Null_Condition::~ZCE_Null_Condition()
{
}

///�ȴ�
void ZCE_Null_Condition::wait(ZCE_Null_Mutex * /*external_mutex*/ )
{
    return ;
}

///����ʱ�䳬ʱ�ĵĵȴ�����ʱ�����
bool ZCE_Null_Condition::systime_wait(ZCE_Null_Mutex * /*external_mutex*/, const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

///���ʱ��ĳ�ʱ�����ȴ�����ʱ�󣬽���
bool ZCE_Null_Condition::duration_wait(ZCE_Null_Mutex * /*external_mutex*/, const ZCE_Time_Value &/*relative_time*/)
{
    return false;
}

/// ��һ���ȴ��̷߳����ź� Signal one waiting thread.
void ZCE_Null_Condition::signal (void)
{
    return;
}

///�����еĵȴ��̹߳㲥�ź� Signal *all* waiting threads.
void ZCE_Null_Condition::broadcast (void)
{
    return;
}

