/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/thread_semaphore.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年1月14日
* @brief      线程的信号灯封装
*/

#pragma once

#include "zce/lock/lock_base.h"
#include "zce/lock/lock_guard.h"
#include "zce/time/time_value.h"

namespace zce
{
/*!
@brief      线程的信号灯封装，使用的是无名的信号灯，
*/
class thread_semaphore
{
public:
    //线程锁的GUARD
    typedef zce::lock_guard<thread_semaphore> LOCK_GUARD;

public:

    //构造函数,默认创建匿名信号灯，线程下一般用匿名信号灯就足够了,sem_name一般用nullptr就足够了
    //匿名信号灯==无名信号灯
    thread_semaphore(unsigned int init_value);
    ~thread_semaphore(void);

    //锁定
    void acquire() noexcept;

    //尝试锁定
    bool try_acquire() noexcept;

    //解锁,
    void release() noexcept;

    //绝对时间超时的的锁定，超时后解锁
    bool try_acquire_until(const zce::time_value& abs_time) noexcept;

    //相对时间的超时锁定，超时后，解锁
    bool try_acquire_for(const zce::time_value& relative_time) noexcept;

protected:
    //线程锁
    sem_t* lock_;
};
}
