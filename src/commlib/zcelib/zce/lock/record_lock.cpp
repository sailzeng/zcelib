#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/file.h"
#include "zce/lock/record_lock.h"

//记录锁的封装，
namespace zce
{
//构造函数
record_lock::record_lock(ZCE_HANDLE file_handle)
{
    open(file_handle);
}

record_lock::record_lock()
{
}
record_lock::~record_lock()
{
    close();
}

//!打开一个文件,同时初始化关联的lock对象
int record_lock::open(const char* file_name,
                      int open_mode,
                      mode_t perms)
{
    //!避免重复打开，用断言保护
    ZCE_ASSERT(record_lock_.handle_ == ZCE_INVALID_HANDLE);
    //!打开这个文件
    ZCE_HANDLE file_handle = zce::open(file_name,
                                       open_mode,
                                       perms);

    if (file_handle == ZCE_INVALID_HANDLE)
    {
        return -1;
    }
    //!标识是自己打开的
    open_by_self_ = true;

    return zce::file_lock_init(&record_lock_,
                               file_handle);
}

//!用一个文件Handle初始化,外部传入的ZCE_HANDLE，我不会关闭，文件
int record_lock::open(ZCE_HANDLE file_handle)
{
    return zce::file_lock_init(&record_lock_, file_handle);
}

//!得到锁文件的句柄
ZCE_HANDLE record_lock::get_file_handle()
{
    return record_lock_.handle_;
}

//!关闭之，如果是Record_Lock内部自己打开的文件（不是文件句柄参数），关闭时会关闭文件
void record_lock::close()
{
    if (open_by_self_)
    {
        zce::close(record_lock_.handle_);
    }
    return;
}

//!文件记录读写锁
int record_lock::flock_rdlock(int  whence,
                              ssize_t start,
                              ssize_t len)
{
    return zce::fcntl_rdlock(&record_lock_,
                             whence,
                             start,
                             len);
}

//!尝试对文件记录进行加读取锁
int record_lock::flock_tryrdlock(int  whence,
                                 ssize_t start,
                                 ssize_t len)
{
    return zce::fcntl_tryrdlock(&record_lock_,
                                whence,
                                start,
                                len);
}

//!对文件记录直接上锁
int record_lock::flock_wrlock(int  whence,
                              ssize_t start,
                              ssize_t len)
{
    return zce::fcntl_wrlock(&record_lock_,
                             whence,
                             start,
                             len);
}

//!尝试对文件记录进行加写取锁
int record_lock::flock_trywrlock(int  whence,
                                 ssize_t start,
                                 ssize_t len)
{
    return zce::fcntl_trywrlock(&record_lock_,
                                whence,
                                start,
                                len);
}

//!解锁
int record_lock::flock_unlock(int  whence,
                              ssize_t start,
                              ssize_t len)
{
    return zce::fcntl_unlock(&record_lock_,
                             whence,
                             start,
                             len);
}
}