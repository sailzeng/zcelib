

#pragma once

#include "zce/os_adapt/common.h"

namespace zce::aio
{

class base
{
    //!
    int fs_open(const char* path,
                int flags,
                int mode);

    //!
    int fs_close(ZCE_HANDLE file);
    //!
    int fs_read(ZCE_HANDLE file,
                const char bufs[],
                unsigned int nbufs,
                int64_t offset);
    //!
    int fs_unlink(const char* path);
    //!
    int fs_write(ZCE_HANDLE file,
                 const char bufs[],
                 unsigned int nbufs,
                 int64_t offset);

    int fs_rename(const char* path,
                  const char* new_path);

    int fs_ftruncate(ZCE_HANDLE file,
                     int64_t offset);

};

}