#pragma once

#include "zce/os_adapt/define.h"

/*!
* @brief      封装POSIX共享内存映射代码，
*
* note
*/
namespace zce
{
class shm_posix
{
public:
    //构造函数
    shm_posix();
    ~shm_posix();

    shm_posix(const shm_posix &) = delete;
    shm_posix& operator=(const shm_posix&) = delete;
public:
    //打开文件，进行映射
    int open(const char* file_name,
             std::size_t shm_size,
             int file_open_mode = O_CREAT | O_RDWR,
             int file_perms_mode = ZCE_SHARE_FILE_PERMS,
             const void* want_address = nullptr,
             int mmap_prot = PROT_READ | PROT_WRITE,
             int mmap_flags = MAP_SHARED,
             std::size_t offset = 0
    );

    //打开文件，进行映射, 简单，推荐使用这个函数
    int open(const char* file_name,
             std::size_t shm_size,
             bool if_restore,
             bool read_only = false,
             bool share_file = true,
             const void* want_address = nullptr,
             std::size_t  offset = 0
    );

    //关闭映射(文件)
    int close();

    //删除映射的文件，当然正在映射(或者还有人在映射)的时候不能删除
    int remove();

    //同步文件
    int flush();

    //返回映射的内存地址
    void* addr();

protected:

    //映射文件的名称
    std::string  shm_name_;

    //映射的内存地址
    void*        mmap_addr_ = nullptr;

    // 映射的文件句柄
    ZCE_HANDLE   mmap_handle_ = ZCE_INVALID_HANDLE;

    //映射的共享内存大小
    std::size_t  shm_size_ = 0;
};
}