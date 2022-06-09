#pragma once

#include "zce/pool/object_pool.h"

namespace zce
{

template< class... T >
class multiobjs_pool
{
public:

    multiobjs_pool()
    {

    }
    ~multiobjs_pool()
    {

    }

    //!
    template<size_t I>
    bool initialize(size_t init_node_size,
                    size_t extend_node_size)
    {
        return std::get<I>(pools_).initialize(init_node_size,
                                              extend_node_size);
    }

    template<size_t I>
    bool terminate()
    {
        return std::get<I>(pools_).terminate();
    }

    //!分配一个对象
    template<size_t I, typename T>
    T* alloc_object()
    {
        return std::get<I>(pools_).alloc_object();
    }

    //归还一个对象
    template<size_t I, typename T>
    void free_object(T* ptr)
    {
        return std::get<I>(pools_).free_object(ptr);
    }
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

protected:
    //!
    std::tuple<object_pool<T>... > pools_;
};

}