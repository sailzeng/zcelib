#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/define.h"
#include "zce/os_adapt/thread.h"

//----------------------------------------------------------------------------------------
namespace zce
{
int pthread_attr_init(pthread_attr_t* attr)
{
    //我暂时只关注这几个变量
#if defined (ZCE_OS_WINDOWS)
    attr->detachstate = PTHREAD_CREATE_JOINABLE;
    attr->inheritsched = PTHREAD_INHERIT_SCHED;
    attr->schedparam.sched_priority = 0;
    attr->stacksize = 0;
    return 0;

#elif defined (ZCE_OS_LINUX)

    return ::pthread_attr_init(attr);
#endif
}

//
int zce::pthread_attr_destroy(pthread_attr_t* attr)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(attr);
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_attr_destroy(attr);
#endif
}

//设置，或者获得线程属性变量属性
//你可以设置，线程的的分离，JOIN属性，堆栈大小，线程的调度优先级
int zce::pthread_attr_getex(const pthread_attr_t* attr,
                            int* detachstate,
                            size_t* stacksize,
                            int* threadpriority
)
{
#if defined (ZCE_OS_WINDOWS)

    *detachstate = attr->detachstate;
    *threadpriority = attr->schedparam.sched_priority;
    *stacksize = attr->stacksize;
    return 0;

#elif defined (ZCE_OS_LINUX)
    int ret = 0;
    ret = ::pthread_attr_getdetachstate(attr, detachstate);

    if (ret != 0)
    {
        return ret;
    }

    sched_param param;
    ret = ::pthread_attr_getschedparam(attr, &param);

    if (ret != 0)
    {
        return ret;
    }

    *threadpriority = param.sched_priority;
    ret = ::pthread_attr_getstacksize(attr, stacksize);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
#endif
}

int zce::pthread_attr_setex(pthread_attr_t* attr,
                            int detachstate,
                            size_t stacksize,
                            int policy,
                            int priority)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_ASSERT(PTHREAD_CREATE_JOINABLE == detachstate || PTHREAD_CREATE_DETACHED == detachstate);
    attr->detachstate = detachstate;
    attr->stacksize = stacksize;
    //修改调度策略继承方式
    attr->inheritsched = PTHREAD_EXPLICIT_SCHED;
    attr->schedparam.sched_priority = priority;
    //Windows 下调度策略没有用
    attr->schedpolicy = policy;
    return 0;

#elif defined (ZCE_OS_LINUX)
    int ret = 0;
    ret = ::pthread_attr_setdetachstate(attr, detachstate);

    if (ret != 0)
    {
        return ret;
    }

    if (policy != 0)
    {
        int ret = pthread_attr_setschedpolicy(attr, policy);
        if (ret != 0)
        {
            return ret;
        }
    }

    //如果有线程优先级，设置
    if (priority > 0)
    {
        //注意这儿，要先设置是否遵守不从夫进程得到调度方案
        ret = ::pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
        if (ret != 0)
        {
            return ret;
        }
        int max_priority = 0, min_priority = 0;
        ret = ::sched_get_priority_max(max_priority);
        if (ret != 0)
        {
            return ret;
        }
        if (priority > max_priority)
        {
            priority = max_priority;
        }
        ret = ::sched_get_priority_min(min_priority);
        if (ret != 0)
        {
            return ret;
        }
        if (priority < min_priority)
        {
            priority = min_priority;
        }
        sched_param param;
        param.sched_priority = priority;
        ret = ::pthread_attr_setschedparam(attr, &param);
        if (ret != 0)
        {
            return ret;
        }
    }

    // 加个判断如果小于最小值，则赋为最小值
    if (stacksize < (size_t)PTHREAD_STACK_MIN)
    {
        stacksize = PTHREAD_STACK_MIN;
    }

    ret = ::pthread_attr_setstacksize(attr, stacksize);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
#endif //#if defined (ZCE_OS_LINUX)
}

//==========================================================================================
//注意，WINDOWS和LINUX下返回值不同，所以我非常非常不建议你用这个传递返回值，
//WINDOWS 可以传递unsigned int ，LINUX传递void *，

#if defined (ZCE_OS_WINDOWS)

class WIN_STARTFUN_ADAPT
{
public:
    //构造函数和析构函数
    WIN_STARTFUN_ADAPT(void *(*start_routine)(void*), void* arg) :
        start_routine_(start_routine),
        arg_(arg)
    {
    }

    ~WIN_STARTFUN_ADAPT()
    {
    }

    //Windows 下必须使用__stdcall
    static unsigned int __stdcall adapt_svc_run(void* adapt_svc)
    {
        WIN_STARTFUN_ADAPT* my_adapt = static_cast<WIN_STARTFUN_ADAPT*>(adapt_svc);
        my_adapt->start_routine_(my_adapt->arg_);
        return 0;
    }
protected:
    typedef void * (*start_routine_fun)(void*);

    //线程启动函数
    start_routine_fun start_routine_;
    //线程启动参数
    void* arg_;
};

#endif //

//创建一个线程,调用线程函数向各个平台兼容模式靠齐，有返回值
int zce::pthread_create(ZCE_THREAD_ID* threadid,
                        const pthread_attr_t* attr,
                        void *(*start_routine)(void*),
                        void* arg)
{
#if defined (ZCE_OS_WINDOWS)

    WIN_STARTFUN_ADAPT adapt_object(start_routine, arg);
    //用CRT的线程创建函数创建线程
    HANDLE thread_handle = (HANDLE)::_beginthreadex(nullptr,
                                                    static_cast<unsigned int>(attr->stacksize),
                                                    WIN_STARTFUN_ADAPT::adapt_svc_run,
                                                    &adapt_object,
                                                    0,
                                                    threadid);
    //注意_beginthreadex的返回值0表示错误，和_beginthread不一样
    if (nullptr == thread_handle)
    {
        return -1;
    }

    //设置线程优先级
    if (PTHREAD_EXPLICIT_SCHED == attr->inheritsched &&
        0 != attr->schedparam.sched_priority)
    {
        //线程相对优先级（取值对应如下）
        BOOL bret = ::SetThreadPriority(thread_handle,
                                        attr->schedparam.sched_priority);

        if (!bret)
        {
            //是否记录一下，呵呵
        }
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_create(threadid,
                            attr,
                            start_routine,
                            arg);

#endif //#if defined (ZCE_OS_LINUX)
}

//这个不是POSIX的封装，但推荐使用
//还是提供一个简单一点的封装吧，这个不用处理pthread_attr_t
int zce::pthread_createex(void* (*start_routine)(void*),
                          void* arg,
                          ZCE_THREAD_ID* threadid,
                          int detachstate,
                          size_t stacksize,
                          int policy,
                          int priority)
{
    int ret = 0;
    pthread_attr_t attr;
    ret = zce::pthread_attr_init(&attr);
    if (0 != ret)
    {
        return ret;
    }

    //设置线程参数
    ret = zce::pthread_attr_setex(&attr,
                                  detachstate,
                                  stacksize,
                                  policy,
                                  priority);

    if (0 != ret)
    {
        zce::pthread_attr_destroy(&attr);
        return ret;
    }

    //创建线程
    ret = zce::pthread_create(threadid,
                              &attr,
                              start_routine,
                              arg);

    if (0 != ret)
    {
        zce::pthread_attr_destroy(&attr);
        return ret;
    }

    return 0;
}

//退出，传递void，各个平台不兼容，所以干脆什么都不传递出来
void zce::pthread_exit(void *return_data)
{
#if defined (ZCE_OS_WINDOWS)

    unsigned int return_ret = 0;
    _endthreadex(return_ret);
    return_data = nullptr;
    return;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_exit(return_data);
#endif //#if defined (ZCE_OS_LINUX)
}

//等待某个JOIN的线程结束,并且得到线程回调函数的返回值
int zce::pthread_join(ZCE_THREAD_ID threadid, void* ret_val)
{
#if defined (ZCE_OS_WINDOWS)

    //OpenThread是一个WIN SERVER 2000后才有的函数 VC6应该没有
    HANDLE thr_handle = (HANDLE)::OpenThread(THREAD_ALL_ACCESS,
                                             FALSE,
                                             threadid);
    if (thr_handle == nullptr)
    {
        return -1;
    }
    DWORD thread_ret;
    ret_val = nullptr;
    if (::WaitForSingleObject(thr_handle, INFINITE) == WAIT_OBJECT_0
        && ::GetExitCodeThread(thr_handle, &thread_ret) != FALSE)
    {
        //此处不要关闭handle（  ::CloseHandle (thr_handle);），
        //因为我们调用的是_endthreadex 结束的线程
        return 0;
    }

    return -1;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::pthread_join(threadid, ret_val);
#endif //#if defined (ZCE_OS_LINUX)
}

//得到线程ID
ZCE_THREAD_ID zce::pthread_self(void)
{
#if defined (ZCE_OS_WINDOWS)
    return ::GetCurrentThreadId();

#elif defined (ZCE_OS_LINUX)
    return ::pthread_self();
#endif
}

//取消一个线程
//这个函数放在这儿，完全是为了满足我的一点点封装意愿，因为在LINUX平台，你未必能取消一个线程
//在外WINDOWS，调用TerminateThread，这个函数也是不被推荐的
int zce::pthread_cancel(ZCE_THREAD_ID threadid)
{
#if defined (ZCE_OS_WINDOWS)

#pragma warning (push)
#pragma warning (disable:6258)
    //OpenThread是一个WIN SERVER 2000后才有的函数 VC6应该没有
    HANDLE thr_handle = (HANDLE)::OpenThread(THREAD_ALL_ACCESS,
                                             FALSE,
                                             threadid);
    if (thr_handle == nullptr)
    {
        errno = GetLastError();
        return -1;
    }
    //强制退出
    BOOL bret = ::TerminateThread(thr_handle, 0);
    if (!bret)
    {
        return -1;
    }

    ::CloseHandle(thr_handle);
    return 0;

#pragma warning (pop)

#elif defined (ZCE_OS_LINUX)
    return ::pthread_cancel(threadid);
#endif
}

//对一个线程进行松绑
int zce::pthread_detach(ZCE_THREAD_ID threadid)
{
#if defined (ZCE_OS_WINDOWS)
    //Windows线程本来就是detach的，呵呵
    ZCE_UNUSED_ARG(threadid);
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_detach(threadid);
#endif
}

//将线程ID转换为HANDLE，只在WIN2000下有用
#if defined ZCE_OS_WINDOWS
ZCE_THREAD_HANDLE pthread_id2handle(ZCE_THREAD_ID threadid)
{
    //OpenThread是一个WIN SERVER 2000后才有的函数 VC6应该没有
    HANDLE thr_handle = (HANDLE)::OpenThread(THREAD_ALL_ACCESS,
                                             FALSE,
                                             threadid
    );
    return thr_handle;
}
#endif

int pthread_yield(void)
{
#if defined (ZCE_OS_WINDOWS)
    ::Sleep(0);
    return 0;

#elif defined (ZCE_OS_LINUX)
    //return ::pthread_yield();
    return sched_yield();
#endif
}

//=================================================================================================================

//destructor 参数对于WIN32平台没有用，建议不用。
//
int pthread_key_create(pthread_key_t* key, void (*destructor)(void*))
{
#if defined (ZCE_OS_WINDOWS)

    ZCE_UNUSED_ARG(destructor);
    *key = ::TlsAlloc();
    //如果返回FALSE标识失败
    if (TLS_OUT_OF_INDEXES == *key)
    {
        errno = GetLastError();
        return -1;
    }
    return 0;

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_key_create(key, destructor);
#endif
}

//
int pthread_key_delete(pthread_key_t key)
{
#if defined (ZCE_OS_WINDOWS)

    BOOL bool_ret = ::TlsFree(key);
    //如果返回FALSE标识失败
    if (!bool_ret)
    {
        errno = GetLastError();
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_key_delete(key);
#endif
}

int pthread_setspecific(pthread_key_t key, const void* data)
{
#if defined (ZCE_OS_WINDOWS)

    BOOL bool_ret = ::TlsSetValue(key, (LPVOID)(data));
    //如果返回FALSE标识失败
    if (!bool_ret)
    {
        errno = GetLastError();
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_setspecific(key, data);
#endif
}

void* pthread_getspecific(pthread_key_t key)
{
#if defined (ZCE_OS_WINDOWS)

    LPVOID  data = ::TlsGetValue(key);
    if (data == 0 && ::GetLastError() != NO_ERROR)
    {
        errno = GetLastError();
        return nullptr;
    }
    else
    {
        return data;
    }

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_getspecific(key);
#endif
}
}