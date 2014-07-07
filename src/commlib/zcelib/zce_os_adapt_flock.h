/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_os_adapt_flock.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2011年9月15日
* @brief      文件（记录）锁的函数，在WIN32，和LINUX 平台通用
*             其实除了flock这个函数，叫记录锁更贴切一点，因为你可以只对文件的一部分进行操作
*
* @details    文件（记录）锁的适配层，兼容2个平台，参考ACE实现的，这个几乎就是COPY
*             flock_xxx ，的函数都是非标准的，是ACE自己的一层封装。但可以说，ACE的封装
*             还是比调用原生API fcntl舒服
*             另外ACE没有提供flock函数，我提供了，对某个文件的全部内容进行操作是使用
*
* @note       而对于记录锁，其其实也是操作系统中有趣的一部分，而且其实个个平台差别不小。
*             LINUX下，记录锁是劝告性的锁（默认是），
*             WINDOWS下是，记录锁是强制的，
*             http://www.ibm.com/developerworks/cn/linux/l-cn-filelock/index.html
*             http://www.cnblogs.com/hustcat/archive/2009/03/10/1408208.html
*             但如果要看UNP V2就会明白，其实最好大家还是遵从相互的约定，使用锁。
*
*/

#ifndef ZCE_LIB_OS_ADAPT_FLOCK_H_
#define ZCE_LIB_OS_ADAPT_FLOCK_H_

#include "zce_os_adapt_predefine.h"

/*!
* @brief      文件锁对象的封装，
*/
struct zce_flock_t
{
    //构造函数和析构函数
    zce_flock_t():
        handle_(ZCE_INVALID_HANDLE),
        open_by_self_(false)
    {
    }
    ~zce_flock_t()
    {
    }

public:

# if defined (ZCE_OS_WINDOWS)

    //OVERLAPPED 主要包括一些文件偏移信息
    OVERLAPPED   overlapped_;

# elif defined (ZCE_OS_LINUX)
    //文件锁flock对象
    struct flock lock_;
# endif

    ///处理的文件句柄 Handle to the underlying file.
    ZCE_HANDLE   handle_;

    ///文件是否是被flock_init函数打开的，如果是自己打开的，在销毁时会关闭文件。
    ///2个flock_init函数中有一个是直接传递handle_，此时open_by_self_ 为false
    ///另外一个传递文件名称作为参数，此时open_by_self_ 为true
    bool         open_by_self_;

};

namespace ZCE_OS
{

//----------------------------------------------------------------------------------------
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
*/
void flock_adjust_params (zce_flock_t *lock,
                          int whence,
                          size_t &start,
                          size_t &len);

/*!
* @brief      文件锁初始化,根据文件名称等参数打开这个文件
* @return         int         0成功，-1失败
* @param[in,out]  lock        文件锁对象
* @param[in]      file_name   文件名称，路径
* @param[in]      open_mode   打开文件的模式
* @param[in]      perms       文件的共享模式
*/
int flock_init (zce_flock_t *lock,
                const char *file_name,
                int open_mode,
                mode_t perms = ZCE_DEFAULT_FILE_PERMS);

/*!
* @brief      文件锁初始化,直接用fd
* @return         int        0成功，-1失败
* @param[in,out]  lock       文件锁对象
* @param[in]      file_hadle 操作的文件句柄
*/
int flock_init (zce_flock_t *lock,
                ZCE_HANDLE file_hadle);

/*!
* @brief      销毁文件锁对象zce_flock_t，也可以同时解开锁，可以同时关闭文件
* @return     int  0成功，-1失败
* @param[in]  lock 文件锁对象
* @note       如果是flock_init函数打开的文件，会销毁掉
*/
int flock_destroy (zce_flock_t *lock);

/*!
* @brief   加文件读取锁，共享锁，如果不能加上锁，会阻塞等待，共享锁不会阻止其他人读取
* @return        int     0成功，-1失败
* @param[in,out] lock    文件锁对象
* @param[in]     whence  计算的起始根源位置，如SEEK_SET，SEEK_CUR，SEEK_END
* @param[in]     start   从根源开始的相对位置
* @param[in]     len     锁定区域的长度，
*/
int flock_rdlock (zce_flock_t *lock,
                  int  whence = SEEK_SET,
                  size_t start = 0,
                  size_t len = 0);

/*!
* @brief  尝试进行加读取锁,如果不能加上锁，会立即返回
* @return        int     0成功，-1失败
* @param[in,out] lock    文件锁对象
* @param[in]     whence  计算的起始根源位置，如SEEK_SET，SEEK_CUR，SEEK_END
* @param[in]     start   从根源开始的相对位置
* @param[in]     len     锁定区域的长度，
*/
int flock_tryrdlock (zce_flock_t *lock,
                     int  whence = SEEK_SET,
                     size_t start = 0,
                     size_t len = 0);

/*!
* @brief      对文件上写锁，排他锁，如果不能加上锁，会阻塞等待
* @return        int     0成功，-1失败
* @param[in,out] lock    文件锁对象
* @param[in]     whence  计算的起始根源位置，如SEEK_SET，SEEK_CUR，SEEK_END
* @param[in]     start   从根源开始的相对位置
* @param[in]     len     锁定区域的长度，
*/
int flock_wrlock (zce_flock_t *lock,
                  int  whence = SEEK_SET,
                  size_t start = 0,
                  size_t len = 0);

/*!
* @brief      尝试进行加写锁（排他锁）,如果不能加上锁，会立即返回
* @return        int     0成功，-1失败
* @param[in,out] lock    文件锁对象
* @param[in]     whence  计算的起始根源位置，如SEEK_SET，SEEK_CUR，SEEK_END
* @param[in]     start   从根源开始的相对位置
* @param[in]     len     锁定区域的长度，
*/
int flock_trywrlock (zce_flock_t *lock,
                     int  whence = SEEK_SET,
                     size_t start = 0,
                     size_t len = 0);

/*!
* @brief      解锁
* @return        int     0成功，-1失败
* @param[in,out] lock    文件锁对象
* @param[in]     whence  计算的起始根源位置，如SEEK_SET，SEEK_CUR，SEEK_END
* @param[in]     start   从根源开始的相对位置
* @param[in]     len     解锁定区域的长度，
*/
int flock_unlock (zce_flock_t *lock,
                  int  whence = SEEK_SET,
                  size_t start = 0,
                  size_t len = 0);

//-----------------------------------------------------------------------------------------
//文件锁


/*!
* @brief      文件锁函数，只对整个文件进行加锁操作  flock - apply or remove an advisory lock on an open file
* @return     int         0成功，-1失败
* @param[in]  file_hadle  操作的文件句柄
* @param[in]  operation   参数为LOCK_SH，LOCK_EX，LOCK_UN，另外读写锁可以加入LOCK_NB标识非阻塞
*                         LOCK_SH : 共享锁
*                         LOCK_EX : 排他锁
*                         LOCK_UN : 解锁
*                         LOCK_NB : 非阻塞，try
*/
int flock(ZCE_HANDLE file_hadle, int operation);

};

#endif //ZCE_LIB_OS_ADAPT_FLOCK_H_

