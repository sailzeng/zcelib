/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/thread_condi.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年2月1日
* @brief
*
* @details
*
* @note
*
*/

#pragma once

#include "zce/lock/lock_base.h"
#include "zce/os_adapt/condi.h"

namespace zce
{
//线程的条件变量类,为了方便用了模版类，但请你直接用两个typedef
//!使用线程MUTEX
class thread_condition : public zce::condition_base
{
public:

    //!构造函数
    thread_condition();

    //!析构函数
    virtual ~thread_condition(void);

    //!等待
    virtual void wait(thread_light_mutex* external_mutex) noexcept;

    //!绝对时间超时的的等待，超时后解锁
    virtual bool wait_until(thread_light_mutex* external_mutex,
                            const zce::time_value& abs_time) noexcept;

    //!相对时间的超时锁定等待，超时后，解锁
    virtual bool wait_for(thread_light_mutex* external_mutex,
                          const zce::time_value& relative_time) noexcept;

    //!给一个等待线程发送信号 Signal one waiting thread.
    void notify_one(void) noexcept override;

    //!给所有的等待线程广播信号 Signal *all* waiting threads.
    void notify_all(void) noexcept override;

protected:

    //!条件变量对象
    pthread_cond_t    lock_;
};

//!使用可递归的MUTEX的类
class Thread_Recursive_Condition : public zce::condition_base
{
public:

    //!构造函数
    Thread_Recursive_Condition();
    //!析构函数
    virtual ~Thread_Recursive_Condition(void);

    //!等待
    virtual void wait(thread_recursive_mutex* external_mutex) noexcept;

    //!绝对时间超时的的等待，超时后解锁
    virtual bool wait_until(thread_recursive_mutex* external_mutex,
                            const zce::time_value& abs_time) noexcept;

    //!相对时间的超时锁定等待，超时后，解锁
    virtual bool wait_for(thread_recursive_mutex* external_mutex,
                          const zce::time_value& relative_time) noexcept;

    //!给一个等待线程发送信号 Signal one waiting thread.
    void notify_one(void) noexcept override;

    //!给所有的等待线程广播信号 Signal *all* waiting threads.
    void notify_all(void) noexcept override;

protected:

    //!条件变量对象
    pthread_cond_t    lock_;
};
}
