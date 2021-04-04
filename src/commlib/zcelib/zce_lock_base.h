/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_base.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年9月13日
* @brief      所有锁类(记录锁除外)的基础类，让你也有多态使用（改变）加锁行为的方式
*             为了纪念我用ACE这么多年，我还是倾向用ACE的一些方式解决问题。
*             虽然我也发现我还是会学习一些BOOST的特性
*
*             请参考
*             http://www.cnblogs.com/fullsail/archive/2012/07/31/2616106.html
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_LOCK_BASE_H_
#define ZCE_LIB_LOCK_BASE_H_

#include "zce_lock_guard.h"
#include "zce_boost_non_copyable.h"

class ZCE_Time_Value;

/*!
* @brief      所有锁类(记录锁除外)的基础类，让你也有多态使用（改变）加锁行为的方式
*             整体的接口类似于BOOST的接口，比如不控制返回值，也参考过一些ACE的代码
* @detail     考虑了很久，最后还是放弃了偷懒，这个类和ZCE_Null_Mutex功能非常接近，表面看可以重用
*             但ZCE_Lock_Base内部的成员，都应该是private的，而ZCE_Null_Mutex的内部成员都应该是
*             public，这个矛盾是不可调和的
*/
class ZCE_Lock_Base : public ZCE_NON_Copyable
{

protected:
    ///构造函数和析构函数，允许析构，不允许构造的写法
    ZCE_Lock_Base (const char * = NULL);
public:
    ///析构函数
    virtual ~ZCE_Lock_Base (void);

    ///允许ZCE_Lock_Ptr_Guard使用一些函数
    friend class ZCE_Lock_Ptr_Guard;

    //为了避免其他人的使用，特此将这些函数隐藏起来
private:
    ///锁定
    virtual void lock();

    ///尝试锁定
    virtual bool try_lock();

    ///解锁,
    virtual void unlock();

    ///解锁读
    virtual void unlock_read();

    ///解锁写
    virtual void unlock_write();

    /*!
    * @brief      绝对时间超时的的锁定，超时后解锁，返回是否超时
    * @return     virtual bool
    * @param      abs_time
    * @note
    */
    virtual bool systime_lock(const ZCE_Time_Value &abs_time);

    /*!
    * @brief      获得锁，等待一个相对时间
    * @return     bool          返回true成功获取锁，false失败，（超时等）
    * @param      relative_time 等待的相对时间，
    */
    virtual bool duration_lock(const ZCE_Time_Value &relative_time );

    ///读取锁
    virtual void lock_read();
    ///尝试读取锁
    virtual bool try_lock_read();

    ///绝对时间,获取读取（共享）锁的，等待至绝对时间超时
    ///@param  abs_time 绝对时间
    virtual bool timed_lock_read(const ZCE_Time_Value & /*abs_time*/);
    ///相对时间,获取读取（共享）锁的，等待至相对时间超时
    ///@param  relative_time 相对时间
    virtual bool duration_lock_read(const ZCE_Time_Value & /*relative_time*/);

    ///写锁定
    virtual void lock_write();
    /*!
    * @brief      尝试读取锁
    * @return     bool  返回true成功获取锁，false失败，
    */
    virtual bool try_lock_write();

    /*!
    * @brief      绝对时间,获取写入（独占）锁的，等待至绝对时间超时
    * @return     bool     返回true成功获取锁，false失败，
    * @param      abs_time 绝对时间
    * @note
    */
    virtual bool timed_lock_write(const ZCE_Time_Value &abs_time);

    /*!
    * @brief      相对时间,获取写入（独占）锁的，等待至相对时间超时
    * @return     bool          返回true成功获取锁，false失败，
    * @param      relative_time 相对时间
    */
    virtual bool duration_lock_write(const ZCE_Time_Value &relative_time);

};

/*!
* @brief      CV,Condition Variable 条件变量的基类，条件变量的
*             扩展应该都是从这个基类扩展
*
*/
class ZCE_Condition_Base : public ZCE_NON_Copyable
{

protected:
    ///构造函数,protected，允许析构，不允许构造的写法
    ZCE_Condition_Base();
public:
    ///析构函数，
    virtual ~ZCE_Condition_Base();

    //为了避免其他人的使用，特此将这些函数隐藏起来
private:

    ///等待,
    virtual void wait (ZCE_Lock_Base *external_mutex );

    ///绝对时间超时的的等待，超时后解锁
    virtual bool systime_wait(ZCE_Lock_Base *external_mutex, const ZCE_Time_Value &abs_time);

    ///相对时间的超时锁定等待，超时后，解锁
    virtual bool duration_wait(ZCE_Lock_Base *external_mutex, const ZCE_Time_Value &relative_time);

    /// 给一个等待线程发送信号 Signal one waiting thread.
    virtual void signal (void);

    ///给所有的等待线程广播信号 Signal *all* waiting threads.
    virtual void broadcast (void);

};

#endif //ZCE_LIB_LOCK_BASE_H_

