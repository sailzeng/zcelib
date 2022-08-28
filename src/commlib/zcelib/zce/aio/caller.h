

#pragma once
#include "zce/os_adapt/common.h"


//!
namespace zce::aio
{

class Worker;

//!
enum AIO_TYPE
{
    FS_BEGIN = 1,
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
    FS_END = 99,
    MYSQL_BEGIN = 100,

    MYSQL_END = 199,

};

struct AIO_Handle
{
    //!
    uint32_t id_;
    //!
    AIO_TYPE  aio_type_;

    //!
    std::function<void(AIO_Handle*)> call_back_;
};

//! FS文件
struct FS_Handle :public AIO_Handle
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

class MySQL_Handle :public AIO_Handle
{

};

namespace caller
{

//!打开某个文件
int fs_open(zce::aio::Worker* worker,
            const char* path,
            int flags,
            int mode,
            std::function<void(AIO_Handle*)> call_back);

//!关闭某个文件
int fs_close(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             std::function<void(AIO_Handle*)> call_back);

//
int fs_lseek(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             off_t offset,
             int whence,
             std::function<void(AIO_Handle*)> call_back);

//!
int fs_read(zce::aio::Worker* worker,
            ZCE_HANDLE handle,
            const char* read_bufs_,
            size_t nbufs,
            ssize_t offset,
            int whence,
            std::function<void(AIO_Handle*)> call_back);

//!
int fs_write(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             const char* write_bufs_,
             size_t nbufs,
             ssize_t offset,
             int whence,
             std::function<void(AIO_Handle*)> call_back);

//!
int fs_unlink(zce::aio::Worker* worker,
              const char* path,
              std::function<void(AIO_Handle*)> call_back);

//!
int fs_rename(zce::aio::Worker* worker,
              const char* path,
              const char* new_path,
              std::function<void(AIO_Handle*)> call_back);
//!
int fs_ftruncate(zce::aio::Worker* worker,
                 ZCE_HANDLE handle,
                 size_t offset,
                 std::function<void(AIO_Handle*)> call_back);
}





}