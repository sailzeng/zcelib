#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/lock/lock_base.h"

/************************************************************************************************************
Class           : zce::Lock_Base
************************************************************************************************************/
namespace zce
{
//构造函数和析构函数
Lock_Base::Lock_Base(const char*)
{
}

Lock_Base::~Lock_Base(void)
{
}

//锁定
void Lock_Base::lock()
{
    return;
}

//尝试锁定
bool Lock_Base::try_lock()
{
    return true;
}

//解锁,
void Lock_Base::unlock()
{
    return;
}

///解锁读
void Lock_Base::unlock_read()
{
    return;
}

///解锁写
void zce::Lock_Base::unlock_write()
{
    return;
}

//绝对时间超时的的锁定，超时后解锁，返回是否超时
bool zce::Lock_Base::lock(const zce::Time_Value& /*abs_time*/)
{
    return true;
}

//相对时间，返回是否超时
bool Lock_Base::lock_for(const zce::Time_Value& /*relative_time*/)
{
    return true;
}

//读取锁
void Lock_Base::lock_read()
{
    return;
}

//尝试读取锁
bool Lock_Base::try_lock_read()
{
    return true;
}

//绝对时间
bool Lock_Base::timed_lock_read(const zce::Time_Value& /*abs_time*/)
{
    return true;
}

//相对时间
bool Lock_Base::duration_lock_read(const zce::Time_Value& /*relative_time*/)
{
    return true;
}

//写锁定
void Lock_Base::lock_write()
{
    return;
}

//尝试读取锁
bool zce::Lock_Base::try_lock_write()
{
    return true;
}

//写锁定超时
bool zce::Lock_Base::timed_lock_write(const zce::Time_Value& /*abs_time*/)
{
    return true;
}

//相对时间
bool zce::Lock_Base::duration_lock_write(const zce::Time_Value& /*relative_time*/)
{
    return true;
}

/************************************************************************************************************
Class           : Condition_Base
************************************************************************************************************/
Condition_Base::Condition_Base()
{
}

Condition_Base::~Condition_Base()
{
}

///等待
void Condition_Base::wait(zce::Lock_Base* /*external_mutex*/)
{
    return;
}

///绝对时间超时的的等待，超时后解锁
bool Condition_Base::systime_wait(zce::Lock_Base* /*external_mutex*/, const zce::Time_Value& /*abs_time*/)
{
    return true;
}

///相对时间的超时锁定等待，超时后，解锁
bool Condition_Base::duration_wait(zce::Lock_Base* /*external_mutex*/, const zce::Time_Value&/*relative_time*/)
{
    return false;
}

/// 给一个等待线程发送信号 Signal one waiting thread.
void Condition_Base::signal(void)
{
    return;
}

///给所有的等待线程广播信号 Signal *all* waiting threads.
void Condition_Base::broadcast(void)
{
    return;
}
}