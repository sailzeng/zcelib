/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   zce/pool/object_pool.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date
* @brief      对象池子，用于对象分配，通过alloc得到对象的指针
*             free归还对象给池子，
* @details    对象池内部没有日志。因为对象池要给日志类使用，
*             如果有日志，会形成交叉引用。
*
*
* @note
*
*/
#pragma once

#include "zce/logger/log_print.h"
#include "zce/container/lord_rings.h"

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
        std::lock_guard<LOCK> lock(lock_);
        init_pool_size_ = init_pool_size;
        extend_size_ = extend_size;
        if (new_fun)
        {
            new_fun_ = new std::function <T* () >(*new_fun);
        }
        bool ret = extend(init_pool_size_);
        if (!ret)
        {
            return false;
        }
        return true;
    }

    //!最后的销毁处理
    void terminate(bool *leak_mem = nullptr)
    {
        std::lock_guard<LOCK> lock(lock_);
        //如果内存没有全部归还
        if (!obj_pool_.full() == 0)
        {
            if (leak_mem)
            {
                *leak_mem = true;
            }
        }
        else
        {
            if (leak_mem)
            {
                *leak_mem = false;
            }
        }
        size_t sz = obj_pool_.size();
        for (size_t i = 0; i < sz; i++)
        {
            T* ptr = nullptr;
            obj_pool_.pop_front(ptr);
            delete ptr;
        }
    }

    //!分配一个对象
    T* alloc_object(bool *extend_pool = nullptr)
    {
        std::lock_guard<LOCK> lock(lock_);
        auto ret = false;
        T* ptr = nullptr;
        if (extend_pool)
        {
            *extend_pool = false;
        }
        if (obj_pool_.size() == 0)
        {
            ret = extend(extend_size_);
            if (!ret)
            {
                return nullptr;
            }
            if (extend_pool)
            {
                *extend_pool = true;
            }
        }
        obj_pool_.pop_front(ptr);
        return ptr;
    }

    //归还一个对象
    void free_object(T* ptr)
    {
        std::lock_guard<LOCK> lock(lock_);
        obj_pool_.push_back(ptr);
        return;
    }

    inline size_t size()
    {
        std::lock_guard<LOCK> lock(lock_);
        return obj_pool_.size();
    }
    inline size_t capacity()
    {
        std::lock_guard<LOCK> lock(lock_);
        return obj_pool_.capacity();
    }
    inline bool empty()
    {
        std::lock_guard<LOCK> lock;
        return obj_pool_.empty();
    }
    inline bool full()
    {
        std::lock_guard<LOCK> lock(lock_);
        return obj_pool_.full();
    }

    //!扩展池子的容量
    bool extend(size_t extend_size)
    {
        bool ret = false;
        size_t pool_capacity = capacity();
        if (pool_capacity == 0)
        {
            ret = obj_pool_.initialize(extend_size);
            if (ret != true)
            {
                return false;
            }
        }
        else
        {
            ret = obj_pool_.resize(pool_capacity + extend_size);
            if (!ret)
            {
                return false;
            }
        }

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

    //也许未来可以加个收缩

protected:

    //! 池子初始化大小
    size_t init_pool_size_ = 0;
    //! 扩展的尺寸
    size_t extend_size_ = 0;

    //! 对象池子
    zce::lord_rings<T*>   obj_pool_;

    //! T的初始化函数，
    std::function <T* ()>* new_fun_ = nullptr;
    //!
    LOCK lock_;
};
}