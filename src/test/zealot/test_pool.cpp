#include "predefine.h"

#include <zce/util/buffer.h>
#include <zce/pool/object_pool.h>
#include <zce/pool/buffer_pool.h>

int test_pool(int /*argc*/, char* /*argv*/[])
{
    zce::queue_buffer_pool pool;
    size_t BUCKET_NUM = 6;
    size_t BUCKET_SIZE_ARY[] = { 128,256,512,768,1024,1536 };
    pool.initialize(BUCKET_NUM, BUCKET_SIZE_ARY, 128, 512);
    zce::queue_buffer* buf = nullptr;
    bool ret = pool.alloc_buffer(64, buf);
    if (!ret)
    {
        ZPP_LOG(RS_DEBUG, "alloc_buffer ret ", ret);
        return -1;
    }
    ZPP_LOG(RS_DEBUG, "alloc_buffer capacity {}", buf->capacity());
    pool.free_buffer(buf);

    ret = pool.alloc_buffer(309, buf);
    if (!ret)
    {
        ZPP_LOG(RS_DEBUG, "alloc_buffer ret ", ret);
        return -1;
    }
    ZPP_LOG(RS_DEBUG, "alloc_buffer capacity {}", buf->capacity());
    pool.free_buffer(buf);

    ret = pool.alloc_buffer(602, buf);
    if (!ret)
    {
        ZPP_LOG(RS_DEBUG, "alloc_buffer 602 ret ", ret);
    }

    pool.free_buffer(buf);

    return 0;
}