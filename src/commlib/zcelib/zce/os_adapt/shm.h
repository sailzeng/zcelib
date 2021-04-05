
#ifndef ZCE_LIB_OS_ADAPT_SHARE_MEMORY_H_
#define ZCE_LIB_OS_ADAPT_SHARE_MEMORY_H_



#include "zce/os_adapt/common.h"

namespace zce
{
//-------------------------------------------------------------------------------------------------
//POSIX的共享内存

//将共享内存和文件进行映射
void *mmap (void *addr,
            size_t len,
            int prot,
            int flags,
            ZCE_HANDLE handle,
            size_t off = 0);

//
int mprotect (const void *addr,
              size_t len,
              int prot);

//将内存变化同步到文件中去，不建议你频繁调用这个函数，OS会自动做这个事情，
//一般情况，你在退出时调用一次就OK了。
int msync (void *addr,
           size_t len,
           int sync = MS_SYNC);

//解除文件映射
int munmap (void *addr,
            size_t len);

//int madvise (caddr_t addr,
//    size_t len,
//    int map_advice);

//Posix内存的open函数shm_open，有文件名称，但没有实际文件的映射
//Create/open POSIX shared memory objects
ZCE_HANDLE shm_open (const char *filename,
                     int mode,
                     mode_t perms = 0);

//Posix内存的删除文件
////Create/open POSIX shared memory objects
int shm_unlink (const char *path);


//-------------------------------------------------------------------------------------------------
//SystemV的共享内存

//提供这组模拟对我来说仅仅是为了好玩，（当然也由于System V有广大的深厚群众基础，至少让你移植起来容易一点）
//我个人对System V的IPC没有爱，一方面毕竟不如POSIX IPC在标准上站住了脚，System V的IPC这方面要弱一点，
//另一方面System V IPC 的接口设计也不如POSIX那么优雅，


/*!
* @brief      创建或者访问一个共享内存区
* @return     ZCE_HANDLE
* @param      sysv_key
* @param      size
* @param      shmflg
* @note
*/
ZCE_HANDLE shmget(key_t sysv_key, size_t size, int shmflg);


/*!
* @brief      打开已经shmget的共享内存区
* @return     void*
* @param      shmid
* @param      shmaddr
* @param      shmflg
* @note
*/
void *shmat(ZCE_HANDLE shmid, const void *shmaddr, int shmflg);


/*!
* @brief      短接这个内存区
* @return     int
* @param      shmaddr
* @note
*/
int shmdt(const void *shmaddr);


/*!
* @brief      对共享内存区提供多种操作
* @return     int
* @param      shmid
* @param      cmd
* @param      buf
* @note
*/
int shmctl(ZCE_HANDLE shmid, int cmd, struct shmid_ds *buf);


};



#endif //ZCE_LIB_OS_ADAPT_SHARE_MEMORY_H_


