#include "zce/predefine.h"
#include "zce/logger/logging.h"
#include "zce/os_adapt/error.h"
#include "zce/os_adapt/file.h"
#include "zce/os_adapt/flock.h"
#include "zce/lock/file_lock.h"

//构造函数
ZCE_File_Lock::ZCE_File_Lock() :
    open_by_self_(false),
    file_len_(0)
{
}

//析构函数
ZCE_File_Lock::~ZCE_File_Lock()
{
    close();
}

//通过文件名称参数初始化文件锁，会打开这个文件
int ZCE_File_Lock::open(const char* file_name,
                        int open_mode,
                        mode_t perms)
{
    int ret = 0;
    //避免重复打开，用断言保护
    ZCE_ASSERT(file_lock_.handle_ == ZCE_INVALID_HANDLE);

    //打开这个文件
    ZCE_HANDLE file_handle = zce::open(file_name,
                                       open_mode,
                                       perms);

    if (file_handle == ZCE_INVALID_HANDLE)
    {
        return -1;
    }

    ret = zce::filesize(file_handle, &file_len_);
    if (0 != ret)
    {
        return ret;
    }

    zce::file_lock_init(&file_lock_, file_handle);

    //标识是自己打开的
    open_by_self_ = true;

    return 0;
}

//通过文件句柄初始化文件锁
int ZCE_File_Lock::open(ZCE_HANDLE file_handle)
{
    int ret = 0;
    ret = zce::filesize(file_handle, &file_len_);
    if (0 != ret)
    {
        return ret;
    }

    zce::file_lock_init(&file_lock_, file_handle);

    return 0;
}

//关闭文件锁
int ZCE_File_Lock::close()
{
    unlock();

    //如果是自己打开的，关闭之
    if (open_by_self_)
    {
        zce::close(file_lock_.handle_);
    }
    return 0;
}

//得到锁文件的句柄
ZCE_HANDLE ZCE_File_Lock::get_file_handle()
{
    return file_lock_.handle_;
}

//读取锁
void ZCE_File_Lock::lock_read()
{
    int ret = 0;
    ret = zce::fcntl_rdlock(&file_lock_, SEEK_SET, 0, file_len_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::flock LOCK_SH", ret);
        return;
    }

    return;
}
//尝试读取锁
bool ZCE_File_Lock::try_lock_read()
{
    int ret = 0;

    ret = zce::fcntl_tryrdlock(&file_lock_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::flock LOCK_SH|LOCK_NB", ret);
        return false;
    }

    return true;
}

//写锁定
void ZCE_File_Lock::lock_write()
{
    int ret = 0;
    ret = zce::fcntl_wrlock(&file_lock_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::fcntl_wrlock LOCK_EX", ret);
        return;
    }
}
//尝试读取锁
bool ZCE_File_Lock::try_lock_write()
{
    int ret = 0;

    ret = zce::fcntl_trywrlock(&file_lock_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::try_lock_write LOCK_EX|LOCK_NB", ret);
        return false;
    }

    return true;
}

//解锁,如果是读写锁也只需要这一个函数
void ZCE_File_Lock::unlock()
{
    int ret = 0;

    ret = zce::fcntl_unlock(&file_lock_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "zce::fcntl_unlock LOCK_UN", ret);
        return;
    }
}