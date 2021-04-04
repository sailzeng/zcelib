#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_lock_base.h"

/************************************************************************************************************
Class           : ZCE_Lock_Base
************************************************************************************************************/

//���캯������������
ZCE_Lock_Base::ZCE_Lock_Base(const char * )
{
}

ZCE_Lock_Base::~ZCE_Lock_Base (void)
{
}

//����
void ZCE_Lock_Base::lock()
{
    return;
}

//��������
bool ZCE_Lock_Base::try_lock()
{
    return true;
}

//����,
void ZCE_Lock_Base::unlock()
{
    return;
}

///������
void ZCE_Lock_Base::unlock_read()
{
    return;
}

///����д
void ZCE_Lock_Base::unlock_write()
{
    return;
}

//����ʱ�䳬ʱ�ĵ���������ʱ������������Ƿ�ʱ
bool ZCE_Lock_Base::systime_lock(const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

//���ʱ�䣬�����Ƿ�ʱ
bool ZCE_Lock_Base::duration_lock(const ZCE_Time_Value & /*relative_time*/ )
{
    return true;
}

//��ȡ��
void ZCE_Lock_Base::lock_read()
{
    return;
}

//���Զ�ȡ��
bool ZCE_Lock_Base::try_lock_read()
{
    return true;
}

//����ʱ��
bool ZCE_Lock_Base::timed_lock_read(const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

//���ʱ��
bool ZCE_Lock_Base::duration_lock_read(const ZCE_Time_Value & /*relative_time*/)
{
    return true;
}

//д����
void ZCE_Lock_Base::lock_write()
{
    return;
}

//���Զ�ȡ��
bool ZCE_Lock_Base::try_lock_write()
{
    return true;
}

//д������ʱ
bool ZCE_Lock_Base::timed_lock_write(const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

//���ʱ��
bool ZCE_Lock_Base::duration_lock_write(const ZCE_Time_Value & /*relative_time*/)
{
    return true;
}

/************************************************************************************************************
Class           : ZCE_Null_Condition
************************************************************************************************************/
ZCE_Condition_Base::ZCE_Condition_Base()
{
}

ZCE_Condition_Base::~ZCE_Condition_Base()
{
}

///�ȴ�
void ZCE_Condition_Base::wait(ZCE_Lock_Base * /*external_mutex*/ )
{
    return ;
}

///����ʱ�䳬ʱ�ĵĵȴ�����ʱ�����
bool ZCE_Condition_Base::systime_wait(ZCE_Lock_Base * /*external_mutex*/, const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

///���ʱ��ĳ�ʱ�����ȴ�����ʱ�󣬽���
bool ZCE_Condition_Base::duration_wait(ZCE_Lock_Base * /*external_mutex*/, const ZCE_Time_Value &/*relative_time*/)
{
    return false;
}

/// ��һ���ȴ��̷߳����ź� Signal one waiting thread.
void ZCE_Condition_Base::signal (void)
{
    return;
}

///�����еĵȴ��̹߳㲥�ź� Signal *all* waiting threads.
void ZCE_Condition_Base::broadcast (void)
{
    return;
}

