#pragma once

#include "zce/util/lord_rings.h"

namespace zce
{
template<typename POOL_OBJ>
class object_pool
{
public:

    //构造函数，析构函数，赋值函数
    object_pool() = default;
    ~object_pool() = default;
    object_pool(const object_pool &) = default;
    object_pool & operator= (const object_pool &) = delete;

    /*!
    * @brief
    * @return     bool 是否初始化成果
    * @param      init_pool_size
    * @param      extend_size  每次扩展的尺寸
    * @param      init_fun     T*的初始化函数
    * @param      clear_fun    T*的清理函数，用于归还时处理
    * @note
    */
    bool initialize(size_t init_pool_size,
                    size_t extend_size,
                    std::function <bool(POOL_OBJ*) > &init_fun,
                    std::function <void(POOL_OBJ*) > &clear_fun)
    {
        extend_size_ = extend_size;
        init_fun_ = init_fun;
        clear_fun_ = clear_fun;
        bool ret = obj_pool_.initialize(init_pool_size);
        if (ret != true)
        {
            return false;
        }
        ret = extend(init_pool_size);
        if (!ret)
        {
            return nullptr;
        }
        pool_capacity_ = init_pool_size;
        return true;
    }

    //最后的销毁处理
    void terminate()
    {
        size_t sz = obj_pool_.size();
        for (size_t i = 0; i < sz; i++)
        {
            POOL_OBJ* ptr = nullptr;
            obj_pool_.pop_front(ptr);
            delete ptr;
        }
    }

    //分配一个对象
    POOL_OBJ *alloc_object()
    {
        auto ret = false;
        POOL_OBJ* ptr = nullptr;
        if (obj_pool_.size() == 0)
        {
            ret = obj_pool_.resize(pool_capacity_ + extend_size_);
            if (!ret)
            {
                return nullptr;
            }
            ret = extend(extend_size_);
            if (!ret)
            {
                return nullptr;
            }
            pool_capacity_ = pool_capacity_ + extend_size_;
        }
        obj_pool_.pop_front(ptr);
        return ptr;
    }

    //归还一个对象
    void free_object(POOL_OBJ* ptr)
    {
        clear_fun_(ptr);
        obj_pool_.push_back(ptr);
        return ptr;
    }

    inline size_t size()
    {
        return obj_pool_->size();
    }
    inline size_t capacity()
    {
        return obj_pool_->capacity();
    }
    inline bool empty()
    {
        return obj_pool_->empty();
    }
    inline bool full()
    {
        return obj_pool_->full();
    }

protected:

    bool extend(size_t extend_size)
    {
        ZCE_LOG(RS_INFO, "[ZCELIB] object_pool<T> [%s] pool size[%u], capacity[%u], extend[%u] , old capacity[%u] .",
                typeid(this).name(),
                obj_pool_.size(),
                obj_pool_.capacity(),
                extend_size,
                pool_capacity_);
        //
        for (size_t i = 0; i < extend_size; ++i)
        {
            POOL_OBJ* new_ptr = new POOL_OBJ();
            if (new_ptr == nullptr)
            {
                return false;
            }
            bool ret = init_fun_(new_ptr);
            if (!ret)
            {
                return false;
            }
            obj_pool_.push_back(new_ptr);
        }
        return true;
    }

protected:

    //池子容量
    size_t pool_capacity_ = 0;
    //扩展的尺寸
    size_t extend_size_ = 0;

    //对象池子
    zce::lord_rings<POOL_OBJ*>   obj_pool_;

    //T的初始化函数，
    std::function <bool(POOL_OBJ*) > init_fun_;

    //T的clear函数，用于收到归还数据后的回收
    std::function <void(POOL_OBJ*) > clear_fun_;
};
}