#include "zce/predefine.h"
#include "zce/aio/worker.h"
#include "zce/aio/caller.h"

namespace zce::aio
{

void AIO_Handle::clear()
{
    aio_type_ = AIO_INVALID;
    id_ = 0;
}

void FS_Handle::clear()
{
    AIO_Handle::clear();
    result_ = 0;
    path_ = nullptr;
    flags_ = 0;
    mode_ = 0;
    handle_ = ZCE_INVALID_HANDLE;
    offset_ = 0;
    whence_ = SEEK_CUR;
    read_bufs_ = nullptr;
    write_bufs_ = nullptr;
    bufs_count_ = 0;
    result_count_ = 0;
    file_stat_ = nullptr;
}

void MySQL_Handle::clear()
{
    AIO_Handle::clear();
}

//====================================================

namespace caller
{
//!异步打开某个文件，完成后回调函数call_back
int fs_open(zce::aio::Worker* worker,
            const char* path,
            int flags,
            int mode,
            std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_OPEN);

    hdl->path_ = path;
    hdl->flags_ = flags;
    hdl->mode_ = mode;
    hdl->call_back_ = call_back;
    auto succ_req = worker->request(hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!关闭某个文件
int fs_close(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_CLOSE);

    hdl->handle_ = handle;
    hdl->call_back_ = call_back;
    auto succ_req = worker->request(hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!移动文件的读写位置
int fs_lseek(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             off_t offset,
             int whence,
             std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_LSEEK);

    hdl->handle_ = handle;
    hdl->offset_ = offset;
    hdl->whence_ = whence;
    hdl->call_back_ = call_back;
    auto succ_req = worker->request(hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步读取文件内容
int fs_read(zce::aio::Worker* worker,
            ZCE_HANDLE handle,
            char* read_bufs,
            size_t nbufs,
            std::function<void(AIO_Handle*)> call_back,
            ssize_t offset,
            int whence)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_READ);

    aio_hdl->handle_ = handle;
    aio_hdl->read_bufs_ = read_bufs;
    aio_hdl->bufs_count_ = nbufs;
    aio_hdl->offset_ = offset;
    aio_hdl->whence_ = whence;
    auto succ_req = worker->request(aio_hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

//!异步写入文件内容
int fs_write(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             const char* write_bufs,
             size_t nbufs,
             std::function<void(AIO_Handle*)> call_back,
             ssize_t offset,
             int whence)
{
    zce::aio::FS_Handle* aio_hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_WRITE);

    aio_hdl->handle_ = handle;
    aio_hdl->write_bufs_ = write_bufs;
    aio_hdl->bufs_count_ = nbufs;
    aio_hdl->offset_ = offset;
    aio_hdl->whence_ = whence;
    auto succ_req = worker->request(aio_hdl);
    if (!succ_req)
    {
        return -1;
    }
    return 0;
}

}
}

