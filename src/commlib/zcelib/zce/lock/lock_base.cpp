#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/lock/lock_base.h"

/************************************************************************************************************
Class           : ZCE_Lock_Base
************************************************************************************************************/

//构造函数和析构函数
ZCE_Lock_Base::ZCE_Lock_Base(const char *)
{
}

ZCE_Lock_Base::~ZCE_Lock_Base(void)
{
}

//锁定
void ZCE_Lock_Base::lock()
{
    return;
}

//尝试锁定
bool ZCE_Lock_Base::try_lock()
{
    return true;
}

//解锁,
void ZCE_Lock_Base::unlock()
{
    return;
}

///解锁读
void ZCE_Lock_Base::unlock_read()
{
    return;
}

///解锁写
void ZCE_Lock_Base::unlock_write()
{
    return;
}

//绝对时间超时的的锁定，超时后解锁，返回是否超时
bool ZCE_Lock_Base::systime_lock(const zce::Time_Value & /*abs_time*/)
{
    return true;
}

//相对时间，返回是否超时
bool ZCE_Lock_Base::duration_lock(const zce::Time_Value & /*relative_time*/)
{
    return true;
}

//读取锁
void ZCE_Lock_Base::lock_read()
{
    return;
}

//尝试读取锁
bool ZCE_Lock_Base::try_lock_read()
{
    return true;
}

//绝对时间
bool ZCE_Lock_Base::timed_lock_read(const zce::Time_Value & /*abs_time*/)
{
    return true;
}

//相对时间
bool ZCE_Lock_Base::duration_lock_read(const zce::Time_Value & /*relative_time*/)
{
    return true;
}

//写锁定
void ZCE_Lock_Base::lock_write()
{
    return;
}

//尝试读取锁
bool ZCE_Lock_Base::try_lock_write()
{
    return true;
}

//写锁定超时
bool ZCE_Lock_Base::timed_lock_write(const zce::Time_Value & /*abs_time*/)
{
    return true;
}

//相对时间
bool ZCE_Lock_Base::duration_lock_write(const zce::Time_Value & /*relative_time*/)
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

///等待
void ZCE_Condition_Base::wait(ZCE_Lock_Base * /*external_mutex*/)
{
    return;
}

///绝对时间超时的的等待，超时后解锁
bool ZCE_Condition_Base::systime_wait(ZCE_Lock_Base * /*external_mutex*/,const zce::Time_Value & /*abs_time*/)
{
    return true;
}

///相对时间的超时锁定等待，超时后，解锁
bool ZCE_Condition_Base::duration_wait(ZCE_Lock_Base * /*external_mutex*/,const zce::Time_Value &/*relative_time*/)
{
    return false;
}

/// 给一个等待线程发送信号 Signal one waiting thread.
void ZCE_Condition_Base::signal(void)
{
    return;
}

///给所有的等待线程广播信号 Signal *all* waiting threads.
void ZCE_Condition_Base::broadcast(void)
{
    return;
}