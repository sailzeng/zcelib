/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/os_adapt/flock.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2011年9月15日
* @brief      文件（记录）锁的函数，在WIN32，和LINUX 平台通用
*             其实除了flock这个函数，叫记录锁更贴切一点，因为你可以只对文件的一部分进行操作
*
* @details    文件（记录）锁的适配层，兼容2个平台，
*             记录锁的，fcntl的操作，没有直接模拟fcntl的操作，部分参考ACE封装。但可以说，
*             ACE的封装还是比调用原生API fcntl舒服的。
*             另外，不同意封装成 fcntl 的另外一个愿意是fcntl是一个大合集。直接用这个函数
*             名字反而会给你错觉。
*             文件锁使用的是flock函数，但不足是，其实Windows下仍然是用的记录锁实现的
*
* @note       而对于记录锁，其其实也是操作系统中有趣的一部分，而且其实个个平台差别不小。
*
*             WINDOWS下，记录锁锁定是区域级别的，作用于整个系统（所有进程、所有句柄），
*             但锁操作和“谁持有这个锁”的状态由具体的句柄决定。
*             比如：
*             HANDLE hFile = CreateFile(...); // 获得句柄
*             // 第一次加独占锁  成功
*             LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, 100, 0, &ol1);
*             // 第二次尝试再加独占锁，非阻塞   失败（锁已存在）
*             LockFileEx(hFile, LOCKFILE_FAIL_IMMEDIATELY | LOCKFILE_EXCLUSIVE_LOCK, 0, 100, 0, &ol2);
*             // 第三次尝试加共享锁，非阻塞     成功！同一进程同一句柄共享锁
*             LockFileEx(hFile, LOCKFILE_FAIL_IMMEDIATELY, 0, 100, 0, &ol3);
*             注意，第三次加锁成功是因为共享锁和独占锁是可以共存的。这个和POSIX的锁是不同的。
*
*             HANDLE h1 = CreateFile(...);
*             HANDLE h2 = CreateFile(...);
*             LockFileEx(h1, LOCKFILE_EXCLUSIVE_LOCK, ...); // 成功
*             LockFileEx(h2, LOCKFILE_FAIL_IMMEDIATELY, ...); // 会失败！因为是另一个 HANDLE
*
*             另外：
*             WINDOWS下是，记录锁是强制的，
*             LINUX下，记录锁是劝告性的锁（默认是），你想干坏事还是能干的。
*             http://www.cnblogs.com/hustcat/archive/2009/03/10/1408208.html
*             但如果要看UNP V2就会明白，其实最好大家还是遵从相互的约定，使用锁。
*
*/

#pragma once

#include "zce/os_adapt/define.h"

namespace zce
{
/*!
* @brief      记录锁对象的封装，
*/
struct file_lock_t
{
public:

# if defined (ZCE_OS_WINDOWS)

    //OVERLAPPED 主要包括一些文件偏移信息
    OVERLAPPED   overlapped_ = { 0 };

# elif defined (ZCE_OS_LINUX)
    //文件锁flock对象
    struct flock lock_;
# endif

    ///处理的文件句柄 Handle to the underlying file.
    ZCE_HANDLE   handle_ = ZCE_INVALID_HANDLE;
};

/*!
* @brief      记录锁，文件锁初始化,直接用文件句柄初始化
* @return         int        0成功，-1失败
* @param[in,out]  lock       文件锁对象
* @param[in]      file_hadle 操作的文件句柄
*/
int file_lock_init(file_lock_t* lock,
                   ZCE_HANDLE file_hadle);

//----------------------------------------------------------------------------------------
// 记录锁的功能，模仿的是fcntl的功能

/*!
* @brief   加文件读取锁，共享锁，如果不能加上锁，会阻塞等待，共享锁不会阻止其他人读取
* @return        int     0成功，-1失败
* @param[in,out] lock    文件锁对象
* @param[in]     whence  计算的起始根源位置，如SEEK_SET，SEEK_CUR，SEEK_END
* @param[in]     start   从根源开始的相对位置
* @param[in]     len     锁定区域的长度，
*/
int fcntl_rdlock(file_lock_t* lock,
                 int  whence = SEEK_SET,
                 ssize_t start = 0,
                 ssize_t len = 0);

/*!
* @brief  尝试进行加读取锁,如果不能加上锁，会立即返回
* @return        int     0成功，-1失败
* @param[in,out] lock    文件锁对象
* @param[in]     whence  计算的起始根源位置，如SEEK_SET，SEEK_CUR，SEEK_END
* @param[in]     start   从根源开始的相对位置
* @param[in]     len     锁定区域的长度，
*/
int fcntl_tryrdlock(file_lock_t* lock,
                    int whence = SEEK_SET,
                    ssize_t start = 0,
                    ssize_t len = 0);

/*!
* @brief      对文件上写锁，排他锁，如果不能加上锁，会阻塞等待
* @return        int     0成功，-1失败
* @param[in,out] lock    文件锁对象
* @param[in]     whence  计算的起始根源位置，如SEEK_SET，SEEK_CUR，SEEK_END
* @param[in]     start   从根源开始的相对位置
* @param[in]     len     锁定区域的长度，
*/
int fcntl_wrlock(file_lock_t* lock,
                 int whence = SEEK_SET,
                 ssize_t start = 0,
                 ssize_t len = 0);

/*!
* @brief      尝试进行加写锁（排他锁）,如果不能加上锁，会立即返回
* @return        int     0成功，-1失败
* @param[in,out] lock    文件锁对象
* @param[in]     whence  计算的起始根源位置，如SEEK_SET，SEEK_CUR，SEEK_END
* @param[in]     start   从根源开始的相对位置
* @param[in]     len     锁定区域的长度，
*/
int fcntl_trywrlock(file_lock_t* lock,
                    int whence = SEEK_SET,
                    ssize_t start = 0,
                    ssize_t len = 0);

/*!
* @brief      解锁
* @return        int     0成功，-1失败
* @param[in,out] lock    文件锁对象
* @param[in]     whence  计算的起始根源位置，如SEEK_SET，SEEK_CUR，SEEK_END
* @param[in]     start   从根源开始的相对位置
* @param[in]     len     解锁定区域的长度，
*/
int fcntl_unlock(file_lock_t* lock,
                 int whence = SEEK_SET,
                 ssize_t start = 0,
                 ssize_t len = 0);

//----------------------------------------------------------------------------------------
// 文件锁的功能，模仿的是flock的功能

/*!
* @brief      文件锁操作
* @return     int
* @param      lock_hadle 注意这个地方是file_lock_t，需要用file_lock_init先初始化
* @param      operation 相应的操作，是LOCK_SH LOCK_EX LOCK_UN LOCK_NB 的组合
*             LOCK_SH 尝试加锁
*             LOCK_EX 强制加锁
*             LOCK_NB 不阻塞，可以和LOCK_SH，LOCK_EX组合使用。用|，如果有锁，立即返回，错误EWOULDBLOCK
*             LOCK_UN 解锁
*/
int flock(file_lock_t& lock_hadle, int operation);
};
