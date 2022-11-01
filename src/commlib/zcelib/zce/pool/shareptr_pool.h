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

namespace zce
{
/*!
* @brief      （单）对象池子，可以用于分配对象，避免反复使用new or delete
*
* @tparam     LOCK 锁，可以是zce::null_lock,也可以是 std::recursive_mutex（优先）
*             std::mutex 也应该可以，不过写代码要注意避免出现递归
* @tparam     T 对象类型，
*/
template<typename LOCK, typename T>
class shareptr_pool
{
public:
    //!对象池子对象
    typedef T object;

    //!构造函数，析构函数，赋值函数
    shareptr_pool() = default;
    ~shareptr_pool() = default;

    /*!
    * @brief
    * @return     bool 是否初始化成果
    * @param      init_pool_size
    * @param      extend_size  每次扩展的尺寸
    * @param      new_fun      T*的new函数，如果为空直接使用new，
    * @note
    */
    void initialize(size_t init_pool_size,
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
        extend(init_pool_size_);
        return;
    }

    //!最后的销毁处理
    void terminate(size_t &leak_mem)
    {
        std::lock_guard<LOCK> lock(lock_);
        size_t sz = obj_pool_.size();
        leak_mem = 0;
        for (size_t i = 0; i < sz; i++)
        {
            if (obj_pool_[i])
            {
                //记录泄露
                if (obj_pool_[i].use_count() > 1)
                {
                    ++leak_mem;
                }
                //反正我不管了，
                obj_pool_[i].reset();
            }
        }
    }

    //! 分配一个对象，
    //! 也许未来可以加个收缩
    std::shared_ptr<T> alloc_object(bool &extend_pool)
    {
        std::lock_guard<LOCK> lock(lock_);
        extend_pool = false;
        size_t sz = obj_pool_.size(), i = 0;
        for (; i < sz; i++)
        {
            find_pos_ = (find_pos_ + 1) % sz;

            assert(obj_pool_[find_pos_]);
            //第一次的时候new分配一个数据
            if (obj_pool_[find_pos_].use_count() == 1)
            {
                return obj_pool_[find_pos_];
            }
        }
        //地主没有余量了
        if (i == sz)
        {
            extend_pool = true;
            extend(extend_size_);
        }

        return  obj_pool_[sz];
    }

    //有点耗时的操作，会检测所有的
    size_t size()
    {
        std::lock_guard<LOCK> lock(lock_);
        size_t use = 0, sz = obj_pool_.size();
        for (size_t i = 0; i < sz; i++)
        {
            if (obj_pool_[i].use_count() > 1)
            {
                ++use;
            }
        }
        return use;
    }
    //! 下面2个函数代码搞得有点丑，目标是为了避免出现递归锁的可能
    //! 计算是否为空
    inline bool empty()
    {
        std::lock_guard<LOCK> lock(lock_);
        size_t use = 0, sz = obj_pool_.size();
        for (size_t i = 0; i < sz; i++)
        {
            if (obj_pool_[i].use_count() > 1)
            {
                ++use;
            }
        }
        return use == 0;
    }
    //是否满
    inline bool full()
    {
        std::lock_guard<LOCK> lock(lock_);
        size_t use = 0, sz = obj_pool_.size();
        for (size_t i = 0; i < sz; i++)
        {
            if (obj_pool_[i].use_count() > 1)
            {
                ++use;
            }
        }
        return use == sz;
    }
    inline size_t capacity()
    {
        std::lock_guard<LOCK> lock(lock_);
        //! 容量就是vector的size
        return obj_pool_.size();
    }

protected:

    //!扩展池子的容量
    void extend(size_t extend_size)
    {
        size_t pool_size = obj_pool_.size();
        obj_pool_.resize(pool_size + extend_size);
        for (size_t i = 0; i < extend_size; ++i)
        {
            if (new_fun_)
            {
                obj_pool_[pool_size + i] = std::shared_ptr<T>(new_fun_());
            }
            else
            {
                obj_pool_[pool_size + i] = std::shared_ptr<T>(new T());
            }
        }
        return;
    }
protected:

    //! 池子初始化大小
    size_t init_pool_size_ = 0;
    //! 扩展的尺寸
    size_t extend_size_ = 0;
    //! 每次寻找的时候，从find_pos_开始寻找，如果每次从0开始慢
    size_t find_pos_ = 0;

    //! 对象池子
    std::vector<std::shared_ptr<T> >  obj_pool_;

    //! T的初始化函数，
    std::function <T* ()> new_fun_;
    //! 锁记录，
    LOCK lock_;
};

/*!
* @brief      多对象share ptr池子，可以用于分配对象，避免反复使用new or delete
*             要分配的对象作为模板参数
* @tparam     LOCK 锁，可以是zce::null_lock,也可以是std::recursive_mutex
* @tparam     ... T 多种对象类型，
*/
template< typename LOCK, typename... T >
class multishare_pool
{
public:

    multishare_pool() = default;
    ~multishare_pool() = default;

    //============================
    //!对某个对象池子进行初始化,使用对象名称作为模板参数
    template<typename O>
    bool initialize(size_t init_node_size,
                    size_t extend_node_size,
                    std::function <O* () >* new_fun = nullptr)
    {
        return std::get<zce::shareptr_pool<LOCK, O> >(pools_).initialize(init_node_size,
                                                                         extend_node_size,
                                                                         new_fun);
    }
    template<typename O>
    void terminate()
    {
        return std::get<zce::shareptr_pool<LOCK, O> >(pools_).terminate();
    }
    template<typename O>
    inline size_t size()
    {
        return std::get<zce::shareptr_pool<LOCK, O> >(pools_).size();
    }
    template<typename O>
    size_t capacity()
    {
        return std::get<zce::shareptr_pool<LOCK, O> >(pools_).capacity();
    }
    template<typename O>
    bool empty()
    {
        return std::get<zce::shareptr_pool<LOCK, O> >(pools_).empty();
    }
    template<typename O>
    bool full()
    {
        return std::get<zce::shareptr_pool<LOCK, O> >(pools_).full();
    }
    //!分配一个对象
    template<typename O>
    O* alloc_object()
    {
        return std::get<zce::shareptr_pool<LOCK, O> >(pools_).alloc_object();
    }
    //归还一个对象
    template<typename O>
    void free_object(O* ptr)
    {
        return std::get<zce::shareptr_pool<LOCK, O> >(pools_).free_object(ptr);
    }

    //============================
    //!对某个对象池子进行初始化,使用对象在tuple的序号作为模板参数
    template<size_t I>
    bool initialize(size_t init_node_size,
                    size_t extend_node_size,
                    std::function <typename std::tuple_element<I, \
                    std::tuple<shareptr_pool<LOCK, T>...> >::type::object* () >* new_fun = nullptr)
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
    typename std::tuple_element<I, std::tuple<shareptr_pool<LOCK, T>...> >::type::object*
        alloc_object()
    {
        return std::get<I>(pools_).alloc_object();
    }
    //归还一个对象
    template<size_t I >
    void free_object(typename std::tuple_element<I,
                     std::tuple<shareptr_pool<LOCK, T>...> >::type::object* ptr)
    {
        return std::get<I>(pools_).free_object(ptr);
    }

protected:
    //!对象池子堆
    std::tuple<shareptr_pool<LOCK, T>... > pools_;
};
}
