#include "zce/predefine.h"
#include "zce/time/time_value.h"
#include "zce/lock/null_lock.h"

/************************************************************************************************************
Class           : Null_Mutex
************************************************************************************************************/
namespace zce
{
//构造函数和析构函数
Null_Mutex::Null_Mutex(const char*) :
    lock_(0)
{
}

Null_Mutex::~Null_Mutex(void)
{
}

//锁定
void Null_Mutex::lock()
{
    return;
}

//尝试锁定
bool Null_Mutex::try_lock()
{
    return true;
}

//解锁,
void Null_Mutex::unlock()
{
    return;
}

//绝对时间超时的的锁定，超时后解锁，返回是否超时
bool Null_Mutex::lock(const zce::Time_Value& /*abs_time*/)
{
    return true;
}

//相对时间，返回是否超时
bool Null_Mutex::lock_for(const zce::Time_Value& /*relative_time*/)
{
    return true;
}

//读取锁
void Null_Mutex::lock_read()
{
    return;
}

//尝试读取锁
bool Null_Mutex::try_lock_read()
{
    return true;
}

//绝对时间
bool Null_Mutex::timed_lock_read(const zce::Time_Value& /*abs_time*/)
{
    return true;
}

//相对时间
bool Null_Mutex::duration_lock_read(const zce::Time_Value& /*relative_time*/)
{
    return true;
}

//写锁定
void Null_Mutex::lock_write()
{
    return;
}

//尝试读取锁
bool Null_Mutex::try_lock_write()
{
    return true;
}

//写锁定超时
bool Null_Mutex::timed_lock_write(const zce::Time_Value& /*abs_time*/)
{
    return true;
}

//相对时间
bool Null_Mutex::duration_lock_write(const zce::Time_Value& /*relative_time*/)
{
    return true;
}

/************************************************************************************************************
Class           : Null_Condition
************************************************************************************************************/
Null_Condition::Null_Condition() :
    lock_(0)
{
}

Null_Condition::~Null_Condition()
{
}

///等待
void Null_Condition::wait(zce::Null_Mutex* /*external_mutex*/)
{
    return;
}

///绝对时间超时的的等待，超时后解锁
bool Null_Condition::systime_wait(zce::Null_Mutex* /*external_mutex*/, const zce::Time_Value& /*abs_time*/)
{
    return true;
}

///相对时间的超时锁定等待，超时后，解锁
bool Null_Condition::duration_wait(zce::Null_Mutex* /*external_mutex*/, const zce::Time_Value&/*relative_time*/)
{
    return false;
}

/// 给一个等待线程发送信号 Signal one waiting thread.
void Null_Condition::signal(void)
{
    return;
}

///给所有的等待线程广播信号 Signal *all* waiting threads.
void Null_Condition::broadcast(void)
{
    return;
}
}