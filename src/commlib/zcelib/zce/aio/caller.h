

#pragma once
#include "zce/os_adapt/common.h"


//!
namespace zce::aio
{

class Worker;

//!
enum AIO_TYPE
{
    AIO_INVALID = 0,

    FS_BEGIN = 1,
    FS_OPEN,
    FS_CLOSE,
    FS_LSEEK,
    FS_READ,
    FS_WRITE,
    //异步读取一个文件
    FS_READFILE,
    //异步写入一个文件
    FS_WRITEFILE,
    //异步获取文件的stat
    FS_STAT,
    FS_FTRUNCATE,
    FS_UNLINK,
    FS_RMDIR,
    FS_MKDIR,
    //异步改名
    FS_RENAME,
    //异步
    FS_SCANDIR,
    FS_END = 99,

    MYSQL_BEGIN = 100,
    MYSQL_END = 199,

};

struct AIO_Handle
{

    virtual void clear() = 0;

    //!
    AIO_TYPE  aio_type_;
    //!
    uint32_t id_;
    //!
    std::function<void(AIO_Handle*)> call_back_;

};

//! FS文件
struct FS_Handle :public AIO_Handle
{
    //!
    virtual void clear();
public:
    //!
    int result_ = 0;
    //
    const char* path_ = nullptr;
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
    size_t bufs_count_ = 0;
    size_t result_count_ = 0;
    //!
    const char* new_path_ = nullptr;
    //!
    struct stat* file_stat_ = nullptr;
};

class MySQL_Handle :public AIO_Handle
{
    //!
    virtual void clear() override;
};

//====================================================

namespace caller
{

//!异步打开某个文件，完成后回调函数call_back
int fs_open(zce::aio::Worker* worker,
            const char* path,
            int flags,
            int mode,
            std::function<void(AIO_Handle*)> call_back);

//!异步关闭某个文件，完成后回调函数call_back
int fs_close(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             std::function<void(AIO_Handle*)> call_back);

//!移动文件的读写位置,
int fs_lseek(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             off_t offset,
             int whence,
             std::function<void(AIO_Handle*)> call_back);

//!异步读取文件内容
int fs_read(zce::aio::Worker* worker,
            ZCE_HANDLE handle,
            char* read_bufs,
            size_t nbufs,
            std::function<void(AIO_Handle*)> call_back,
            ssize_t offset = 0,
            int whence = SEEK_CUR);

//!异步写入文件内容
int fs_write(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             const char* write_bufs,
             size_t nbufs,
             std::function<void(AIO_Handle*)> call_back,
             ssize_t offset = 0,
             int whence = SEEK_CUR);

//!异步打开文件，读取文件内容，然后关闭
int fs_read_file(zce::aio::Worker* worker,
                 const char* path,
                 char* read_bufs,
                 size_t nbufs,
                 std::function<void(AIO_Handle*)> call_back,
                 ssize_t offset = 0);

//!异步打开文件，写入文件内容，然后关闭
int fs_write_file(zce::aio::Worker* worker,
                  const char* path,
                  const char* write_bufs,
                  size_t nbufs,
                  std::function<void(AIO_Handle*)> call_back,
                  ssize_t offset = 0);

//!异步删除文件
int fs_unlink(zce::aio::Worker* worker,
              const char* path,
              std::function<void(AIO_Handle*)> call_back);

//!异步改名
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