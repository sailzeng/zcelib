/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_process_semaphore.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2013年1月15日
* @brief      进程间同步用的信号灯，
*
*/
#ifndef ZCE_LIB_LOCK_PROCESS_SEMAPHORE_H_
#define ZCE_LIB_LOCK_PROCESS_SEMAPHORE_H_

#include "zce_lock_base.h"
#include "zce_lock_guard.h"

/*!
* @brief      进程的信号灯类
*             LINUX下用有名信号灯，
*             WINDOWS下用带名称的信号灯，
*/
class ZCE_Process_Semaphore  : public ZCE_Lock_Base
{

public:

    ///线程锁的GUARD
    typedef ZCE_Lock_Guard<ZCE_Process_Semaphore> LOCK_GUARD;

public:

    /*!
    * @brief      构造函数,默认创建匿名信号灯，
    * @param      init_value  信号灯初始化的值
    * @param      sem_name    信号灯的名称
    */
    ZCE_Process_Semaphore (unsigned int init_value,
                           const char *sem_name
                          );
    virtual ~ZCE_Process_Semaphore (void);

    ///锁定
    virtual void lock();

    ///尝试锁定
    virtual bool try_lock();

    //解锁,
    virtual void unlock();

    /*!
    * @brief      绝对时间超时的的锁定，超时后解锁
    * @return     bool      true获得了锁，false，无法获得锁
    * @param      abs_time  超时的绝对时间
    */
    virtual bool systime_lock(const ZCE_Time_Value &abs_time);

    /*!
    * @brief      相对时间的超时锁定，超时后，解锁
    * @return     bool           true获得了锁，false，无法获得锁
    * @param      relative_time  超时的绝对时间
    */
    virtual bool duration_lock(const ZCE_Time_Value &relative_time);

protected:

    ///信号灯对象
    sem_t            *lock_;

    ///信号灯的名称，不得不记录下来，或者是共享内存的文件名称
    char              sema_name_[PATH_MAX + 1];

};

#endif //ZCE_LIB_LOCK_PROCESS_SEMAPHORE_H_

