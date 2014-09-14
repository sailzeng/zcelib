#include "zce_predefine.h"
#include "zce_trace_log_debug.h"
#include "zce_os_adapt_error.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_flock.h"
#include "zce_lock_file_lock.h"

//构造函数
ZCE_File_Lock::ZCE_File_Lock():
    lock_file_hadle_(ZCE_INVALID_HANDLE),
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
int ZCE_File_Lock::open(const char *file_name,
                        int open_mode,
                        mode_t perms)
{
    int ret = 0;
    //避免重复打开，用断言保护
    ZCE_ASSERT(lock_file_hadle_ == ZCE_INVALID_HANDLE);

    //打开这个文件
    lock_file_hadle_ = ZCE_LIB::open (file_name,
                                      open_mode,
                                      perms);

    if (lock_file_hadle_ == ZCE_INVALID_HANDLE)
    {
        return -1;
    }

    ret = ZCE_LIB::filesize(lock_file_hadle_, &file_len_);
    if (0 != ret)
    {
        return ret;
    }

    ZCE_LIB::flock_init(&file_lock_hdl_, lock_file_hadle_);

    //标识是自己打开的
    open_by_self_ = true;

    return 0;
}

//通过文件句柄初始化文件锁
int ZCE_File_Lock::open(ZCE_HANDLE file_handle)
{
    int ret = 0;
    lock_file_hadle_ = file_handle;
    ret = ZCE_LIB::filesize(lock_file_hadle_, &file_len_);
    if (0 != ret)
    {
        return ret;
    }

    ZCE_LIB::flock_init(&file_lock_hdl_, lock_file_hadle_);

    return 0;
}

//关闭文件锁
int ZCE_File_Lock::close()
{
    unlock();

    //如果是自己打开的，关闭之
    if (open_by_self_)
    {
        ZCE_LIB::close(lock_file_hadle_);
    }
    ZCE_LIB::flock_destroy(&file_lock_hdl_);
    return 0;
}

//得到锁文件的句柄
ZCE_HANDLE ZCE_File_Lock::get_file_handle()
{
    return lock_file_hadle_;
}

//读取锁
void ZCE_File_Lock::lock_read()
{
    int ret = 0;
    ret = ZCE_LIB::flock_rdlock(&file_lock_hdl_, SEEK_SET, 0, file_len_);

    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::flock LOCK_SH", ret);
        return;
    }

    return;
}
//尝试读取锁
bool ZCE_File_Lock::try_lock_read()
{
    int ret = 0;

    ret = ZCE_LIB::flock_tryrdlock(&file_lock_hdl_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::flock LOCK_SH|LOCK_NB", ret);
        return false;
    }

    return true;
}

//写锁定
void ZCE_File_Lock::lock_write()
{
    int ret = 0;
    ret = ZCE_LIB::flock_wrlock(&file_lock_hdl_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::flock LOCK_EX", ret);
        return;
    }
}
//尝试读取锁
bool ZCE_File_Lock::try_lock_write()
{
    int ret = 0;

    ret = ZCE_LIB::flock_trywrlock(&file_lock_hdl_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::flock LOCK_EX|LOCK_NB", ret);
        return false;
    }

    return true;
}

//解锁,如果是读写锁也只需要这一个函数
void ZCE_File_Lock::unlock()
{
    int ret = 0;

    ret = ZCE_LIB::flock_unlock(&file_lock_hdl_, SEEK_SET, 0, file_len_);
    if (0 != ret)
    {
        ZCE_TRACE_FAIL_RETURN(RS_ERROR, "ZCE_LIB::flock LOCK_UN", ret);
        return;
    }
}

