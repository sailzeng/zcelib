#pragma once

#include "zce/buffer/cycle_buffer.h"
#include "zce/buffer/queue_buffer.h"
#include "zce/pool/dataptr_pool.h"
#include "zce/lock/null_lock.h"
#include "zce/util/singleton.h"
#include "zce/util/mpl.h"

namespace zce
{
/*!
* @brief      一个内存buffer的池子，可以给出若干buffer的预制长度，
*             虫子内部保存多种初始化的buffer，选择最合适的分配
* @tparam     LOCK 锁，可以是zce::null_lock,也可以是std::mutex，看是否需要现场安全
* @tparam     B Buffer的类型，可以是cycle_buffer or queue_buffer
*/
template<typename LOCK, typename B>
class buffer_pool
{
protected:

    //每个桶里面存放一种尺寸的B
    typedef zce::data_pool<LOCK, B>  bucket;

public:
    //
    typedef B buffer;

    //! 构造函数，析构函数，赋值函数
    buffer_pool() = default;
    ~buffer_pool()
    {
        terminate();
    }

    /*!
    * @brief      初始化
    * @return     bool
    * @param      bucket_num          桶的数量，
    * @param      bucket_size_ary     每个桶装的buffer的size的队列，
    *                                 队列长度由bucket_num决定
    * @param      init_node_size      每个桶初始化的尺寸
    * @param      extend_node_size    每个桶扩展的尺寸
    * @note
    */
    bool initialize(size_t bucket_num,
                    const size_t bucket_size_ary[],
                    size_t init_node_size,
                    size_t extend_node_size)
    {
        bool ret = false;
        bucket_number_ = bucket_num;
        bucket_bufsize_ = new size_t[bucket_num];
        memcpy(bucket_bufsize_, bucket_size_ary, bucket_num * sizeof(size_t));
        std::sort(bucket_bufsize_,
                  bucket_bufsize_ + bucket_num);
        pools_ = new bucket[bucket_num];

        for (size_t i = 0; i < bucket_num; ++i)
        {
            std::function<B* ()> new_fun =
                std::bind(zce::new_helper<B,size_t>::invoke,
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

    //! 结束销毁
    void terminate()
    {
        for (size_t i = 0; i < bucket_number_; ++i)
        {
            pools_[i].terminate();
        }
        if (bucket_bufsize_)
        {
            delete bucket_bufsize_;
            bucket_bufsize_ = nullptr;
        }
        if (pools_)
        {
            delete[] pools_;
            pools_ = nullptr;
        }
    }

    //! 跟前期待的buf的尺寸，分配buffer，
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

    //! 释放buf
    void free_buffer(B* buf)
    {
        bucket* node = get_bucket(buf->capacity());
        assert(node);
        if (node)
        {
            node->free_object(buf);
        }
    }

    //! dump信息
    void dump(zce::LOG_PRIORITY log_priority)
    {
        for (size_t i = 0; i < bucket_number_; i++)
        {
            pools_[i].dump(log_priority);
        }
    }

protected:

    //! 取得合适的桶，
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

    //! 桶的数量
    size_t bucket_number_ = 0;
    //! 桶的容量
    size_t* bucket_bufsize_ = nullptr;
    //! 桶组成的池子
    bucket* pools_ = nullptr;
};

typedef buffer_pool<zce::null_lock, cycle_buffer> cycle_buffer_pool;
typedef buffer_pool<zce::null_lock, queue_buffer> queue_buffer_pool;
typedef buffer_pool<std::mutex, cycle_buffer> cycle_buffer_pool_s;
typedef buffer_pool<std::mutex, queue_buffer> queue_buffer_pool_s;

typedef zce::singleton<cycle_buffer_pool> cycle_buffer_pool_inst;
typedef zce::singleton<queue_buffer_pool> queue_buffer_pool_inst;
typedef zce::singleton<cycle_buffer_pool_s> cycle_buffer_pool_s_inst;
typedef zce::singleton<queue_buffer_pool_s> queue_buffer_pool_s_inst;
}