#pragma once

#include "zce/util/buffer.h"
#include "zce/pool/object_pool.h"

namespace zce
{
class buffer_pool
{
protected:

    //每个桶里面存放一种尺寸的BUFFER
    typedef zce::object_pool<zce::queue_buffer>  bucket;

public:
    //构造函数，析构函数，赋值函数
    buffer_pool() = default;
    ~buffer_pool() = default;
    buffer_pool(const buffer_pool &) = delete;
    buffer_pool & operator= (const buffer_pool &) = delete;

    //初始化
    bool initialize(size_t bucket_num,
                    size_t bucket_size_ary[],
                    size_t init_node_size,
                    size_t extend_node_size)
    {
        bool ret = false;
        bucket_number_ = bucket_num;
        bucket_capacity_.assign(bucket_size_ary, bucket_size_ary + bucket_num);
        std::sort(bucket_capacity_.begin(), bucket_capacity_.end());
        pools_.resize(bucket_num);
        for (size_t i = 0; i < bucket_num; ++i)
        {
            std::function<bool(zce::queue_buffer *)> init_fun =
                std::bind(&zce::queue_buffer::initialize, 
                          std::placeholders::_1,
                          bucket_capacity_[i]);
            std::function<void(zce::queue_buffer *)> clear_fun =
                std::bind(&zce::queue_buffer::clear, 
                          std::placeholders::_1);

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

    //
    void free_buffer(zce::queue_buffer *buf)
    {
        bucket * node = get_bucket(buf->capacity());
        assert(node);
        if (node)
        {
            node->free_object(buf);
        }
    }

    //
    void dump()
    {
        for (size_t i = 0; i < bucket_number_; i++)
        {

        }
    }

protected:

    //取得桶的数量
    bucket *get_bucket(size_t expect_buf_size)
    {
        for (size_t i = 0; i < bucket_number_; i++)
        {
            if (bucket_capacity_[i] >= expect_buf_size)
            {
                return &pools_[i];
            }
        }
        //超纲了，没有合适的分配给你
        return nullptr;
    }

protected:

    //桶的数量
    size_t bucket_number_;
    //桶的容量
    std::vector<size_t> bucket_capacity_;
    //桶组成的池子
    std::vector<bucket> pools_;
};
}