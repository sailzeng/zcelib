
#ifndef ZCE_LIB_SHARE_MEMORY_SYSTEM_V_H_
#define ZCE_LIB_SHARE_MEMORY_SYSTEM_V_H_

#include "zce_os_adapt_predefine.h"

/*********************************************************************************
class ZCE_ShareMem_SystemV 封装SystemV共享内存映射代码，和POSIX和MMAP的最大区别是，没有映射文件
*********************************************************************************/
class ZCE_ShareMem_SystemV : public ZCE_NON_Copyable
{

public:
    //构造函数
    ZCE_ShareMem_SystemV();
    ~ZCE_ShareMem_SystemV();

public:
    //打开文件，进行映射
    int open(key_t sysv_key,
             std::size_t shm_size,
             int shmget_flg = IPC_CREAT | SHM_R | SHM_W,
             int shmat_flg = 0,
             const void *want_address = NULL
            );

    //打开文件，进行映射, 简单，推荐使用这个函数
    int open(key_t sysv_key,
             std::size_t shm_size,
             bool fail_if_exist,
             bool read_only = false,
             const void *want_address = NULL
            );

    //关闭映射(文件)
    int close();

    //删除映射的文件，当然正在映射(或者还有人在映射)的时候不能删除
    int remove();

    //返回映射的内存地址
    void *addr();

protected:

    //key_t，共享内存的唯一标识。看作是个整数把。
    key_t               sysv_key_;

    // 映射的文件句柄
    ZCE_HANDLE          sysv_shmid_;

    //映射的共享内存大小
    std::size_t         shm_size_;

    //映射的内存地址
    void               *shm_addr_;
};

#endif //ZCE_LIB_SHARE_MEMORY_SYSTEM_V_H_

