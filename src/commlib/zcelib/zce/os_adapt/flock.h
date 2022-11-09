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
*             LINUX下，记录锁是劝告性的锁（默认是），你想干坏事还是能干的。
*             WINDOWS下是，记录锁是强制的，
*             http://www.ibm.com/developerworks/cn/linux/l-cn-filelock/index.html
*             http://www.cnblogs.com/hustcat/archive/2009/03/10/1408208.html
*             但如果要看UNP V2就会明白，其实最好大家还是遵从相互的约定，使用锁。
*
*/

#ifndef ZCE_LIB_OS_ADAPT_FLOCK_H_
#define ZCE_LIB_OS_ADAPT_FLOCK_H_

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

//

/*!
* @brief      调整参数，主要是让他在Windows平台也能使用 内部函数,外部不要使用，
*             两个平台在参数的使用上并不太相同，WINDOWS的API没有考虑相对位置这些概念，
*             所以必须在使用前进行一下调整，比如，开始位置SEEK_SET,0,长度0，其实是锁定
*             整个文件，但LockFileEx没有这样的表示方法，必须调整
* @param[in,out] lock    文件锁对象
* @param[in]     whence  计算的起始根源位置，如SEEK_SET，SEEK_CUR，SEEK_END
* @param[in]     start   从根源开始的相对位置
* @param[in]     len     锁定区域的长度，
* @note          平台的不兼容会带来某种风险，Windows下一旦文件大小调整，锁锁定的区域就不对了，
*                所以在需要兼容的环境，最好文件大小是不调整的，
*/
void fcntl_lock_adjust_params(file_lock_t* lock,
                              int whence,
                              ssize_t& start,
                              ssize_t& len);

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

#endif //ZCE_LIB_OS_ADAPT_FLOCK_H_
