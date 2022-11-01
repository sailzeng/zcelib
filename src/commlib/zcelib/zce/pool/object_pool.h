/*!
* @copyright  2004-2021  Apache License, Version 2.0 FULLSAIL
* @filename   zce/pool/dataptr_pool.h
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
* @brief      对象池子2，可以用于分配对象，避免反复使用new or delete
*             对象在分配的时候构造，释放的时候析构
* @tparam     LOCK 锁，可以是zce::null_lock,也可以是std::mutex or std::recursive_mutex
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
    * @note
    */
    bool initialize(size_t init_pool_size,
                    size_t extend_size)
    {
        std::lock_guard<LOCK> lock(lock_);
        init_pool_size_ = init_pool_size;
        extend_size_ = extend_size;

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
        if (!voidptr_pool_.full() == 0)
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
        size_t sz = voidptr_pool_.size();
        for (size_t i = 0; i < sz; i++)
        {
            void* ptr = nullptr;
            voidptr_pool_.pop_front(ptr);
            delete ptr;
        }
    }

    //! 构造函数和销毁的方式，constructor 和 destroy 成对使用
    //! 用于你需要使用构造函数初始化指针的地方，需要析构释放资源的地方
    template<typename... Args>
    object_pool::object *constructor(Args&&... args)
    {
        std::lock_guard<LOCK> lock(lock_);
        void *ptr = alloc_ptr();
        return new(ptr) object_pool::object(args...);
    }
    //! 析构释放指针
    void destroy(object_pool::object* obj)
    {
        std::lock_guard<LOCK> lock(lock_);
        obj->~T();
        free_ptr((void *)obj);
    }

    inline size_t size()
    {
        std::lock_guard<LOCK> lock(lock_);
        return voidptr_pool_.size();
    }
    inline size_t capacity()
    {
        std::lock_guard<LOCK> lock(lock_);
        return voidptr_pool_.capacity();
    }
    inline bool empty()
    {
        std::lock_guard<LOCK> lock;
        return voidptr_pool_.empty();
    }
    inline bool full()
    {
        std::lock_guard<LOCK> lock(lock_);
        return voidptr_pool_.full();
    }

protected:

    //也许未来可以加个收缩
    //!分配一个对象
    void* alloc_ptr()
    {
        auto ret = false;
        void* ptr = nullptr;
        if (voidptr_pool_.size() == 0)
        {
            ret = extend(extend_size_);
            if (!ret)
            {
                return nullptr;
            }
        }
        voidptr_pool_.pop_front(ptr);
        return ptr;
    }

    //归还一个对象
    void free_ptr(void* ptr)
    {
        voidptr_pool_.push_back(ptr);
        return;
    }

    //!扩展池子的容量
    bool extend(size_t extend_size)
    {
        bool ret = false;
        //不要直接使用capacity函数，会形成递归锁，
        size_t pool_capacity = voidptr_pool_.capacity();
        if (pool_capacity == 0)
        {
            ret = voidptr_pool_.initialize(extend_size);
            if (ret != true)
            {
                return false;
            }
        }
        else
        {
            ret = voidptr_pool_.resize(pool_capacity + extend_size);
            if (!ret)
            {
                return false;
            }
        }
        const size_t sz_object = sizeof(T);
        //
        for (size_t i = 0; i < extend_size; ++i)
        {
            void *new_ptr = new char[sz_object];
            voidptr_pool_.push_back(new_ptr);
        }
        return true;
    }
protected:

    //! 池子初始化大小
    size_t init_pool_size_ = 0;
    //! 扩展的尺寸
    size_t extend_size_ = 0;

    //! 对象池子
    zce::lord_rings<void*>   voidptr_pool_;
    //!
    LOCK lock_;
};

//=======================================================================================

/*!
* @brief      多对象池子，可以用于分配对象，避免反复使用new or delete
*             要分配的对象作为模板参数
* @tparam     LOCK 锁，可以是zce::null_lock,也可以是std::recursive_mutex
*
* @tparam     ... T 多种对象类型，
*/
template< typename LOCK, typename... T >
class multiobjs_pool
{
public:

    multiobjs_pool() = default;
    ~multiobjs_pool() = default;

    //============================
    //!对某个对象池子进行初始化,使用对象名称作为模板参数
    template<typename O>
    bool initialize(size_t init_node_size,
                    size_t extend_node_size)
    {
        return std::get<zce::object_pool<LOCK, O> >(pools_).initialize(init_node_size,
                                                                       extend_node_size);
    }
    template<typename O>
    void terminate()
    {
        return std::get<zce::object_pool<LOCK, O> >(pools_).terminate();
    }
    template<typename O>
    inline size_t size()
    {
        return std::get<zce::object_pool<LOCK, O> >(pools_).size();
    }
    template<typename O>
    size_t capacity()
    {
        return std::get<zce::object_pool<LOCK, O> >(pools_).capacity();
    }
    template<typename O>
    bool empty()
    {
        return std::get<zce::object_pool<LOCK, O> >(pools_).empty();
    }
    template<typename O>
    bool full()
    {
        return std::get<zce::object_pool<LOCK, O> >(pools_).full();
    }
    //!分配一个对象
    template<typename O, typename... Args>
    O* constructor(Args&&... args)
    {
        return std::get<zce::object_pool<LOCK, O> >(pools_).constructor(args...);
    }
    //归还一个对象
    template<typename O>
    void destroy(O* ptr)
    {
        return std::get<zce::object_pool<LOCK, O> >(pools_).destroy(ptr);
    }

    //============================
    //!对某个对象池子进行初始化,使用对象在tuple的序号作为模板参数
    template<size_t I>
    bool initialize(size_t init_node_size,
                    size_t extend_node_size)
    {
        return std::get<I>(pools_).initialize(init_node_size,
                                              extend_node_size);
    }
    //!对某个对象池子进行销毁,使用对象在tuple的序号作为模板参数
    template<size_t I>
    void terminate()
    {
        return std::get<I>(pools_).terminate();
    }
    //对对象池子
    template<size_t I>
    inline size_t size()
    {
        return std::get<I>(pools_).size();
    }
    template<size_t I>
    size_t capacity()
    {
        return std::get<I>(pools_).capacity();
    }
    template<size_t I>
    bool empty()
    {
        return std::get<I>(pools_).empty();
    }
    template<size_t I>
    bool full()
    {
        return std::get<I>(pools_).full();
    }

    //!分配一个对象
    template<size_t I, typename... Args>
    typename std::tuple_element<I, std::tuple<object_pool<LOCK, T>...> >::type::object*
        constructor(Args&&... args)
    {
        return std::get<I>(pools_).constructor(args...);
    }
    //归还一个对象
    template<size_t I >
    void destroy(typename std::tuple_element<I,
                 std::tuple<object_pool<LOCK, T>...> >::type::object* ptr)
    {
        return std::get<I>(pools_).destroy(ptr);
    }

protected:
    //!对象池子堆
    std::tuple<object_pool<LOCK, T>... > pools_;
};
}