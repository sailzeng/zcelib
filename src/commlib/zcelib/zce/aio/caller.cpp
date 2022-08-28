#include "zce/predefine.h"
#include "zce/aio/worker.h"
#include "zce/aio/caller.h"

namespace zce::aio
{

namespace caller
{
//!
int fs_lseek(zce::aio::Worker* worker,
             ZCE_HANDLE handle,
             off_t offset,
             int whence,
             std::function<void(AIO_Handle*)> call_back)
{
    zce::aio::FS_Handle* hdl = (FS_Handle*)
        worker->alloc_handle(AIO_TYPE::FS_LSEEK);

    hdl->aio_type_ = AIO_TYPE::FS_LSEEK;
    hdl->handle_ = handle;
    hdl->offset_ = offset;
    hdl->whence_ = whence;
    hdl->call_back_ = call_back;
    auto succ = worker->request(hdl);
    if (!succ)
    {
        return -1;
    }
    return 0;
}

}
}

