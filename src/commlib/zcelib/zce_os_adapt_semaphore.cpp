
#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_time.h"
#include "zce_os_adapt_semaphore.h"

//为什么不让我用ACE，卫生棉！，卫生棉！！！！！卫生棉卫生棉卫生棉！！！！！！！！

//初始化，创建一个无名（匿名）信号灯,对应的销毁函数sem_destroy
int ZCE_LIB::sem_init(sem_t *sem,
                     int pshared,
                     unsigned int init_value,
                     unsigned int max_val)
{

#if defined (ZCE_OS_WINDOWS)

    //字符串长度为0表示是匿名信号灯
    sem->sem_unnamed_ = true;

    ZCE_UNUSED_ARG (pshared);

    HANDLE sem_handle = ::CreateSemaphoreA(NULL,
                                           init_value,
                                           max_val,
                                           NULL);

    if (sem_handle == 0)
    {
        return -1;
    }
    else
    {
        // Make sure to set errno to ERROR_ALREADY_EXISTS if necessary.
        errno = ::GetLastError ();
        sem->sem_hanlde_ = sem_handle;
        return 0;
    }

#elif defined (ZCE_OS_LINUX)
    //
    //非标准参数
    ZCE_UNUSED_ARG(max_val);
    return ::sem_init (sem,
                       pshared,
                       init_value);
#endif
}

//销毁无名(匿名)信号灯
//sem_destroy不会释放sem，因为这是你分配的内存
int ZCE_LIB::sem_destroy(sem_t *sem)
{
#if defined (ZCE_OS_WINDOWS)

    ::CloseHandle(sem->sem_hanlde_);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::sem_destroy(sem);
#endif
}

//打开(有名)信号灯,最大值不是标准参数，所以用默认只修饰了，这个主要用于创建有名信号灯,
//打开后，要使用sem_close，sem_unlink
sem_t *ZCE_LIB::sem_open(const char *name,
                        int oflag,
                        mode_t mode,
                        unsigned int init_value,
                        unsigned int max_val)
{
#if defined (ZCE_OS_WINDOWS)

    ZCE_UNUSED_ARG (oflag);
    ZCE_UNUSED_ARG (mode);

    //为了保存变量还是先new出来
    sem_t *ret_sem = new sem_t ();
    ret_sem->sem_unnamed_ = false;

    HANDLE sem_handle = ::CreateSemaphoreA(NULL,
                                           init_value,
                                           max_val,
                                           name);

    if (sem_handle == 0)
    {
        delete ret_sem;
        ret_sem = NULL;
        return SEM_FAILED;
    }
    else
    {
        // Make sure to set errno to ERROR_ALREADY_EXISTS if necessary.
        errno = ::GetLastError ();

        ret_sem->sem_hanlde_ = sem_handle;
        return ret_sem;
    }

#elif defined (ZCE_OS_LINUX)
    //非标准参数
    ZCE_UNUSED_ARG(max_val);

    return ::sem_open (name,
                       oflag,
                       mode,
                       init_value);
#endif
}

//关闭信号灯
int ZCE_LIB::sem_close(sem_t *sem)
{
#if defined (ZCE_OS_WINDOWS)
    ::CloseHandle(sem->sem_hanlde_);
    delete sem;
    sem = NULL;
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::sem_close(sem);
#endif
}

//删除信号灯
int ZCE_LIB::sem_unlink(const char *name)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG (name);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::sem_unlink(name);
#endif
}

int ZCE_LIB::sem_post (sem_t *sem)
{

#if defined (ZCE_OS_WINDOWS)

    const LONG ONCE_POST_NUMBER = 1;
    BOOL  ret_bool = ::ReleaseSemaphore (sem->sem_hanlde_,
                                         ONCE_POST_NUMBER,
                                         NULL);

    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::sem_post(sem);
#endif

}

int ZCE_LIB::sem_post (sem_t *sem, u_int release_count)
{
#if defined (ZCE_OS_WINDOWS)

    BOOL  ret_bool = ::ReleaseSemaphore (sem->sem_hanlde_,
                                         release_count,
                                         NULL);

    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    //
    for (size_t i = 0; i < release_count; ++i)
    {
        if (::sem_post(sem) == -1)
        {
            return -1;
        }
    }

    return 0;
#endif
}

int ZCE_LIB::sem_trywait (sem_t *sem)
{

#if defined (ZCE_OS_WINDOWS)

    //等待0s，相当于无阻塞，
    DWORD result = ::WaitForSingleObject (sem->sem_hanlde_, 0);

    if (result == WAIT_OBJECT_0)
    {
        return 0;
    }
    else
    {
        if (result == WAIT_TIMEOUT)
        {
            //Posix的错误应该是设置成EAGAIN的,这个和Mutex的返回还不一样，Mutex好像是EBUSY
            errno = EAGAIN;
        }
        else
        {
            errno = GetLastError();
        }

        // This is a hack, we need to find an appropriate mapping...
        return -1;
    }

#elif defined (ZCE_OS_LINUX)
    // POSIX semaphores set errno to EAGAIN if trywait fails
    return ::sem_trywait (sem);
#endif

}

//信号灯锁定
int ZCE_LIB::sem_wait (sem_t *sem)
{
#if defined (ZCE_OS_WINDOWS)

    //INFINITE标识一致等待
    DWORD result = ::WaitForSingleObject (sem->sem_hanlde_, INFINITE);

    if (result == WAIT_OBJECT_0)
    {
        return 0;
    }
    else
    {
        errno = GetLastError();
        return -1;
    }

#elif defined (ZCE_OS_LINUX)
    // POSIX semaphores set errno to EAGAIN if trywait fails
    return ::sem_wait (sem);
#endif
}

//信号灯超时锁定
int ZCE_LIB::sem_timedwait(sem_t *sem, const ::timespec *abs_timeout_spec)
{

#if defined (ZCE_OS_WINDOWS)

    assert(abs_timeout_spec);

    //得到相对时间，这个折腾，
    timeval now_time = ZCE_LIB::gettimeofday();
    timeval abs_time = ZCE_LIB::make_timeval(abs_timeout_spec);

    timeval timeout_time = ZCE_LIB::timeval_sub(abs_time, now_time, true);

    //等待时间触发
    DWORD retsult = ::WaitForSingleObject (sem->sem_hanlde_,
                                           static_cast<DWORD>( ZCE_LIB::total_milliseconds(timeout_time)));

    if (WAIT_OBJECT_0 == retsult || WAIT_ABANDONED == retsult)
    {
        return 0;
    }
    else if (WAIT_TIMEOUT == retsult)
    {
        errno = ETIMEDOUT;
        return -1;
    }
    else
    {
        // This is a hack, we need to find an appropriate mapping...
        errno = ::GetLastError ();
        return -1;
    }

    /* NOTREACHED */

#elif defined (ZCE_OS_LINUX)

    int ret = ::sem_timedwait (sem, abs_timeout_spec);

    //一般的系统ETIME 和 ETIMEDOUT 的错误值都不太一样，按LINUX手册，返回的应该是ETIMEDOUT
    if (ret == -1 && errno == ETIME)
    {
        errno = ETIMEDOUT;
    }

    return ret;
#endif
}

//信号灯超时锁定,非标准实现,使用timeval结构，
int ZCE_LIB::sem_timedwait(sem_t *sem, const timeval *abs_timeout_val)
{
    assert(abs_timeout_val);
    //这个时间是绝对值时间，要调整为相对时间
    ::timespec abs_timeout_spec = ZCE_LIB::make_timespec(abs_timeout_val);
    return ZCE_LIB::sem_timedwait(sem, &abs_timeout_spec);
}

//返回当前信号灯的当前值, 很遗憾，WINDOWS下不支持，ReleaseSemaphore有类似功能，但是lReleaseCount参数不能为0
//如果用ReleaseSemaphore和WaitForSingleObject拼凑一个那么可能更超级糟糕，
//微软的API实现的真烂。Visual studio也承诺增加这个特性，但是在2010版本是不要做指望了。
int ZCE_LIB::sem_getvalue(sem_t *sem, int *sval)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(sem);
    ZCE_UNUSED_ARG(sval);
    errno = EINVAL;
    return -1;
#elif defined (ZCE_OS_LINUX)
    return ::sem_getvalue(sem, sval);
#endif
}

