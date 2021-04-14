/******************************************************************************************
Copyright           : 2002-2003, FXL Technology (Shenzhen) Company Limited.
FileName            : zce/os_adapt/layer.h
Author              : Sail (ZENGXING)/Author name here
Version             :
Date Of Creation    : 2011-5-1
Description         : 时间操作的适配器层，主要还是向LINUX下靠拢

Others              :
Function List       :
1.  ......
Modification History:
1.Date  :
Author  :
Modification  :
******************************************************************************************/
#include "zce/predefine.h"
#include "zce/os_adapt/common.h"
#include "zce/util/non_copyable.h"
#include "zce/os_adapt/shm.h"
#include "zce/os_adapt/file.h"
#include "zce/os_adapt/error.h"
#include "zce/logger/logging.h"
#include "zce/shared_mem/mmap.h"

//构造函数
ZCE_ShareMem_Mmap::ZCE_ShareMem_Mmap():
    mmap_addr_(NULL),
    mmap_handle_(ZCE_INVALID_HANDLE),
    shm_size_(0)
{
}

ZCE_ShareMem_Mmap::~ZCE_ShareMem_Mmap()
{
    if (mmap_addr_)
    {
        close();
    }
}

//打开文件，进行映射
int ZCE_ShareMem_Mmap::open(const char *file_name,
                            std::size_t shm_size,
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

    mmap_handle_ = zce::open(file_name, file_open_mode, file_perms_mode);

    //如果没有成功打开文件
    if (mmap_handle_ == ZCE_INVALID_HANDLE)
    {
        ZCE_LOG(RS_ERROR, "[zcelib] Mmap(file map) memory open fail ,zce::shm_open fail. last error =%d",
                zce::last_error());
        return -1;
    }

    // 在访问内存时，文件句柄不能关闭
    mmap_file_name_ = file_name;

    //如果打开模式使用了O_TRUNC，表示重新打开
    if (file_open_mode & O_TRUNC)
    {
        zce::ftruncate(mmap_handle_, static_cast<long>(shm_size + offset));
    }
    else
    {
        //对文件的尺寸进行检查，如果不对，也不进行处理
        size_t filelen =  0;
        ret =  zce::filesize(mmap_handle_, &filelen);

        if ( 0 != ret)
        {
            ZCE_LOG(RS_ERROR, "[zcelib]  Mmap(file map) memory  open fail ,zce::file size ret =%ld last error=%d",
                    filelen,
                    zce::last_error());
            zce::close(mmap_handle_);
            mmap_handle_ = ZCE_INVALID_HANDLE;
            return -1;
        }
        //文件是一个新文件，或者空文件，
        else if ( 0 == filelen )
        {
            zce::ftruncate(mmap_handle_, static_cast<long>(shm_size + offset));
        }
        //文件已经存在，而且还有数据(大小)，
        else
        {
            //文件的空间都不够的情况
            if (filelen < static_cast<size_t> (shm_size + offset))
            {
                ZCE_LOG(RS_ERROR, "[zcelib] Posix memory open fail ,old file size(%lu) < request file size(%lu). ",
                        filelen,
                        shm_size + offset);
                zce::close(mmap_handle_);
                mmap_handle_ = ZCE_INVALID_HANDLE;
                return -1;
            }
        }
    }

    //进行共享内存映射
    void *nonconst_addr = const_cast<void *>( want_address);
    mmap_addr_ = zce::mmap (nonconst_addr,
                            shm_size,
                            mmap_prot,
                            mmap_flags,
                            mmap_handle_,
                            offset);

    //返回值不正确
    if (!mmap_addr_ ||  MAP_FAILED == mmap_addr_)
    {
        // 打开到一半，失败鸟，文件句柄还是需要关闭啊
        zce::close(mmap_handle_);
        mmap_handle_ = ZCE_INVALID_HANDLE;
        return -1;
    }

    //记录设置的尺寸
    shm_size_ = shm_size;

    return 0;
}

//打开文件，进行映射, 简单
int ZCE_ShareMem_Mmap::open(const char *file_name,
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
    return this->open(file_name,
                      shm_size,
                      file_open_mode,
                      file_perms_mode,
                      want_address,
                      mmap_prot,
                      mmap_flags,
                      offset);

}

//关闭文件
int ZCE_ShareMem_Mmap::close()
{
    //断言保证不出现没有open就调用close的情况
    ZCE_ASSERT(mmap_addr_ != NULL );
    ZCE_ASSERT(mmap_handle_ != ZCE_INVALID_HANDLE);

    int ret = 0;
    ret = zce::munmap (mmap_addr_, shm_size_);
    mmap_addr_ = NULL;

    // 关闭文件
    zce::close(mmap_handle_);
    mmap_handle_ = ZCE_INVALID_HANDLE;

    if (ret != 0)
    {
        return -1;
    }

    return 0;
}

//删除映射的文件，当然正在映射的时候不能删除
int ZCE_ShareMem_Mmap::remove()
{
    return zce::unlink(mmap_file_name_.c_str());
}

//同步文件
int ZCE_ShareMem_Mmap::flush()
{
    return zce::msync(mmap_addr_, shm_size_, MS_SYNC);
}



