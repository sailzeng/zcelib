/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce_lock_record_lock.h
* @author     Sailzeng <sailerzeng@gmail.com>
* @version
* @date       2012年10月1日
* @brief      记录锁的封装，用于锁定文件中的一部分字节，字段
*             只是OS层上薄薄的一层封装，
*             另外由于，记录的特殊原因，他不能从ZCE_Lock_Base上面继承了，
* @details    有些DB，和我见过的某些产品曾经用记录锁来用同步用户的某些操作，
*             建议参考一下@ref zce_os_adapt_flock.h
* @note       由于Windows 下记录锁，锁区域参数size 为0时，并不锁定从起始偏移
*             到文件偏移的最大值，所以，所以如果文件大小变化，会出现部分区域没有锁定
*
*/
#ifndef ZCE_LIB_LOCK_RECORD_LOCK_H_
#define ZCE_LIB_LOCK_RECORD_LOCK_H_

#include "zce_lock_base.h"
#include "zce_os_adapt_flock.h"

/*!
* @brief      记录锁，用于锁定文件中的一部分字节，字段
*
*/
class ZCE_Record_Lock : public ZCE_NON_Copyable
{

public:

    /*!
    * @brief      构造函数
    * @param      file_handle 文件句柄
    */
    ZCE_Record_Lock(ZCE_HANDLE file_handle);

    /// 构造函数
    ZCE_Record_Lock();
    /// 析构函数
    ~ZCE_Record_Lock();

public:

    /*!
    * @brief      打开一个文件,同时初始化关联的lock对象
    * @return     int 0成功，-1失败
    * @param[in]  file_name 文件名称
    * @param[in]  open_mode 打开模式
    * @param[in]  perms     权限参数
    */
    int open(const char *file_name,
             int open_mode,
             mode_t perms = ZCE_DEFAULT_FILE_PERMS);

    /*!
    * @brief      用一个文件Handle初始化
    * @return     int   0成功，-1失败
    * @param      file_handle 已经打开的文件的句柄，
    */
    int open(ZCE_HANDLE file_handle);

    /*!
    * @brief      关闭之，
    */
    void close();

    /*!
    * @brief      得到记录锁文件的句柄
    * @return     ZCE_HANDLE
    */
    ZCE_HANDLE get_file_handle();

    //注意这儿的读写锁标识读写锁方式，而不是读写操作
    //而且基于劝告性或者强制性的锁，对于读写

    ///文件记录读写锁,参数参考@ref flock_rdlock
    int flock_rdlock (int  whence,
                      ssize_t start,
                      ssize_t len);

    ///尝试对文件记录进行加读取锁
    int flock_tryrdlock (int  whence,
                         ssize_t start,
                         ssize_t len);

    ///对文件记录直接上锁
    int flock_wrlock (int  whence,
                      ssize_t start,
                      ssize_t len);

    ///尝试对文件记录进行加写取锁
    int flock_trywrlock (int  whence,
                         ssize_t start,
                         ssize_t len);

    ///解锁
    int flock_unlock (int  whence,
                      ssize_t start,
                      ssize_t len);

protected:

    ///记录锁，
    zce_flock_t    record_lock_;

};

#endif //ZCE_LIB_LOCK_RECORD_LOCK_H_
