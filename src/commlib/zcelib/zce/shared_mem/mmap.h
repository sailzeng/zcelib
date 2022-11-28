/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   mmap.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011-5-1
* @brief      封装MMAP共享内存映射文件函数
*
*
* @details
*
*
*
* @note
*
*/
#pragma once

#include "zce/os_adapt/define.h"
#include "zce/util/non_copyable.h"

namespace zce
{
class shm_mmap
{
public:
    //构造函数
    shm_mmap();
    ~shm_mmap();

    shm_mmap(const shm_mmap &) = delete;
    shm_mmap& operator=(const shm_mmap&) = delete;
public:
    //!打开文件，进行映射
    int open(const char* file_name,
             std::size_t shm_size,
             int file_open_mode = O_CREAT | O_RDWR,
             int file_perms_mode = ZCE_SHARE_FILE_PERMS,
             const void* want_address = nullptr,
             int mmap_prot = PROT_READ | PROT_WRITE,
             int mmap_flags = MAP_SHARED,
             std::size_t offset = 0);

    //!打开文件，进行映射, 简单，推荐使用这个函数
    int open(const char* file_name,
             std::size_t shm_size,
             bool if_restore,
             bool read_only = false,
             bool share_file = true,
             const void* want_address = nullptr,
             std::size_t  offset = 0);

    //关闭文件
    int close();

    //删除映射的文件，当然正在映射的时候不能删除
    int remove();

    //同步文件
    int flush();

    ///返回映射的内存地址
    inline void* addr()
    {
        return mmap_addr_;
    }

    ///返回文件名称
    inline const char* file_name()
    {
        return mmap_file_name_.c_str();
    }
protected:

    //映射文件的名称
    std::string   mmap_file_name_;

    //映射的内存地址
    void*         mmap_addr_ = nullptr;

    // 映射的文件句柄
    ZCE_HANDLE    mmap_handle_ = ZCE_INVALID_HANDLE;

    //映射的共享内存大小
    std::size_t   shm_size_ = 0;
};
} //namespace zce