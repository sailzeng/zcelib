#pragma once

#include "zce/os_adapt/common.h"

namespace zce
{
/*!
* @brief      封装SystemV共享内存映射代码，和POSIX和MMAP的最大区别是，
*             没有映射文件
* note
*/
class shm_systemv : public zce::non_copyable
{
public:
    //构造函数
    shm_systemv();
    ~shm_systemv();

public:
    //打开文件，进行映射
    int open(key_t sysv_key,
             std::size_t shm_size,
             int shmget_flg = IPC_CREAT | SHM_R | SHM_W,
             int shmat_flg = 0,
             const void* want_address = nullptr
    );

    //打开文件，进行映射, 简单，推荐使用这个函数
    int open(key_t sysv_key,
             std::size_t shm_size,
             bool fail_if_exist,
             bool read_only = false,
             const void* want_address = nullptr
    );

    //关闭映射(文件)
    int close();

    //删除映射的文件，当然正在映射(或者还有人在映射)的时候不能删除
    int remove();

    //返回映射的内存地址
    void* addr();

protected:

    //key_t，共享内存的唯一标识。看作是个整数把。
    key_t               sysv_key_;

    // 映射的文件句柄
    ZCE_HANDLE          sysv_shmid_;

    //映射的共享内存大小
    std::size_t         shm_size_;

    //映射的内存地址
    void* shm_addr_;
};
}