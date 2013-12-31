//Posix的共享内存在模拟的时候有一个问题，Linux下的Posix的共享内存不光有共享内存的特性，
//还有文件的特性，而且会统一的放到一个目录/dev/shm目录下
//所以不太好用Window的非文件映射贡献内存模拟.
//所以只有换一下方式，在Windows下也找一个公用目录,

#include "zce_predefine.h"
#include "zce_os_adapt_predefine.h"
#include "zce_boost_non_copyable.h"
#include "zce_os_adapt_shm.h"
#include "zce_os_adapt_file.h"
#include "zce_os_adapt_dirent.h"
#include "zce_os_adapt_error.h"
#include "zce_trace_log_debug.h"
#include "zce_share_mem_posix.h"

//构造函数
ZCE_ShareMem_Posix::ZCE_ShareMem_Posix():
    mmap_addr_(NULL),
    mmap_handle_(ZCE_INVALID_HANDLE),
    shm_size_(0)
{
}

ZCE_ShareMem_Posix::~ZCE_ShareMem_Posix()
{
    if (mmap_addr_)
    {
        close();
    }
}

//打开文件，进行映射
int ZCE_ShareMem_Posix::open(const char *shm_name,
                             std::size_t shm_size ,
                             int file_open_mode,
                             int file_perms_mode,
                             const void *want_address,
                             int mmap_prot,
                             int mmap_flags,
                             std::size_t offset)
{
    //避免重入调用open函数，如果出现断言表示多次调用open,
    ZCE_ASSERT(NULL == mmap_addr_);
    ZCE_ASSERT(ZCE_INVALID_HANDLE == mmap_handle_);

    int ret = 0;

    //带入参数是INVALID_HANDLE_VALUE 时，可以使用不用文件映射的共享内存
    //MMAP文件的句柄
    mmap_handle_ = ZCE_OS::shm_open(shm_name, file_open_mode, file_perms_mode);

    //如果没有成功打开文件
    if (mmap_handle_ == ZCE_INVALID_HANDLE)
    {
        ZLOG_ERROR("[zcelib] Posix memory open fail, name=%s ,ZCE_OS::shm_open fail. last error =%d",
                   shm_name,
                   ZCE_OS::last_error());
        return -1;
    }

    shm_name_ = shm_name;

    //如果打开模式使用了O_TRUNC，表示重新打开
    if (file_open_mode & O_TRUNC)
    {
        ZCE_OS::ftruncate(mmap_handle_, static_cast<long>(shm_size + offset));
    }
    else
    {
        //对文件的尺寸进行检查，如果不对，也不进行处理
        size_t filelen = 0;
        ret  = ZCE_OS::filesize(mmap_handle_, &filelen);

        //不能得到文件尺寸
        if ( 0 != ret )
        {
            ZLOG_ERROR("[zcelib] Posix memory open fail, name=%s ,ZCE_OS::filesize ret =%ld last error=%d",
                       shm_name,
                       filelen,
                       ZCE_OS::last_error());
            ZCE_OS::close(mmap_handle_);
            mmap_handle_ = ZCE_INVALID_HANDLE;
            return -1;
        }
        //文件是一个新文件，或者空文件，
        else if ( 0 == filelen )
        {
            ZCE_OS::ftruncate(mmap_handle_, static_cast<long>(shm_size + offset));
        }
        //文件已经存在，而且还有数据(大小)，
        else
        {
            //文件的空间都不够的情况
            if (filelen < static_cast<size_t> (shm_size + offset))
            {
                ZLOG_ERROR("[zcelib] Posix memory open fail, name=%s, old file size(%lu) < request file size(%lu). ",
                           shm_name,
                           filelen,
                           shm_size + offset);
                ZCE_OS::close(mmap_handle_);
                mmap_handle_ = ZCE_INVALID_HANDLE;
                return -1;
            }
        }

    }

    //进行共享内存映射
    void *nonconst_addr = const_cast<void *>(want_address);
    mmap_addr_ = ZCE_OS::mmap (nonconst_addr,
                               shm_size,
                               mmap_prot,
                               mmap_flags,
                               mmap_handle_,
                               offset);

    //
    if (!mmap_addr_)
    {
        ZCE_OS::close(mmap_handle_);
        mmap_handle_ = ZCE_INVALID_HANDLE;
        return -1;
    }

    shm_size_ = shm_size;
    return 0;
}

//打开文件，进行映射, 简单
int ZCE_ShareMem_Posix::open(const char *shm_name,
                             std::size_t shm_size,
                             bool if_restore,
                             bool read_only,
                             bool share_file,
                             const void *want_address,
                             std::size_t  offset )
{
    int file_open_mode = (O_CREAT);
    int mmap_prot = PROT_READ;
    int mmap_flags = 0;
    int file_perms_mode = 0;

    //为什么默认让其他用户也可以读取，因为有些时候我们要用+s的root权限启动服务器，
    if (share_file)
    {
        mmap_flags |= MAP_SHARED;
        file_perms_mode = ZCE_SHARE_FILE_PERMS;
    }
    else
    {
        mmap_flags |= MAP_PRIVATE;
        file_perms_mode = ZCE_PRIVATE_FILE_PERMS;
    }

    //如果不是恢复，对文件进行截断
    if (!if_restore)
    {
        file_open_mode |= (O_TRUNC);
    }

    //如果不是只读的
    if (!read_only)
    {
        file_open_mode |= O_RDWR;
        mmap_prot |= PROT_WRITE;
    }
    else
    {
        file_open_mode |= O_RDONLY;
    }

    //如果有希望设置的地址，
    if (want_address)
    {
        mmap_flags |= MAP_FIXED;
    }

    //
    return this->open(shm_name,
                      shm_size,
                      file_open_mode,
                      file_perms_mode,
                      want_address,
                      mmap_prot,
                      mmap_flags,
                      offset);

}

//关闭文件
int ZCE_ShareMem_Posix::close()
{
    //断言保证不出现没有open就调用close的情况
    ZCE_ASSERT(mmap_addr_ != NULL );
    ZCE_ASSERT(mmap_handle_ != ZCE_INVALID_HANDLE);

    int ret = 0;
    ret = ZCE_OS::munmap(mmap_addr_, shm_size_);
    mmap_addr_ = NULL;
    shm_size_ = 0;

    ZCE_OS::close(mmap_handle_);
    mmap_handle_ = ZCE_INVALID_HANDLE;

    if (ret != 0)
    {
        return -1;
    }

    return 0;
}

//删除映射的文件，当然正在映射的时候不能删除
int ZCE_ShareMem_Posix::remove()
{
    return ZCE_OS::shm_unlink(shm_name_.c_str());
}

//同步文件
int ZCE_ShareMem_Posix::flush()
{
    return ZCE_OS::msync(mmap_addr_, shm_size_, MS_SYNC);
}

//返回映射的内存地址
void *ZCE_ShareMem_Posix::addr()
{
    return mmap_addr_;
}
