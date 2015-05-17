#include "zce_predefine.h"
#include "zce_time_value.h"
#include "zce_lock_null_lock.h"

/************************************************************************************************************
Class           : ZCE_Null_Mutex
************************************************************************************************************/
//构造函数和析构函数
ZCE_Null_Mutex::ZCE_Null_Mutex(const char * ):
    lock_(0)
{
}

ZCE_Null_Mutex::~ZCE_Null_Mutex (void)
{
}

//锁定
void ZCE_Null_Mutex::lock()
{
    return;
}

//尝试锁定
bool ZCE_Null_Mutex::try_lock()
{
    return true;
}

//解锁,
void ZCE_Null_Mutex::unlock()
{
    return;
}

//绝对时间超时的的锁定，超时后解锁，返回是否超时
bool ZCE_Null_Mutex::systime_lock(const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

//相对时间，返回是否超时
bool ZCE_Null_Mutex::duration_lock(const ZCE_Time_Value & /*relative_time*/ )
{
    return true;
}

//读取锁
void ZCE_Null_Mutex::lock_read()
{
    return;
}

//尝试读取锁
bool ZCE_Null_Mutex::try_lock_read()
{
    return true;
}

//绝对时间
bool ZCE_Null_Mutex::timed_lock_read(const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

//相对时间
bool ZCE_Null_Mutex::duration_lock_read(const ZCE_Time_Value & /*relative_time*/)
{
    return true;
}

//写锁定
void ZCE_Null_Mutex::lock_write()
{
    return;
}

//尝试读取锁
bool ZCE_Null_Mutex::try_lock_write()
{
    return true;
}

//写锁定超时
bool ZCE_Null_Mutex::timed_lock_write(const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

//相对时间
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

///等待
void ZCE_Null_Condition::wait(ZCE_Null_Mutex * /*external_mutex*/ )
{
    return ;
}

///绝对时间超时的的等待，超时后解锁
bool ZCE_Null_Condition::systime_wait(ZCE_Null_Mutex * /*external_mutex*/ , const ZCE_Time_Value & /*abs_time*/)
{
    return true;
}

///相对时间的超时锁定等待，超时后，解锁
bool ZCE_Null_Condition::duration_wait(ZCE_Null_Mutex * /*external_mutex*/, const ZCE_Time_Value &/*relative_time*/)
{
    return false;
}

/// 给一个等待线程发送信号 Signal one waiting thread.
void ZCE_Null_Condition::signal (void)
{
    return;
}

///给所有的等待线程广播信号 Signal *all* waiting threads.
void ZCE_Null_Condition::broadcast (void)
{
    return;
}

