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

#ifndef ZCE_LIB_LOCK_THREAD_SPIN_H_
#define ZCE_LIB_LOCK_THREAD_SPIN_H_

#include "zce/lock/lock_base.h"
#include "zce/lock/lock_guard.h"
#include "zce/time/time_value.h"

/*!
* @brief      SPIN自旋锁，线程互斥锁，
*             如果你就是为了保护一段代码，某个数据。需求类似临界区，可以使用
*             这个东东，
*             Windows下是用临界区+SPIN实现的，所以嘛，同时注意临界区是可递归的
*             LINUX下用的就是SPIN 锁
*/
class ZCE_Thread_Spin_Mutex: public ZCE_Lock_Base
{
public:
    ///线程锁的GUARD
    typedef ZCE_Lock_Guard<ZCE_Thread_Spin_Mutex> LOCK_GUARD;

public:

    //构造函数
    ZCE_Thread_Spin_Mutex();
    ///析构函数
    virtual ~ZCE_Thread_Spin_Mutex(void);

    ///锁定
    virtual void lock() override;

    ///尝试锁定
    virtual bool try_lock() override;

    ///解锁,
    virtual void unlock() override;

    ///取出内部的锁的指针
    pthread_spinlock_t *get_lock();

protected:

    ///SPIN线程锁
    pthread_spinlock_t  lock_;
};

#endif //ZCE_LIB_LOCK_THREAD_SPIN_H_
