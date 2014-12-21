#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_os_adapt_predefine.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_flock.h"

void ZCE_LIB::flock_adjust_params (zce_flock_t *lock,
                                   int whence,
                                   size_t &start,
                                   size_t &len)
{

#if defined (ZCE_OS_WINDOWS)

    int ret = 0;

    switch (whence)
    {
        case SEEK_SET:
            break;

        case SEEK_CUR:
        {
            LARGE_INTEGER offset;
            LARGE_INTEGER distance;
            distance.QuadPart = 0;

            if (!::SetFilePointerEx (lock->handle_,
                                     distance,
                                     &offset,
                                     FILE_CURRENT))
            {
                errno = ZCE_LIB::last_error();
                return;
            }

            start += static_cast<ssize_t> (offset.QuadPart);
        }
        break;

        case SEEK_END:
        {
            size_t file_size = 0;
            ret = ZCE_LIB::filesize (lock->handle_, &file_size);

            if ( ret != 0)
            {
                return;
            }

            start += file_size;
        }
        break;
    }

    LARGE_INTEGER large_start;
    large_start.QuadPart  = start;

    lock->overlapped_.Offset = large_start.LowPart;
    lock->overlapped_.OffsetHigh = large_start.HighPart;

    //如果长度为0，标识对全文件进行操作，在WINDOWS平台调整成文件长度
    //注意这儿的适配是有一些问题的，因为linux的文件锁如果长度是0，是锁定从开始到文件偏移的最大可能值。
    if (len == 0)
    {
        size_t file_size = 0;
        ret = ZCE_LIB::filesize (lock->handle_, &file_size);

        if ( ret != 0)
        {
            return;
        }

        len = file_size - start;
    }

#elif defined (ZCE_OS_LINUX)

    lock->lock_.l_whence = whence;
    lock->lock_.l_start = start;
    lock->lock_.l_len = len;
#endif
}


//文件锁初始化zce_flock_t,直接用fd
int ZCE_LIB::flock_init (zce_flock_t *lock,
                         ZCE_HANDLE file_hadle)
{

#if defined (ZCE_OS_WINDOWS)

    // Once initialized, these values are never changed.
    lock->overlapped_.Internal = 0;
    lock->overlapped_.InternalHigh = 0;
    lock->overlapped_.hEvent = 0;

    //后面会进行调整。
    lock->overlapped_.Offset = 0;
    lock->overlapped_.OffsetHigh = 0;

#endif

    lock->handle_ = file_hadle;
    return 0;
}

//销毁锁？这样解释不是特别合理，其实应该说，销毁zce_flock_t这个结构
void ZCE_LIB::flock_destroy (zce_flock_t * /*lock*/)
{
    return;
}

//whence == SEEK_SET,SEEK_CUR
int ZCE_LIB::flock_unlock (zce_flock_t *lock,
                           int  whence,
                           size_t start,
                           size_t len)
{
#if defined (ZCE_OS_WINDOWS)

    ZCE_LIB::flock_adjust_params (lock, whence, start, len);

    LARGE_INTEGER large_len;
    large_len.QuadPart  = len;

    //解除锁定
    BOOL ret_bool = ::UnlockFileEx (lock->handle_,
                                    0,
                                    large_len.LowPart,
                                    large_len.HighPart,
                                    &lock->overlapped_);

    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    ZCE_LIB::flock_adjust_params (lock, whence, start, len);
    // Unlock file.
    lock->lock_.l_type = F_UNLCK;
    return ::fcntl(lock->handle_,
                   F_SETLK,
                   reinterpret_cast<long> (&lock->lock_));
#endif

}

int ZCE_LIB::flock_rdlock (zce_flock_t *lock,
                           int  whence,
                           size_t start,
                           size_t len)
{

#if defined (ZCE_OS_WINDOWS)
    ZCE_LIB::flock_adjust_params (lock, whence, start, len);

    LARGE_INTEGER large_len;
    large_len.QuadPart  = len;
    //第2个参数为0表示是共享锁
    BOOL ret_bool = ::LockFileEx (lock->handle_,
                                  0,
                                  0,
                                  large_len.LowPart,
                                  large_len.HighPart,
                                  &lock->overlapped_);

    //出现错误
    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    ZCE_LIB::flock_adjust_params (lock, whence, start, len);
    // set read lock
    lock->lock_.l_type = F_RDLCK;
    //block, if no access 注意F_SETLKW和F_SETLK的区别
    return ::fcntl(lock->handle_,
                   F_SETLKW,
                   reinterpret_cast<long> (&lock->lock_));

#endif
}

//测试是否可以加锁，如果不行立即返回
int ZCE_LIB::flock_tryrdlock (::zce_flock_t *lock,
                              int  whence,
                              size_t start,
                              size_t len)
{

#if defined (ZCE_OS_WINDOWS)

    //调整参数，因为WINDOWS参数的一些麻烦
    ZCE_LIB::flock_adjust_params (lock, whence, start, len);

    LARGE_INTEGER large_len;
    large_len.QuadPart  = len;
    //LOCKFILE_FAIL_IMMEDIATELY表示立即进行测试，失败不阻塞，也就是try了
    BOOL ret_bool = ::LockFileEx (lock->handle_,
                                  LOCKFILE_FAIL_IMMEDIATELY,
                                  0,
                                  large_len.LowPart,
                                  large_len.HighPart,
                                  &lock->overlapped_);

    //出现错误
    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    ZCE_LIB::flock_adjust_params (lock, whence, start, len);
    lock->lock_.l_type = F_RDLCK;         // set read lock

    int result = 0;
    // Does not block, if no access, returns -1 and set errno = EBUSY; 注意F_SETLKW和F_SETLK的区别
    result = ::fcntl(lock->handle_,
                     F_SETLK,
                     reinterpret_cast<long> (&lock->lock_));

    if (result == -1 && (errno == EACCES || errno == EAGAIN))
    {
        errno = EBUSY;
    }

    return 0;
#endif
}

//尝试进行写锁定
int ZCE_LIB::flock_trywrlock (zce_flock_t *lock,
                              int  whence,
                              size_t start,
                              size_t len)
{

#if defined (ZCE_OS_WINDOWS)

    ZCE_LIB::flock_adjust_params (lock, whence, start, len);

    LARGE_INTEGER large_len;
    large_len.QuadPart  = len;

    BOOL ret_bool = ::LockFileEx (lock->handle_,
                                  LOCKFILE_FAIL_IMMEDIATELY | LOCKFILE_EXCLUSIVE_LOCK,
                                  0,
                                  large_len.LowPart,
                                  large_len.HighPart,
                                  &lock->overlapped_);

    //出现错误
    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    ZCE_LIB::flock_adjust_params (lock, whence, start, len);
    lock->lock_.l_type = F_WRLCK;         // set write lock

    int result = 0;
    // Does not block, if no access, returns -1 and set errno = EBUSY;
    result = ::fcntl(lock->handle_,
                     F_SETLK,
                     reinterpret_cast<long> (&lock->lock_));

    if (result == -1 && (errno == EACCES || errno == EAGAIN))
    {
        errno = EBUSY;
    }

    return 0;
#endif
}

int ZCE_LIB::flock_wrlock (zce_flock_t *lock,
                           int whence,
                           size_t start,
                           size_t len)
{
#if defined (ZCE_OS_WINDOWS)

    ZCE_LIB::flock_adjust_params (lock, whence, start, len);

    LARGE_INTEGER large_len;
    large_len.QuadPart  = len;
    //LOCKFILE_EXCLUSIVE_LOCK表示独占锁，也就是写锁
    BOOL ret_bool = ::LockFileEx (lock->handle_,
                                  LOCKFILE_EXCLUSIVE_LOCK,
                                  0,
                                  large_len.LowPart,
                                  large_len.HighPart,
                                  &lock->overlapped_);

    //出现错误
    if (!ret_bool)
    {
        return -1;
    }

    return 0;

#elif defined (ZCE_OS_LINUX)

    ZCE_LIB::flock_adjust_params (lock, whence, start, len);
    // set write lock
    lock->lock_.l_type = F_WRLCK;
    // block, if no access
    return ::fcntl(lock->handle_,
                   F_SETLKW,
                   reinterpret_cast<long> (&lock->lock_));

#endif
}

//LOCK_SH
//Place a shared lock. More than one process may hold a shared lock for a given file at a given time.
//LOCK_EX
//Place an exclusive lock. Only one process may hold an exclusive lock for a given file at a given time.
//LOCK_UN
//Remove an existing lock held by this process.
//LOCK_NB
//A call to flock() may block if an incompatible lock is held by another process. To make a non-blocking request, include LOCK_NB (by ORing) with any of the above operations.

//文件锁函数，只对一个文件进行加锁
//int ZCE_LIB::flock(ZCE_HANDLE file_hadle, int operation)
//{
//#if defined (ZCE_OS_WINDOWS)
//
//    zce_flock_t lock_handle;
//
//    int ret = 0;
//    ret = ZCE_LIB::flock_init (&lock_handle,
//                               file_hadle);
//
//    //读取锁，共享锁
//    if (LOCK_SH & operation )
//    {
//        //如果是不阻塞的
//        if ( LOCK_NB & operation)
//        {
//            ret = ZCE_LIB::flock_tryrdlock (&lock_handle,
//                                            SEEK_SET,
//                                            0,
//                                            0);
//        }
//        //如果是阻塞调用
//        else
//        {
//            ret = ZCE_LIB::flock_rdlock (&lock_handle,
//                                         SEEK_SET,
//                                         0,
//                                         0);
//        }
//
//    }
//    //写锁
//    else if (LOCK_EX & operation)
//    {
//        //如果是不阻塞的
//        if ( LOCK_NB & operation)
//        {
//            ret = ZCE_LIB::flock_trywrlock (&lock_handle,
//                                            SEEK_SET,
//                                            0,
//                                            0);
//        }
//        //如果是阻塞调用
//        else
//        {
//            ret = ZCE_LIB::flock_wrlock (&lock_handle,
//                                         SEEK_SET,
//                                         0,
//                                         0);
//        }
//    }
//    //解开锁
//    else if ( LOCK_UN & operation)
//    {
//        ret = ZCE_LIB::flock_unlock (&lock_handle,
//                                     SEEK_SET,
//                                     0,
//                                     0);
//    }
//    else
//    {
//        errno = EINVAL;
//        ret = -1;
//    }
//
//    if (ret != 0)
//    {
//        ZLOG_MSG(RS_ERROR,"[zcelib] ZCE_LIB::flock fail. operation =%d ret =%d", operation, ret);
//    }
//
//    //其实完全可以不掉用这个函数,调用他是为了让你觉得公正,
//    ZCE_LIB::flock_destroy(&lock_handle);
//
//    return ret;
//
//#elif defined (ZCE_OS_LINUX)
//    return ::flock(file_hadle, operation);
//#endif
//
//}
//
