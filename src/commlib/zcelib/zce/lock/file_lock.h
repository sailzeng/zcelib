/*!
* @copyright  2004-2013  Apache License, Version 2.0 FULLSAIL
* @filename   zce/lock/file_lock.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date       2013年1月14日
* @brief      文件锁，用于整个锁定文件，但实现其实是用记录锁实现的。
*             因为Windows 没有真正意义上的文件锁
*             记录锁清参考@ref zce/lock/record_lock.h
*
* @details
*
* @note
*
*/

#ifndef ZCE_LIB_LOCK_FILE_LOCK_H_
#define ZCE_LIB_LOCK_FILE_LOCK_H_

#include "zce/util/non_copyable.h"
#include "zce/lock/lock_guard.h"

/*!
* @brief      文件锁，对于整个文件进行操作
*/
class ZCE_File_Lock : public zce::NON_Copyable
{
public:

    ///读锁的GUARD
    typedef ZCE_Read_Guard<ZCE_File_Lock> LOCK_READ_GUARD;
    ///写锁的GUARD
    typedef ZCE_Write_Guard<ZCE_File_Lock> LOCK_WRITE_GUARD;

public:

    //构造函数
    ZCE_File_Lock();
    ~ZCE_File_Lock();

public:

    //通过文件名称参数初始化文件锁，会打开这个文件
    int open(const char *file_name,
             int open_mode,
             mode_t perms = ZCE_DEFAULT_FILE_PERMS);

    //通过文件句柄初始化文件锁
    int open(ZCE_HANDLE file_handle);

    //关闭文件锁，如果是我打开的文件，我关闭，
    int close();

    //得到锁文件的句柄
    ZCE_HANDLE get_file_handle();

    //注意这儿的读写锁标识读写锁方式，而不是读写操作

    //读取锁
    void lock_read();
    //尝试读取锁
    bool try_lock_read();

    //写锁定
    void lock_write();
    //尝试读取锁
    bool try_lock_write();

    //解锁,如果是读写锁也只需要这一个函数
    void unlock();

protected:

    ///自己打开的这个文件，也就是调用过open函数
    bool          open_by_self_ = false;

    ///
    size_t        file_len_ = 0;

    ///文件锁句柄
    zce::file_lock_t   file_lock_;

};

#endif //ZCE_LIB_LOCK_FILE_LOCK_H_

