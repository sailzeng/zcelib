/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_thread_mutex.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年9月12日
* @brief      线程的互斥量封装，分成3种方式
*             线程的互斥量，有轻量的锁，递归锁，非递归锁，读写锁
*             ZCE_Thread_Light_Mutex  轻量级的线程锁，WINDODOWS下用临界区模拟，LINUX用pthread_mutex, 可递归，但是没有超时处理
*             ZCE_Thread_Recursive_Mutex 递归的线程锁 WINDODOWS下用MUTEX模拟，可递归，有超时处理
*             ZCE_Thread_NONR_Mutex 不可以递归的线程锁 WINDODOWS用信号灯模拟的MUTEX，不可以递归，有超时处理
*
*             如果是类似临界区的需求，而且希望更快，可以考虑SPIN LOCK，
*/

#ifndef ZCE_LIB_LOCK_THREAD_MUTEX_H_
#define ZCE_LIB_LOCK_THREAD_MUTEX_H_

#include "zce_lock_base.h"
#include "zce_lock_guard.h"
#include "zce_time_value.h"


class ZCE_Time_Value;


/*!
* @brief      轻量级的线程互斥锁，一般情况下推荐使用，轻主要体现在Windows平台下用临界区
*             Windows下是用临界区实现的，所以嘛，同时注意临界区是可递归的
*             因为临界区没有超时处理能录，这个封装也就不提供了。（临界区本来也不应该有超时）
*/
class ZCE_Thread_Light_Mutex : public ZCE_Lock_Base
{

public:
    ///线程锁的GUARD
    typedef ZCE_Lock_Guard<ZCE_Thread_Light_Mutex> LOCK_GUARD;

public:

    //构造函数
    ZCE_Thread_Light_Mutex ();
    ///析构函数
    virtual ~ZCE_Thread_Light_Mutex (void);

    ///锁定
    virtual void lock();

    ///尝试锁定
    virtual bool try_lock();

    ///解锁,
    virtual void unlock();

    ///取出内部的锁的指针
    pthread_mutex_t *get_lock();

protected:
    //线程锁
    pthread_mutex_t  lock_;


};



/*!
* @brief      线程递归锁，Windows下用Mutex，Linux用pthread_mutex
*             相对而言，推荐使用递归锁,
*/
class ZCE_Thread_Recursive_Mutex : public ZCE_Lock_Base
{
public:

    ///递归锁的GUARD
    typedef ZCE_Lock_Guard<ZCE_Thread_Recursive_Mutex> LOCK_GUARD;

public:

    ///构造函数
    ZCE_Thread_Recursive_Mutex ();
    virtual ~ZCE_Thread_Recursive_Mutex (void);

    ///锁定
    virtual void lock();

    ///尝试锁定
    virtual bool try_lock();

    ///解锁,
    virtual void unlock();


    ///绝对时间超时的的锁定，超时后解锁
    virtual bool systime_lock(const ZCE_Time_Value &abs_time);

    ///相对时间的超时锁定，超时后，解锁
    virtual bool duration_lock(const ZCE_Time_Value &relative_time);

    ///取出内部的锁的指针
    pthread_mutex_t *get_lock();

protected:
    ///线程锁
    pthread_mutex_t  lock_;


};


/*!
* @brief      线程的非递归锁，提供这个完全是为了满足一些爱好，因为递归锁本质上满足非递归的需求。
*             Windows下的互斥量和临界区其实是可以递归的，所以只用信号灯了模拟非递归锁
*             201209 pthread_mutex内部我已经合入了非递归锁功能，用信号灯模拟
*/
class ZCE_Thread_NONR_Mutex : public ZCE_Lock_Base
{



public:
    ///锁的GUARD
    typedef ZCE_Lock_Guard<ZCE_Thread_NONR_Mutex> LOCK_GUARD;

public:

    ///构造函数
    ZCE_Thread_NONR_Mutex ();
    ///析构函数
    virtual ~ZCE_Thread_NONR_Mutex (void);
public:
    ///锁定
    virtual void lock();

    ///尝试锁定
    virtual bool try_lock();

    ///解锁
    virtual void unlock();


    ///绝对时间超时的的锁定，超时后解锁
    virtual bool systime_lock(const ZCE_Time_Value &abs_time);

    ///相对时间的超时锁定，超时后，解锁
    virtual bool duration_lock(const ZCE_Time_Value &relative_time);

protected:


    ///线程锁,在Windows下我也已经模拟的非递归的锁
    pthread_mutex_t  lock_;



};



#endif //ZCE_LIB_LOCK_THREAD_MUTEX_H_

