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
*
* 曾经我也想过一了百了  (中岛美嘉)
* 就因为看着海鸥在码头上悲鸣
* 随波逐流浮沉的海鸟啊
* 也将我的过去啄食 展翅飞去吧
* 曾经我也想过一了百了
* 因为生日那天杏花绽放
* 在那筛落阳光的树荫下小睡
* 大概就会像未能转生的虫 就此适应于土里长眠了吧
* 薄荷糖 渔港的灯塔
* 生锈的拱桥 被丢弃的自行车
* 杵立在木造车站的暖炉前
* 心却哪儿都不能就此启程
* 今日和昨日相同
* 想要更好的明天 今天就须有所行动
* 我知道 我都知道 但是
* 曾经我也想过一了百了
* 因为心早就被掏空
* 心不能被填满的哭泣着
* 因为我仍渴望着什麼
* 曾经我也想过一了百了
* 因为那松开的鞋带
* 我无法好好将它系紧
* 如同不懂得系紧某人一般
* 曾经我也想过一了百了
* 因为少年凝视着我
* 跪着在床上谢罪吧
* 向过去的我说声抱歉
* 电脑透出淡淡的光
* 楼上房间传来的动静
* 门口对讲机的声音
* 困在鸟笼中的少年捂住耳朵
* 与无形的敌人战斗着
* 他是三坪房间里的唐吉诃德
* 最后的结局 却是抖丑陋不堪
* 曾经我也想过一了百了
* 因为有人说我是冷漠的人
* 想要被爱的哭泣着
* 是因为终于尝到人间温暖
* 曾经我也想过一了百了
* 你美丽的笑着
* 满脑子想着自我了结
* 终究因为活着这事太过于刻骨
* 曾经我也想过一了百了
* 我还没有遇见你
* 因为有像你一样的人存在
* 我稍稍喜欢上这个世界了
* 因为有像你一样的人存在
* 我开始稍稍期待着这个世界
*/

#pragma once

#include "zce/os_adapt/thread.h"

namespace zce
{
//!需要继承的处理的函数,理论上重载这一个函数就OK

/*!
* @brief      用自己封装的pthread函数构建的TASK类型，每个线程一个对象
*             为什么保留这个类，这个类实现的线程可以设置堆栈大小，
* @note       对象不可拷贝复制，只能移动拷贝
*/
class thread_task
{
public:

    ///构造函数,析构函数
    thread_task() = default;
    ~thread_task() = default;

    thread_task(const thread_task &) = delete;
    thread_task& operator=(const thread_task&) = delete;

    //有移动构造
    thread_task(thread_task &&);
    thread_task& operator=(thread_task&&);

protected:

    //_invoker_helper 是一个辅助的调用类，因为activate有不同的函数，
    //必须进行不同的适配
    template <class Fn>
    class _invoker_helper
    {
    public:
        _invoker_helper() = default;
        ~_invoker_helper() = default;

        int operator()(Fn &func,
                       int detachstate,
                       size_t stacksize,
                       ZCE_THREAD_ID *threadid)
        {
            int ret = zce::pthread_createex(_invoker_helper::svc_fuc,
                                            (void *)(&func),
                                            threadid,
                                            detachstate,
                                            stacksize,
                                            0);
            return ret;
        }

        static void svc_fuc(void *vfunc)
        {
            Fn *func = (Fn *)(vfunc);
            (*func)();
            zce::pthread_exit();
        }
    };

public:

    //!激活一个线程,根据args参数，执行fp函数，
    template <class Call, class... Args >
    int activate(Call &&fp, Args&&... args)
    {
        int ret = 0;
        auto svc_func =
            std::bind(std::forward<Call>(fp), std::forward<Args>(args)...);
        group_id_ = INVALID_GROUP_ID;

        thread_task::__invoker_helper<decltype(svc_func)>(svc_func,
                                                          PTHREAD_CREATE_JOINABLE,
                                                          0,
                                                          &thread_id_,
                                                          &ret);
        return ret;
    }

    //!激活一个线程,根据args参数，执行fp函数，
    //! detachstate 为PTHREAD_CREATE_DETACHED or PTHREAD_CREATE_JOINABLE
    template <class Call, class... Args >
    int activate(int detachstate,
                 Call &&fp,
                 Args&&... args)
    {
        int ret = 0;
        auto svc_func =
            std::bind(std::forward<Call>(fp), std::forward<Args>(args)...);
        group_id_ = INVALID_GROUP_ID;
        thread_task::__invoker_helper<decltype(svc_func)>(svc_func,
                                                          detachstate,
                                                          0,
                                                          &thread_id_,
                                                          &ret);
        return ret;
    }

    /*!
    * @brief      激活一个线程，激活后，线程开始运行
    * @return     int ==0标识成功，非0失败
    * @param[in]  group_id 线程管理器可以对相同GROUP ID的线程进行一些操作
    * @param[out] threadid 返回的线程ID，
    * @param[in]  detachstate 产生分离的线程还是JOIN的线程 PTHREAD_CREATE_DETACHED or PTHREAD_CREATE_JOINABLE
    * @param[in]  stacksize 堆栈大小 为0表示默认，如果你需要使用很多线程，清调整这个大小，WIN 一般是1M，LINUX一般是10M(8M)
    * @note       线程优先级这类，我暂时没有实现，各种平台差异很大
    */
    template <class Call, class... Args >
    int activate(int group_id,
                 int detachstate,
                 size_t stacksize,
                 Call &&fp,
                 Args&&... args)
    {
        int ret = 0;
        auto svc_func =
            std::bind(std::forward<Call>(fp), std::forward<Args>(args)...);
        group_id_ = group_id;
        thread_task::__invoker_helper<decltype(svc_func)>(svc_func,
                                                          detachstate,
                                                          stacksize,
                                                          &thread_id_,
                                                          &ret);
        return ret;
    }

    //!@note 说明下一下，写activate曾经想用模板函数直接完成，但是没有找到
    //!      调用一个static 模板函数指针的方法，

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

    template <class Fn>
    static void __invoker_helper(Fn &func,
                                 int detachstate,
                                 size_t stacksize,
                                 ZCE_THREAD_ID *threadid,
                                 int *int_ret)
    {
        //注意这儿，注意这儿，static 函数的模板函数调用，要加template
        *int_ret = zce::pthread_createex(thread_task::template __svc_fuc<Fn>,
                                         (void *)(&func),
                                         threadid,
                                         detachstate,
                                         stacksize,
                                         0);
    }

    //包装的线程执行函数
    template <class Fn>
    static void __svc_fuc(void *vfunc)
    {
        Fn *func = (Fn *)(vfunc);
        (*func)();
        zce::pthread_exit();
    }

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
