#pragma once

#include "zce/logger/log_print.h"
#include "zce/container/lord_rings.h"

namespace zce
{
/*!
* @brief      （单）对象(DATA)池子，可以用于分配对象，避免反复使用new or delete
*             对象已经构造完成，分配时不需要构建对象，
* @tparam     LOCK 锁，可以是zce::null_lock,也可以是std::recursive_mutex（最好）
*             std::mutex也应该可以用，但总担心不小心写错了，多次调用导致递归
* @tparam     T 对象类型，
*/
template<typename LOCK, typename T>
class dataptr_pool
{
public:
    //!对象池子对象
    typedef T object;

    //!构造函数，析构函数，赋值函数
    dataptr_pool() = default;
    ~dataptr_pool() = default;

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
            new_fun_ = (*new_fun);
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

protected:
    //!扩展池子的容量, 没有加锁，依靠外面加锁
    bool extend(size_t extend_size)
    {
        bool ret = false;
        size_t pool_capacity = obj_pool_.capacity();
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
                new_ptr = (new_fun_)();
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
    std::function <T* ()> new_fun_;
    //!
    LOCK lock_;
};

//=======================================================================================
/*!
* @brief      多对象池子，可以用于分配对象，避免反复使用new or delete
*             要分配的对象作为模板参数
* @tparam     LOCK 锁，可以是zce::null_lock,也可以是std::recursive_mutex
* @tparam     ... T 多种对象类型，
*/
template< typename LOCK, typename... T >
class multidata_pool
{
public:

    multidata_pool() = default;
    ~multidata_pool() = default;

    //============================
    //!对某个对象池子进行初始化,使用对象名称作为模板参数
    template<typename O>
    bool initialize(size_t init_node_size,
                    size_t extend_node_size,
                    std::function <O* () >* new_fun = nullptr)
    {
        return std::get<zce::dataptr_pool<LOCK, O> >(pools_).initialize(init_node_size,
                                                                        extend_node_size,
                                                                        new_fun);
    }
    template<typename O>
    void terminate()
    {
        return std::get<zce::dataptr_pool<LOCK, O> >(pools_).terminate();
    }
    template<typename O>
    inline size_t size()
    {
        return std::get<zce::dataptr_pool<LOCK, O> >(pools_).size();
    }
    template<typename O>
    size_t capacity()
    {
        return std::get<zce::dataptr_pool<LOCK, O> >(pools_).capacity();
    }
    template<typename O>
    bool empty()
    {
        return std::get<zce::dataptr_pool<LOCK, O> >(pools_).empty();
    }
    template<typename O>
    bool full()
    {
        return std::get<zce::dataptr_pool<LOCK, O> >(pools_).full();
    }
    //!分配一个对象
    template<typename O>
    O* alloc_object()
    {
        return std::get<zce::dataptr_pool<LOCK, O> >(pools_).alloc_object();
    }
    //归还一个对象
    template<typename O>
    void free_object(O* ptr)
    {
        return std::get<zce::dataptr_pool<LOCK, O> >(pools_).free_object(ptr);
    }

    //============================
    //!对某个对象池子进行初始化,使用对象在tuple的序号作为模板参数
    template<size_t I>
    bool initialize(size_t init_node_size,
                    size_t extend_node_size,
                    std::function <typename std::tuple_element<I, \
                    std::tuple<dataptr_pool<LOCK, T>...> >::type::object* () >* new_fun = nullptr)
    {
        return std::get<I>(pools_).initialize(init_node_size,
                                              extend_node_size,
                                              new_fun);
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
    template<size_t I>
    typename std::tuple_element<I, std::tuple<dataptr_pool<LOCK, T>...> >::type::object*
        alloc_object()
    {
        return std::get<I>(pools_).alloc_object();
    }
    //归还一个对象
    template<size_t I >
    void free_object(typename std::tuple_element<I,
                     std::tuple<dataptr_pool<LOCK, T>...> >::type::object* ptr)
    {
        return std::get<I>(pools_).free_object(ptr);
    }

protected:
    //!对象池子堆
    std::tuple<dataptr_pool<LOCK, T>... > pools_;
};
}