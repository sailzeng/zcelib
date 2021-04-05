/**
* @copyright 2004-2012  Apache License, Version 2.0 FULLSAIL
* @filename  zce/thread/thread_task.h
* @author    Sailzeng <sailerzeng@gmail.com>
* @version
* @date      2011年6月18日
* @brief     跨平台的线程对象封装，为了封装的的方便，这个实现是一个线程一个对象，
*            线程可以用个一个GROUP ID再次包装一下
*
* @details   内部封装用的是自己OS层的pthread_XXX的函数，一样可以join和detach
*            我参考了ACE的实现，但我实在不明确ACE为啥要实现在一个TASK类里面启动
             多个线程有啥好处，而且为了这个将代码写的晦涩了很多
*
*
* @note
*
*/

#ifndef ZCE_LIB_THREAD_TASK_H_
#define ZCE_LIB_THREAD_TASK_H_

#include "zce/util/non_copyable.h"

/*!
* @brief      用自己封装的pthread函数构建的TASK类型，每个线程一个对象
*
* @note       对象不可拷贝复制，
*/
class ZCE_Thread_Task : public ZCE_NON_Copyable
{

public:

    ///构造函数
    ZCE_Thread_Task();
    ///析构函数
    virtual ~ZCE_Thread_Task();

public:

    /*!
    * @brief      激活一个线程，激活后，线程开始运行
    * @return     int ==0标识成功，非0失败
    * @param[in]  group_id 线程管理器可以对相同GROUP ID的线程进行一些操作
    * @param[out] threadid 返回的线程ID，
    * @param[in]  detachstate 产生分离的线程还是JOIN的线程 PTHREAD_CREATE_DETACHED or PTHREAD_CREATE_JOINABLE
    * @param[in]  stacksize 堆栈大小 为0表示默认，如果你需要使用很多线程，清调整这个大小，WIN 一般是1M，LINUX一般是10M(8M)
    * @param[in]  threadpriority 优先级，为0表示默认
    * @note
    */
    int activate(int group_id,
                 ZCE_THREAD_ID *threadid,
                 int detachstate = PTHREAD_CREATE_JOINABLE,
                 size_t stacksize = 0,
                 int threadpriority = 0);

    ///线程结束后的返回值int 类型
    int thread_return();

    ///得到group id
    int group_id() const;

    ///得到这个线程对象关联的线程ID
    ZCE_THREAD_ID thread_id() const;

    ///分离，不再进行绑定
    int detach();

    ///等待线程退出后join
    int wait_join();

    ///线程让出CPU的时间
    int yield();

protected:

    ///需要继承的处理的函数,理论上重载这一个函数就OK
    virtual int svc (void);

protected:

    ///静态函数，也就是要执行的函数，里面调用svc
    static void svc_run (void *args);

public:

    ///无效的组ID
    static const int INVALID_GROUP_ID = -1;

protected:

    ///线程的GROUP ID,
    int                     group_id_;

    ///线程的ID
    ZCE_THREAD_ID           thread_id_;

    ///线程的返回值
    int                     thread_return_;

};

#endif //#ifndef ZCE_LIB_THREAD_TASK_H_

