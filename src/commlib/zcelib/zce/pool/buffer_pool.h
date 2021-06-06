#pragma once

#include "zce/util/buffer.h"
#include "zce/pool/object_pool.h"

namespace zce
{
class buffer_pool
{
protected:

    typedef zce::object_pool<zce::queue_buffer>  bucket;
public:
    //构造函数，析构函数，赋值函数
    buffer_pool() = default;
    ~buffer_pool() = default;
    buffer_pool(const buffer_pool &) = delete;
    buffer_pool & operator= (const buffer_pool &) = delete;

    bool initialize(size_t bucket_num,
                    size_t bucket_size_ary[],
                    size_t init_node_size,
                    size_t extend_node_size)
    {
        bool ret = false;
        bucket_size_ = bucket_num;
        bucket_capacity_.assign(bucket_size_ary, bucket_size_ary + bucket_num);
        std::sort(bucket_capacity_.begin(), bucket_capacity_.end());
        pools_.resize(bucket_num);
        for (size_t i = 0; i < bucket_num; ++i)
        {
            std::function<bool(zce::queue_buffer *)> init_fun =
                std::bind(&zce::queue_buffer::initialize, std::placeholders::_1, bucket_capacity_[i]);
            std::function<void(zce::queue_buffer *)> clear_fun =
                std::bind(&zce::queue_buffer::clear, std::placeholders::_1);

            ret = pools_[i].initialize(init_node_size,
                                       extend_node_size,
                                       init_fun,
                                       clear_fun);
            if (ret != true)
            {
                return ret;
            }
        }
        return true;
    }

    bool alloc_buffer(size_t expect_buf_size,
                      zce::queue_buffer *&buf)
    {
        bucket * node = get_bucket(expect_buf_size);
        if (node)
        {
            buf = node->alloc_object();
        }
        else
        {
            return false;
        }
        return true;
    }

    void free_buffer(zce::queue_buffer *buf)
    {
        bucket * node = get_bucket(buf->capacity());
        if (node)
        {
            buf = node->alloc_object();
        }
    }

protected:

    bucket *get_bucket(size_t expect_buf_size)
    {
        for (size_t i = 0; i < bucket_size_; i++)
        {
            if (bucket_capacity_[i] >= expect_buf_size)
            {
                return &pools_[i];
            }
        }
        return nullptr;
    }

protected:

    //
    size_t bucket_size_;
    //
    std::vector<size_t> bucket_capacity_;
    //
    std::vector<bucket> pools_;
};
}