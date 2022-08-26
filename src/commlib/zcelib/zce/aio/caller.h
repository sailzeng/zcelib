

#pragma once
#include "zce/os_adapt/common.h"


//!
namespace zce::aio
{
class Worker;
//!
enum class AIO_TYPE
{
    FS_OPEN,
    FS_CLOSE,
    FS_LSEEK,
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

struct AIO_base
{
    //!
    AIO_TYPE  aio_type_;

    //!
    uint32_t id_;

    //!
    
};

//! FS文件
struct AIO_FS :public AIO_base
{
public:
    //!
    int result_ = 0;
    //
    char* path_ = nullptr;
    //!
    int flags_ = 0;
    //!
    int mode_ = 0;
    //! 
    ZCE_HANDLE handle_ = ZCE_INVALID_HANDLE;
    //!文件偏移的参数
    ssize_t offset_ = 0;
    int whence_ = SEEK_CUR;
    //!
    char* read_bufs_ = nullptr;
    const char* write_bufs_ = nullptr;
    size_t nbufs_ = 0;
    //!
    struct stat* file_stat_;
    //!

    //!

};

class AIO_MySQL :public AIO_base
{

};



class Caller
{
public:
    //!
    Caller(zce::aio::Worker*);
    ~Caller() = default;

    //!打开某个文件
    int fs_open(const char* path,
                int flags,
                int mode);

    //!关闭某个文件
    int fs_close(ZCE_HANDLE handle);

    //
    int fs_lseek(ZCE_HANDLE handle,
                 off_t offset,
                 int whence);

    //!
    int fs_read(ZCE_HANDLE handle,
                const char* read_bufs_,
                size_t nbufs,
                ssize_t offset = 0,
                int whence = SEEK_CUR);

    //!
    int fs_write(ZCE_HANDLE handle,
                 const char* write_bufs_,
                 size_t nbufs,
                 ssize_t offset = 0,
                 int whence = SEEK_CUR);

    //!
    int fs_unlink(const char* path);

    //!
    int fs_rename(const char* path,
                  const char* new_path);
    //!
    int fs_ftruncate(ZCE_HANDLE handle,
                     size_t offset);
protected:
    //!

protected:
    
    //!
    zce::aio::Worker* worker_ = nullptr;

};

}