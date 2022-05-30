#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/common.h"
#include "zce/os_adapt/mutex.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/semaphore.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/condi.h"

//用多个WINDOWS20008的条件变量对象模拟PTHREAD CV条件变量
#ifndef ZCE_IS_USE_WIN2008_SIMULATE_PCV
#define ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond) ( (ZCE_BIT_IS_SET((cond)->outer_lock_type_ , PTHREAD_MUTEX_RECURSIVE)) \
                                                && (ZCE_BIT_ISNOT_SET((cond)->outer_lock_type_ , PTHREAD_MUTEX_TIMEOUT)) )
#endif

//ZCE_SUPPORT_WINSVR2008 == 1里面的代码都自成一段，看代码的时候注意
//都是WIN SERVER 2008后，WINDOWS自己的条件变量的封装，

//
int zce::pthread_condattr_init(pthread_condattr_t* attr)
{
#if defined (ZCE_OS_WINDOWS)

    //WINDOWS下默认就是递归的，你要我搞个非递归的我还要折腾
    attr->outer_lock_type_ = PTHREAD_MUTEX_RECURSIVE;
    //初始化名称
    attr->cv_name_[PATH_MAX] = '\0';
    attr->cv_name_[0] = '\0';
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_condattr_init(attr);
#endif
}

//
int zce::pthread_condattr_destroy(pthread_condattr_t* attr)
{
#if defined (ZCE_OS_WINDOWS)
    ZCE_UNUSED_ARG(attr);
    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_condattr_destroy(attr);
#endif
}

//条件变量对象的初始化
int zce::pthread_cond_init(pthread_cond_t* cond,
                           const pthread_condattr_t* attr)
{
#if defined (ZCE_OS_WINDOWS)

    if (attr)
    {
        cond->outer_lock_type_ = attr->outer_lock_type_;
    }
    else
    {
        cond->outer_lock_type_ = PTHREAD_MUTEX_RECURSIVE;
    }

    //WIN SERVER 2008，VISTA 后支持条件变量
#if defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008

    //如果是线程内部的，而且是递归的，而且没有超时功能，可以用Windows的条件条件变量干活
    if (ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond))
    {
        ::InitializeConditionVariable(&(cond->cv_object_));
        return 0;
    }

#endif

    cond->simulate_cv_.block_sema_ = NULL;
    cond->simulate_cv_.finish_broadcast_ = NULL;
    cond->simulate_cv_.waiters_ = 0;
    cond->simulate_cv_.was_broadcast_ = false;

    char* sem_block_ptr = NULL, * sem_finish_ptr = NULL;
    char sem_block_name[PATH_MAX + 1], sem_finish_name[PATH_MAX + 1];
    sem_block_name[PATH_MAX] = '\0';
    sem_finish_name[PATH_MAX] = '\0';

    //这段代码只在WIN32下用，我简化了
    pthread_mutexattr_t waiters_lock_attr;
    zce::pthread_mutexattr_init(&waiters_lock_attr);
    waiters_lock_attr.lock_shared_ = PTHREAD_PROCESS_PRIVATE;
    waiters_lock_attr.lock_type_ = PTHREAD_MUTEX_RECURSIVE;

    //初始化线程的互斥量
    int result = 0;
    result = zce::pthread_mutex_init(&cond->simulate_cv_.waiters_lock_,
                                     &waiters_lock_attr);

    if (result != 0)
    {
        return result;
    }

    cond->simulate_cv_.block_sema_ = zce::sem_open(sem_block_ptr,
                                                   O_CREAT,
                                                   ZCE_DEFAULT_FILE_PERMS,
                                                   0);

    //如果失败了，要回收前面获得的资源
    if (!cond->simulate_cv_.block_sema_)
    {
        zce::pthread_mutex_destroy(&cond->simulate_cv_.waiters_lock_);
        return -1;
    }

    cond->simulate_cv_.finish_broadcast_ = zce::sem_open(sem_finish_ptr,
                                                         O_CREAT,
                                                         ZCE_DEFAULT_FILE_PERMS,
                                                         0);

    //如果失败了，要回收前面获得的资源,这种分段申请资源最麻烦
    if (!cond->simulate_cv_.finish_broadcast_)
    {
        zce::pthread_mutex_destroy(&cond->simulate_cv_.waiters_lock_);
        zce::sem_close(cond->simulate_cv_.block_sema_);
        //其实没用
        //zce::sem_unlink(sem_block_name);
        return EINVAL;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_init(cond, attr);
#endif
}

//初始化条件变量对象，不同的平台给不同的默认定义
//非标准，但是建议你使用，简单多了,
//如果要多进程共享，麻烦你老给个名字，同时在LINUX平台下，你必须pthread_condattr_t放入共享内存
int zce::pthread_cond_initex(pthread_cond_t* cond,
                             bool win_mutex_or_sema)
{
    //前面有错误返回，
    int result = 0;

    pthread_condattr_t attr;
    result = zce::pthread_condattr_init(&attr);
    if (0 != result)
    {
        return result;
    }

    //这个是我在WINDOWS下用的，用于某些时候我可以用临界区，而不是更重的互斥量
#if defined (ZCE_OS_WINDOWS)
    //默认还是用递归的锁
    attr.outer_lock_type_ |= PTHREAD_MUTEX_RECURSIVE;

    if (win_mutex_or_sema)
    {
        attr.outer_lock_type_ |= PTHREAD_MUTEX_TIMEOUT;
    }

#elif defined (ZCE_OS_LINUX)
    ZCE_UNUSED_ARG(win_mutex_or_sema);

    result = ::pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
    if (0 != result)
    {
        return result;
    }
#endif

    result = zce::pthread_cond_init(cond, &attr);
    zce::pthread_condattr_destroy(&attr);

    if (0 != result)
    {
        return EINVAL;
    }

    return 0;
}

//条件变量对象的销毁
int zce::pthread_cond_destroy(pthread_cond_t* cond)
{
#if defined (ZCE_OS_WINDOWS)

    //使用WINDOWS的条件变量
#if defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008

    if (ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond))
    {
        //WINDOWS的条件变量没有释放
        return 0;
    }

#endif

    zce::pthread_mutex_destroy(&cond->simulate_cv_.waiters_lock_);

    zce::sem_close(cond->simulate_cv_.block_sema_);
    zce::sem_close(cond->simulate_cv_.finish_broadcast_);

    //WIN平台下，无须调用这个函数，偷懒
    //zce::sem_unlink(sem_name);

    cond->simulate_cv_.block_sema_ = NULL;
    cond->simulate_cv_.finish_broadcast_ = NULL;

    cond->simulate_cv_.waiters_ = 0;
    cond->simulate_cv_.was_broadcast_ = false;

    return 0;
#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_destroy(cond);
#endif
}

//条件变量等待一段时间，超时后继续
int zce::pthread_cond_timedwait(pthread_cond_t* cond,
                                pthread_mutex_t* external_mutex,
                                const ::timespec* abs_timespec_out)
{
#if defined (ZCE_OS_WINDOWS)

    //如果外部的MUTEX的类型和共享方式不是我们所预期的，滚蛋
    if (external_mutex->lock_type_ != cond->outer_lock_type_)
    {
        ZCE_ASSERT(false);
        return EINVAL;
    }

    //使用WINDOWS2008的条件变量
#if defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008

    if (ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond))
    {
        DWORD wait_msec = INFINITE;

        //如果有超时，计算相对超时时间
        if (abs_timespec_out)
        {
            //得到相对时间，这个折腾，
            timeval now_time = zce::gettimeofday();
            timeval abs_time = zce::make_timeval(abs_timespec_out);

            timeval timeout_time = zce::timeval_sub(abs_time, now_time, true);
            wait_msec = static_cast<DWORD>(zce::total_milliseconds(timeout_time));
        }

        //WINDOWS的条件变量没有释放
        BOOL bret = ::SleepConditionVariableCS(
            &(cond->cv_object_),
            &(external_mutex->thr_nontimeout_mutex_),
            wait_msec);

        if (bret == FALSE)
        {
            //SleepConditionVariableCS 是在GetLastError看结果，
            //这些API的设计继续反映出WINDOWS的前后不一。
            if (::GetLastError() == WAIT_TIMEOUT)
            {
                return ETIMEDOUT;
            }
            else
            {
                return EINVAL;
            }
        }

        return 0;
    }

#endif  //使用WINDOWS2008的条件变量

    // Prevent race conditions on the <waiters_> count.
    zce::pthread_mutex_lock(&(cond->simulate_cv_.waiters_lock_));
    ++(cond->simulate_cv_.waiters_);
    zce::pthread_mutex_unlock(&(cond->simulate_cv_.waiters_lock_));

    int result = 0;

    //对外部的锁重新解锁，
    //不对释放资源进行错误处理，如果释放失败，我能如何呢
    zce::pthread_mutex_unlock(external_mutex);

    ///@note这个地方存在某种争议，也就是上面这步和下面这步是否要
    ///形成原子操作，这个问题在Douglas C. Schmidt and Irfan Pyarali的论文中有过描述，
    ///但是因为我们用的是信号灯，所以这儿即使有人插队到这个地方得到external_mutex，发出了
    ///signal或者广播，也不会造成下面死锁，我这样认为，呵呵。
    ///当然如果真有问题，就换成SignalObjectAndWait，

    //如果是超时等待，就进行等待
    if (abs_timespec_out)
    {
        result = zce::sem_timedwait(cond->simulate_cv_.block_sema_,
                                    abs_timespec_out);
    }
    else
    {
        result = zce::sem_wait(cond->simulate_cv_.block_sema_);
    }

    //记录错误
    if (result != 0)
    {
        result = zce::last_error_with_default(EINVAL);
    }

    //同步，避免竞争
    zce::pthread_mutex_lock(&cond->simulate_cv_.waiters_lock_);
    //信号灯已经退出，减少等待的总数
    --(cond->simulate_cv_.waiters_);
    bool const last_waiter = (cond->simulate_cv_.was_broadcast_
                              && cond->simulate_cv_.waiters_ == 0);
    zce::pthread_mutex_unlock(&cond->simulate_cv_.waiters_lock_);

    if (result == 0)
    {
        //这就是我特别看不懂的地方，理论意图应该是最后一个，告知广播者，
        //如果是最后一个人，通知broadcaster对象，我们是最后一个人了，你可以退出了，不用等了。
        //这个地方用信号灯其实有一些问题，因为不利于公平性，但由于这个模拟要求广播的时候，外部锁
        //必现加上，所以问题不大
        if (last_waiter)
        {
            // Release the signaler/broadcaster if we're the last waiter.
            zce::sem_post(cond->simulate_cv_.finish_broadcast_);
        }
    }

    //对外部的锁重新加上
    zce::pthread_mutex_lock(external_mutex);

    return result;

#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_cond_timedwait(cond,
                                    external_mutex,
                                    abs_timespec_out);

#endif
}

//条件变量等待一段时间，超时后继续,时间变量用我内部统一的timeval
int zce::pthread_cond_timedwait(pthread_cond_t* cond,
                                pthread_mutex_t* external_mutex,
                                const timeval* abs_timeout_val)
{
    assert(abs_timeout_val);
    //这个时间是绝对值时间，要调整为相对时间
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::pthread_cond_timedwait(cond,
                                       external_mutex,
                                       &abs_timeout_spec);
}

//条件变量等待
int zce::pthread_cond_wait(pthread_cond_t* cond,
                           pthread_mutex_t* external_mutex)
{
#if defined (ZCE_OS_WINDOWS)
    //这样写是为了避免函数冲突告警，
    const ::timespec* abs_timespec_out = NULL;
    return zce::pthread_cond_timedwait(cond,
                                       external_mutex,
                                       abs_timespec_out);
#elif defined (ZCE_OS_LINUX)
    //
    return ::pthread_cond_wait(cond,
                               external_mutex);
#endif
}

//
int zce::pthread_cond_broadcast(pthread_cond_t* cond)
{
#if defined (ZCE_OS_WINDOWS)

    //在调用这个方式前，外部的锁必须是锁上的，（这个地方略有疑问，其实POSIX并没有特别明确说明此问题）
    // The <external_mutex> must be locked before this call is made.

    //使用WINDOWS的条件变量
#if defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008

    if (ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond))
    {
        ::WakeAllConditionVariable(&(cond->cv_object_));
        return 0;
    }

#endif

    // This is needed to ensure that <waiters_> and <was_broadcast_> are
    // consistent relative to each other.
    zce::pthread_mutex_lock(&cond->simulate_cv_.waiters_lock_);
    bool have_waiters = false;

    if (cond->simulate_cv_.waiters_ > 0)
    {
        // We are broadcasting, even if there is just one waiter...
        // Record the fact that we are broadcasting.  This helps the
        // cond_wait() method know how to optimize itself.  Be sure to
        // set this with the <waiters_lock_> held.
        cond->simulate_cv_.was_broadcast_ = true;
        have_waiters = true;
    }

    zce::pthread_mutex_unlock(&cond->simulate_cv_.waiters_lock_);
    int result = 0;

    if (have_waiters)
    {
        //ACE比较喜欢这种if的方式，我不是特别习惯，但在多层处理的过程中这个方法也还凑合
        //唤醒所有的等待者,
        if (zce::sem_post(cond->simulate_cv_.block_sema_, cond->simulate_cv_.waiters_) != 0)
        {
            result = EINVAL;
        }

        //注意这儿，这儿的实现是不完美的，因为其实从语义上讲，上面这句话和下面这句话也必须是原子的，
        //否则，也许post block_sema_ 后，wait的线程取得执行权利，finish_broadcast_已经post了，
        //那么下面就没有任何作用了，ACE的源代码里面是用的SignalObjectAndWait
        //但ACE的实现也要求大家调用broadcast是，外部锁是加上的，所以吧
        // Wait for all the awakened threads to acquire their part of
        // the counting semaphore.
        else if (zce::sem_wait(cond->simulate_cv_.finish_broadcast_) != 0)
        {
            result = EINVAL;
        }

        //由于这个函数要求外部锁是锁上的，所以was_broadcast_的调整也OK
        // This is okay, even without the <waiters_lock_> held because
        // no other waiter threads can wake up to access it.
        cond->simulate_cv_.was_broadcast_ = false;
    }

    return result;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_broadcast(cond);
#endif
}

//发信号
int zce::pthread_cond_signal(pthread_cond_t* cond)
{
#if defined (ZCE_OS_WINDOWS)

    //在调用这个方式前，外部的锁必须是锁上的，
    //使用WINDOWS的条件变量
#if defined ZCE_DEPEND_WINVER && ZCE_DEPEND_WINVER >= 2008

    if (ZCE_IS_USE_WIN2008_SIMULATE_PCV(cond))
    {
        ::WakeConditionVariable(&(cond->cv_object_));
        return 0;
    }

#endif

    int result = 0;
    //是否有人在等待
    zce::pthread_mutex_lock(&cond->simulate_cv_.waiters_lock_);
    bool const have_waiters = cond->simulate_cv_.waiters_ > 0;
    zce::pthread_mutex_unlock(&cond->simulate_cv_.waiters_lock_);

    if (have_waiters)
    {
        result = zce::sem_post(cond->simulate_cv_.block_sema_);

        if (0 != result)
        {
            return EINVAL;
        }
    }

    // No-op
    return 0;

#elif defined (ZCE_OS_LINUX)
    return ::pthread_cond_signal(cond);
#endif
}