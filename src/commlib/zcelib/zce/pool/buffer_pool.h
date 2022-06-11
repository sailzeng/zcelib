#pragma once

#include "zce/util/buffer.h"
#include "zce/pool/object_pool.h"

namespace zce
{
template<typename B>
class buffer_pool
{
protected:

    //每个桶里面存放一种尺寸的B
    typedef zce::object_pool<B>  bucket;

public:
    //
    typedef B buffer;

    //!构造函数，析构函数，赋值函数
    buffer_pool() = default;
    buffer_pool(const buffer_pool&) = delete;
    buffer_pool& operator= (const buffer_pool&) = delete;
    ~buffer_pool()
    {
        terminate();
    }


    typedef B* (*FunType)(size_t);
    /*!
    * @brief      初始化
    * @return     bool
    * @param      bucket_num          桶的数量，
    * @param      bucket_size_ary     每个桶装的buffer的size的队列，队列长度由bucket_num决定
    * @param      init_node_size      每个桶初始化的尺寸
    * @param      extend_node_size    每个桶扩展的尺寸
    * @note
    */
    bool initialize(size_t bucket_num,
                    const size_t bucket_size_ary[],
                    std::function <B* (size_t) > new_f,
                    size_t init_node_size,
                    size_t extend_node_size)
    {
        bool ret = false;
        bucket_number_ = bucket_num;
        bucket_bufsize_.assign(bucket_size_ary, bucket_size_ary + bucket_num);
        std::sort(bucket_bufsize_.begin(), bucket_bufsize_.end());
        pools_.resize(bucket_num);

        for (size_t i = 0; i < bucket_num; ++i)
        {
            std::function<B* ()> new_fun =
                std::bind(&B::new_self,
                          bucket_bufsize_[i]);
            ret = pools_[i].initialize(init_node_size,
                                       extend_node_size,
                                       &new_fun);
            if (ret != true)
            {
                return ret;
            }
        }
        return true;
    }

    //结束销毁
    void terminate()
    {
        for (size_t i = 0; i < bucket_number_; ++i)
        {
            pools_[i].terminate();
        }
    }

    bool alloc_buffer(size_t expect_buf_size,
                      B*& buf)
    {
        bucket* node = get_bucket(expect_buf_size);
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
    void free_buffer(B* buf)
    {
        bucket* node = get_bucket(buf->capacity());
        assert(node);
        if (node)
        {
            node->free_object(buf);
        }
    }

    //
    void dump(zce::LOG_PRIORITY log_priority)
    {
        for (size_t i = 0; i < bucket_number_; i++)
        {
            ZCE_LOG(log_priority, "Buffer Pool [%d] bucket bufsize [%u] capacity[%u] size[%u]",
                    i,
                    bucket_bufsize_[i],
                    pools_[i].capacity(),
                    pools_[i].size());
        }
    }

protected:

    //取得合适的桶，
    bucket* get_bucket(size_t expect_buf_size)
    {
        for (size_t i = 0; i < bucket_number_; i++)
        {
            if (bucket_bufsize_[i] >= expect_buf_size)
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
    std::vector<size_t> bucket_bufsize_;
    //桶组成的池子
    std::vector<bucket> pools_;
};

typedef buffer_pool<cycle_buffer> cycle_buffer_pool;
typedef buffer_pool<queue_buffer> queue_buffer_pool;
}