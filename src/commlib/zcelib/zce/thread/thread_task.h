/**
* @copyright 2004-2012  Apache License, Version 2.0 FULLSAIL
* @filename  zce/thread/thread_task.h
* @author    Sailzeng <sailzeng.cn@gmail.com>
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

#pragma once

namespace zce
{
/*!
* @brief      用自己封装的pthread函数构建的TASK类型，每个线程一个对象
*
* @note       对象不可拷贝复制，
*/
class thread_task
{
public:

    ///构造函数,析构函数
    thread_task() = default;
    ~thread_task() = default;

    thread_task(const thread_task &) = delete;
    thread_task& operator=(const thread_task&) = delete;
public:

    class thread_invoker
    {
    };

    template <class Fn, class... Args>
    static void thread_invoker(std::function<Fn(Args...)> &func,
                               int detachstate,
                               size_t stacksize,
                               ZCE_THREAD_ID *threadid,
                               int &ret_int)
    {
        ret_int = zce::pthread_createex(thread_task::svc_fuc,
                                        &func,
                                        threadid,
                                        PTHREAD_CREATE_JOINABLE,
                                        0,
                                        0);
    }

    template <class Fn, class... Args>
    static void svc_fuc(void *func)
    {
        std::function<Fn(Args...)> *func_call = (std::function<Fn(Args...)> *)func;
        func_call->();
    }

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
                 ZCE_THREAD_ID* threadid,
                 int detachstate = PTHREAD_CREATE_JOINABLE,
                 size_t stacksize = 0,
                 int threadpriority = 0);

    template <class Fn, class... Args>
    int activate(Fn&& fn, Args&&... args)
    {
        int ret = 0;
        std::function<Fn(Args...)> svc_func = std::bind(
            std::forward(fn), std::forward<Args>(args)...);
        thread_invoker(svc_func, &thread_id_, ret);
        group_id_ = INVALID_GROUP_ID;
        return ret;
    }

    template <class Fn, class... Args>
    int activate(int detachstate,
                 Fn&& fn,
                 Args&&... args)
    {
        int ret = 0;
        std::function<Fn(Args...)> svc_func = std::bind(
            std::forward(fn), std::forward<Args>(args)...);
        thread_invoker(svc_func, &thread_id_, ret);
        group_id_ = INVALID_GROUP_ID;
        return ret;
    }

    template <class Fn, class... Args>
    int activate(int detachstate,
                 size_t stacksize,
                 int threadpriority,
                 Fn&& fn,
                 Args&&... args)
    {
        int ret = 0;
        std::function<Fn(Args...)> svc_func = std::bind(
            std::forward(fn), std::forward<Args>(args)...);
        thread_invoker(svc_func, &thread_id_, ret);
        group_id_ = INVALID_GROUP_ID;
        return ret;
    }

    //!线程结束后的返回值int 类型
    int thread_return();

    //!得到group id
    int group_id() const;

    //!得到这个线程对象关联的线程ID
    ZCE_THREAD_ID thread_id() const;

    //!分离，不再进行绑定
    int detach();

    //!等待线程退出后join
    int wait_join();

    //!线程让出CPU的时间
    int yield();

protected:

    //!需要继承的处理的函数,理论上重载这一个函数就OK
    int svc_fuc(void);

protected:

    //!静态函数，也就是要执行的函数，里面调用svc
    static void svc_run(void* args);

public:

    //!无效的组ID
    static const int INVALID_GROUP_ID = -1;

protected:

    //!线程的GROUP ID,
    int                     group_id_ = INVALID_GROUP_ID;

    //!线程的ID
    ZCE_THREAD_ID           thread_id_ = 0;

    //!线程的返回值
    int                     thread_return_ = -1;
};

//========================================================================================

/*!
* @brief      线程的等待管理器
*
* @note       本来是在线程ZCE_Thread_Base 内部处理的，但是嵌入过多，而且用大量的static 变量，
*             也影响ZCE_Thread_Base的性能
*/
class thread_task_wait
{
protected:

    //记录需要等待的线程信息，通过开关使用
    struct MANAGE_WAIT_INFO
    {
    public:

        MANAGE_WAIT_INFO(ZCE_THREAD_ID wait_thr_id, int wait_group_id) :
            wait_thr_id_(wait_thr_id),
            wait_group_id_(wait_group_id)
        {
        }
        ~MANAGE_WAIT_INFO()
        {
        }
    public:
        //线程的ID
        ZCE_THREAD_ID     wait_thr_id_;
        //分组ID
        int               wait_group_id_;
    };

public:
    //构造函数，允许你拥有实例，但推荐你用单件处理
    thread_task_wait();
    ~thread_task_wait();

    thread_task_wait(const thread_task_wait &) = delete;
    thread_task_wait& operator=(const thread_task_wait&) = delete;

    //如果需要管理处理，要自己登记，
    void record_wait_thread(ZCE_THREAD_ID wait_thr_id, int wait_group_id = 0);
    //登记一个要进行等待处理等待线程
    void record_wait_thread(const zce::thread_task* wait_thr_task);

    //等所有的线程退出
    void wait_all();

    //
    void wait_group(int group_id);

public:

    //单子函数
    static thread_task_wait* instance();
    //清理单子的函数
    static void clear_inst();

protected:
    //用list管理，性能不是特别好，但考虑到要中间删除因素等等，忍了
    typedef std::list <MANAGE_WAIT_INFO>   MANAGE_WAIT_THREAD_LIST;

    //单子实例
    static thread_task_wait* instance_;

protected:

    //所有希望等待的线程记录
    MANAGE_WAIT_THREAD_LIST   wait_thread_list_;
};
}
