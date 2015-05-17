#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_thread.h"

//为什么不让我用ACE，卫生棉！，卫生棉！！！！！卫生棉卫生棉卫生棉！！！！！！！！

//----------------------------------------------------------------------------------------
//
int ZCE_LIB::pthread_attr_init(pthread_attr_t *attr)
{
    //我暂时只关注这几个变量
#if defined (ZCE_OS_WINDOWS)
    attr->detachstate = PTHREAD_CREATE_DETACHED;
    attr->inheritsched = PTHREAD_INHERIT_SCHED;
    attr->schedparam.sched_priority = 0;
    attr->stacksize = 0;
    return 0;

#elif defined (ZCE_OS_LINUX)

    return ::pthread_attr_init (attr);
#endif

}

//
int ZCE_LIB::pthread_attr_destroy(pthread_attr_t *attr )
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(attr);
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_attr_destroy (attr);
#endif

}

//设置，或者获得线程属性变量属性
//你可以设置，线程的的分离，JOIN属性，堆栈大小，线程的调度优先级
int ZCE_LIB::pthread_attr_getex(const pthread_attr_t *attr,
                                int *detachstate,
                                size_t *stacksize,
                                int *threadpriority
                               )
{
#if defined (ZCE_OS_WINDOWS)

    *detachstate = attr->detachstate;
    *threadpriority = attr->schedparam.sched_priority;
    *stacksize = attr->stacksize;
    return 0;

#elif defined (ZCE_OS_LINUX)
    int ret = 0;
    ret = ::pthread_attr_getdetachstate (attr, detachstate);

    if (ret != 0)
    {
        return ret;
    }

    sched_param param;
    ret = ::pthread_attr_getschedparam (attr, &param);

    if (ret != 0)
    {
        return ret;
    }

    *threadpriority  = param.sched_priority;
    ret = ::pthread_attr_getstacksize (attr, stacksize);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
#endif
}

int ZCE_LIB::pthread_attr_setex(pthread_attr_t *attr,
                                int detachstate,
                                size_t stacksize,
                                int threadpriority
                               )
{

#if defined (ZCE_OS_WINDOWS)
    assert( PTHREAD_CREATE_JOINABLE == detachstate || PTHREAD_CREATE_DETACHED == detachstate );
    attr->detachstate = detachstate;
    attr->stacksize = stacksize;
    //修改调度策略继承方式
    attr->inheritsched = PTHREAD_EXPLICIT_SCHED;
    attr->schedparam.sched_priority = threadpriority;

    return 0;

#elif defined (ZCE_OS_LINUX)
    int ret = 0;
    ret = ::pthread_attr_setdetachstate (attr, detachstate);

    if (ret != 0)
    {
        return ret;
    }

    //注意这儿，要先设置是否遵守不从夫进程得到调度方案
    ret = ::pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);

    if (ret != 0)
    {
        return ret;
    }

    sched_param param;
    param.sched_priority = threadpriority;
    ret = ::pthread_attr_setschedparam (attr, &param);

    if (ret != 0)
    {
        return ret;
    }

    // 加个判断如果小于最小值，则赋为最小值
    if (stacksize < PTHREAD_STACK_MIN)
    {
        stacksize = PTHREAD_STACK_MIN;
    }

    ret = ::pthread_attr_setstacksize (attr, stacksize);

    if (ret != 0)
    {
        return ret;
    }

    return 0;
#endif //#if defined (ZCE_OS_LINUX)
}

//线程启动函数的适配器
class THREAD_START_FUN_ADAPT
{
protected:
    typedef void (*START_ROUTINE_FUN)(void *);

    //线程启动函数
    START_ROUTINE_FUN   start_routine_;
    //线程启动参数
    void               *arg_;

public:
    //构造函数和析构函数
    THREAD_START_FUN_ADAPT(void (*start_routine)(void *), void *arg):
        start_routine_(start_routine),
        arg_(arg)
    {
    }

    ~THREAD_START_FUN_ADAPT()
    {
    }
#if defined (ZCE_OS_WINDOWS)
    //
    static unsigned int __stdcall adapt_svc_run(void *adapt_svc)
    {
        THREAD_START_FUN_ADAPT *my_adapt = static_cast<THREAD_START_FUN_ADAPT *>(adapt_svc);
        my_adapt->start_routine_(my_adapt->arg_);
        delete my_adapt;
        unsigned int return_data = 0;
        return return_data;
    }
#elif defined (ZCE_OS_LINUX)
    //
    static void *adapt_svc_run(void *adapt_svc)
    {
        THREAD_START_FUN_ADAPT *my_adapt = static_cast<THREAD_START_FUN_ADAPT *>(adapt_svc);
        my_adapt->start_routine_(my_adapt->arg_);
        delete my_adapt;
        void *return_dword = 0;
        return return_dword;
    }
#endif //
};

//创建一个一个线程,为了方便，我这儿也不提供以创建就挂起的功能了，
//注意start_routine，我和pthread_create用的不一致喔。
//为什么void (*start_routine)(void*) 不适用LINUX下的标准呢，我至少犹豫两次，甚至反复改改代码，
//ACE的实现方式是对不同平台做了区别对待，好处是大家没有犯错的可能，缺点是无法写出一致的代码，必须依靠宏
//而我选择谁也不去迁就，不提供返回值给你，如果你需要返回值，请用线程类的封装，而不是API
//因为WIN32下返回unsigned int，LINUX下返回void *，我要考虑平台兼容性，将谁转换给谁都不合适,所以我放弃
//所以统一没有返回值，给你出错的可能，反而害死自己
int ZCE_LIB::pthread_create(ZCE_THREAD_ID *threadid,
                            const pthread_attr_t *attr,
                            void (*start_routine)(void *),
                            void *arg)
{
#if defined (ZCE_OS_WINDOWS)

    //这个参数new了之后，在创建了线程后才能delete,
    //所以真正的删除点在，THREAD_START_FUN_ADAPT::adapt_svc_run这个地方，
    THREAD_START_FUN_ADAPT *adapt_object = new THREAD_START_FUN_ADAPT(start_routine, arg);
    //用CRT的线程创建函数创建线程
    HANDLE thread_handle = (HANDLE)::_beginthreadex(NULL,
                                                    static_cast<unsigned int>(attr->stacksize),
                                                    THREAD_START_FUN_ADAPT::adapt_svc_run,
                                                    adapt_object,
                                                    0,
                                                    threadid);

    if (ZCE_INVALID_HANDLE == thread_handle)
    {
        delete adapt_object;
        return -1;
    }

    //设置线程优先级
    if (PTHREAD_EXPLICIT_SCHED == attr->inheritsched &&
        0 != attr->schedparam.sched_priority )
    {
        //线程相对优先级（取值对应如下）
        BOOL bret = ::SetThreadPriority(thread_handle,
                                        attr->schedparam.sched_priority);

        if ( !bret )
        {
            //是否记录一下，呵呵
        }
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    THREAD_START_FUN_ADAPT *adapt_object = new THREAD_START_FUN_ADAPT(start_routine, arg);
    return ::pthread_create (threadid, attr,
                             THREAD_START_FUN_ADAPT::adapt_svc_run,
                             adapt_object);

#endif //#if defined (ZCE_OS_LINUX)
}

//这个不是POSIX的封装，但推荐使用
//还是提供一个简单一点的封装吧，这个不用处理pthread_attr_t
int ZCE_LIB::pthread_createex(void (*start_routine)(void *),
                              void *arg,
                              ZCE_THREAD_ID *threadid,
                              int detachstate,
                              size_t stacksize,
                              int threadpriority
                             )
{
    int ret = 0;
    pthread_attr_t attr;
    ret = ZCE_LIB::pthread_attr_init(&attr);

    if (0 != ret)
    {
        return ret;
    }

    //设置线程参数
    ret = ZCE_LIB::pthread_attr_setex(&attr,
                                      detachstate,
                                      stacksize,
                                      threadpriority
                                     );

    if (0 != ret)
    {
        ZCE_LIB::pthread_attr_destroy(&attr);
        return ret;
    }

    //创建线程
    ret = ZCE_LIB::pthread_create(threadid,
                                  &attr,
                                  start_routine,
                                  arg);

    if (0 != ret)
    {
        ZCE_LIB::pthread_attr_destroy(&attr);
        return ret;
    }

    return 0;
}

//退出，传递void，各个平台不兼容，所以干脆什么都不传递出来
void ZCE_LIB::pthread_exit( void )
{
#if defined (ZCE_OS_WINDOWS)

    unsigned int return_data = 0;
    _endthreadex(return_data);
    return;

#elif defined (ZCE_OS_LINUX)
    void *return_data = NULL;
    return ::pthread_exit (return_data);
#endif //#if defined (ZCE_OS_LINUX)
}

//等待某个JOIN的线程结束,但不理会返回值
int ZCE_LIB::pthread_join(ZCE_THREAD_ID threadid)
{
    ZCE_THR_FUNC_RETURN ret_val;
    return ZCE_LIB::pthread_join (threadid, &ret_val);
}

//==========================================================================================
//注意，WINDOWS和LINUX下返回值不同，所以我非常非常不建议你用这个传递返回值，
//WINDOWS 可以传递unsigned int ，LINUX传递void *，

#if defined (ZCE_OS_WINDOWS)

class WIN_THREAD_STARTFUN_ADAPT
{
protected:
    typedef unsigned int (*WIN_START_ROUTINE_FUN)(void *);

    //线程启动函数
    WIN_START_ROUTINE_FUN start_routine_;
    //线程启动参数
    void                 *arg_;

public:
    //构造函数和析构函数
    WIN_THREAD_STARTFUN_ADAPT(unsigned int (*start_routine)(void *), void *arg):
        start_routine_(start_routine),
        arg_(arg)
    {
    }

    ~WIN_THREAD_STARTFUN_ADAPT()
    {
    }

    //
    static unsigned int __stdcall adapt_svc_run(void *adapt_svc)
    {
        WIN_THREAD_STARTFUN_ADAPT *my_adapt = static_cast<WIN_THREAD_STARTFUN_ADAPT *>(adapt_svc);
        unsigned int return_data = my_adapt->start_routine_(my_adapt->arg_);
        return return_data;
    }

};

#endif //

//创建一个线程,调用线程函数向各个平台兼容模式靠齐，有返回值
int ZCE_LIB::pthread_create(ZCE_THREAD_ID *threadid,
                            const pthread_attr_t *attr,
                            ZCE_THR_FUNC_RETURN (* start_routine)(void *),
                            void *arg)
{
#if defined (ZCE_OS_WINDOWS)

    WIN_THREAD_STARTFUN_ADAPT adapt_object(start_routine, arg);
    //用CRT的线程创建函数创建线程
    HANDLE thread_handle = (HANDLE)::_beginthreadex(NULL,
                                                    static_cast<unsigned int>(attr->stacksize),
                                                    THREAD_START_FUN_ADAPT::adapt_svc_run,
                                                    &adapt_object,
                                                    0,
                                                    threadid);

    if (ZCE_INVALID_HANDLE == thread_handle)
    {
        return -1;
    }

    //设置线程优先级
    if (PTHREAD_EXPLICIT_SCHED == attr->inheritsched &&
        0 != attr->schedparam.sched_priority )
    {
        //线程相对优先级（取值对应如下）
        BOOL bret = ::SetThreadPriority(thread_handle,
                                        attr->schedparam.sched_priority);

        if ( !bret )
        {
            //是否记录一下，呵呵
        }
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_create (threadid,
                             attr,
                             start_routine,
                             arg);

#endif //#if defined (ZCE_OS_LINUX)
}

//退出一个线程，可以得到返回值
void ZCE_LIB::pthread_exit(ZCE_THR_FUNC_RETURN thr_ret)
{
#if defined (ZCE_OS_WINDOWS)
    return _endthreadex(thr_ret);

#elif defined (ZCE_OS_LINUX)
    return ::pthread_exit (thr_ret);
#endif //#if defined (ZCE_OS_LINUX)
}

//等待某个JOIN的线程结束,并且得到线程回调函数的返回值
int ZCE_LIB::pthread_join(ZCE_THREAD_ID threadid, ZCE_THR_FUNC_RETURN *ret_val)
{

#if defined (ZCE_OS_WINDOWS)

    //OpenThread是一个WIN SERVER 2000后才有的函数 VC6应该没有
    HANDLE thr_handle = (HANDLE)::OpenThread(THREAD_ALL_ACCESS,
                                             FALSE,
                                             threadid
                                            );

    DWORD thread_ret;

    if (::WaitForSingleObject (thr_handle, INFINITE) == WAIT_OBJECT_0
        && ::GetExitCodeThread (thr_handle, &thread_ret) != FALSE)
    {
        *ret_val = thread_ret;
        //此处不要关闭handle（  ::CloseHandle (thr_handle);），因为我们调用的是_endthreadex 结束的线程
        return 0;
    }

    return -1;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::pthread_join (threadid, ret_val);
#endif //#if defined (ZCE_OS_LINUX)
}

//得到线程ID
ZCE_THREAD_ID ZCE_LIB::pthread_self(void)
{
#if defined (ZCE_OS_WINDOWS)
    return ::GetCurrentThreadId();
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::pthread_self ();
#endif //#if defined (ZCE_OS_LINUX)
}

//取消一个线程
//这个函数放在这儿，完全是为了满足我的一点点封装意愿，因为在LINUX平台，你未必能取消一个线程
//在外WINDOWS，调用TerminateThread，这个函数也是不被推荐的
int ZCE_LIB::pthread_cancel(ZCE_THREAD_ID threadid)
{
#if defined (ZCE_OS_WINDOWS)
    //OpenThread是一个WIN SERVER 2000后才有的函数 VC6应该没有
    HANDLE thr_handle = (HANDLE)::OpenThread(THREAD_ALL_ACCESS,
                                             FALSE,
                                             threadid
                                            );
    //强制退出
    BOOL bret = ::TerminateThread(thr_handle, 0);

    if (!bret)
    {
        return -1;
    }

    ::CloseHandle(thr_handle);
    return 0;

#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::pthread_cancel (threadid);
#endif //#if defined (ZCE_OS_LINUX)
}

//对一个线程进行松绑
int ZCE_LIB::pthread_detach(ZCE_THREAD_ID threadid)
{
#if defined (ZCE_OS_WINDOWS)
    //Windows线程本来就是detach的，呵呵
    ZCE_UNUSED_ARG(threadid);
    return 0;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::pthread_detach (threadid);
#endif //#if defined (ZCE_OS_LINUX)
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
#endif //

int ZCE_LIB::pthread_yield(void)
{
#if defined (ZCE_OS_WINDOWS)
    ::Sleep(0);
    return 0;
#endif //#if defined (ZCE_OS_WINDOWS)

#if defined (ZCE_OS_LINUX)
    return ::pthread_yield ();
#endif //#if defined (ZCE_OS_LINUX)
}

//=================================================================================================================

//destructor 参数对于WIN32平台没有用，建议不用。
//
int ZCE_LIB::pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
#if defined (ZCE_OS_WINDOWS)

    ZCE_UNUSED_ARG(destructor);

    *key = ::TlsAlloc ();

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
int ZCE_LIB::pthread_key_delete(pthread_key_t key)
{

#if defined (ZCE_OS_WINDOWS)

    BOOL bool_ret =  ::TlsFree (key);

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

int ZCE_LIB::pthread_setspecific(pthread_key_t key, const void *data)
{

#if defined (ZCE_OS_WINDOWS)

    BOOL bool_ret =  ::TlsSetValue (key, (LPVOID)( data));

    //如果返回FALSE标识失败
    if (!bool_ret)
    {
        errno = GetLastError();
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_setspecific (key, data);
#endif
}

void *ZCE_LIB::pthread_getspecific (pthread_key_t key)
{
#if defined (ZCE_OS_WINDOWS)

    LPVOID  data = ::TlsGetValue (key);

    if (data == 0 && ::GetLastError () != NO_ERROR)
    {
        errno = GetLastError();
        return NULL;
    }
    else
    {
        return data;
    }

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_getspecific (key);

#endif

}

