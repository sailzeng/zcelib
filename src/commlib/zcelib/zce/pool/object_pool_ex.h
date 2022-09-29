/*!
* @copyright  2004-2022  Apache License, Version 2.0 FULLSAIL
* @filename   zce/pool/object_pool_ex.h
* @author     Sailzeng <sailzeng.cn@gmail.com>
* @version
* @date
* @brief
* @details    对象池子的扩展，增加了一些日志记录功能，
*             object_pool有一个缺点，他被日志类使用，必须保持简介
*
*
* @note
*
*/

#pragma once

#include "zce/logger/logging.h"
#include "zce/pool/object_pool.h"

namespace zce
{
template<typename LOCK, typename T>
class object_pool_ex
{
public:
    //!对象池子对象
    typedef zce::object_pool<LOCK, T> object_pool;

    //!构造函数，析构函数，赋值函数
    object_pool_ex() = default;
    ~object_pool_ex() = default;

    bool initialize(size_t init_pool_size,
                    size_t extend_size,
                    std::function <T* () >* new_fun = nullptr)
    {
        bool ret = obj_pool_.initialize(init_pool_size,
                                        extend_size,
                                        new_fun);
        dump(RS_INFO);
        if (!ret)
        {
            ZCE_LOG(RS_INFO, "obj_pool_.initialize fail.");
        }
        return ret;
    }

    void terminate()
    {
        bool leak_mem;
        obj_pool_.terminate(&leak_mem);
        if (leak_mem)
        {
            dump(RS_INFO);
        }
    }

    T* alloc_object()
    {
        bool extend_pool;
        T * obj = obj_pool_.alloc_object(&extend_pool);
        if (extend_pool)
        {
            dump(RS_INFO);
        }
        if (!obj)
        {
            ZCE_LOG(RS_INFO, " type[%s] alloc_object fail.",
                    typeid(T).name());
        }
        return obj;
    }

    //! dump信息
    void dump(zce::LOG_PRIORITY log_priority)
    {
        ZCE_LOG(log_priority,
                "object_pool_ex [%s]  capacity[%u] size [%u]",
                typeid(T).name(),
                obj_pool_.capacity(),
                obj_pool_.size());
    }

    zce::object_pool<LOCK, T> obj_pool_;
};
}