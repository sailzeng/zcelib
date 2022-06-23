

#pragma once
#include "zce/os_adapt/common.h"


//!
namespace zce::aio
{
//!
enum class FS_TYPE
{
    FS_OPEN,
    FS_CLOSE,
    FS_READ,
    FS_WRITE,
    FS_STAT,
    FS_FTRUNCATE,
    FS_UNLINK,
    FS_RMDIR,
    FS_MKDIR,
    FS_RENAME,
    FS_SCANDIR,
};
//! 
class FS_TYPE_t
{
    //!
    FS_TYPE  fs_type_;
    //!
    int result_ = 0;
    //
    char* path_ = nullptr;
    //!
    int open_flags_ = 0;
    //!
    int open_mode_ = 0;

    //! 
    ZCE_HANDLE handle_ = ZCE_INVALID_HANDLE;
    //!
    
    //!

};


class base
{
public:
    //!
    base(zce::aio::manager*);
    ~base() = default;

    //!打开某个文件
    int fs_open(const char* path,
                int flags,
                int mode);

    //!关闭某个文件
    int fs_close(ZCE_HANDLE file);


    //!
    int fs_read(ZCE_HANDLE file,
                const char bufs[],
                size_t nbufs,
                int64_t offset);
    //!
    int fs_unlink(const char* path);
    //!
    int fs_write(ZCE_HANDLE file,
                 const char bufs[],
                 unsigned int nbufs,
                 int64_t offset);
    //!
    int fs_rename(const char* path,
                  const char* new_path);
    //!
    int fs_ftruncate(ZCE_HANDLE file,
                     int64_t offset);
protected:
    //!

protected:
    //!
    int id_;
    //!
    zce::aio::manager* manager_ = nullptr;
    
};

}