/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   zce/pool/object_pool.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date
* @brief
* @details
*
*
*
* @note
*
*/
#pragma once

#include "zce/logger/logging.h"
#include "zce/util/lord_rings.h"

namespace zce
{
/*!
* @brief      （单）对象池子，可以用于分配对象，避免反复使用new or delete
*
* @tparam     LOCK 锁，可以是zce::null_lock,也可以是std::mutex
* @tparam     T 对象类型，
*/
template<typename LOCK, typename T>
class object_pool
{
public:
    //!对象池子对象
    typedef T object;

    //!构造函数，析构函数，赋值函数
    object_pool() = default;
    ~object_pool() = default;
    object_pool(const object_pool&) = default;
    object_pool& operator= (const object_pool&) = delete;

    /*!
    * @brief
    * @return     bool 是否初始化成果
    * @param      init_pool_size
    * @param      extend_size  每次扩展的尺寸
    * @param      new_fun      T*的new函数，如果为空直接使用new，
    * @note
    */
    bool initialize(size_t init_pool_size,
                    size_t extend_size,
                    std::function <T* () >* new_fun = nullptr)
    {
        std::lock_guard<LOCK> lock;
        extend_size_ = extend_size;
        if (new_fun)
        {
            new_fun_ = new std::function <T* () >(*new_fun);
        }
        bool ret = obj_pool_.initialize(init_pool_size);
        if (ret != true)
        {
            return false;
        }
        ret = extend(init_pool_size);
        if (!ret)
        {
            return false;
        }
        pool_capacity_ = init_pool_size;
        return true;
    }

    //!最后的销毁处理
    void terminate()
    {
        std::lock_guard<LOCK> lock;
        size_t sz = obj_pool_.size();
        for (size_t i = 0; i < sz; i++)
        {
            T* ptr = nullptr;
            obj_pool_.pop_front(ptr);
            delete ptr;
        }
    }

    //!分配一个对象
    T* alloc_object()
    {
        std::lock_guard<LOCK> lock;
        auto ret = false;
        T* ptr = nullptr;
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
    void free_object(T* ptr)
    {
        std::lock_guard<LOCK> lock;
        obj_pool_.push_back(ptr);
        return;
    }

    inline size_t size()
    {
        std::lock_guard<LOCK> lock;
        return obj_pool_.size();
    }
    inline size_t capacity()
    {
        std::lock_guard<LOCK> lock;
        return obj_pool_.capacity();
    }
    inline bool empty()
    {
        std::lock_guard<LOCK> lock;
        return obj_pool_.empty();
    }
    inline bool full()
    {
        std::lock_guard<LOCK> lock;
        return obj_pool_.full();
    }

protected:

    bool extend(size_t extend_size)
    {
        ZCE_LOG(RS_INFO, "[ZCELIB] object_pool<T> [%s] pool size[%u], "
                "capacity[%u], extend[%u] , old capacity[%u] .",
                typeid(this).name(),
                obj_pool_.size(),
                obj_pool_.capacity(),
                extend_size,
                pool_capacity_);
        //
        for (size_t i = 0; i < extend_size; ++i)
        {
            T* new_ptr = nullptr;
            if (new_fun_)
            {
                new_ptr = (*new_fun_)();
            }
            else
            {
                new_ptr = new T();
            }
            if (new_ptr == nullptr)
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
    zce::lord_rings<T*>   obj_pool_;

    //T的初始化函数，
    std::function <T* ()>* new_fun_ = nullptr;
    //
    LOCK lock_;
};

}