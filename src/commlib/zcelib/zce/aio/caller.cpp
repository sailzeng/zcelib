#include "zce/predefine.h"
#include "zce/aio/worker.h"
#include "zce/aio/caller.h"

namespace zce::aio
{

////!打开某个文件
//int caller::fs_open(const char* path,
//                    int flags,
//                    int mode)
//{
//
//}
//
////!关闭某个文件
//int caller::fs_close(ZCE_HANDLE handle)
//{
//
//}
//
////
int Caller::fs_lseek(ZCE_HANDLE handle,
                     off_t offset,
                     int whence)
{
    AIO_FS afs;
    afs.aio_type_ = AIO_TYPE::FS_LSEEK;
    afs.handle_ = handle;
    afs.offset_ = offset;
    afs.whence_ = whence;
    return worker_->fs_work(this, afs);
}

}

