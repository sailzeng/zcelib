#pragma once

#include "zce/pool/object_pool.h"

namespace zce
{
/*!
* @brief      多对象池子，可以用于分配对象，避免反复使用new or delete
*             要分配的对象作为模板参数
* @tparam     LOCK 锁，可以是zce::null_lock,也可以是std::mutex
* @tparam     ... T 多种对象类型，
*/
template< typename LOCK, typename... T >
class multiobjs_pool
{
public:

    multiobjs_pool()
    {
    }
    ~multiobjs_pool()
    {
    }

    //==============================
    //!对某个对象池子进行初始化,使用对象名称作为模板参数
    template<typename O>
    bool initialize(size_t init_node_size,
                    size_t extend_node_size,
                    std::function <O* () >* new_fun = nullptr)
    {
        return std::get<object_pool_t<O> >(pools_).initialize(init_node_size,
                                                              extend_node_size,
                                                              new_fun);
    }
    template<typename O>
    void terminate()
    {
        return std::get<object_pool<O> >(pools_).terminate();
    }
    template<typename O>
    inline size_t size()
    {
        return std::get<object_pool<O> >(pools_).size();
    }
    template<typename O>
    size_t capacity()
    {
        return std::get<object_pool<O> >(pools_).capacity();
    }
    template<typename O>
    bool empty()
    {
        return std::get<object_pool<O> >(pools_).empty();
    }
    template<typename O>
    bool full()
    {
        return std::get<object_pool<O> >(pools_).full();
    }
    //!分配一个对象
    template<typename O>
    typename O* alloc_object()
    {
        return std::get<object_pool<O> >(pools_).alloc_object();
    }
    //归还一个对象
    template<typename O>
    void free_object(O* ptr)
    {
        return std::get<object_pool<O> >(pools_).free_object(ptr);
    }

    //=======================================
    //!对某个对象池子进行初始化,使用对象在tuple的序号作为模板参数
    template<size_t I>
    bool initialize(size_t init_node_size,
                    size_t extend_node_size,
                    std::function <typename std::tuple_element<I, \
                    std::tuple<object_pool<LOCK, T>...> >::type::object* () >* new_fun = nullptr)
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
    typename std::tuple_element<I, std::tuple<object_pool<LOCK, T>...> >::type::object*
        alloc_object()
    {
        return std::get<I>(pools_).alloc_object();
    }
    //归还一个对象
    template<size_t I >
    void free_object(typename std::tuple_element<I,
                     std::tuple<object_pool<LOCK, T>...> >::type::object* ptr)
    {
        return std::get<I>(pools_).free_object(ptr);
    }

protected:
    //!对象池子堆
    std::tuple<object_pool<LOCK, T>... > pools_;
};

}