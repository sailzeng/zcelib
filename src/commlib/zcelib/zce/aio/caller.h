

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
    FS_READFILE,
    FS_WRITEFILE,
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
    //!清理
    virtual void clear();
public:
    //!结果
    int result_ = -1;
    //路径，注意，这儿保存的是指针，在返回前对应的数据要能使用
    const char* path_ = nullptr;
    //!打开文件标志
    int flags_ = 0;
    //!打开文件模式
    int mode_ = 0;
    //! 
    ZCE_HANDLE handle_ = ZCE_INVALID_HANDLE;

    //!文件偏移的参数
    ssize_t offset_ = 0;
    //!
    int whence_ = SEEK_CUR;

    //!读取，写入的buf，buf的长度，结果的长度
    char* read_bufs_ = nullptr;
    const char* write_bufs_ = nullptr;
    size_t bufs_count_ = 0;
    size_t result_count_ = 0;

    //!改名的路径
    const char* new_path_ = nullptr;

    //!
    struct stat* file_stat_ = nullptr;

    //!scandir打开
    const char* dirname_ = nullptr;
    //!scandir返回的dirent，你
    struct dirent*** namelist_ = nullptr;
};

class MySQL_Handle :public AIO_Handle
{
    //!
    virtual void clear() override;
};

//====================================================


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

//!异步截断文件
int fs_ftruncate(zce::aio::Worker* worker,
                 ZCE_HANDLE handle,
                 size_t offset,
                 std::function<void(AIO_Handle*)> call_back);

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

//!异步获取stat
int fs_stat(zce::aio::Worker* worker,
            const char* path,
            struct stat* file_stat,
            std::function<void(AIO_Handle*)> call_back);

//!异步scandir
int fs_scandir(zce::aio::Worker* worker,
               const char* dirname,
               struct dirent*** namelist,
               std::function<void(AIO_Handle*)> call_back);

//!异步建立dir
int fs_mkdir(zce::aio::Worker* worker,
             const char* dirname,
             int mode,
             std::function<void(AIO_Handle*)> call_back);

//!异步删除dir
int fs_rmdir(zce::aio::Worker* worker,
             const char* dirname,
             std::function<void(AIO_Handle*)> call_back);


//==================================================================
//AIO 文件处理相关的awaiter等待体
struct await_aiofs
{
    await_aiofs(zce::aio::Worker* worker,
                zce::aio::FS_Handle* fs_hdl);
    ~await_aiofs() = default;

    //是否准备好
    bool await_ready();
    //挂起操作
    void await_suspend(std::coroutine_handle<> awaiting);

    //!恢复后返回结果
    FS_Handle await_resume();

    //!回调函数
    void resume(AIO_Handle* return_hdl);

    //!工作者，具有请求，应答管道，处理IO多线程的管理者
    zce::aio::Worker* worker_ = nullptr;
    //!请求的文件操作句柄
    zce::aio::FS_Handle* fs_hdl_ = nullptr;
    //!完成后返回的句柄
    zce::aio::FS_Handle return_hdl_;
    //!协程的句柄（调用者）
    std::coroutine_handle<> awaiting_;
};

//AIO 协程的co_await 函数

//!协程co_await AIO读取文件
await_aiofs co_read_file(zce::aio::Worker* worker,
                         const char* path,
                         char* read_bufs,
                         size_t nbufs,
                         ssize_t offset = 0);
//!协程co_await AIO写入文件
await_aiofs co_write_file(zce::aio::Worker* worker,
                          const char* path,
                          const char* write_bufs,
                          size_t nbufs,
                          ssize_t offset = 0);

}