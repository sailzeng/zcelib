/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/thread_spin.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年1月31日
* @brief      SPIN 自旋锁的封装，
*             WINDOWS下用SPIN + 临界区封装
*             LINUX下用SPIN MUTEX封装
*
* @details    如果就是一些临界区保护，可以用这个封装实验一下，
*             大部分性能数据，都表示大部分测试结果的反馈是还不错的。
*
*             SPIN 在冲撞特别多的情况，使用要当心一点。
*             WINDOPWS 的实现可能更加优雅一点，他会在SPIN达到一定的数量的情况下
*             进入等待，
*
*/

#pragma once

#include "zce/lock/lock_base.h"
#include "zce/lock/lock_guard.h"
#include "zce/time/time_value.h"

namespace zce
{
/*!
* @brief      SPIN自旋锁，线程互斥锁，
*             如果你就是为了保护一段代码，某个数据。需求类似临界区，可以使用
*             这个东东，
*             Windows下是用临界区+SPIN实现的，所以嘛，同时注意临界区是可递归的
*             LINUX下用的就是SPIN 锁
*/
class thread_spin_mutex : public zce::lock_base
{
public:
    ///线程锁的GUARD
    typedef zce::lock_guard<zce::thread_spin_mutex> LOCK_GUARD;

public:

    //构造函数
    thread_spin_mutex() noexcept;
    ///析构函数
    ~thread_spin_mutex(void);

    ///锁定
    void lock()noexcept;

    ///尝试锁定
    bool try_lock()noexcept;

    ///解锁,
    void unlock()noexcept;

    ///取出内部的锁的指针
    pthread_spinlock_t* get_lock();

protected:

    ///SPIN线程锁
    pthread_spinlock_t  lock_;
};
}
