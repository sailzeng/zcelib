#pragma once

#include "zce/util/lord_rings.h"

namespace zce
{
template<typename T>
class object_pool
{
public:

    object_pool() = default;
    ~object_pool() = default;
    object_pool(const object_pool &) = delete;
    object_pool & operator= (const object_pool &) = delete;

    bool initialize(size_t init_pool_size,
                    size_t extend_size,
                    std::function <bool(T*) > &init_fun,
                    std::function <void(T*) > &clear_fun)
    {
        pool_capacity_ = init_pool_size;
        extend_size_ = extend_size;
        init_fun_ = init_fun;
        clear_fun_ = clear_fun;
        bool ret = obj_pool_.initialize(pool_capacity_);
        if (ret != true)
        {
            return ret;
        }
        for (size_t i = 0; i < pool_capacity_; i++)
        {
            T* new_ptr = new T();
            if (new_ptr == nullptr)
            {
                return false;
            }
            ret = init_fun_(new_ptr);
            if (ret != true)
            {
                return ret;
            }
            obj_pool_.push_back(new_ptr);
        }

        return true;
    }

    //
    T *alloc_obj()
    {
        T* ptr = nullptr;
        if (obj_pool_.size() == 0)
        {
            ZCE_LOG(RS_INFO, "[ZCELIB] object_pool<T> [%s] size[%u], pool capacity[%u], capacity[%u] , resize[%u] .",
                    typeid(this).name(),
                    obj_pool_.size(),
                    obj_pool_.capacity(),
                    pool_capacity_,
                    pool_capacity_ + extend_size_);

            obj_pool_.resize(pool_capacity_ + extend_size_);
            pool_capacity_ = pool_capacity_ + extend_size_;
            //用模型克隆N-1个Trans
            for (size_t i = 0; i < extend_size_; ++i)
            {
                T* new_ptr = new T();
                if (new_ptr == nullptr)
                {
                    return nullptr;
                }
                ret = init_fun_(new_ptr);
                if (ret != true)
                {
                    return nullptr;
                }
                obj_pool_.push_back(new_ptr);
            }
        }
        obj_pool_.pop_front(ptr);
        return ptr;
    }

    //
    void return_obj(T* ptr)
    {
        clear_fun_(ptr);
        obj_pool_.push_back(ptr);
        return ptr;
    }

protected:
    //
    size_t pool_capacity_ = 0;
    //
    size_t extend_size_ = 0;

    //
    zce::lordrings<T*>   obj_pool_;

    //T的初始化函数，
    std::function <bool(T*) > init_fun_;

    //T的clear函数，用于收到归还数据后的回收
    std::function <void(T*) > clear_fun_;
};
}