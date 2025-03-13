#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/define.h"
#include "zce/os_adapt/mutex.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/semaphore.h"
#include "zce/os_adapt/time.h"
#include "zce/os_adapt/condi.h"
#include "zce/lock/win_simulate.h"

#if defined (ZCE_OS_WINDOWS)

//====================================================================================================
//条件变量对象的初始化
int zce::ws_cond_init(ws_cv_t* cond)
{
    cond->outer_lock_type_ = PTHREAD_MUTEX_RECURSIVE;

    cond->block_sema_ = NULL;
    cond->finish_broadcast_ = NULL;
    cond->waiters_ = 0;
    cond->was_broadcast_ = false;

    char* sem_block_ptr = NULL,* sem_finish_ptr = NULL;
    char sem_block_name[PATH_MAX + 1],sem_finish_name[PATH_MAX + 1];
    sem_block_name[PATH_MAX] = '\0';
    sem_finish_name[PATH_MAX] = '\0';

    //这段代码只在WIN32下用，我简化了
    pthread_mutexattr_t waiters_lock_attr;
    zce::pthread_mutexattr_init(&waiters_lock_attr);
    waiters_lock_attr.lock_shared_ = PTHREAD_PROCESS_PRIVATE;
    waiters_lock_attr.lock_type_ = PTHREAD_MUTEX_RECURSIVE;

    //初始化线程的互斥量
    int result = 0;
    result = zce::pthread_mutex_init(&cond->waiters_lock_,
                                     &waiters_lock_attr);

    if (result != 0)
    {
        return result;
    }

    cond->block_sema_ = zce::sem_open(sem_block_ptr,
                                      O_CREAT,
                                      ZCE_DEFAULT_FILE_PERMS,
                                      0);

    //如果失败了，要回收前面获得的资源
    if (!cond->block_sema_)
    {
        zce::pthread_mutex_destroy(&cond->waiters_lock_);
        return -1;
    }

    cond->finish_broadcast_ = zce::sem_open(sem_finish_ptr,
                                            O_CREAT,
                                            ZCE_DEFAULT_FILE_PERMS,
                                            0);

    //如果失败了，要回收前面获得的资源,这种分段申请资源最麻烦
    if (!cond->finish_broadcast_)
    {
        zce::pthread_mutex_destroy(&cond->waiters_lock_);
        zce::sem_close(cond->block_sema_);
        return EINVAL;
    }

    return 0;
}

//初始化条件变量对象，不同的平台给不同的默认定义
//非标准，但是建议你使用，简单多了,
//如果要多进程共享，麻烦你老给个名字，同时在LINUX平台下，你必须pthread_condattr_t放入共享内存
int zce::ws_cond_initex(ws_cv_t* cond,
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
    //默认还是用递归的锁
    attr.outer_lock_type_ |= PTHREAD_MUTEX_RECURSIVE;

    if (win_mutex_or_sema)
    {
        attr.outer_lock_type_ |= PTHREAD_MUTEX_TIMEOUT;
    }

    result = zce::ws_cond_init(cond);
    if (0 != result)
    {
        return EINVAL;
    }

    return 0;
}

//条件变量对象的销毁
int zce::ws_cond_destroy(ws_cv_t* cond)
{
    zce::pthread_mutex_destroy(&cond->waiters_lock_);

    zce::sem_close(cond->block_sema_);
    zce::sem_close(cond->finish_broadcast_);

    //WIN平台下，无须调用这个函数，偷懒
    //zce::sem_unlink(sem_name);

    cond->block_sema_ = NULL;
    cond->finish_broadcast_ = NULL;

    cond->waiters_ = 0;
    cond->was_broadcast_ = false;

    return 0;
}

//条件变量等待一段时间，超时后继续
int zce::ws_cond_timedwait(ws_cv_t* cond,
                           pthread_mutex_t* external_mutex,
                           const ::timespec* abs_timespec_out)
{
    //如果外部的MUTEX的类型和共享方式不是我们所预期的，滚蛋
    if (external_mutex->lock_type_ != cond->outer_lock_type_)
    {
        ZCE_ASSERT(false);
        return EINVAL;
    }

    // Prevent race conditions on the <waiters_> count.
    zce::pthread_mutex_lock(&(cond->waiters_lock_));
    ++(cond->waiters_);
    zce::pthread_mutex_unlock(&(cond->waiters_lock_));

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
        result = zce::sem_timedwait(cond->block_sema_,
                                    abs_timespec_out);
    }
    else
    {
        result = zce::sem_wait(cond->block_sema_);
    }

    //记录错误
    if (result != 0)
    {
        result = zce::last_error_with_default(EINVAL);
    }

    //同步，避免竞争
    zce::pthread_mutex_lock(&cond->waiters_lock_);
    //信号灯已经退出，减少等待的总数
    --(cond->waiters_);
    bool const last_waiter = (cond->was_broadcast_
                              && cond->waiters_ == 0);
    zce::pthread_mutex_unlock(&cond->waiters_lock_);

    if (result == 0)
    {
        //这就是我特别看不懂的地方，理论意图应该是最后一个，告知广播者，
        //如果是最后一个人，通知broadcaster对象，我们是最后一个人了，你可以退出了，不用等了。
        //这个地方用信号灯其实有一些问题，因为不利于公平性，但由于这个模拟要求广播的时候，外部锁
        //必现加上，所以问题不大
        if (last_waiter)
        {
            // Release the signaler/broadcaster if we're the last waiter.
            zce::sem_post(cond->finish_broadcast_);
        }
    }

    //对外部的锁重新加上
    zce::pthread_mutex_lock(external_mutex);

    return result;
}

//条件变量等待一段时间，超时后继续,时间变量用我内部统一的timeval
int zce::ws_cond_timedwait(ws_cv_t* cond,
                           pthread_mutex_t* external_mutex,
                           const timeval* abs_timeout_val)
{
    assert(abs_timeout_val);
    //这个时间是绝对值时间，要调整为相对时间
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::ws_cond_timedwait(cond,
                                  external_mutex,
                                  &abs_timeout_spec);
}

//条件变量等待
int zce::ws_cond_wait(ws_cv_t* cond,
                      pthread_mutex_t* external_mutex)
{
    //这样写是为了避免函数冲突告警，
    const ::timespec* abs_timespec_out = NULL;
    return zce::ws_cond_timedwait(cond,
                                  external_mutex,
                                  abs_timespec_out);
}

//
int zce::ws_cond_broadcast(ws_cv_t* cond) noexcept
{
    //在调用这个方式前，外部的锁必须是锁上的，（这个地方略有疑问，其实POSIX并没有特别明确说明此问题）
    // The <external_mutex> must be locked before this call is made.

    // This is needed to ensure that <waiters_> and <was_broadcast_> are
    // consistent relative to each other.
    zce::pthread_mutex_lock(&cond->waiters_lock_);
    bool have_waiters = false;

    if (cond->waiters_ > 0)
    {
        // We are broadcasting, even if there is just one waiter...
        // Record the fact that we are broadcasting.  This helps the
        // cond_wait() method know how to optimize itself.  Be sure to
        // set this with the <waiters_lock_> held.
        cond->was_broadcast_ = true;
        have_waiters = true;
    }

    zce::pthread_mutex_unlock(&cond->waiters_lock_);
    int result = 0;

    if (have_waiters)
    {
        //ACE比较喜欢这种if的方式，我不是特别习惯，但在多层处理的过程中这个方法也还凑合
        //唤醒所有的等待者,
        if (zce::sem_post(cond->block_sema_,cond->waiters_) != 0)
        {
            result = EINVAL;
        }

        //注意这儿，这儿的实现是不完美的，因为其实从语义上讲，上面这句话和下面这句话也必须是原子的，
        //否则，也许post block_sema_ 后，wait的线程取得执行权利，finish_broadcast_已经post了，
        //那么下面就没有任何作用了，ACE的源代码里面是用的SignalObjectAndWait
        //但ACE的实现也要求大家调用broadcast是，外部锁是加上的，所以吧
        // Wait for all the awakened threads to acquire their part of
        // the counting semaphore.
        else if (zce::sem_wait(cond->finish_broadcast_) != 0)
        {
            result = EINVAL;
        }

        //由于这个函数要求外部锁是锁上的，所以was_broadcast_的调整也OK
        // This is okay, even without the <waiters_lock_> held because
        // no other waiter threads can wake up to access it.
        cond->was_broadcast_ = false;
    }

    return result;
}

//发信号
int zce::ws_cond_signal(ws_cv_t* cond) noexcept
{
    //在调用这个方式前，外部的锁必须是锁上的，

    int result = 0;
    //是否有人在等待
    zce::pthread_mutex_lock(&cond->waiters_lock_);
    bool const have_waiters = cond->waiters_ > 0;
    zce::pthread_mutex_unlock(&cond->waiters_lock_);

    if (have_waiters)
    {
        result = zce::sem_post(cond->block_sema_);

        if (0 != result)
        {
            return EINVAL;
        }
    }

    // No-op
    return 0;
}

//====================================================================================================
//读写锁的对象的初始化
int zce::ws_rwlock_init(ws_rwlock_t* rwlock,
                        bool priority_to_write)
{
    //考虑再三，我把重复初始化，是否初始化的各种判定删除了，感觉…………，没必要

    //其他倒霉蛋只能模拟
    int result = 0;
    rwlock->priority_to_write_ = priority_to_write;

    pthread_mutexattr_t mutex_attr;
    zce::pthread_mutexattr_init(&mutex_attr);
    mutex_attr.lock_shared_ = PTHREAD_PROCESS_PRIVATE;
    mutex_attr.lock_type_ = PTHREAD_MUTEX_RECURSIVE;

    //初始化几个同步对象

    //一些数据区改写的保护
    if ((result = zce::pthread_mutex_init(&rwlock->rw_mutex_,&mutex_attr)) != 0)
    {
        return EINVAL;
    }

    //等待读取的条件变量初始化
    if ((result = zce::pthread_cond_initex(&rwlock->rw_condreaders_,
        false)) != 0)
    {
        zce::pthread_mutex_destroy(&rwlock->rw_mutex_);
        return EINVAL;
    }

    //等待写入的条件变量初始化
    if ((result = zce::pthread_cond_initex(&rwlock->rw_condwriters_,
        false)) != 0)
    {
        zce::pthread_cond_destroy(&rwlock->rw_condreaders_);
        zce::pthread_mutex_destroy(&rwlock->rw_mutex_);
        return EINVAL;
    }

    rwlock->rw_nwaitreaders_ = 0;
    rwlock->rw_nwaitwriters_ = 0;
    rwlock->rw_refcount_ = 0;

    return 0;
}

//读写锁的对象的销毁
int zce::ws_rwlock_destroy(ws_rwlock_t* rwlock)
{
    //还有等待的，不能销毁
    if (rwlock->rw_refcount_ != 0
        || rwlock->rw_nwaitreaders_ != 0
        || rwlock->rw_nwaitwriters_ != 0)
    {
        return EBUSY;
    }

    zce::pthread_mutex_destroy(&rwlock->rw_mutex_);
    zce::pthread_cond_destroy(&rwlock->rw_condreaders_);
    zce::pthread_cond_destroy(&rwlock->rw_condwriters_);

    return 0;
}

//获得读取的锁
int zce::ws_rwlock_rdlock(ws_rwlock_t* rwlock)
{
    //加上保护锁
    int  result = zce::pthread_mutex_lock(&rwlock->rw_mutex_);
    if (0 != result)
    {
        return (result);
    }

    //PTHREAD WIN32的实现在这个地方没有用条件变量等待，原因位置，个人认为这是有瑕疵的，
    //可能会导致进入高CPU循环

    //等待获得读写锁，如果有人在写，或者有要写入的人在等待，偏向写优先
    while ((rwlock->rw_refcount_ < 0)
           || (true == rwlock->priority_to_write_ && rwlock->rw_nwaitwriters_ > 0))
    {
        rwlock->rw_nwaitreaders_++;
        //进入wait函数，rw_mutex_会被打开，让其他人活动，出来的时候会获得
        result = zce::pthread_cond_wait(&rwlock->rw_condreaders_,
                                        &(rwlock->rw_mutex_));
        rwlock->rw_nwaitreaders_--;

        if (result != 0)
        {
            break;
        }
    }

    //获得的了读的锁控制
    if (result == 0)
    {
        rwlock->rw_refcount_++;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);
}

//尝试获取读取锁
int zce::ws_rwlock_tryrdlock(ws_rwlock_t* rwlock)
{
    int result = zce::pthread_mutex_lock(&rwlock->rw_mutex_);

    if (result != 0)
    {
        return (result);
    }

    //如果有人在写，或者有要写入的人在等待,那么就不能get读取锁
    if ((rwlock->rw_refcount_ < 0)
        || (true == rwlock->priority_to_write_ && rwlock->rw_nwaitwriters_ > 0))
    {
        result = EBUSY;
    }
    else
    {
        rwlock->rw_refcount_++;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);
}

//读取锁的超时锁定，这个代码UNP V2并没有给出，
int zce::ws_rwlock_timedrdlock(ws_rwlock_t* rwlock,
                               const ::timespec* abs_timeout_spec)
{
    int result = zce::pthread_mutex_timedlock(&rwlock->rw_mutex_,
                                              abs_timeout_spec);

    //我有点理解为啥phtread的很多函数用绝对时间了，abs_timeout_spec
    if (result != 0)
    {
        return (result);
    }

    //等待获得读写锁，如果有人在写，或者有要写入的人在等待，偏向写优先
    while ((rwlock->rw_refcount_ < 0)
           || (true == rwlock->priority_to_write_ && rwlock->rw_nwaitwriters_ > 0))
    {
        rwlock->rw_nwaitreaders_++;
        //进入wait函数，rw_mutex_会被打开，让其他人活动，出来的时候会获得
        result = zce::pthread_cond_timedwait(&rwlock->rw_condreaders_,
                                             &(rwlock->rw_mutex_),
                                             abs_timeout_spec);
        rwlock->rw_nwaitreaders_--;

        if (result != 0)
        {
            break;
        }
    }

    //获得的了读的锁控制
    if (result == 0)
    {
        rwlock->rw_refcount_++;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);
}

//非标准，读取锁的超时锁定，时间参数调整成timeval，
int zce::ws_rwlock_timedrdlock(ws_rwlock_t* rwlock,
                               const timeval* abs_timeout_val)
{
    //这个时间是绝对值时间，要调整为相对时间
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::ws_rwlock_timedrdlock(rwlock,&abs_timeout_spec);
}

//获取写锁
int zce::ws_rwlock_wrlock(ws_rwlock_t* rwlock)
{
    int result = pthread_mutex_lock(&rwlock->rw_mutex_);
    if (result != 0)
    {
        return (result);
    }

    //如果有人在使用锁，无论读写，就要等待，如果读取优先，如果有人还在等待读，也等待
    while ((rwlock->rw_refcount_ != 0)
           || (false == rwlock->priority_to_write_ && rwlock->rw_nwaitreaders_ > 0))
    {
        rwlock->rw_nwaitwriters_++;
        result = zce::pthread_cond_wait(&rwlock->rw_condwriters_,
                                        &(rwlock->rw_mutex_));
        rwlock->rw_nwaitwriters_--;

        if (result != 0)
        {
            break;
        }
    }

    //标识锁被写者获得
    if (result == 0)
    {
        rwlock->rw_refcount_ = -1;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);
}

//尝试能否拥有写锁，非阻塞方式
int zce::ws_rwlock_trywrlock(ws_rwlock_t* rwlock)
{
    int result = zce::pthread_mutex_lock(&rwlock->rw_mutex_);
    if (result != 0)
    {
        return (result);
    }

    //如果有读，写者存在，就不能拥有写锁
    if ((rwlock->rw_refcount_ != 0)
        || (false == rwlock->priority_to_write_ && rwlock->rw_nwaitreaders_ > 0))
    {
        result = EBUSY;
    }
    //否则就拥有写锁
    else
    {
        rwlock->rw_refcount_ = -1;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);
}

//获取写锁，并且等待到超时为止，
int zce::ws_rwlock_timedwrlock(ws_rwlock_t* rwlock,
                               const ::timespec* abs_timeout_spec)
{
    int result = zce::pthread_mutex_timedlock(&rwlock->rw_mutex_,abs_timeout_spec);
    if (result != 0)
    {
        return (result);
    }

    //如果有人在使用锁，无论读写，就要等待，如果读取优先，如果有人还在等待读，也等待
    while ((rwlock->rw_refcount_ != 0)
           || (false == rwlock->priority_to_write_ && rwlock->rw_nwaitreaders_ > 0))
    {
        rwlock->rw_nwaitwriters_++;
        result = zce::pthread_cond_timedwait(&rwlock->rw_condwriters_,
                                             &(rwlock->rw_mutex_),
                                             abs_timeout_spec);
        rwlock->rw_nwaitwriters_--;

        if (result != 0)
        {
            break;
        }
    }

    if (result == 0)
    {
        rwlock->rw_refcount_ = -1;
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);
}

//非标准，读取锁的超时锁定，时间参数调整成timeval，
int zce::ws_rwlock_timedwrlock(ws_rwlock_t* rwlock,
                               const timeval* abs_timeout_val)
{
    //这个时间是绝对值时间，要调整为相对时间
    ::timespec abs_timeout_spec = zce::make_timespec(abs_timeout_val);
    return zce::ws_rwlock_timedwrlock(rwlock,&abs_timeout_spec);
}

//解除锁定，这个函数可以解除读取锁定和写入锁定，不需要特别指明
int zce::ws_rwlock_unlock(ws_rwlock_t* rwlock)
{
    //上锁，
    int result = zce::pthread_mutex_lock(&rwlock->rw_mutex_);
    if (result != 0)
    {
        return (result);
    }

    //如果是读者占用了锁，
    if (rwlock->rw_refcount_ > 0)
    {
        rwlock->rw_refcount_--;
    }
    //如果是作家（写者）占用了锁，
    else if (rwlock->rw_refcount_ == -1)
    {
        rwlock->rw_refcount_ = 0;
    }
    //理论上不会到这儿
    else
    {
        //到这儿，应该是你代码写错了，没有加锁，但是你调用了解锁函数
    }

    //根据读优先还是写入优先，进行处理
    //如果写优先
    if (rwlock->priority_to_write_)
    {
        //如果这时候，有写入的人等待，优先给作家发个信号
        if (rwlock->rw_nwaitwriters_ > 0)
        {
            if (rwlock->rw_refcount_ == 0)
            {
                result = zce::pthread_cond_signal(&rwlock->rw_condwriters_);
            }
        }
        //如果这时候，有读者的在等待，给读者做个广播
        else if (rwlock->rw_nwaitreaders_ > 0)
        {
            result = zce::pthread_cond_broadcast(&rwlock->rw_condreaders_);
        }
    }
    //如果是读取优先
    else
    {
        if (rwlock->rw_nwaitreaders_ > 0)
        {
            result = zce::pthread_cond_broadcast(&rwlock->rw_condreaders_);
        }
        //如果这时候，有写入的人等待，优先给作家发个信号
        else if (rwlock->rw_nwaitwriters_ > 0)
        {
            if (rwlock->rw_refcount_ == 0)
            {
                result = zce::pthread_cond_signal(&rwlock->rw_condwriters_);
            }
        }
        //如果这时候，有读者的在等待，给读者做个广播
    }

    zce::pthread_mutex_unlock(&rwlock->rw_mutex_);
    return (result);
}

#endif //#if defined (ZCE_OS_WINDOWS)